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

/* メインスレッドで呼ばれる: done キューに溜まった行を 1 件ずつロードして LIST_REFRESH（1 行ずつ描画） */
static void xtouch_history_cover_done_timer_cb(lv_timer_t *t)
{
  (void)t;
  int row;
  if (xQueueReceive(s_history_done_queue, &row, 0) != pdTRUE)
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

/* 遅延後に実行: getMyTasks → 一覧表示 → cover を 1 枚ずつワーカーに投げ（ブロックしない）。失敗時はリトライを遅延で再スケジュール。 */
static void xtouch_history_delayed_fetch_cb(lv_timer_t *t)
{
  int retry_count = (t && t->user_data) ? (int)(intptr_t)t->user_data : 0;
  const int limit = 10;
  if (!cloud.getMyTasks(limit))
  {
    if (retry_count < XTOUCH_HISTORY_FETCH_RETRY_MAX - 1)
    {
      lv_timer_t *again = lv_timer_create(xtouch_history_delayed_fetch_cb, XTOUCH_HISTORY_FETCH_RETRY_DELAY_MS, (void *)(intptr_t)(retry_count + 1));
      lv_timer_set_repeat_count(again, 1);
    }
    return;
  }
  struct XTOUCH_MESSAGE_DATA eventData = { 0, 0 };
  xtouch_history_cover_clear();
  lv_msg_send(XTOUCH_HISTORY_LIST_REFRESH, &eventData);

  if (s_history_download_queue)
    xQueueReset(s_history_download_queue);
  int n = (xtouch_history_count < XTOUCH_HISTORY_COVER_SLOTS) ? xtouch_history_count : XTOUCH_HISTORY_COVER_SLOTS;
  for (int row = 0; row < n; row++)
  {
    if (!xtouch_history_tasks[row].cover_url[0])
      continue;
    xQueueSend(s_history_download_queue, &row, 0);
  }
}

/* XTOUCH_HISTORY_FETCH 受信時は即 getMyTasks せず、遅延タイマーで非同期実行（Printers 直後の SSL メモリ競合を避ける）。user_data はリトライ回数（初回は 0）。 */
static void xtouch_history_on_fetch(lv_msg_t *m, void *user_data)
{
  (void)m;
  (void)user_data;
  lv_timer_t *once = lv_timer_create(xtouch_history_delayed_fetch_cb, XTOUCH_HISTORY_FETCH_DELAY_MS, (void *)(intptr_t)0);
  lv_timer_set_repeat_count(once, 1);
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

static void xtouch_history_subscribe_events_impl(void)
{
  if (s_history_download_queue == NULL)
  {
    s_history_download_queue = xQueueCreate(XTOUCH_HISTORY_COVER_QUEUE_LEN, sizeof(int));
    s_history_done_queue = xQueueCreate(XTOUCH_HISTORY_COVER_QUEUE_LEN, sizeof(int));
    if (s_history_download_queue && s_history_done_queue)
    {
      xTaskCreate(xtouch_history_cover_task, "hist_cover", XTOUCH_HISTORY_COVER_TASK_STACK_WORDS, NULL, 1, NULL);
      s_history_done_timer = lv_timer_create(xtouch_history_cover_done_timer_cb, XTOUCH_HISTORY_COVER_POLL_MS, NULL);
      lv_timer_set_repeat_count(s_history_done_timer, -1);
    }
  }
  lv_msg_subscribe(XTOUCH_HISTORY_FETCH, (lv_msg_subscribe_cb_t)xtouch_history_on_fetch, NULL);
  lv_msg_subscribe(XTOUCH_HISTORY_REPRINT, (lv_msg_subscribe_cb_t)xtouch_history_on_reprint, NULL);
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
