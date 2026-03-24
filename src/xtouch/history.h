/**
 * History 画面用: Cloud 履歴取得・再印刷のイベント購読。
 * UI は XTOUCH_HISTORY_FETCH / XTOUCH_HISTORY_REPRINT を送信し、
 * ここで購読して cloud.getMyTasks / cloud.submitReprintTask を呼ぶ。
 * サムネイル DL は別タスクで 1 枚ずつ行い、完了するたびにメインでロードして UI を更新する（ブロックしない）。
 */
#if defined(__XTOUCH_SCREEN_50__)

#include "ui/ui_msgs.h"
#include "types.h"
#include "cloud.hpp"
#include "xtouch/net.h"
#include "xtouch/thumbnail.h"
#include "xtouch/trays.h"
#include <SD.h>
#include <cstdio>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define XTOUCH_HISTORY_COVER_QUEUE_LEN 10
#define XTOUCH_HISTORY_COVER_TASK_STACK_WORDS 6144
#define XTOUCH_HISTORY_COVER_POLL_MS 150
/** Printers→History 直後の SSL メモリ競合を避けるため、遷移してからこの ms 後に API 取得開始 */
#define XTOUCH_HISTORY_FETCH_DELAY_MS 900
/** 取得失敗時のリトライ間隔（初回より長めに） */
#define XTOUCH_HISTORY_FETCH_RETRY_DELAY_MS 2500
/** 最大リトライ回数（初回含めてこの回数まで試す） */
#define XTOUCH_HISTORY_FETCH_RETRY_MAX 3

static QueueHandle_t s_history_download_queue = NULL;
static QueueHandle_t s_history_done_queue = NULL;
static lv_timer_t *s_history_done_timer = NULL;
static QueueHandle_t s_history_fetch_queue = NULL;
static QueueHandle_t s_history_fetch_done_queue = NULL;
static lv_timer_t *s_history_fetch_done_timer = NULL;
static QueueHandle_t s_history_detail_queue = NULL;
static QueueHandle_t s_history_detail_done_queue = NULL;
static lv_timer_t *s_history_detail_done_timer = NULL;

typedef struct
{
  int retry_count;
} xtouch_history_fetch_req_t;

typedef struct
{
  int ok;
  int retry_count;
} xtouch_history_fetch_res_t;

typedef struct
{
  int history_index;
} xtouch_history_detail_req_t;

typedef struct
{
  int ok;
  int history_index;
  int map_count;
} xtouch_history_detail_res_t;

static unsigned xtouch_color_dist_sq_rrggbbaa(const char *a, const char *b)
{
  unsigned ar = 0, ag = 0, ab = 0, br = 0, bg = 0, bb = 0;
  if (a && strlen(a) >= 6)
    (void)sscanf(a, "%2x%2x%2x", &ar, &ag, &ab);
  if (b && strlen(b) >= 6)
    (void)sscanf(b, "%2x%2x%2x", &br, &bg, &bb);
  int dr = (int)ar - (int)br;
  int dg = (int)ag - (int)bg;
  int db = (int)ab - (int)bb;
  return (unsigned)(dr * dr + dg * dg + db * db);
}

/* メインスレッド: 遅延後に fetch リクエストをキューへ投入 */
static void xtouch_history_enqueue_fetch_cb(lv_timer_t *t)
{
  int retry_count = (t && t->user_data) ? (int)(intptr_t)t->user_data : 0;
  if (s_history_fetch_queue)
    xQueueReset(s_history_fetch_queue);
  xtouch_history_fetch_req_t req = { retry_count };
  if (s_history_fetch_queue)
    xQueueSend(s_history_fetch_queue, &req, 0);
}

/* task_id をファイル名に使うため英数字のみにし、path に /tmp/{id}.png を書き込む。
 * Home / Printers と完全に同じファイル名で共有キャッシュとする。戻り値: 0=OK */
static int xtouch_history_cover_path_for_task_id(const char *task_id, char *path, size_t path_size)
{
  if (!task_id || !path || path_size < 16)
    return -1;
  char safe[XTOUCH_HISTORY_TASK_ID_LEN];
  size_t j = 0;
  for (size_t i = 0; task_id[i] != '\0' && j < sizeof(safe) - 1; i++)
  {
    char c = task_id[i];
    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')
      safe[j++] = c;
  }
  safe[j] = '\0';
  if (j == 0)
    return -1;
  (void)snprintf(path, path_size, "/tmp/%s.png", safe);
  return 0;
}

/* ワーカータスク: DL キューから row を取り、1 枚だけ DL して done キューに投げる（メインをブロックしない） */
static void xtouch_history_cover_task(void *pv)
{
  (void)pv;
  int row;
  for (;;)
  {
    if (xQueueReceive(s_history_download_queue, &row, portMAX_DELAY) != pdTRUE)
      continue;
    if (xTouchConfig.xTouchHideAllThumbnails)
      continue;
    if (row < 0 || row >= XTOUCH_HISTORY_COVER_SLOTS)
      continue;
    if (row >= xtouch_history_count || !xtouch_history_tasks[row].cover_url[0])
      continue;
    char path[64];
    if (xtouch_history_cover_path_for_task_id(xtouch_history_tasks[row].task_id, path, sizeof(path)) != 0)
      continue;
    if (SD.exists(path))
    {
      xQueueSend(s_history_done_queue, &row, 0);
      continue;
    }
    if (downloadFileToSDCard(xtouch_history_tasks[row].cover_url, path) != 0)
      xQueueSend(s_history_done_queue, &row, 0);
  }
}

/* ワーカータスク: fetch キューから要求を取り、Cloud から履歴を取得して結果を done キューに投げる（UI/LVGL をブロックしない） */
static void xtouch_history_fetch_task(void *pv)
{
  (void)pv;
  xtouch_history_fetch_req_t req;
  for (;;)
  {
    if (xQueueReceive(s_history_fetch_queue, &req, portMAX_DELAY) != pdTRUE)
      continue;
    const int limit = 10;
    xtouch_history_fetch_res_t res;
    res.retry_count = req.retry_count;
    res.ok = cloud.getMyTasks(limit) ? 1 : 0;
    if (s_history_fetch_done_queue)
      xQueueSend(s_history_fetch_done_queue, &res, 0);
  }
}

/* ワーカータスク: Reprint 用に 1タスクだけ詳細取得（GET /my/task/<id> の filaments[] を擬似マッピングに展開） */
static void xtouch_history_detail_task(void *pv)
{
  (void)pv;
  xtouch_history_detail_req_t req;
  for (;;)
  {
    if (xQueueReceive(s_history_detail_queue, &req, portMAX_DELAY) != pdTRUE)
      continue;
    xtouch_history_detail_res_t res;
    res.history_index = req.history_index;
    res.ok = 0;
    res.map_count = 0;
    if (req.history_index >= 0 && req.history_index < xtouch_history_count && xtouch_history_tasks[req.history_index].valid)
    {
      const char *tid = xtouch_history_tasks[req.history_index].task_id;
#ifdef XTOUCH_DEBUG
      ConsoleDebug.print(F("[xPTouch][HISTORY] detail: before getMyTaskAmsDetailMapping tid="));
      ConsoleDebug.println(tid ? tid : "(null)");
#endif
      int cnt = cloud.getMyTaskAmsDetailMapping(tid, xtouch_history_selected_ams_map, XTOUCH_HISTORY_AMS_MAP_MAX);
#ifdef XTOUCH_DEBUG
      ConsoleDebug.print(F("[xPTouch][HISTORY] detail: after getMyTaskAmsDetailMapping cnt="));
      ConsoleDebug.println(cnt);
#endif
      if (cnt >= 0)
      {
        if (cnt > XTOUCH_HISTORY_AMS_MAP_MAX)
          cnt = XTOUCH_HISTORY_AMS_MAP_MAX;
        xtouch_history_selected_ams_map_count = cnt;
        res.ok = 1;
        res.map_count = cnt;
        for (int i = 0; i < cnt && i < XTOUCH_HISTORY_AMS_MAP_MAX; i++)
        {
          const xtouch_history_ams_map_t *m = &xtouch_history_selected_ams_map[i];
          const char *want_type = (m->filamentType[0] && strcmp(m->filamentType, "null") != 0) ? m->filamentType : NULL;
          unsigned best_d = 0xFFFFFFFFu;
          int best_ams = -1;
          int best_tray = -1;
          int first_loaded_ams = -1;
          int first_loaded_tray = -1;
          int first_type_loaded_ams = -1;
          int first_type_loaded_tray = -1;
          for (int ams_id = 0; ams_id < XTOUCH_BAMBU_AMS_UNITS; ams_id++)
          {
            if (((bambuStatus.ams_exist_bits >> ams_id) & 1) == 0)
              continue;
            for (int tray_id = 0; tray_id < XTOUCH_BAMBU_AMS_SLOTS_PER_UNIT; tray_id++)
            {
              uint32_t st = get_tray_status((uint8_t)ams_id, (uint8_t)tray_id);
              if ((st & 1) == 0)
                continue; /* 未装填はデフォルト選択に使わない */
              if (first_loaded_ams < 0)
              {
                first_loaded_ams = ams_id;
                first_loaded_tray = tray_id;
              }
              const char *tt = get_tray_type((uint8_t)ams_id, (uint8_t)tray_id);
              int type_ok = 1;
              if (want_type && (!tt || !tt[0] || strcmp(tt, "null") == 0 || strcasecmp(tt, want_type) != 0))
                type_ok = 0;
              if (type_ok && first_type_loaded_ams < 0)
              {
                first_type_loaded_ams = ams_id;
                first_type_loaded_tray = tray_id;
              }
              if (!type_ok)
                continue;
              const char *tc = get_tray_color((uint8_t)ams_id, (uint8_t)tray_id);
              unsigned d = xtouch_color_dist_sq_rrggbbaa(m->sourceColor, tc);
              if (best_ams < 0 || d < best_d)
              {
                best_d = d;
                best_ams = ams_id;
                best_tray = tray_id;
              }
            }
          }
          if (best_ams >= 0)
          {
            xtouch_history_reprint_pick_ams[i] = (uint8_t)best_ams;
            xtouch_history_reprint_pick_tray[i] = (uint8_t)best_tray;
          }
          else if (first_type_loaded_ams >= 0)
          {
            xtouch_history_reprint_pick_ams[i] = (uint8_t)first_type_loaded_ams;
            xtouch_history_reprint_pick_tray[i] = (uint8_t)first_type_loaded_tray;
          }
          else if (first_loaded_ams >= 0)
          {
            xtouch_history_reprint_pick_ams[i] = (uint8_t)first_loaded_ams;
            xtouch_history_reprint_pick_tray[i] = (uint8_t)first_loaded_tray;
          }
          else if (m->amsId >= 0 && m->amsId < XTOUCH_BAMBU_AMS_UNITS)
          {
            xtouch_history_reprint_pick_ams[i] = (uint8_t)m->amsId;
            xtouch_history_reprint_pick_tray[i] = (uint8_t)(m->slotId & 0xFF);
          }
          else
          {
            xtouch_history_reprint_pick_ams[i] = 0;
            xtouch_history_reprint_pick_tray[i] = 0;
          }
        }
      }
      else
      {
        xtouch_history_selected_ams_map_count = 0;
      }
    }
    else
    {
      /* インデックス不正でも「取得完了」扱いにして UI が Loading のまま固まらないようにする */
      xtouch_history_selected_ams_map_count = 0;
      res.ok = 0;
      res.map_count = 0;
    }
#ifdef XTOUCH_DEBUG
    ConsoleDebug.println(F("[xPTouch][HISTORY] detail: before xQueueSend(done)"));
#endif
    if (s_history_detail_done_queue)
      xQueueSend(s_history_detail_done_queue, &res, 0);
#ifdef XTOUCH_DEBUG
    ConsoleDebug.println(F("[xPTouch][HISTORY] detail: after xQueueSend(done)"));
#endif
  }
}

/* メインスレッドで呼ばれる: done キューに溜まった行を 1 件ずつロードして LIST_REFRESH（1 行ずつ描画） */
static void xtouch_history_cover_done_timer_cb(lv_timer_t *t)
{
  (void)t;
  int row;
  if (xQueueReceive(s_history_done_queue, &row, 0) != pdTRUE)
    return;
  if (xTouchConfig.xTouchHideAllThumbnails)
    return;
  if (row >= xtouch_history_count)
    return;
  char path[64];
  if (xtouch_history_cover_path_for_task_id(xtouch_history_tasks[row].task_id, path, sizeof(path)) != 0)
    return;
  if (xtouch_history_cover_load_path(row, path))
  {
    struct XTOUCH_MESSAGE_DATA eventData = { 0, 0 };
    lv_msg_send(XTOUCH_HISTORY_LIST_REFRESH, &eventData);
  }
}

/* メインスレッド: fetch 結果を受け取り UI 更新 or リトライをスケジュール */
static void xtouch_history_fetch_done_timer_cb(lv_timer_t *t)
{
  (void)t;
  xtouch_history_fetch_res_t res;
  if (xQueueReceive(s_history_fetch_done_queue, &res, 0) != pdTRUE)
    return;
  if (!res.ok)
  {
    if (res.retry_count < XTOUCH_HISTORY_FETCH_RETRY_MAX - 1)
    {
      lv_timer_t *again =
          lv_timer_create(xtouch_history_enqueue_fetch_cb, XTOUCH_HISTORY_FETCH_RETRY_DELAY_MS, (void *)(intptr_t)(res.retry_count + 1));
      lv_timer_set_repeat_count(again, 1);
    }
    return;
  }
  struct XTOUCH_MESSAGE_DATA eventData = { 0, 0 };
  xtouch_history_cover_clear();
  lv_msg_send(XTOUCH_HISTORY_LIST_REFRESH, &eventData);

  if (s_history_download_queue)
    xQueueReset(s_history_download_queue);
  if (!xTouchConfig.xTouchHideAllThumbnails)
  {
    int n = (xtouch_history_count < XTOUCH_HISTORY_COVER_SLOTS) ? xtouch_history_count : XTOUCH_HISTORY_COVER_SLOTS;
    for (int row = 0; row < n; row++)
    {
      if (!xtouch_history_tasks[row].cover_url[0])
        continue;
      xQueueSend(s_history_download_queue, &row, 0);
    }
  }
}

/* メインスレッド: Reprint 詳細取得完了を通知（画面側で再描画） */
static void xtouch_history_detail_done_timer_cb(lv_timer_t *t)
{
  (void)t;
  xtouch_history_detail_res_t res;
  if (xQueueReceive(s_history_detail_done_queue, &res, 0) != pdTRUE)
    return;
  struct XTOUCH_MESSAGE_DATA eventData = { 0, 0 };
  lv_msg_send(XTOUCH_HISTORY_REPRINT_DETAIL_READY, &eventData);
}

/* XTOUCH_HISTORY_FETCH 受信時は即 getMyTasks せず、遅延タイマーで非同期実行（Printers 直後の SSL メモリ競合を避ける）。user_data はリトライ回数（初回は 0）。 */
static void xtouch_history_on_fetch(lv_msg_t *m, void *user_data)
{
  (void)m;
  (void)user_data;
  lv_timer_t *once =
      lv_timer_create(xtouch_history_enqueue_fetch_cb, XTOUCH_HISTORY_FETCH_DELAY_MS, (void *)(intptr_t)0);
  lv_timer_set_repeat_count(once, 1);
}

static void xtouch_history_on_reprint_slot_picked(void *user_data, lv_msg_t *m)
{
  (void)user_data;
  const struct XTOUCH_MESSAGE_DATA *payload =
      m ? (const struct XTOUCH_MESSAGE_DATA *)lv_msg_get_payload(m) : NULL;
  if (!payload)
    return;
  int mi = (int)payload->data;
  unsigned long long d2 = payload->data2;
  uint8_t ams = (uint8_t)(d2 & 0xFFu);
  uint8_t tray = (uint8_t)((d2 >> 8) & 0xFFu);
  if (mi < 0 || mi >= xtouch_history_selected_ams_map_count || mi >= XTOUCH_HISTORY_AMS_MAP_MAX)
    return;
  xtouch_history_reprint_pick_ams[mi] = ams;
  xtouch_history_reprint_pick_tray[mi] = tray;
}

/* UI から Reprint 画面が開かれたタイミングで 1件の詳細を取りに行く */
static void xtouch_history_on_reprint_detail_fetch(void *user_data, lv_msg_t *m)
{
  (void)user_data;
  const struct XTOUCH_MESSAGE_DATA *payload =
      m ? (const struct XTOUCH_MESSAGE_DATA *)lv_msg_get_payload(m) : NULL;
  if (!payload)
    return;
  int idx = (int)payload->data;
  /* 同じ idx の詳細が取得済みなら再取得しない（Reprint 画面の再描画で無限ループするのを防ぐ） */
  if (idx == xtouch_history_selected_detail_index && xtouch_history_selected_ams_map_count >= 0)
    return;
  /* 取得中(loading)が同じ idx なら enqueue しない */
  if (idx == xtouch_history_selected_detail_index && xtouch_history_selected_ams_map_count < 0)
    return;

  xtouch_history_selected_detail_index = idx;
  xtouch_history_selected_ams_map_count = -1; /* loading */
  if (s_history_detail_queue)
  {
    xQueueReset(s_history_detail_queue);
    xtouch_history_detail_req_t req = { idx };
    xQueueSend(s_history_detail_queue, &req, 0);
  }
}

/* LVGL の lv_msg 購読コールバック: (void *user_data, lv_msg_t *m)。
 * payload は常に lv_msg_get_payload(m) から取得する（Printers 画面と同じパターン）。 */
static void xtouch_history_on_reprint(void *user_data, lv_msg_t *m)
{
  (void)user_data;
  const struct XTOUCH_MESSAGE_DATA *payload =
      m ? (const struct XTOUCH_MESSAGE_DATA *)lv_msg_get_payload(m) : NULL;
  if (!payload)
    return;
  int idx = (int)payload->data;
  if (idx < 0 || idx >= xtouch_history_count || !xtouch_history_tasks[idx].valid)
    return;
  if (cloud.submitReprintTask(idx))
  {
    struct XTOUCH_MESSAGE_DATA done = { 0, 0 };
    lv_msg_send(XTOUCH_HISTORY_REPRINT_DONE, &done);
  }
}

/* 新しい History リプリント設定画面からの確定用。
 * ひとまず printer_slot / filament は Cloud 側の submitReprintTask に反映せず、
 * 既存の submitReprintTask と同じ挙動で再印刷だけ行う。 */
static void xtouch_history_on_reprint_with_options(void *user_data, lv_msg_t *m)
{
  (void)user_data;
  const struct XTOUCH_MESSAGE_DATA *payload =
      m ? (const struct XTOUCH_MESSAGE_DATA *)lv_msg_get_payload(m) : NULL;
  if (!payload)
    return;
  int idx = (int)payload->data;
  if (idx < 0 || idx >= xtouch_history_count || !xtouch_history_tasks[idx].valid)
    return;
  if (cloud.submitReprintTask(idx))
  {
    struct XTOUCH_MESSAGE_DATA done = { 0, 0 };
    lv_msg_send(XTOUCH_HISTORY_REPRINT_DONE, &done);
  }
}

static void xtouch_history_on_thumbnails_hide_changed(lv_msg_t *m, void *user_data)
{
  (void)m;
  (void)user_data;
  if (xTouchConfig.xTouchHideAllThumbnails)
  {
    xtouch_history_cover_clear();
    if (s_history_download_queue)
      xQueueReset(s_history_download_queue);
  }
  else
  {
    int n = (xtouch_history_count < XTOUCH_HISTORY_COVER_SLOTS) ? xtouch_history_count : XTOUCH_HISTORY_COVER_SLOTS;
    for (int row = 0; row < n; row++)
    {
      char path[64];
      if (xtouch_history_cover_path_for_task_id(xtouch_history_tasks[row].task_id, path, sizeof(path)) != 0)
        continue;
      if (SD.exists(path))
        (void)xtouch_history_cover_load_path(row, path);
      else if (xtouch_history_tasks[row].cover_url[0] && s_history_download_queue)
        xQueueSend(s_history_download_queue, &row, 0);
    }
  }
  struct XTOUCH_MESSAGE_DATA eventData = { 0, 0 };
  lv_msg_send(XTOUCH_HISTORY_LIST_REFRESH, &eventData);
  lv_msg_send(XTOUCH_HISTORY_REPRINT_DETAIL_READY, &eventData);
}

static void xtouch_history_subscribe_events_impl(void)
{
  if (s_history_download_queue == NULL)
  {
    s_history_download_queue = xQueueCreate(XTOUCH_HISTORY_COVER_QUEUE_LEN, sizeof(int));
    s_history_done_queue = xQueueCreate(XTOUCH_HISTORY_COVER_QUEUE_LEN, sizeof(int));
    s_history_fetch_queue = xQueueCreate(2, sizeof(xtouch_history_fetch_req_t));
    s_history_fetch_done_queue = xQueueCreate(2, sizeof(xtouch_history_fetch_res_t));
    s_history_detail_queue = xQueueCreate(1, sizeof(xtouch_history_detail_req_t));
    s_history_detail_done_queue = xQueueCreate(1, sizeof(xtouch_history_detail_res_t));
    if (s_history_download_queue && s_history_done_queue && s_history_fetch_queue && s_history_fetch_done_queue &&
        s_history_detail_queue && s_history_detail_done_queue)
    {
      xTaskCreate(xtouch_history_cover_task, "hist_cover", XTOUCH_HISTORY_COVER_TASK_STACK_WORDS, NULL, 1, NULL);
      s_history_done_timer = lv_timer_create(xtouch_history_cover_done_timer_cb, XTOUCH_HISTORY_COVER_POLL_MS, NULL);
      lv_timer_set_repeat_count(s_history_done_timer, -1);
      xTaskCreate(xtouch_history_fetch_task, "hist_fetch", XTOUCH_HISTORY_COVER_TASK_STACK_WORDS, NULL, 1, NULL);
      s_history_fetch_done_timer = lv_timer_create(xtouch_history_fetch_done_timer_cb, XTOUCH_HISTORY_COVER_POLL_MS, NULL);
      lv_timer_set_repeat_count(s_history_fetch_done_timer, -1);
      xTaskCreate(xtouch_history_detail_task, "hist_detail", XTOUCH_HISTORY_COVER_TASK_STACK_WORDS, NULL, 1, NULL);
      s_history_detail_done_timer = lv_timer_create(xtouch_history_detail_done_timer_cb, XTOUCH_HISTORY_COVER_POLL_MS, NULL);
      lv_timer_set_repeat_count(s_history_detail_done_timer, -1);
    }
  }
  lv_msg_subscribe(XTOUCH_HISTORY_FETCH, (lv_msg_subscribe_cb_t)xtouch_history_on_fetch, NULL);
  lv_msg_subscribe(XTOUCH_HISTORY_REPRINT, (lv_msg_subscribe_cb_t)xtouch_history_on_reprint, NULL);
  lv_msg_subscribe(XTOUCH_HISTORY_REPRINT_WITH_OPTIONS, (lv_msg_subscribe_cb_t)xtouch_history_on_reprint_with_options, NULL);
  lv_msg_subscribe(XTOUCH_HISTORY_REPRINT_DETAIL_FETCH, (lv_msg_subscribe_cb_t)xtouch_history_on_reprint_detail_fetch, NULL);
  lv_msg_subscribe(XTOUCH_HISTORY_REPRINT_SLOT_PICKED, (lv_msg_subscribe_cb_t)xtouch_history_on_reprint_slot_picked, NULL);
  lv_msg_subscribe(XTOUCH_THUMBNAILS_HIDE_MODE_CHANGED, (lv_msg_subscribe_cb_t)xtouch_history_on_thumbnails_hide_changed, NULL);
}

void xtouch_history_subscribe_events(void)
{
  xtouch_history_subscribe_events_impl();
}

#else

void xtouch_history_subscribe_events(void)
{
  (void)0;
}

#endif
