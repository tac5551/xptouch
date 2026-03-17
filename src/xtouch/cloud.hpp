#pragma once

#include <set>
#ifdef __XTOUCH_SCREEN_50__
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

bool xtouch_cloud_pair_loop_exit = false;
#include <WiFiClientSecure.h>
#include <cstring>

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

class BambuCloud
{

private:
  String _region;
  String _auth_token;
  /** getDeviceList / getSlicerSetting で共有。都度 new せずヒープを抑える。 */
  WiFiClientSecure *_ssl_client = nullptr;

  WiFiClientSecure &sslClient()
  {
    if (!_ssl_client)
      _ssl_client = new WiFiClientSecure();
    return *_ssl_client;
  }

public:
  String _email;
  bool loggedIn = false;

  bool getDeviceList(DynamicJsonDocument *&doc)
  {
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
    char auth_buf[320];
    snprintf(auth_buf, sizeof(auth_buf), "Bearer %s", _auth_token.c_str());
    http.addHeader("Authorization", auth_buf);
    http.setTimeout(8000);
    int code = http.GET();
    String response = (code == 200) ? http.getString() : "";
    http.end();
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
    DynamicJsonDocument config = xtouch_filesystem_readJson(SD, xtouch_paths_provisioning, false, 2048);
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
    char auth_buf[320];
    snprintf(auth_buf, sizeof(auth_buf), "Bearer %s", _auth_token.c_str());
    http.addHeader("Authorization", auth_buf);
    http.setTimeout(8000);
    int code = http.GET();
    String response = (code == 200) ? http.getString() : "";
    http.end();
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
    if (!task_id || !*task_id || !out_url || out_size == 0)
      return false;
    if (!loggedIn)
      return false;

    String host = _region == "China" ? "api.bambulab.cn" : "api.bambulab.com";
    String path = String("/v1/iot-service/api/user/task/") + task_id;
    String url = String("https://") + host + path;

#ifdef XTOUCH_DEBUG
    ConsoleDebug.print(F("[xPTouch][CLOUD] getTaskThumbnailUrl task_id="));
    ConsoleDebug.print(task_id);
    ConsoleDebug.print(F(" url="));
    ConsoleDebug.println(url);
#endif

    WiFiClientSecure &c = sslClient();
    c.stop();
    c.setTimeout(8000);
    c.setInsecure();

    yield();
    HTTPClient http;
    http.begin(c, url);
    char auth_buf[320];
    snprintf(auth_buf, sizeof(auth_buf), "Bearer %s", _auth_token.c_str());
    http.addHeader("Authorization", auth_buf);
    http.setTimeout(8000);
    int code = http.GET();
#ifdef XTOUCH_DEBUG
    ConsoleDebug.print(F("[xPTouch][CLOUD] getTaskThumbnailUrl code="));
    ConsoleDebug.println(code);
#endif
    String response = http.getString();
#ifdef XTOUCH_DEBUG
    ConsoleDebug.print(F("[xPTouch][CLOUD] getTaskThumbnailUrl resp_len="));
    ConsoleDebug.println(response.length());
    if (code != 200)
    {
      ConsoleDebug.print(F("[xPTouch][CLOUD] getTaskThumbnailUrl non-200 body="));
      ConsoleDebug.println(response);
    }
#endif
    http.end();
    if (response.length() == 0)
      return false;

#ifdef XTOUCH_DEBUG
    /* デバッグ用に task レスポンス全体を SD に保存 */
    char dump_path[64];
    snprintf(dump_path, sizeof(dump_path), "/tmp/task_%s.json", task_id);
    File dump = SD.open(dump_path, FILE_WRITE);
    if (dump)
    {
      dump.print(response);
      dump.close();
      ConsoleDebug.print(F("[xPTouch][CLOUD] saved task JSON to "));
      ConsoleDebug.println(dump_path);
    }
#endif

    /* JSON 全体をパースせず、テキスト検索で context.plates[0].thumbnail.url を抜き出す */
    const char *raw = response.c_str();
    size_t raw_len = response.length();

    /* S3 の署名付き URL 用 2KB。5inch(PSRAM あり)のときは PSRAM に配置。 */
#if defined(__XTOUCH_SCREEN_50__) && defined(CONFIG_SPIRAM)
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
#ifdef XTOUCH_DEBUG
      ConsoleDebug.println(F("[xPTouch][CLOUD] getTaskThumbnailUrl thumbnail.url empty"));
#endif
      return false;
    }

#ifdef XTOUCH_DEBUG
    ConsoleDebug.print(F("[xPTouch][CLOUD] getTaskThumbnailUrl extracted url="));
    ConsoleDebug.println(url_c);
#endif

    strncpy(out_url, url_c, out_size - 1);
    out_url[out_size - 1] = '\0';
    return true;
  }

#ifdef __XTOUCH_SCREEN_50__
  /** GET /v1/user-service/my/tasks?limit=N&deviceId=xxx[&after=id] で印刷履歴を取得。after 省略時は先頭から、指定時は続きを xtouch_history_tasks に追加。自機のみ。 */
  bool getMyTasks(int limit, const char *after = nullptr)
  {
    if (!loggedIn)
      return false;
    if (limit <= 0 || limit > XTOUCH_HISTORY_TASKS_MAX)
      limit = XTOUCH_HISTORY_TASKS_MAX;

    String host = _region == "China" ? "api.bambulab.cn" : "api.bambulab.com";
    String path = String("/v1/user-service/my/tasks?limit=") + String(limit);
    if (xTouchConfig.xTouchSerialNumber[0] != '\0')
      path += String("&deviceId=") + String(xTouchConfig.xTouchSerialNumber);
    if (after && after[0] != '\0')
      path += String("&after=") + String(after);
    String url = String("https://") + host + path;

    WiFiClientSecure &c = sslClient();
    c.stop();
    c.setTimeout(12000);
    c.setInsecure();
    yield();
    HTTPClient http;
    http.begin(c, url);
    char auth_buf[320];
    snprintf(auth_buf, sizeof(auth_buf), "Bearer %s", _auth_token.c_str());
    http.addHeader("Authorization", auth_buf);
    http.setTimeout(12000);
    int code = http.GET();
    String response = (code == 200) ? http.getString() : "";
    http.end();
    if (code != 200 || response.length() == 0)
    {
      if (!after)
        xtouch_history_count = 0;
      return false;
    }

    const char *raw = response.c_str();
    const size_t raw_len = response.length();
    const char *hits_key = "\"hits\"";
    const char *p = strstr(raw, hits_key);
    if (!p || (size_t)(p - raw) >= raw_len)
    {
      if (!after)
        xtouch_history_count = 0;
      return false;
    }
    p = (const char *)memchr(p, '[', raw_len - (size_t)(p - raw));
    if (!p)
      p = raw + raw_len;
    else
      p++;

    int start = after ? xtouch_history_count : 0;
    if (start >= XTOUCH_HISTORY_TASKS_MAX)
      return true;
    int n = start;
    while (n < XTOUCH_HISTORY_TASKS_MAX && (size_t)(p - raw) < raw_len)
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

      xtouch_history_task_t *t = &xtouch_history_tasks[n];
      memset(t, 0, sizeof(*t));
      long id_val = cloud_parse_json_long_key(obj_start, obj_len, "id");
      snprintf(t->task_id, sizeof(t->task_id), "%ld", id_val);
      cloud_parse_json_str_key(obj_start, obj_len, "modelId", t->model_id, sizeof(t->model_id));
      cloud_parse_json_str_key(obj_start, obj_len, "title", t->title, sizeof(t->title));
      cloud_parse_json_str_key(obj_start, obj_len, "cover", t->cover_url, sizeof(t->cover_url));
      cloud_parse_json_str_key(obj_start, obj_len, "deviceName", t->device_name, sizeof(t->device_name));
      cloud_parse_json_str_key(obj_start, obj_len, "startTime", t->start_time, sizeof(t->start_time));
      cloud_parse_json_str_key(obj_start, obj_len, "endTime", t->end_time, sizeof(t->end_time));
      t->profile_id = cloud_parse_json_int_key(obj_start, obj_len, "profileId");
      t->plate_index = cloud_parse_json_int_key(obj_start, obj_len, "plateIndex");
      t->status = cloud_parse_json_int_key(obj_start, obj_len, "status");
      t->is_printable = cloud_parse_json_bool_key(obj_start, obj_len, "isPrintable") ? 1 : 0;
      t->valid = 1;
      n++;

      p = obj_end + 1;
    }
    xtouch_history_count = n;
    return true;
  }

  /**
   * 履歴タスクを再印刷する。
   * GET /v1/user-service/my/tasks で取得した 1 件分 JSON（id 付き）を xtouch_history_tasks[].raw_json に保持しておき、
   * ここでは id を削除し、deviceId を現在ペア中のデバイスに上書きして POST /v1/user-service/my/task に投げる。
   * つまり「/my/tasks の 1 行（id だけ消したもの）をそのまま送り返す」イメージ。
   */
  bool submitReprintTask(int history_index)
  {
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

    StaticJsonDocument<1024> doc;
    doc["modelId"] = t->model_id;
    doc["title"] = t->title[0] ? t->title : "Reprint";
    if (t->cover_url[0])
      doc["cover"] = t->cover_url;
    doc["profileId"] = t->profile_id;
    doc["plateIndex"] = t->plate_index;
    doc["deviceId"] = device_id;

    JsonArray mapping = doc.createNestedArray("amsDetailMapping");
    JsonObject m = mapping.createNestedObject();
    m["ams"] = 0;
    m["sourceColor"] = "000000FF";
    m["targetColor"] = "000000FF";
    m["filamentId"] = "GFL03";
    m["filamentType"] = "PLA";
    m["targetFilamentType"] = "";
    m["weight"] = 0.0;
    m["nozzleId"] = 1;
    m["amsId"] = 0;
    m["slotId"] = 0;

    doc["mode"] = "lan_file";

    String body;
    serializeJson(doc, body);
    Serial.println("[Cloud] submitReprintTask body:");
    Serial.println(body);

    HTTPClient http;
    http.begin(c, url);
    char auth_buf[320];
    snprintf(auth_buf, sizeof(auth_buf), "Bearer %s", _auth_token.c_str());
    http.addHeader("Authorization", auth_buf);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(15000);
    int code = http.POST(body);
    String response = http.getString();
    http.end();

    if (code >= 200 && code < 300)
    {
      Serial.println("[Cloud] submitReprintTask OK");
      return true;
    }
    Serial.printf("[Cloud] submitReprintTask failed code=%d %s\n", code, response.c_str());
    return false;
  }
#endif

  void selectPrinter()
  {

    DynamicJsonDocument printers = xtouch_filesystem_readJson(SD, xtouch_paths_printers, false);
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

    // H2C/H2D/H2S は一覧・printer.json に出さない
    auto isExcludedProduct = [](JsonVariant v) -> bool {
      if (!v.containsKey("dev_product_name")) return false;
      const char *product = v["dev_product_name"].as<const char *>();
      return product && (strcmp(product, "H2C") == 0 || strcmp(product, "H2D") == 0 || strcmp(product, "H2S") == 0);
    };

    for (JsonVariant v : devices)
    {
      if (isExcludedProduct(v)) continue;
      Serial.println("\n===========================================");
      serializeJsonPretty(v, Serial);
      if (!printers.containsKey(v["dev_id"].as<String>()))
      {
        printers[v["dev_id"].as<String>()] = v;
      }
    }

    serializeJsonPretty(printers, Serial);
    xtouch_filesystem_writeJson(SD, xtouch_paths_printers, printers);

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

      JsonObject currentPrinterSettings = loadPrinters()[xTouchConfig.xTouchSerialNumber]["settings"];
      xTouchConfig.xTouchChamberSensorEnabled = currentPrinterSettings.containsKey("chamberTemp") ? currentPrinterSettings["chamberTemp"].as<bool>() : false;
      xTouchConfig.xTouchAuxFanEnabled = currentPrinterSettings.containsKey("auxFan") ? currentPrinterSettings["auxFan"].as<bool>() : false;
      xTouchConfig.xTouchChamberFanEnabled = currentPrinterSettings.containsKey("chamberFan") ? currentPrinterSettings["chamberFan"].as<bool>() : false;

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

      JsonObject currentPrinterSettings = loadPrinters()[xTouchConfig.xTouchSerialNumber]["settings"];
      xTouchConfig.xTouchChamberSensorEnabled = currentPrinterSettings.containsKey("chamberTemp") ? currentPrinterSettings["chamberTemp"].as<bool>() : false;
      xTouchConfig.xTouchAuxFanEnabled = currentPrinterSettings.containsKey("auxFan") ? currentPrinterSettings["auxFan"].as<bool>() : false;
      xTouchConfig.xTouchChamberFanEnabled = currentPrinterSettings.containsKey("chamberFan") ? currentPrinterSettings["chamberFan"].as<bool>() : false;

      savePrinterPair(selected["dev_id"].as<String>(), selected["dev_model_name"].as<String>(), selected["name"].as<String>());
    }
    delete deviceListDocument; // 使い終わったら必ず解放する
    deviceListDocument = nullptr;
    loadScreen(-1);
  }

  void savePrinterPair(String usn, String modelName, String printerName)
  {

    DynamicJsonDocument doc = xtouch_filesystem_readJson(SD, xtouch_paths_pair, false);

    doc["paired"] = usn.c_str();
    doc["model"] = modelName.c_str();
    doc["printerName"] = printerName.c_str();

    xtouch_filesystem_writeJson(SD, xtouch_paths_pair, doc);
  }

  bool isPaired()
  {
    if (!xtouch_filesystem_exist(SD, xtouch_paths_pair))
    {
      return false;
    }
    DynamicJsonDocument doc = xtouch_filesystem_readJson(SD, xtouch_paths_pair, false);
    return doc["paired"].as<String>() != "";
  }

  void loadPair()
  {
    DynamicJsonDocument doc = xtouch_filesystem_readJson(SD, xtouch_paths_pair, false);
    setCurrentDevice(doc["paired"].as<String>());
    setCurrentModel(doc["model"].as<String>());
    setPrinterName(doc["printerName"].as<String>());
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
    return xtouch_filesystem_readJson(SD, xtouch_paths_printers, false);
  }

  void clearDeviceList()
  {
    DynamicJsonDocument pairDoc(32);
    xtouch_filesystem_writeJson(SD, xtouch_paths_printers, pairDoc);
  }

  void clearPairList()
  {
    xtouch_filesystem_deleteFile(SD, xtouch_paths_pair);
  }
  void clearTokens()
  {
    DynamicJsonDocument config = xtouch_filesystem_readJson(SD, xtouch_paths_provisioning, false, 2048);
    config["cloud-authToken"] = "";
    xtouch_filesystem_writeJson(SD, xtouch_paths_provisioning, config);
  }

  void unpair()
  {
    ConsoleInfo.println("[xPTouch][SSDP] Unpairing device");
    DynamicJsonDocument pairFile = xtouch_filesystem_readJson(SD, xtouch_paths_pair, false);
    pairFile["paired"] = "";
    xtouch_filesystem_writeJson(SD, xtouch_paths_pair, pairFile);
    ESP.restart();
  }

  void saveAuthTokens()
  {
    DynamicJsonDocument config = xtouch_filesystem_readJson(SD, xtouch_paths_provisioning);
    config["cloud-authToken"] = _auth_token;
    xtouch_filesystem_writeJson(SD, xtouch_paths_provisioning, config, false, 2048);
    ESP.restart();
  }

  void loadAuthTokens()
  {
    ConsoleLog.println(ESP.getFreeHeap());
    DynamicJsonDocument config = xtouch_filesystem_readJson(SD, xtouch_paths_provisioning, false, 2048);
    _auth_token = config["cloud-authToken"].as<String>();
    DynamicJsonDocument wifiConfig = xtouch_filesystem_readJson(SD, xtouch_paths_provisioning);
    _region = wifiConfig["cloud-region"].as<const char *>();
    _email = wifiConfig["cloud-email"].as<String>();
    loggedIn = true;
  }

  bool hasAuthTokens()
  {
    if (!xtouch_filesystem_exist(SD, xtouch_paths_provisioning))
    {
      return false;
    }
    DynamicJsonDocument config = xtouch_filesystem_readJson(SD, xtouch_paths_provisioning, false, 2048);
    return config["cloud-authToken"].as<String>() != "";
  }
};
BambuCloud cloud;
