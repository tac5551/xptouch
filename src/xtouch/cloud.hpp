#pragma once

#include <set>
#ifdef __XTOUCH_PLATFORM_S3__
#include "esp_attr.h"
#endif
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "mbedtls/base64.h"
#include "types.h"
// #include "date.h"
#include "bbl-certs.h"
#include "filesystem.h"
#include "sdcard.h"
#include "paths.h"
#include "globals.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

bool xtouch_cloud_pair_loop_exit = false;
#include <WiFiClientSecure.h>
#include <cstring>
#include <strings.h>

/** JSON 文字列から key の直後の数値を1つ取り出す（"key":123 や "key":[230] に対応）。ヒープを使わない。 */
static int cloud_parse_json_int_key(const char *json, size_t len, const char *key)
{
  char needle[80];
  snprintf(needle, sizeof(needle), "\"%s\"", key);
  const char *p = strstr(json, needle);
  if (!p || (size_t)(p - json) >= len)
    return 0;
  p = (const char *)memchr(p, ':', len - (size_t)(p - json));
  if (!p || (size_t)(p - json) >= len)
    return 0;
  p++;
  while ((size_t)(p - json) < len && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'))
    p++;
  if ((size_t)(p - json) >= len)
    return 0;
  if (*p == '[')
  {
    p++;
    while ((size_t)(p - json) < len && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '"'))
      p++;
  }
  int val = 0;
  while ((size_t)(p - json) < len && *p >= '0' && *p <= '9')
  {
    val = val * 10 + (*p - '0');
    p++;
  }
  return val;
}

/** オブジェクト直下(depth=1)の "key":<int> を取得（ネスト配下の同名キー誤検出を避ける）。 */
static int cloud_parse_json_int_key_top_level(const char *json, size_t len, const char *key)
{
  if (!json || !key || !key[0] || len == 0)
    return 0;
  char needle[80];
  snprintf(needle, sizeof(needle), "\"%s\"", key);
  const size_t needle_len = strlen(needle);
  int obj_depth = 0;
  bool in_string = false;
  bool esc = false;
  for (size_t i = 0; i < len; i++)
  {
    char c = json[i];
    if (in_string)
    {
      if (esc)
      {
        esc = false;
        continue;
      }
      if (c == '\\')
      {
        esc = true;
        continue;
      }
      if (c == '"')
        in_string = false;
      continue;
    }
    if (obj_depth == 1 && c == '"')
    {
      if (i + needle_len <= len && memcmp(json + i, needle, needle_len) == 0)
      {
        size_t p = i + needle_len;
        while (p < len && (json[p] == ' ' || json[p] == '\t' || json[p] == '\n' || json[p] == '\r'))
          p++;
        if (p < len && json[p] == ':')
        {
          p++;
          while (p < len && (json[p] == ' ' || json[p] == '\t' || json[p] == '\n' || json[p] == '\r'))
            p++;
          int val = 0;
          while (p < len && json[p] >= '0' && json[p] <= '9')
          {
            val = val * 10 + (json[p] - '0');
            p++;
          }
          return val;
        }
      }
    }
    if (c == '"')
    {
      in_string = true;
      continue;
    }
    if (c == '{')
    {
      obj_depth++;
      continue;
    }
    if (c == '}')
    {
      if (obj_depth > 0)
        obj_depth--;
      continue;
    }
  }
  return 0;
}

/** JSON 文字列から key の直後の文字列値 "key":"value" を out にコピー。ヒープを使わない。 */
static void cloud_parse_json_str_key(const char *json, size_t len, const char *key, char *out, size_t out_size)
{
  if (!out || out_size == 0)
    return;
  out[0] = '\0';
  char needle[80];
  snprintf(needle, sizeof(needle), "\"%s\"", key);
  const char *p = strstr(json, needle);
  if (!p || (size_t)(p - json) >= len)
    return;
  p = (const char *)memchr(p, ':', len - (size_t)(p - json));
  if (!p || (size_t)(p - json) >= len)
    return;
  p++;
  while ((size_t)(p - json) < len && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'))
    p++;
  if ((size_t)(p - json) >= len || *p != '"')
    return;
  p++;
  const char *start = p;
  while ((size_t)(p - json) < len && *p != '"')
    p++;
  size_t n = (size_t)(p - start);
  if (n >= out_size)
    n = out_size - 1;
  memcpy(out, start, n);
  out[n] = '\0';
}

/** key の直後の数値を long で取得（id 用）。ヒープを使わない。 */
static long cloud_parse_json_long_key(const char *json, size_t len, const char *key)
{
  char needle[80];
  snprintf(needle, sizeof(needle), "\"%s\"", key);
  const char *p = strstr(json, needle);
  if (!p || (size_t)(p - json) >= len)
    return 0;
  p = (const char *)memchr(p, ':', len - (size_t)(p - json));
  if (!p || (size_t)(p - json) >= len)
    return 0;
  p++;
  while ((size_t)(p - json) < len && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'))
    p++;
  if ((size_t)(p - json) >= len)
    return 0;
  long val = 0;
  int neg = (*p == '-') ? (p++, 1) : 0;
  while ((size_t)(p - json) < len && *p >= '0' && *p <= '9')
  {
    val = val * 10 + (long)(*p - '0');
    p++;
  }
  return neg ? -val : val;
}

/** オブジェクト文字列内に "key":true があるか。 */
static bool cloud_parse_json_bool_key(const char *json, size_t len, const char *key)
{
  char needle[96];
  snprintf(needle, sizeof(needle), "\"%s\":true", key);
  const char *p = strstr(json, needle);
  return p && (size_t)(p - json) < len;
}

#ifdef __XTOUCH_PLATFORM_S3__
#include <cstdlib>
#include <stdint.h>
extern "C" const char *get_tray_color(uint8_t ams_id, uint8_t tray_id);
extern "C" const char *get_tray_setting_id(uint8_t ams_id, uint8_t tray_id);
extern "C" const char *get_tray_color_reprint(uint8_t ams_id, uint8_t tray_id);
extern "C" const char *get_tray_setting_id_reprint(uint8_t ams_id, uint8_t tray_id);

static double cloud_parse_json_double_key(const char *json, size_t len, const char *key)
{
  char needle[80];
  snprintf(needle, sizeof(needle), "\"%s\"", key);
  const char *p = strstr(json, needle);
  if (!p || (size_t)(p - json) >= len)
    return 0.0;
  p = (const char *)memchr(p, ':', len - (size_t)(p - json));
  if (!p)
    return 0.0;
  p++;
  while ((size_t)(p - json) < len && (*p == ' ' || *p == '\t'))
    p++;
  return strtod(p, nullptr);
}

static void cloud_format_rrggbbaa(const char *tray_c, const char *fallback, char *out, size_t out_sz)
{
  if (out_sz < 9 || !out)
    return;
  out[0] = '\0';
  if (tray_c && tray_c[0])
  {
    size_t L = strlen(tray_c);
    if (L >= 8)
    {
      memcpy(out, tray_c, 8);
      out[8] = '\0';
      return;
    }
    if (L >= 6)
    {
      memcpy(out, tray_c, 6);
      memcpy(out + 6, "FF", 3);
      return;
    }
  }
  const char *fb = (fallback && fallback[0]) ? fallback : "000000FF";
  strncpy(out, fb, out_sz - 1);
  out[8] = '\0';
}

static int cloud_parse_ams_detail_mapping(const char *obj, size_t obj_len, xtouch_history_ams_map_t *maps, int max_maps)
{
  const char *end = obj + obj_len;
  const char *k = strstr(obj, "\"amsDetailMapping\"");
  if (!k || k >= end)
    return 0;
  const char *br = strchr(k, '[');
  if (!br || br >= end)
    return 0;
  int cnt = 0;
  const char *p = br + 1;
  while (p < end && cnt < max_maps)
  {
    while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ','))
      p++;
    if (p >= end)
      break;
    if (*p == ']')
      break;
    if (*p != '{')
    {
      p++;
      continue;
    }
    const char *os = p;
    int depth = 0;
    const char *oe = nullptr;
    for (const char *q = p; q < end; q++)
    {
      if (*q == '{')
        depth++;
      else if (*q == '}')
      {
        depth--;
        if (depth == 0)
        {
          oe = q;
          break;
        }
      }
    }
    if (!oe)
      break;
    size_t ol = (size_t)(oe - os + 1);
    xtouch_history_ams_map_t *m = &maps[cnt];
    memset(m, 0, sizeof(*m));
    m->ams = cloud_parse_json_int_key(os, ol, "ams");
    m->amsId = cloud_parse_json_int_key(os, ol, "amsId");
    m->slotId = cloud_parse_json_int_key(os, ol, "slotId");
    m->nozzleId = cloud_parse_json_int_key(os, ol, "nozzleId");
    m->weight = cloud_parse_json_double_key(os, ol, "weight");
    cloud_parse_json_str_key(os, ol, "filamentId", m->filamentId, sizeof(m->filamentId));
    cloud_parse_json_str_key(os, ol, "filamentType", m->filamentType, sizeof(m->filamentType));
    cloud_parse_json_str_key(os, ol, "sourceColor", m->sourceColor, sizeof(m->sourceColor));
    cloud_parse_json_str_key(os, ol, "targetColor", m->targetColor, sizeof(m->targetColor));
    cloud_parse_json_str_key(os, ol, "targetFilamentType", m->targetFilamentType, sizeof(m->targetFilamentType));
    cnt++;
    p = oe + 1;
  }
  return cnt;
}

/** HTTP レスポンスストリームから amsDetailMapping の配列要素だけを抽出し、maps に詰める。
 * 大きい JSON Document を確保せず、各 "{...}" を小バッファに切り出して既存の cloud_parse_json_* で読む。
 * 本文読取はタイムアウトまで read を試す（connected に依存しない）。task 詳細の filaments は getString + バッファ側を参照。 */
static int cloud_parse_ams_detail_mapping_from_stream(HTTPClient &http, Stream &s, unsigned long timeout_ms,
                                                      xtouch_history_ams_map_t *maps, int max_maps)
{

  ConsoleVerbose.println("[xPTouch][V][CLOUD] cloud_parse_ams_detail_mapping_from_stream");

  if (!maps || max_maps <= 0)
    return -1;
  static const char needle[] = "\"amsDetailMapping\"";
  int needle_i = 0;
  bool found = false;
  unsigned long start_ms = millis();

  while (millis() - start_ms < timeout_ms)
  {
    int ch = s.read();
    if (ch < 0)
    {
      delay(1);
      continue;
    }
    if (ch == (int)needle[needle_i])
    {
      needle_i++;
      if (needle[needle_i] == '\0')
      {
        found = true;
        break;
      }
    }
    else
    {
      needle_i = (ch == (int)needle[0]) ? 1 : 0;
    }
  }
  if (!found)
    return 0;

  ConsoleVerbose.println("[xPTouch][V][CLOUD] cloud_parse_ams_detail_mapping_from_stream found");

  /* ':' まで進める */
  while (millis() - start_ms < timeout_ms)
  {
    int ch = s.read();
    if (ch < 0)
    {
      delay(1);
      continue;
    }
    if (ch == ':')
      break;
  }
  /* '[' まで進める */
  while (millis() - start_ms < timeout_ms)
  {
    int ch = s.read();
    if (ch < 0)
    {
      delay(1);
      continue;
    }
    if (ch == '[')
      break;
  }

  int cnt = 0;
  for (;;)
  {
    if (cnt >= max_maps)
      break;

    int ch = -1;
    while (millis() - start_ms < timeout_ms)
    {
      ch = s.read();
      if (ch < 0)
      {
        delay(1);
        continue;
      }
      if (ch == '{' || ch == ']')
        break;
    }
    if (ch == ']')
      break;
    if (ch != '{')
      break;

    char obj[768];
    size_t olen = 0;
    obj[olen++] = '{';
    int depth = 1;
    while (millis() - start_ms < timeout_ms && depth > 0)
    {
      int c2 = s.read();
      if (c2 < 0)
      {
        delay(1);
        continue;
      }
      if (olen + 1 < sizeof(obj))
        obj[olen++] = (char)c2;
      if (c2 == '{')
        depth++;
      else if (c2 == '}')
        depth--;
    }
    if (olen >= sizeof(obj))
      olen = sizeof(obj) - 1;
    obj[olen] = '\0';
    if (depth != 0)
      break;

    ConsoleDebug.printf("[xPTouch][F][CLOUD] cloud_parse_ams_detail_mapping_from_stream obj=%s\n", obj);

    xtouch_history_ams_map_t *m = &maps[cnt];
    memset(m, 0, sizeof(*m));
    m->ams = cloud_parse_json_int_key(obj, olen, "ams");
    m->amsId = cloud_parse_json_int_key(obj, olen, "amsId");
    m->slotId = cloud_parse_json_int_key(obj, olen, "slotId");
    m->nozzleId = cloud_parse_json_int_key(obj, olen, "nozzleId");
    m->weight = cloud_parse_json_double_key(obj, olen, "weight");
    cloud_parse_json_str_key(obj, olen, "filamentId", m->filamentId, sizeof(m->filamentId));
    cloud_parse_json_str_key(obj, olen, "filamentType", m->filamentType, sizeof(m->filamentType));
    cloud_parse_json_str_key(obj, olen, "sourceColor", m->sourceColor, sizeof(m->sourceColor));
    cloud_parse_json_str_key(obj, olen, "targetColor", m->targetColor, sizeof(m->targetColor));
    cloud_parse_json_str_key(obj, olen, "targetFilamentType", m->targetFilamentType, sizeof(m->targetFilamentType));
    cnt++;
  }
  return cnt;
}

/** GET /my/task/&lt;id&gt; の JSON 本文から filaments[] だけ走査し Reprint 用 maps を生成する。
 * ESP32 の HTTPClient::getStream() からの逐次 read は本文が届かず常に -1 になる環境があるため、
 * 呼び出し側は http.getString() 等で本文を確保してから本関数に渡す。 */
static int cloud_rb_next(const char *data, size_t len, size_t *idx)
{
  if (*idx >= len)
    return -1;
  return (unsigned char)data[(*idx)++];
}

static int cloud_parse_reprint_mapping_from_buffer(const char *data, size_t len, xtouch_history_ams_map_t *maps, int max_maps)
{
  if (!maps || max_maps <= 0 || !data || len == 0)
    return -1;

  size_t idx = 0;
  static const char needle_fil[] = "\"filaments\"";

  for (;;)
  {
    int fi = 0;
    while (idx < len)
    {
      int ch = cloud_rb_next(data, len, &idx);
      if (ch < 0)
        break;
      if (ch == (int)needle_fil[fi])
      {
        fi++;
        if (needle_fil[fi] == '\0')
          break;
      }
      else
      {
        fi = (ch == (int)needle_fil[0]) ? 1 : 0;
      }
    }
    if (needle_fil[fi] != '\0')
      return 0;

    while (idx < len)
    {
      int ch = cloud_rb_next(data, len, &idx);
      if (ch < 0)
        return 0;
      if (ch == ':')
        break;
    }

    int ch_first = -1;
    while (idx < len)
    {
      int ch = cloud_rb_next(data, len, &idx);
      if (ch < 0)
        return 0;
      if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
        continue;
      ch_first = ch;
      break;
    }
    if (ch_first == '[')
      break;
    if (ch_first == 'n')
    {
      const char rest[] = "ull";
      bool ok = true;
      for (size_t ri = 0; ri < sizeof(rest) - 1 && ok; ri++)
      {
        if (idx >= len)
        {
          ok = false;
          break;
        }
        int c2 = cloud_rb_next(data, len, &idx);
        if (c2 != (int)(unsigned char)rest[ri])
          ok = false;
      }
      continue;
    }
  }

  int cnt = 0;
  for (;;)
  {
    if (cnt >= max_maps)
      break;

    int ch = -1;
    while (idx < len)
    {
      ch = cloud_rb_next(data, len, &idx);
      if (ch < 0)
        break;
      if (ch == '{' || ch == ']')
        break;
    }
    if (ch == ']')
      break;
    if (ch != '{')
      break;

    char obj[768];
    size_t olen = 0;
    obj[olen++] = '{';
    int depth = 1;
    while (idx < len && depth > 0)
    {
      int c2 = cloud_rb_next(data, len, &idx);
      if (c2 < 0)
        break;
      if (olen + 1 < sizeof(obj))
        obj[olen++] = (char)c2;
      if (c2 == '{')
        depth++;
      else if (c2 == '}')
        depth--;
    }
    if (olen >= sizeof(obj))
      olen = sizeof(obj) - 1;
    obj[olen] = '\0';
    if (depth != 0)
      break;

    xtouch_history_ams_map_t *m = &maps[cnt];
    memset(m, 0, sizeof(*m));

    char color_buf[16] = { 0 };
    char type_buf[20] = { 0 };
    char id_buf[16] = { 0 };
    cloud_parse_json_str_key(obj, olen, "id", id_buf, sizeof(id_buf));
    cloud_parse_json_str_key(obj, olen, "type", type_buf, sizeof(type_buf));
    cloud_parse_json_str_key(obj, olen, "color", color_buf, sizeof(color_buf));
    double used_g = cloud_parse_json_double_key(obj, olen, "used_g");

    const char *c0 = color_buf;
    if (c0 && c0[0] == '#')
      c0++;
    if (c0 && strlen(c0) >= 6)
    {
      snprintf(m->sourceColor, sizeof(m->sourceColor), "%.6sFF", c0);
      strlcpy(m->targetColor, m->sourceColor, sizeof(m->targetColor));
    }
    else
    {
      strlcpy(m->sourceColor, "808080FF", sizeof(m->sourceColor));
      strlcpy(m->targetColor, "808080FF", sizeof(m->targetColor));
    }

    strlcpy(m->filamentId, id_buf, sizeof(m->filamentId));
    strlcpy(m->filamentType, type_buf, sizeof(m->filamentType));
    m->weight = used_g;
    m->ams = 0;
    m->amsId = 0;
    m->slotId = 0;
    m->nozzleId = 1;
    m->targetFilamentType[0] = '\0';

    cnt++;
  }

  return cnt;
}

#ifdef __XTOUCH_PLATFORM_S3__
/** Task 詳細の filaments[].id が "2" のような内部IDのとき、POST の filamentId（GFLxx 等）に寄せる */
static bool cloud_filament_id_looks_like_bambu_code(const char *s)
{
  if (!s || !s[0])
    return false;
  return (s[0] == 'G' && s[1] == 'F' && strlen(s) >= 4);
}

static const char *cloud_default_filament_id_for_type(const char *ftype)
{
  if (!ftype || !ftype[0])
    return "GFL03";
  if (strcasecmp(ftype, "PLA") == 0)
    return "GFL03";
  if (strcasecmp(ftype, "PETG") == 0)
    return "GFG99";
  if (strcasecmp(ftype, "ABS") == 0)
    return "GFB99";
  if (strcasecmp(ftype, "TPU") == 0)
    return "GFU99";
  return "GFL03";
}

static void cloud_resolve_filament_id_for_reprint(const char *raw_id, const char *ftype, char *out, size_t out_sz)
{
  if (cloud_filament_id_looks_like_bambu_code(raw_id))
  {
    strlcpy(out, raw_id, out_sz);
    return;
  }
  strlcpy(out, cloud_default_filament_id_for_type(ftype), out_sz);
}
#endif

/* HTTPClient + getStream() が環境によって Panic することがあるため、
 * WiFiClientSecure 等の Client から直接本文を走査できる版も用意する。 */
template <typename ClientT>
static int cloud_parse_ams_detail_mapping_from_client(ClientT &c, unsigned long timeout_ms,
                                                      xtouch_history_ams_map_t *maps, int max_maps)
{
  if (!maps || max_maps <= 0)
    return -1;
  static const char needle[] = "\"amsDetailMapping\"";
  int needle_i = 0;
  bool found = false;
  unsigned long start_ms = millis();

  while (millis() - start_ms < timeout_ms && (c.connected() || c.available()))
  {
    int ch = c.read();
    if (ch < 0)
    {
      delay(1);
      continue;
    }
    if (ch == (int)needle[needle_i])
    {
      needle_i++;
      if (needle[needle_i] == '\0')
      {
        found = true;
        break;
      }
    }
    else
    {
      needle_i = (ch == (int)needle[0]) ? 1 : 0;
    }
  }
  if (!found)
    return 0;

  /* ':' まで進める */
  while (millis() - start_ms < timeout_ms && (c.connected() || c.available()))
  {
    int ch = c.read();
    if (ch < 0)
    {
      delay(1);
      continue;
    }
    if (ch == ':')
      break;
  }
  /* '[' まで進める */
  while (millis() - start_ms < timeout_ms && (c.connected() || c.available()))
  {
    int ch = c.read();
    if (ch < 0)
    {
      delay(1);
      continue;
    }
    if (ch == '[')
      break;
  }

  int cnt = 0;
  for (;;)
  {
    if (cnt >= max_maps)
      break;

    int ch = -1;
    while (millis() - start_ms < timeout_ms && (c.connected() || c.available()))
    {
      ch = c.read();
      if (ch < 0)
      {
        delay(1);
        continue;
      }
      if (ch == '{' || ch == ']')
        break;
    }
    if (ch == ']')
      break;
    if (ch != '{')
      break;

    char obj[768];
    size_t olen = 0;
    obj[olen++] = '{';
    int depth = 1;
    while (millis() - start_ms < timeout_ms && depth > 0 && (c.connected() || c.available()))
    {
      int c2 = c.read();
      if (c2 < 0)
      {
        delay(1);
        continue;
      }
      if (olen + 1 < sizeof(obj))
        obj[olen++] = (char)c2;
      if (c2 == '{')
        depth++;
      else if (c2 == '}')
        depth--;
    }
    if (olen >= sizeof(obj))
      olen = sizeof(obj) - 1;
    obj[olen] = '\0';
    if (depth != 0)
      break;

    xtouch_history_ams_map_t *m = &maps[cnt];
    memset(m, 0, sizeof(*m));
    m->ams = cloud_parse_json_int_key(obj, olen, "ams");
    m->amsId = cloud_parse_json_int_key(obj, olen, "amsId");
    m->slotId = cloud_parse_json_int_key(obj, olen, "slotId");
    m->nozzleId = cloud_parse_json_int_key(obj, olen, "nozzleId");
    m->weight = cloud_parse_json_double_key(obj, olen, "weight");
    cloud_parse_json_str_key(obj, olen, "filamentId", m->filamentId, sizeof(m->filamentId));
    cloud_parse_json_str_key(obj, olen, "filamentType", m->filamentType, sizeof(m->filamentType));
    cloud_parse_json_str_key(obj, olen, "sourceColor", m->sourceColor, sizeof(m->sourceColor));
    cloud_parse_json_str_key(obj, olen, "targetColor", m->targetColor, sizeof(m->targetColor));
    cloud_parse_json_str_key(obj, olen, "targetFilamentType", m->targetFilamentType, sizeof(m->targetFilamentType));
    cnt++;
  }
  return cnt;
}

/** クラウドの dev_product_name / deviceModel が A1 Mini 系か（表記ゆれ対応）。 */
static bool cloud_history_name_is_a1_mini(const char *s)
{
  if (!s || !s[0])
    return false;
  if (strcasecmp(s, "A1 Mini") == 0)
    return true;
  if (strcasecmp(s, "A1Mini") == 0)
    return true;
  return false;
}

/** bind の dev_product_name と tasks の deviceModel を並べ、同一 History グループなら true。
 * 固定グループ: X1 Carbon / P1S / P1P / P2S / A1。A1 Mini は共有グループに含めず、他機種との履歴共有もしない。 */
static bool cloud_history_device_models_compatible(const char *local_product, const char *task_device_model)
{
  if (!local_product || !task_device_model || !local_product[0] || !task_device_model[0])
    return false;
  if (strcmp(local_product, task_device_model) == 0)
    return true;
  if (cloud_history_name_is_a1_mini(local_product) || cloud_history_name_is_a1_mini(task_device_model))
    return false;
  static const char *const k_x1_p1_full_plate[] = {"X1 Carbon", "P1S", "P1P", "P2S", "A1", nullptr};
  bool loc = false, task = false;
  for (int i = 0; k_x1_p1_full_plate[i]; i++)
  {
    if (strcmp(local_product, k_x1_p1_full_plate[i]) == 0)
      loc = true;
    if (strcmp(task_device_model, k_x1_p1_full_plate[i]) == 0)
      task = true;
  }
  return loc && task;
}
#endif

class BambuCloud
{

private:
  String _region;
  String _auth_token;
  /** getDeviceList / getSlicerSetting で共有。都度 new せずヒープを抑える。 */
  WiFiClientSecure *_ssl_client = nullptr;
  SemaphoreHandle_t _http_mutex = nullptr;

  void httpLock()
  {
    if (_http_mutex == nullptr)
      _http_mutex = xSemaphoreCreateMutex();
    if (_http_mutex)
      xSemaphoreTake(_http_mutex, portMAX_DELAY);
  }

  void httpUnlock()
  {
    if (_http_mutex)
      xSemaphoreGive(_http_mutex);
  }

  struct HttpLockGuard
  {
    BambuCloud *self;
    explicit HttpLockGuard(BambuCloud *s) : self(s)
    {
      if (self)
        self->httpLock();
    }
    ~HttpLockGuard()
    {
      if (self)
        self->httpUnlock();
    }
  };

  WiFiClientSecure &sslClient()
  {
    if (!_ssl_client)
      _ssl_client = new WiFiClientSecure();
    return *_ssl_client;
  }

public:
  String _email;
  bool loggedIn = false;

  /** downloadFileToSDCard 等の別 TLS 接続と mbedTLS 内部ヒープを競合させない（並行で SSL -32512 になるのを防ぐ） */
  void httpLockExternal() { httpLock(); }
  void httpUnlockExternal() { httpUnlock(); }

  bool getDeviceList(DynamicJsonDocument *&doc)
  {
    HttpLockGuard _g(this);
    Serial.println(_region);
    Serial.println("Getting device list from Bambu Cloud");
    String url = _region == "China" ? "https://api.bambulab.cn/v1/iot-service/api/user/bind" : "https://api.bambulab.com/v1/iot-service/api/user/bind";
    String host = _region == "China" ? "api.bambulab.cn" : "api.bambulab.com";

    WiFiClientSecure &client = sslClient();
    client.stop();
    client.setTimeout(500);
    client.setInsecure();
    static char response_buf[2048];
    size_t response_len = 0;
    if (!client.connect(host.c_str(), 443))
      Serial1.println("Connection failed!");
    else
    {
      Serial1.println("Connected to server!");

      String request = "GET " + url + " HTTP/1.1\r\n";
      request += "Host: " + String(host) + "\r\n";
      request += "Authorization: Bearer " + _auth_token + "\r\n";
      request += "Connection: close\r\n";
      request += "\r\n";

      Serial.println("Sending request:");
      Serial.print(request);
      client.print(request);

      /* ヘッダーは \r\n\r\n までバイト読みで捨てる（readStringUntil の String 確保を避ける） */
      int state = 0;
      while (client.connected() || client.available())
      {
        int b = client.read();
        if (b < 0)
        {
          delay(1);
          continue;
        }
        if (state == 0)
          state = (b == '\r') ? 1 : 0;
        else if (state == 1)
          state = (b == '\n') ? 2 : (b == '\r') ? 1 : 0;
        else if (state == 2)
          state = (b == '\r') ? 3 : 0;
        else if (state == 3)
        {
          if (b == '\n')
            break;
          state = (b == '\r') ? 1 : 0;
        }
      }
      Serial1.println("headers received");

      while (client.available() && response_len < sizeof(response_buf) - 1)
      {
        int b = client.read();
        if (b < 0)
          break;
        response_buf[response_len++] = (char)b;
      }
      response_buf[response_len] = '\0';
      Serial1.println("\ndata received");

      client.stop();
    }
    Serial1.println("Connection closed");

    doc = new DynamicJsonDocument(2048);
    DeserializationError error = deserializeJson(*doc, response_buf);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      return false;
    }
    return true;
  }

  /** GET /v1/iot-service/api/slicer/setting/{setting_id} で温度範囲を取得。共通 sslClient + HTTPClient。 */
  bool getSlicerSetting(const char *setting_id, int *out_min, int *out_max, char *out_filament_id = nullptr, size_t out_filament_id_size = 0)
  {
    HttpLockGuard _g(this);
    if (!setting_id || !*setting_id || !out_min || !out_max)
      return false;
    *out_min = 0;
    *out_max = 0;
    String host = _region == "China" ? "api.bambulab.cn" : "api.bambulab.com";
    String path = String("/v1/iot-service/api/slicer/setting/") + setting_id;
Serial.printf("[Cloud getSlicerSetting] setting_id=%d\n", setting_id);
  
    String url = String("https://") + host + path;
    WiFiClientSecure &c = sslClient();
    c.stop();
    c.setTimeout(8000);
    c.setInsecure();

    yield();
    HTTPClient http;
    http.begin(c, url);
    http.setReuse(false);
    http.useHTTP10(true);
    char auth_buf[320];
    snprintf(auth_buf, sizeof(auth_buf), "Bearer %s", _auth_token.c_str());
    http.addHeader("Authorization", auth_buf);
    http.addHeader("Connection", "close");
    http.setTimeout(8000);
    int code = http.GET();
    String response = (code == 200) ? http.getString() : "";
    http.end();
    c.stop();
  Serial.printf("[Cloud getSlicerSetting] http code=%d\n", code);
    if (response.length() == 0)
      return false;
    const char *raw = response.c_str();
    size_t raw_len = response.length();
// #ifdef XTOUCH_DEBUG
    Serial.printf("[Cloud getSlicerSetting] url=%s\n", url.c_str());
    Serial.printf("[Cloud getSlicerSetting] id=%s raw_len=%u\n", setting_id, (unsigned)raw_len);
    Serial.print("[Cloud getSlicerSetting] raw=");
    for (size_t i = 0; i < raw_len; i++)
      Serial.print(raw[i]);
    Serial.println();
// #endif
    if (response.length() == 0)
      return false;
    *out_min = cloud_parse_json_int_key(raw, raw_len, "nozzle_temperature_range_low");
    *out_max = cloud_parse_json_int_key(raw, raw_len, "nozzle_temperature_range_high");
    if (out_filament_id && out_filament_id_size > 0)
      cloud_parse_json_str_key(raw, raw_len, "filament_id", out_filament_id, out_filament_id_size);
    return (*out_min > 0 || *out_max > 0);
  }

  String getUsername() const
  {
    // cloud-username
    DynamicJsonDocument config = xtouch_filesystem_readJson(xtouch_sdcard_fs(), xtouch_paths_provisioning, false, 2048);
    return config["cloud-username"].as<String>();
  }

  String getAuthToken() const
  {
    return _auth_token;
  }

  const char *getMqttCloudHost() const
  {
    return _region == "China" ? "cn.mqtt.bambulab.com" : "us.mqtt.bambulab.com";
  }

  String getRegion()
  {
    return _region;
  }

  /** 現在のデバイスの最新タスクが task_id と一致するか簡易チェックする。
   *  GET /v1/user-service/my/tasks?limit=1&deviceId=xxx を叩き、先頭要素の id と比較するだけで、
   *  xtouch_history_* などのグローバル状態は一切更新しない。 */
  bool isCurrentTaskForDevice(const char *task_id)
  {
    HttpLockGuard _g(this);
    if (!loggedIn || !task_id || !*task_id)
      return false;
    if (!xTouchConfig.xTouchSerialNumber[0])
      return false;

    String host = _region == "China" ? "api.bambulab.cn" : "api.bambulab.com";
    String path = String("/v1/user-service/my/tasks?limit=1&deviceId=") + String(xTouchConfig.xTouchSerialNumber);
    String url = String("https://") + host + path;

    WiFiClientSecure &c = sslClient();
    c.stop();
    c.setTimeout(8000);
    c.setInsecure();
    yield();
    HTTPClient http;
    http.begin(c, url);
    http.setReuse(false);
    http.useHTTP10(true);
    char auth_buf[320];
    snprintf(auth_buf, sizeof(auth_buf), "Bearer %s", _auth_token.c_str());
    http.addHeader("Authorization", auth_buf);
    http.addHeader("Connection", "close");
    http.setTimeout(8000);
    int code = http.GET();
    String response = (code == 200) ? http.getString() : "";
    http.end();
    c.stop();
    if (code != 200 || response.length() == 0)
      return false;

    const char *raw = response.c_str();
    size_t raw_len = response.length();
    const char *hits_key = "\"hits\"";
    const char *p = strstr(raw, hits_key);
    if (!p || (size_t)(p - raw) >= raw_len)
      return false;
    p = (const char *)memchr(p, '[', raw_len - (size_t)(p - raw));
    if (!p)
      return false;
    /* 最初の '{...}' の id を見るだけ */
    while ((size_t)(p - raw) < raw_len && *p != '{' && *p != ']')
      p++;
    if (*p != '{')
      return false;
    const char *obj_start = p;
    int depth = 0;
    const char *obj_end = nullptr;
    for (const char *q = p; (size_t)(q - raw) < raw_len; q++)
    {
      if (*q == '{')
        depth++;
      else if (*q == '}')
      {
        depth--;
        if (depth == 0)
        {
          obj_end = q;
          break;
        }
      }
    }
    if (!obj_end)
      return false;
    size_t obj_len = (size_t)(obj_end - obj_start + 1);
    long id_val = cloud_parse_json_long_key(obj_start, obj_len, "id");
    if (id_val <= 0)
      return false;
    char latest_id[32];
    snprintf(latest_id, sizeof(latest_id), "%ld", id_val);
    return strcmp(latest_id, task_id) == 0;
  }

  /** GET /v1/iot-service/api/user/task/{task_id} から plates[0].thumbnail.url を取得する。 */
  bool getTaskThumbnailUrl(const char *task_id, char *out_url, size_t out_size)
  {
#ifdef __XTOUCH_PLATFORM_S3__
    HttpLockGuard _g(this);
#endif
    if (!task_id || !*task_id || !out_url || out_size == 0)
      return false;
    if (!loggedIn)
      return false;

    String host = _region == "China" ? "api.bambulab.cn" : "api.bambulab.com";
    String path = String("/v1/iot-service/api/user/task/") + task_id;
    String url = String("https://") + host + path;

    ConsoleVerbose.printf("[xPTouch][V][CLOUD] getTaskThumbnailUrl task_id=%s url=%s\n", task_id, url.c_str());

    WiFiClientSecure &c = sslClient();
    c.stop();
    c.setTimeout(8000);
    c.setInsecure();

    yield();
    HTTPClient http;
    http.begin(c, url);
    http.setReuse(false);
    http.useHTTP10(true);
    char auth_buf[320];
    snprintf(auth_buf, sizeof(auth_buf), "Bearer %s", _auth_token.c_str());
    http.addHeader("Authorization", auth_buf);
    http.addHeader("Connection", "close");
    http.setTimeout(8000);
    int code = http.GET();
    String response = http.getString();
#ifdef XTOUCH_DEBUG_VERBOSE
    ConsoleVerbose.printf("[xPTouch][V][CLOUD] getTaskThumbnailUrl code=%d resp_len=%d\n", code, response.length());
    if (code != 200)
    {
      ConsoleVerbose.printf("[xPTouch][V][CLOUD] getTaskThumbnailUrl non-200 body=%s\n", response.c_str());
    }
#endif
    http.end();
    c.stop();
    if (response.length() == 0)
      return false;

#ifdef XTOUCH_DEBUG_VERBOSE
    /* Cloud個別デバッグ: task レスポンス全体を SD に保存 */
    char dump_path[64];
    snprintf(dump_path, sizeof(dump_path), "/tmp/task_%s.json", task_id);
    File dump = xtouch_sdcard_open(dump_path, FILE_WRITE);
    if (dump)
    {
      dump.print(response);
      dump.close();
      ConsoleDebug.printf("[xPTouch][V][CLOUD] saved task JSON to %s\n", dump_path);
    }
#endif

    /* JSON 全体をパースせず、テキスト検索で context.plates[0].thumbnail.url を抜き出す */
    const char *raw = response.c_str();
    size_t raw_len = response.length();

    /* S3 の署名付き URL 用 2KB。5inch(PSRAM あり)のときは PSRAM に配置。 */
#if defined(__XTOUCH_PLATFORM_S3__) && defined(CONFIG_SPIRAM)
    static EXT_RAM_ATTR char url_buf[2048];
#else
    static char url_buf[2048];
#endif
    url_buf[0] = '\0';

    /* context.plates[0].thumbnail.url を取得（configs[].url と混同しないよう "thumbnail" を経由）。 */
    const char *plates_pos = strstr(raw, "\"plates\"");
    if (plates_pos)
    {
      size_t rest_len = raw_len - (size_t)(plates_pos - raw);
      const char *thumb_pos = strstr(plates_pos, "\"thumbnail\"");
      if (thumb_pos && (size_t)(thumb_pos - plates_pos) < rest_len)
        cloud_parse_json_str_key(thumb_pos, rest_len - (size_t)(thumb_pos - plates_pos), "url", url_buf, sizeof(url_buf));
    }
    if (!url_buf[0])
      cloud_parse_json_str_key(raw, raw_len, "cover", url_buf, sizeof(url_buf));
    const char *url_c = url_buf;
    if (!url_c || !url_c[0])
    {
      ConsoleVerbose.println("[xPTouch][V][CLOUD] getTaskThumbnailUrl thumbnail.url empty");
      return false;
    }

    ConsoleVerbose.printf("[xPTouch][V][CLOUD] getTaskThumbnailUrl extracted url=%s\n", url_c);

    strncpy(out_url, url_c, out_size - 1);
    out_url[out_size - 1] = '\0';
    return true;
  }

#ifdef __XTOUCH_PLATFORM_S3__
  /** GET /v1/user-service/my/tasks（deviceId なし）。フィルタ後 want 件に達するまで after で追取得。生 hits は累計 50 件までで打ち切り（after 引数ありは 1 回のみ・上限なし）。 */
  bool getMyTasks(int limit, const char *after = nullptr)
  {
    HttpLockGuard _g(this);
    if (!loggedIn)
      return false;

    char dev_product_filter[64];
    dev_product_filter[0] = '\0';
    {
      DynamicJsonDocument printers = loadPrinters();
      JsonObject root = printers.as<JsonObject>();
      if (root.containsKey(xTouchConfig.xTouchSerialNumber))
      {
        JsonObject dev = root[xTouchConfig.xTouchSerialNumber].as<JsonObject>();
        if (!dev.isNull() && dev.containsKey("dev_product_name"))
        {
          const char *pn = dev["dev_product_name"].as<const char *>();
          if (pn && pn[0])
          {
            strncpy(dev_product_filter, pn, sizeof(dev_product_filter) - 1);
            dev_product_filter[sizeof(dev_product_filter) - 1] = '\0';
          }
        }
      }
    }

    String host = _region == "China" ? "api.bambulab.cn" : "api.bambulab.com";
    const bool append_one_shot = (after && after[0] != '\0');
    int req_limit;
    int want_visible;
    if (append_one_shot)
    {
      req_limit = (limit <= 0 || limit > XTOUCH_HISTORY_TASKS_MAX) ? XTOUCH_HISTORY_TASKS_MAX : limit;
      want_visible = XTOUCH_HISTORY_TASKS_MAX;
    }
    else
    {
      want_visible = (limit <= 0 || limit > XTOUCH_HISTORY_TASKS_MAX) ? XTOUCH_HISTORY_TASKS_MAX : limit;
      req_limit = 10;
    }

    char cursor_after[XTOUCH_HISTORY_TASK_ID_LEN];
    cursor_after[0] = '\0';
    if (append_one_shot)
    {
      strncpy(cursor_after, after, sizeof(cursor_after) - 1);
      cursor_after[sizeof(cursor_after) - 1] = '\0';
    }

    int n = append_one_shot ? xtouch_history_count : 0;
    if (append_one_shot && n >= XTOUCH_HISTORY_TASKS_MAX)
      return true;

    const int k_history_raw_total_max = 50;
    int raw_total = 0;
    const int max_pages = 10;
    for (int page_ix = 0; page_ix < max_pages; page_ix++)
    {
      if (!append_one_shot && raw_total >= k_history_raw_total_max)
        break;
      if (!append_one_shot && n >= want_visible)
        break;
      if (append_one_shot && n >= XTOUCH_HISTORY_TASKS_MAX)
        break;

      String path = String("/v1/user-service/my/tasks?limit=") + String(req_limit);
      if (cursor_after[0] != '\0')
        path += String("&after=") + String(cursor_after);
      String url = String("https://") + host + path;

      WiFiClientSecure &c = sslClient();
      c.stop();
      c.setTimeout(12000);
      c.setInsecure();
      yield();
      HTTPClient http;
      http.begin(c, url);
      http.setReuse(false);
      http.useHTTP10(true);
      char auth_buf[320];
      snprintf(auth_buf, sizeof(auth_buf), "Bearer %s", _auth_token.c_str());
      http.addHeader("Authorization", auth_buf);
      http.setTimeout(12000);
      http.addHeader("Connection", "close");
      int code = http.GET();
      String response = (code == 200) ? http.getString() : "";
      http.end();
      c.stop();
      if (code != 200 || response.length() == 0)
      {
        if (!append_one_shot && page_ix == 0)
        {
          xtouch_history_count = 0;
          return false;
        }
        break;
      }

      const char *raw = response.c_str();
      const size_t raw_len = response.length();
      const char *hits_key = "\"hits\"";
      const char *p = strstr(raw, hits_key);
      if (!p || (size_t)(p - raw) >= raw_len)
      {
        if (!append_one_shot && page_ix == 0)
        {
          xtouch_history_count = 0;
          return false;
        }
        break;
      }
      p = (const char *)memchr(p, '[', raw_len - (size_t)(p - raw));
      if (!p)
        p = raw + raw_len;
      else
        p++;

      int raw_hits = 0;
      char last_raw_id[XTOUCH_HISTORY_TASK_ID_LEN];
      last_raw_id[0] = '\0';

      /* hits 配列は末尾まで走査し、最終要素 id を after 用に残す（フィルタで先に want 件満たしても同ページ内の残りを読む） */
      while ((size_t)(p - raw) < raw_len)
      {
        while ((size_t)(p - raw) < raw_len && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ','))
          p++;
        if ((size_t)(p - raw) >= raw_len || *p == ']')
          break;
        if (*p != '{')
        {
          p++;
          continue;
        }
        const char *obj_start = p;
        int depth = 0;
        const char *obj_end = nullptr;
        for (const char *q = p; (size_t)(q - raw) < raw_len; q++)
        {
          if (*q == '{')
            depth++;
          else if (*q == '}')
          {
            depth--;
            if (depth == 0)
            {
              obj_end = q;
              break;
            }
          }
        }
        if (!obj_end)
          break;
        size_t obj_len = (size_t)(obj_end - obj_start + 1);

        raw_hits++;
        long id_val = cloud_parse_json_long_key(obj_start, obj_len, "id");
        snprintf(last_raw_id, sizeof(last_raw_id), "%ld", id_val);

        const bool want_copy =
            append_one_shot ? (n < XTOUCH_HISTORY_TASKS_MAX) : (n < want_visible && n < XTOUCH_HISTORY_TASKS_MAX);

        char device_model[64];
        device_model[0] = '\0';
        cloud_parse_json_str_key(obj_start, obj_len, "deviceModel", device_model, sizeof(device_model));

        bool filtered_ok;
        if (dev_product_filter[0] != '\0')
          filtered_ok = cloud_history_device_models_compatible(dev_product_filter, device_model);
        else
        {
          /* printer.json に dev_product_name が無い場合、全件表示すると他機種履歴が混ざるため hits の deviceId で自機のみに限定 */
          char task_dev_id[32];
          task_dev_id[0] = '\0';
          cloud_parse_json_str_key(obj_start, obj_len, "deviceId", task_dev_id, sizeof(task_dev_id));
          filtered_ok = (task_dev_id[0] && xTouchConfig.xTouchSerialNumber[0] &&
                         strcmp(task_dev_id, xTouchConfig.xTouchSerialNumber) == 0);
        }

        if (filtered_ok && want_copy)
        {
          xtouch_history_task_t *t = &xtouch_history_tasks[n];
          memset(t, 0, sizeof(*t));
          snprintf(t->task_id, sizeof(t->task_id), "%ld", id_val);
          cloud_parse_json_str_key(obj_start, obj_len, "modelId", t->model_id, sizeof(t->model_id));
          cloud_parse_json_str_key(obj_start, obj_len, "title", t->title, sizeof(t->title));
          cloud_parse_json_str_key(obj_start, obj_len, "cover", t->cover_url, sizeof(t->cover_url));
          cloud_parse_json_str_key(obj_start, obj_len, "deviceName", t->device_name, sizeof(t->device_name));
          cloud_parse_json_str_key(obj_start, obj_len, "deviceModel", t->device_model, sizeof(t->device_model));
          cloud_parse_json_str_key(obj_start, obj_len, "startTime", t->start_time, sizeof(t->start_time));
          cloud_parse_json_str_key(obj_start, obj_len, "endTime", t->end_time, sizeof(t->end_time));
          t->profile_id = cloud_parse_json_int_key(obj_start, obj_len, "profileId");
          t->plate_index = cloud_parse_json_int_key(obj_start, obj_len, "plateIndex");
          int parsed_status = cloud_parse_json_int_key_top_level(obj_start, obj_len, "status");
          if (parsed_status == 0)
            parsed_status = cloud_parse_json_int_key(obj_start, obj_len, "status");
          t->status = parsed_status;
          t->is_printable = cloud_parse_json_bool_key(obj_start, obj_len, "isPrintable") ? 1 : 0;
          t->has_ams_mapping =
              (strstr(obj_start, "\"amsDetailMapping\"") != nullptr || strstr(obj_start, "\"filaments\"") != nullptr) ? 1 : 0;
          t->valid = 1;
          n++;
        }

        p = obj_end + 1;

        if (append_one_shot && n >= XTOUCH_HISTORY_TASKS_MAX)
          break;
      }

      xtouch_history_count = n;

      if (append_one_shot)
        return true;

      raw_total += raw_hits;
      if (raw_hits < req_limit)
        break;
      if (n >= want_visible)
        break;
      if (!last_raw_id[0])
        break;
      if (raw_total >= k_history_raw_total_max)
        break;
      strncpy(cursor_after, last_raw_id, sizeof(cursor_after) - 1);
      cursor_after[sizeof(cursor_after) - 1] = '\0';
    }

    return true;
  }

  /** 履歴 task_id の詳細 GET /my/task/&lt;id&gt; から amsDetailMapping[] を読み、Reprint 用の行を maps に詰める（失敗時は -1）。 */
  int getMyTaskAmsDetailMapping(const char *task_id, xtouch_history_ams_map_t *maps, int max_maps)
  {
    HttpLockGuard _g(this);
    if (!loggedIn || !task_id || !task_id[0] || !maps || max_maps <= 0)
      return -1;
    String host = _region == "China" ? "api.bambulab.cn" : "api.bambulab.com";
    String url = String("https://") + host + String("/v1/user-service/my/task/") + String(task_id);

    /* 共有 sslClient() の再利用が不安定な環境があるため、このリクエストはローカル client で完結させる */
    WiFiClientSecure c;
    c.setTimeout(12000);
    c.setInsecure();
    yield();
    HTTPClient http;
    http.begin(c, url);
    http.setReuse(false);
    http.useHTTP10(true);
    char auth_buf[320];
    snprintf(auth_buf, sizeof(auth_buf), "Bearer %s", _auth_token.c_str());
    http.addHeader("Authorization", auth_buf);
    http.addHeader("Connection", "close");
    http.setTimeout(12000);
    int code = http.GET();
    if (code != 200)
    {
      http.end();
      c.stop();
      return -1;
    }

    ConsoleVerbose.println("[xPTouch][V][CLOUD] GET /v1/user-service/my/task/ OK");
    String body = http.getString();
    ConsoleVerbose.printf("[xPTouch][V][CLOUD] task detail JSON len=%d\n", (unsigned)body.length());
    int cnt = cloud_parse_ams_detail_mapping(body.c_str(), body.length(), maps, max_maps);

#ifdef XTOUCH_DEBUG_VERBOSE
    ConsoleVerbose.printf("[xPTouch][V][CLOUD] cloud_parse_ams_detail_mapping (my/task) cnt=%d\n", cnt);
    ConsoleVerbose.printf("[xPTouch][V][CLOUD] amsDetailMapping parse task_id=%s\n", task_id);
    {
      const int nprint = (cnt > 16) ? 16 : cnt;
      for (int i = 0; i < nprint; i++)
      {
        const xtouch_history_ams_map_t *m = &maps[i];
        ConsoleVerbose.printf("[xPTouch][V][CLOUD] amsMap[%d] id=%s type=%s srcColor=%s tgtColor=%s tgtType=%s w=%.2f ams=%d amsId=%d slot=%d nozzle=%d\n", i,
                            m->filamentId[0] ? m->filamentId : "(empty)", m->filamentType[0] ? m->filamentType : "?",
                            m->sourceColor[0] ? m->sourceColor : "?", m->targetColor[0] ? m->targetColor : "?",
                            m->targetFilamentType[0] ? m->targetFilamentType : "?", m->weight, m->ams, m->amsId, m->slotId, m->nozzleId);
      }
      if (cnt > 16)
        ConsoleVerbose.printf("[xPTouch][V][CLOUD] amsMap ... (truncated to 16 lines)\n");
    }
#endif
    http.end();
    c.stop();
    return cnt;
  }

  /**
   * /v1/user-service/my/task/<id> から再印刷に必要な基本情報だけ抜き出す。
   * submitReprintTaskByTaskId 内で HttpLockGuard を保持している前提。
   */
  bool getMyTaskBasicForReprint_no_lock(const char *task_id, xtouch_history_task_t *out)
  {
    if (!task_id || !task_id[0] || !out)
      return false;
    memset(out, 0, sizeof(*out));

    String host = _region == "China" ? "api.bambulab.cn" : "api.bambulab.com";
    String url = String("https://") + host + String("/v1/user-service/my/task/") + String(task_id);

    WiFiClientSecure c;
    c.setTimeout(12000);
    c.setInsecure();
    yield();
    HTTPClient http;
    http.begin(c, url);
    http.setReuse(false);
    http.useHTTP10(true);
    char auth_buf[320];
    snprintf(auth_buf, sizeof(auth_buf), "Bearer %s", _auth_token.c_str());
    http.addHeader("Authorization", auth_buf);
    http.addHeader("Connection", "close");
    http.setTimeout(12000);
    int code = http.GET();
    if (code != 200)
    {
      http.end();
      c.stop();
      return false;
    }

    ConsoleVerbose.printf("[xPTouch][V][CLOUD] GET /v1/user-service/my/task/ basic OK task_id=%s\n", task_id);
    String body = http.getString();
    const char *raw = body.c_str();
    const size_t raw_len = body.length();

    cloud_parse_json_str_key(raw, raw_len, "modelId", out->model_id, sizeof(out->model_id));
    cloud_parse_json_str_key(raw, raw_len, "title", out->title, sizeof(out->title));
    cloud_parse_json_str_key(raw, raw_len, "cover", out->cover_url, sizeof(out->cover_url));
    cloud_parse_json_str_key(raw, raw_len, "deviceName", out->device_name, sizeof(out->device_name));
    cloud_parse_json_str_key(raw, raw_len, "deviceModel", out->device_model, sizeof(out->device_model));
    cloud_parse_json_str_key(raw, raw_len, "startTime", out->start_time, sizeof(out->start_time));
    cloud_parse_json_str_key(raw, raw_len, "endTime", out->end_time, sizeof(out->end_time));
    out->profile_id = cloud_parse_json_int_key(raw, raw_len, "profileId");
    out->plate_index = cloud_parse_json_int_key(raw, raw_len, "plateIndex");
    out->status = cloud_parse_json_int_key(raw, raw_len, "status");
    out->is_printable = cloud_parse_json_bool_key(raw, raw_len, "isPrintable") ? 1 : 0;

    out->has_ams_mapping =
        (strstr(raw, "\"amsDetailMapping\"") != nullptr || strstr(raw, "\"filaments\"") != nullptr) ? 1 : 0;
    out->valid = 1;

    http.end();
    c.stop();
    return out->valid && out->model_id[0] != '\0';
  }

  /**
   * /v1/user-service/my/task/<id> から再印刷に必要な基本情報を取得（内部で HttpLockGuard 保護）。
   * getMyTaskBasicForReprint_no_lock の安全版。
   */
  bool getMyTaskBasicForReprint(const char *task_id, xtouch_history_task_t *out)
  {
    HttpLockGuard _g(this);
    return getMyTaskBasicForReprint_no_lock(task_id, out);
  }

  #if 0
  /**
   * History 一覧の history_index ベース reprint（task_id 経路へ統一したため廃止）
   */
  bool submitReprintTask(int history_index)
  {
    HttpLockGuard _g(this);
    Serial.printf("[Cloud] submitReprintTask(%d) loggedIn=%d count=%d\n", history_index, (int)loggedIn, xtouch_history_count);
    if (!loggedIn || history_index < 0 || history_index >= xtouch_history_count)
    {
      Serial.printf("[Cloud] submitReprintTask abort: !loggedIn or bad index\n");
      return false;
    }
    const xtouch_history_task_t *t = &xtouch_history_tasks[history_index];
    Serial.printf("[Cloud] submitReprintTask valid=%d is_printable=%d model_id[0]=%d\n", (int)t->valid, (int)t->is_printable, (int)t->model_id[0]);
    if (!t->valid || !t->is_printable || !t->model_id[0])
    {
      Serial.println("[Cloud] submitReprintTask abort: !valid or !is_printable or no model_id");
      return false;
    }
    const char *device_id = xTouchConfig.xTouchSerialNumber;
    Serial.printf("[Cloud] submitReprintTask device_id=%s\n", device_id ? device_id : "(null)");
    if (!device_id || !device_id[0])
    {
      Serial.println("[Cloud] submitReprintTask abort: no device_id (pair first)");
      return false;
    }

    /* JSON body: create_task 最小構成。
     *  - modelId / title / deviceId / profileId / plateIndex / cover
     *  - amsDetailMapping は 1エントリだけ固定で埋める（GFL03 / PLA 相当）
     *  - mode は "lan_file"
     */
    String host = _region == "China" ? "api.bambulab.cn" : "api.bambulab.com";
    String path = "/v1/user-service/my/task";
    String url = String("https://") + host + path;

    WiFiClientSecure &c = sslClient();
    c.stop();
    c.setTimeout(15000);
    c.setInsecure();
    yield();

    /* TLS(esp-sha) の内部ヒープ確保と競合しやすいので、JSON は必要最小限の容量に抑える */
    /* Dynamic はヒープ断片化リスクがあるため使わない。
     * ローカル(スタック)確保も避け、静的領域を再利用する。 */
    static StaticJsonDocument<2560> doc;
    doc.clear();
    doc["modelId"] = t->model_id;
    doc["title"] = t->title[0] ? t->title : "Reprint";
    if (t->cover_url[0])
      doc["cover"] = t->cover_url;
    doc["profileId"] = t->profile_id;
    doc["plateIndex"] = t->plate_index;
    doc["deviceId"] = device_id;

    JsonArray mapping = doc.createNestedArray("amsDetailMapping");
    if (xtouch_history_selected_ams_map_count > 0)
    {
      int cnt = xtouch_history_selected_ams_map_count;
      if (cnt > XTOUCH_HISTORY_AMS_MAP_MAX)
        cnt = XTOUCH_HISTORY_AMS_MAP_MAX;
      for (int i = 0; i < cnt; i++)
      {
        const xtouch_history_ams_map_t *src = &xtouch_history_selected_ams_map[i];
        uint8_t pick_ams = xtouch_history_reprint_pick_ams[i];
        uint8_t pick_tray = xtouch_history_reprint_pick_tray[i];
        JsonObject m = mapping.createNestedObject();
        m["ams"] = src->ams;
        m["nozzleId"] = src->nozzleId;
        m["weight"] = src->weight;
        uint8_t sid_ams = (pick_tray == 254) ? 0 : pick_ams;
        uint8_t sid_tray = (pick_tray == 254) ? 254 : pick_tray;
        const char *picked_setting_id = get_tray_setting_id(sid_ams, sid_tray);
        if (!(picked_setting_id && picked_setting_id[0]))
        {
          Serial.printf("[Cloud] submitReprintTask abort: missing tray setting_id map[%d] ams=%u tray=%u\n", i, (unsigned)sid_ams, (unsigned)sid_tray);
          return false;
        }
        if (!src->filamentType[0])
        {
          Serial.printf("[Cloud] submitReprintTask abort: missing filamentType map[%d]\n", i);
          return false;
        }
        if (!(src->sourceColor[0] && strlen(src->sourceColor) >= 6))
        {
          Serial.printf("[Cloud] submitReprintTask abort: missing sourceColor map[%d]\n", i);
          return false;
        }
        m["filamentId"] = picked_setting_id;
        m["filamentType"] = src->filamentType;
        m["targetFilamentType"] = src->targetFilamentType[0] ? src->targetFilamentType : "";
        if (pick_tray == 254)
        {
          m["amsId"] = 255;
          m["slotId"] = 0;
        }
        else
        {
          m["amsId"] = (int)pick_ams;
          m["slotId"] = (int)pick_tray;
        }
        const char *tc = (pick_tray == 254) ? get_tray_color(0, 254) : get_tray_color(pick_ams, pick_tray);
        if (!(tc && tc[0] && strlen(tc) >= 6))
        {
          Serial.printf("[Cloud] submitReprintTask abort: missing tray color map[%d] ams=%u tray=%u\n", i, (unsigned)pick_ams, (unsigned)pick_tray);
          return false;
        }
        char srcbuf[16];
        cloud_format_rrggbbaa(src->sourceColor, src->sourceColor, srcbuf, sizeof(srcbuf));
        char cbuf[16];
        cloud_format_rrggbbaa(tc, tc, cbuf, sizeof(cbuf));
        m["sourceColor"] = srcbuf;
        m["targetColor"] = cbuf;
      }
    }
    else
    {
      /* amsDetailMapping が無い履歴は、ユーザーが明示的にマッピングできないためリプリント禁止 */
      Serial.println("[Cloud] submitReprintTask abort: no filament mapping rows (open Reprint screen first)");
      return false;
    }

    doc["mode"] = "lan_file";

    if (doc.overflowed())
    {
      Serial.println("[Cloud] submitReprintTask abort: json doc overflowed");
      return false;
    }

    String body;
    size_t body_len = measureJson(doc);
    body.reserve(body_len + 64);
    serializeJson(doc, body);
    Serial.println("[Cloud] submitReprintTask body:");
    Serial.println(body);

    HTTPClient http;
    http.begin(c, url);
    http.setReuse(false);
    http.useHTTP10(true);
    char auth_buf[320];
    snprintf(auth_buf, sizeof(auth_buf), "Bearer %s", _auth_token.c_str());
    http.addHeader("Authorization", auth_buf);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Connection", "close");
    http.setTimeout(15000);
    int code = http.POST(body);
    String response = http.getString();
    http.end();
    c.stop();

    if (code >= 200 && code < 300)
    {
      Serial.println("[Cloud] submitReprintTask OK");
      return true;
    }
    Serial.printf("[Cloud] submitReprintTask failed code=%d %s\n", code, response.c_str());
    return false;
  }
  #endif

  /**
   * task_id だけから再印刷する（History の tasks 一覧が空でも動かす）。
   * amsDetailMapping は xtouch_history_selected_ams_map_* に依存。
   */
  bool submitReprintTaskByTaskId(const char *task_id)
  {
    HttpLockGuard _g(this);
    Serial.printf("[Cloud] submitReprintTaskByTaskId(%s) loggedIn=%d count=%d\n", task_id ? task_id : "(null)", (int)loggedIn, xtouch_history_count);
    if (!loggedIn || !task_id || !task_id[0])
    {
      Serial.println("[Cloud] submitReprintTaskByTaskId abort: !loggedIn or empty task_id");
      return false;
    }

    xtouch_history_task_t t;
    if (!getMyTaskBasicForReprint_no_lock(task_id, &t))
    {
      Serial.println("[Cloud] submitReprintTaskByTaskId abort: get basic failed");
      return false;
    }

    Serial.printf("[Cloud] submitReprintTaskByTaskId valid=%d is_printable=%d model_id[0]=%d\n", (int)t.valid, (int)t.is_printable, (int)t.model_id[0]);
    if (!t.valid || !t.is_printable || !t.model_id[0])
    {
      Serial.println("[Cloud] submitReprintTaskByTaskId abort: !valid or !is_printable or no model_id");
      return false;
    }

    const char *device_id = xTouchConfig.xTouchSerialNumber;
    int dd_slot = xtouch_history_reprint_printer_dd_slot;
    if (dd_slot > 0)
    {
      int oi = dd_slot - 1;
      if (oi >= 0 && oi < xtouch_other_printer_count && xtouch_other_printer_dev_ids[oi][0])
        device_id = xtouch_other_printer_dev_ids[oi];
    }
    Serial.printf("[Cloud] submitReprintTaskByTaskId device_id=%s dd_slot=%d\n", device_id ? device_id : "(null)", dd_slot);
    if (!device_id || !device_id[0])
    {
      Serial.println("[Cloud] submitReprintTaskByTaskId abort: no device_id (pair first)");
      return false;
    }

    String host = _region == "China" ? "api.bambulab.cn" : "api.bambulab.com";
    String path = "/v1/user-service/my/task";
    String url = String("https://") + host + path;

    WiFiClientSecure &c = sslClient();
    c.stop();
    c.setTimeout(15000);
    c.setInsecure();
    yield();

    static StaticJsonDocument<2560> doc;
    doc.clear();
    doc["modelId"] = t.model_id;
    doc["title"] = t.title[0] ? t.title : "Reprint";
    if (t.cover_url[0])
      doc["cover"] = t.cover_url;
    doc["profileId"] = t.profile_id;
    doc["plateIndex"] = t.plate_index;
    doc["deviceId"] = device_id;

    JsonArray mapping = doc.createNestedArray("amsDetailMapping");
    if (xtouch_history_selected_ams_map_count > 0)
    {
      int cnt = xtouch_history_selected_ams_map_count;
      if (cnt > XTOUCH_HISTORY_AMS_MAP_MAX)
        cnt = XTOUCH_HISTORY_AMS_MAP_MAX;
      for (int i = 0; i < cnt; i++)
      {
        const xtouch_history_ams_map_t *src = &xtouch_history_selected_ams_map[i];
        uint8_t pick_ams = xtouch_history_reprint_pick_ams[i];
        uint8_t pick_tray = xtouch_history_reprint_pick_tray[i];
        JsonObject m = mapping.createNestedObject();
        m["ams"] = src->ams;
        m["nozzleId"] = src->nozzleId;
        m["weight"] = src->weight;
        uint8_t sid_ams = (pick_tray == 254) ? 0 : pick_ams;
        uint8_t sid_tray = (pick_tray == 254) ? 254 : pick_tray;
        const char *picked_setting_id = get_tray_setting_id_reprint(sid_ams, sid_tray);
        if (!(picked_setting_id && picked_setting_id[0]))
        {
          Serial.printf("[Cloud] submitReprintTaskByTaskId abort: missing tray setting_id map[%d] ams=%u tray=%u\n", i, (unsigned)sid_ams, (unsigned)sid_tray);
          return false;
        }
        if (!src->filamentType[0])
        {
          Serial.printf("[Cloud] submitReprintTaskByTaskId abort: missing filamentType map[%d]\n", i);
          return false;
        }
        if (!(src->sourceColor[0] && strlen(src->sourceColor) >= 6))
        {
          Serial.printf("[Cloud] submitReprintTaskByTaskId abort: missing sourceColor map[%d]\n", i);
          return false;
        }
        m["filamentId"] = picked_setting_id;
        m["filamentType"] = src->filamentType;
        m["targetFilamentType"] = src->targetFilamentType[0] ? src->targetFilamentType : "";
        if (pick_tray == 254)
        {
          m["amsId"] = 255;
          m["slotId"] = 0;
        }
        else
        {
          m["amsId"] = (int)pick_ams;
          m["slotId"] = (int)pick_tray;
        }
        const char *tc = (pick_tray == 254) ? get_tray_color_reprint(0, 254) : get_tray_color_reprint(pick_ams, pick_tray);
        if (!(tc && tc[0] && strlen(tc) >= 6))
        {
          Serial.printf("[Cloud] submitReprintTaskByTaskId abort: missing tray color map[%d] ams=%u tray=%u\n", i, (unsigned)pick_ams, (unsigned)pick_tray);
          return false;
        }
        char srcbuf[16];
        cloud_format_rrggbbaa(src->sourceColor, src->sourceColor, srcbuf, sizeof(srcbuf));
        char cbuf[16];
        cloud_format_rrggbbaa(tc, tc, cbuf, sizeof(cbuf));
        m["sourceColor"] = srcbuf;
        m["targetColor"] = cbuf;
      }
    }
    else
    {
      Serial.println("[Cloud] submitReprintTaskByTaskId abort: no filament mapping rows (open Reprint screen first)");
      return false;
    }

    doc["mode"] = "lan_file";

    if (doc.overflowed())
    {
      Serial.println("[Cloud] submitReprintTaskByTaskId abort: json doc overflowed");
      return false;
    }

    String body;
    size_t body_len = measureJson(doc);
    body.reserve(body_len + 64);
    serializeJson(doc, body);
    Serial.println("[Cloud] submitReprintTaskByTaskId body:");
    Serial.println(body);

    HTTPClient http;
    http.begin(c, url);
    http.setReuse(false);
    http.useHTTP10(true);
    char auth_buf[320];
    snprintf(auth_buf, sizeof(auth_buf), "Bearer %s", _auth_token.c_str());
    http.addHeader("Authorization", auth_buf);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Connection", "close");
    http.setTimeout(15000);
    int code = http.POST(body);
    String response = http.getString();
    http.end();
    c.stop();

    if (code >= 200 && code < 300)
    {
      Serial.println("[Cloud] submitReprintTaskByTaskId OK");
      return true;
    }
    Serial.printf("[Cloud] submitReprintTaskByTaskId failed code=%d %s\n", code, response.c_str());
    return false;
  }
#endif

  void selectPrinter()
  {
    DynamicJsonDocument *deviceListDocument = nullptr;
    JsonArray devices;
    if (getDeviceList(deviceListDocument))
    {
      if (deviceListDocument != nullptr)
      {
        devices = (*deviceListDocument)["devices"].as<JsonArray>();
      }
      else
      {
        return;
      }
    }
    else
    {
      return;
    }

    // H2C/H2D/H2S は Cloud 取得反映時・printer.json に保存しない（一覧・SD 上のデバイス一覧から除外）
    auto isExcludedProduct = [](JsonVariant v) -> bool {
      if (!v.containsKey("dev_product_name")) return false;
      const char *product = v["dev_product_name"].as<const char *>();
      return product && (strcmp(product, "H2C") == 0 || strcmp(product, "H2D") == 0 || strcmp(product, "H2S") == 0);
    };

    /* printer.json は SD 上の古い内容とマージしない。Cloud devices のみを正に書き込む（H2 系は上記で除外）。 */
    DynamicJsonDocument printers_new(XTOUCH_PRINTERS_JSON_DOC_CAP);
    JsonObject root_new = printers_new.to<JsonObject>();
    for (JsonVariant v : devices)
    {
      if (isExcludedProduct(v))
        continue;
      const char *dev_id = v["dev_id"].as<const char *>();
      if (!dev_id || !dev_id[0])
        continue;

      Serial.println("\n===========================================");
      serializeJsonPretty(v, Serial);

      JsonObject dst = root_new.createNestedObject(dev_id);
      for (JsonPair kv : v.as<JsonObject>())
        dst[kv.key()] = kv.value();
    }

    serializeJsonPretty(printers_new, Serial);
    xtouch_filesystem_writeJson(xtouch_sdcard_fs(), xtouch_paths_printers, printers_new);

    // 除外後件数を数える（filteredDoc を使わずメモリ節約）
    size_t filteredCount = 0;
    for (JsonVariant v : devices)
    {
      if (!isExcludedProduct(v)) filteredCount++;
    }

    if (filteredCount == 0)
    {
      Serial.println("No devices found in Bambu Cloud (or all excluded)");

      lv_label_set_text(introScreenCaption, LV_SYMBOL_CHARGE " No Cloud Registered Devices");
      lv_timer_handler();
      lv_task_handler();
      delay(3000);
      ESP.restart();
      return;
    }

    // n 番目（0-based）の非除外デバイスを返す
    auto getNthNonExcluded = [&devices, &isExcludedProduct](size_t n) -> JsonVariant {
      size_t idx = 0;
      for (JsonVariant v : devices)
      {
        if (isExcludedProduct(v)) continue;
        if (idx == n) return v;
        idx++;
      }
      return JsonVariant();
    };

    if (filteredCount == 1)
    {
      JsonVariant single = getNthNonExcluded(0);
      setCurrentDevice(single["dev_id"].as<String>());
      setCurrentModel(single["dev_model_name"].as<String>());
      setPrinterName(single["name"].as<String>());

      applyStoredPrinterJsonSettingsToConfig();

      savePrinterPair(single["dev_id"].as<String>(), single["dev_model_name"].as<String>(), single["name"].as<String>());

      return;
    }

    loadScreen(5);

    String output = "";
    for (JsonVariant v : devices)
    {
      if (isExcludedProduct(v)) continue;
      Serial.println("\n===========================================");
      serializeJsonPretty(v, Serial);
      output = output + LV_SYMBOL_CHARGE + " " + v["dev_id"].as<String>() + " (" + v["name"].as<String>() + ")\n";
    }

    if (!output.isEmpty())
    {
      output.remove(output.length() - 1);
      lv_roller_set_options(ui_printerPairScreenRoller, output.c_str(), LV_ROLLER_MODE_NORMAL);
      lv_obj_clear_flag(ui_printerPairScreenSubmitButton, LV_OBJ_FLAG_HIDDEN);

      while (!xtouch_cloud_pair_loop_exit)
      {
        lv_timer_handler();
        lv_task_handler();
      }
      uint16_t currentIndex = lv_roller_get_selected(ui_printerPairScreenRoller);
      JsonVariant selected = getNthNonExcluded(currentIndex);
      setCurrentDevice(selected["dev_id"].as<String>());
      setCurrentModel(selected["dev_model_name"].as<String>());
      setPrinterName(selected["name"].as<String>());

      applyStoredPrinterJsonSettingsToConfig();

      savePrinterPair(selected["dev_id"].as<String>(), selected["dev_model_name"].as<String>(), selected["name"].as<String>());
    }
    delete deviceListDocument; // 使い終わったら必ず解放する
    deviceListDocument = nullptr;
    loadScreen(-1);
  }

  void savePrinterPair(String usn, String modelName, String printerName)
  {

    DynamicJsonDocument doc = xtouch_filesystem_readJson(xtouch_sdcard_fs(), xtouch_paths_pair, false);

    doc["paired"] = usn.c_str();
    doc["model"] = modelName.c_str();
    doc["printerName"] = printerName.c_str();

    xtouch_filesystem_writeJson(xtouch_sdcard_fs(), xtouch_paths_pair, doc);
    strncpy(xTouchConfig.xTouchPairedSerialNumber, usn.c_str(), sizeof(xTouchConfig.xTouchPairedSerialNumber) - 1);
    xTouchConfig.xTouchPairedSerialNumber[sizeof(xTouchConfig.xTouchPairedSerialNumber) - 1] = '\0';
  }

  bool isPaired()
  {
    if (!xtouch_filesystem_exist(xtouch_sdcard_fs(), xtouch_paths_pair))
    {
      return false;
    }
    DynamicJsonDocument doc = xtouch_filesystem_readJson(xtouch_sdcard_fs(), xtouch_paths_pair, false);
    return doc["paired"].as<String>() != "";
  }

  void loadPair()
  {
    DynamicJsonDocument doc = xtouch_filesystem_readJson(xtouch_sdcard_fs(), xtouch_paths_pair, false);
    setCurrentDevice(doc["paired"].as<String>());
    setCurrentModel(doc["model"].as<String>());
    setPrinterName(doc["printerName"].as<String>());
    strncpy(xTouchConfig.xTouchPairedSerialNumber, xTouchConfig.xTouchSerialNumber,
            sizeof(xTouchConfig.xTouchPairedSerialNumber) - 1);
    xTouchConfig.xTouchPairedSerialNumber[sizeof(xTouchConfig.xTouchPairedSerialNumber) - 1] = '\0';
  }

  /** printer.json の現在シリアル直下の settings を xTouchConfig に反映（ペア確定・行切替え後など）。 */
  void applyStoredPrinterJsonSettingsToConfig()
  {
    DynamicJsonDocument lp = loadPrinters();
    JsonObject root = lp.as<JsonObject>();
    JsonObject st;
    if (root.containsKey(xTouchConfig.xTouchSerialNumber))
    {
      JsonObject dev = root[xTouchConfig.xTouchSerialNumber].as<JsonObject>();
      if (!dev.isNull() && dev.containsKey("settings"))
        st = dev["settings"].as<JsonObject>();
    }
    xTouchConfig.xTouchChamberSensorEnabled =
        (!st.isNull() && st.containsKey("chamberTemp")) ? st["chamberTemp"].as<bool>() : false;
    xTouchConfig.xTouchAuxFanEnabled =
        (!st.isNull() && st.containsKey("auxFan")) ? st["auxFan"].as<bool>() : false;
    xTouchConfig.xTouchChamberFanEnabled =
        (!st.isNull() && st.containsKey("chamberFan")) ? st["chamberFan"].as<bool>() : false;
  }

  void setCurrentDevice(String deviceId)
  {
    strcpy(xTouchConfig.xTouchSerialNumber, deviceId.c_str());
  }

  void setPrinterName(String printerName)
  {
    strcpy(xTouchConfig.xTouchPrinterName, printerName.c_str());
  }

  void setCurrentModel(String model)
  {
    strcpy(xTouchConfig.xTouchPrinterModel, model.c_str());
  }

  DynamicJsonDocument loadPrinters()
  {
    return xtouch_filesystem_readJson(xtouch_sdcard_fs(), xtouch_paths_printers, false, XTOUCH_PRINTERS_JSON_DOC_CAP);
  }

  void clearDeviceList()
  {
    DynamicJsonDocument pairDoc(32);
    xtouch_filesystem_writeJson(xtouch_sdcard_fs(), xtouch_paths_printers, pairDoc);
  }

  void clearPairList()
  {
    xtouch_filesystem_deleteFile(xtouch_sdcard_fs(), xtouch_paths_pair);
  }
  void clearTokens()
  {
    DynamicJsonDocument config = xtouch_filesystem_readJson(xtouch_sdcard_fs(), xtouch_paths_provisioning, false, 2048);
    config["cloud-authToken"] = "";
    xtouch_filesystem_writeJson(xtouch_sdcard_fs(), xtouch_paths_provisioning, config);
  }

  void unpair()
  {
    ConsoleInfo.println("[xPTouch][I][SSDP] Unpairing device");
    DynamicJsonDocument pairFile = xtouch_filesystem_readJson(xtouch_sdcard_fs(), xtouch_paths_pair, false);
    pairFile["paired"] = "";
    xtouch_filesystem_writeJson(xtouch_sdcard_fs(), xtouch_paths_pair, pairFile);
    ESP.restart();
  }

  void saveAuthTokens()
  {
    DynamicJsonDocument config = xtouch_filesystem_readJson(xtouch_sdcard_fs(), xtouch_paths_provisioning);
    config["cloud-authToken"] = _auth_token;
    xtouch_filesystem_writeJson(xtouch_sdcard_fs(), xtouch_paths_provisioning, config, false, 2048);
    ESP.restart();
  }

  void loadAuthTokens()
  {
    ConsoleVerbose.println(ESP.getFreeHeap());
    DynamicJsonDocument config = xtouch_filesystem_readJson(xtouch_sdcard_fs(), xtouch_paths_provisioning, false, 2048);
    _auth_token = config["cloud-authToken"].as<String>();
    DynamicJsonDocument wifiConfig = xtouch_filesystem_readJson(xtouch_sdcard_fs(), xtouch_paths_provisioning);
    _region = wifiConfig["cloud-region"].as<const char *>();
    _email = wifiConfig["cloud-email"].as<String>();
    loggedIn = true;
  }

  bool hasAuthTokens()
  {
    if (!xtouch_filesystem_exist(xtouch_sdcard_fs(), xtouch_paths_provisioning))
    {
      return false;
    }
    DynamicJsonDocument config = xtouch_filesystem_readJson(xtouch_sdcard_fs(), xtouch_paths_provisioning, false, 2048);
    return config["cloud-authToken"].as<String>() != "";
  }
};
BambuCloud cloud;
