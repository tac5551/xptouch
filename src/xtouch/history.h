/**
 * History 画面用: Cloud 履歴取得・再印刷のイベント購読。
 * UI は XTOUCH_HISTORY_FETCH / XTOUCH_HISTORY_REPRINT_WITH_OPTIONS を送信し、
 * ここで購読して cloud.getMyTasks / cloud.submitReprintTask を呼ぶ。
 * サムネイル DL は thumbnail.h の thumb_dl ワーカー（Home と共通キュー）で 1 枚ずつ行い、完了するたびにメインでロードする。
 */
#if defined(__XTOUCH_PLATFORM_S3__)

#include "ui/ui_msgs.h"
#include "types.h"
#include "cloud.hpp"
#include "xtouch/net.h"
#include "xtouch/thumbnail.h"
#include "xtouch/trays.h"
#include "xtouch/sdcard.h"
#include <cstdio>
#include <cstring>
#include <strings.h>
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
#define XTOUCH_HISTORY_FETCH_PAGE_LIMIT 15

static QueueHandle_t s_history_done_queue = NULL;
static lv_timer_t *s_history_done_timer = NULL;
static QueueHandle_t s_history_fetch_queue = NULL;
static QueueHandle_t s_history_fetch_done_queue = NULL;
static lv_timer_t *s_history_fetch_done_timer = NULL;
static lv_timer_t *s_history_progress_timer = NULL;
static QueueHandle_t s_history_detail_queue = NULL;
static QueueHandle_t s_history_detail_done_queue = NULL;
static lv_timer_t *s_history_detail_done_timer = NULL;
static int s_history_progress_next_row = 0;
static int s_history_progress_target_row = 0;
static bool s_history_list_progress_done = true;
static uint8_t s_history_cover_pending_ready[XTOUCH_HISTORY_COVER_SLOTS] = {0};
static char s_history_last_append_after[XTOUCH_HISTORY_TASK_ID_LEN] = {0};
static void xtouch_history_enqueue_cover_row(int row);
static int xtouch_history_cover_path_for_task_id(const char *task_id, char *path, size_t path_size);

static bool xtouch_history_task_id_equals(const char *a, const char *b)
{
  if (!a || !b)
    return false;
  if (!a[0] || !b[0])
    return false;
  return strcmp(a, b) == 0;
}

/* 同じ task_id が混入したときに後ろ側を落として先頭優先で一意化する */
static int xtouch_history_dedup_tasks_inplace(void)
{
  int removed = 0;
  if (xtouch_history_count <= 1)
    return 0;
  int write_idx = 0;
  for (int i = 0; i < xtouch_history_count; i++)
  {
    bool dup = false;
    for (int j = 0; j < write_idx; j++)
    {
      if (xtouch_history_task_id_equals(xtouch_history_tasks[j].task_id, xtouch_history_tasks[i].task_id))
      {
        dup = true;
        break;
      }
    }
    if (dup)
    {
      removed++;
      continue;
    }
    if (write_idx != i)
      memcpy(&xtouch_history_tasks[write_idx], &xtouch_history_tasks[i], sizeof(xtouch_history_tasks[0]));
    write_idx++;
  }
  for (int i = write_idx; i < xtouch_history_count; i++)
    memset(&xtouch_history_tasks[i], 0, sizeof(xtouch_history_tasks[0]));
  xtouch_history_count = write_idx;
  return removed;
}

static void xtouch_history_apply_cover_row(int row)
{
  if (xTouchConfig.xTouchHideAllThumbnails)
    return;
  if (row < 0 || row >= xtouch_history_count)
    return;
  if (xTouchConfig.currentScreenIndex != 15)
    return;
  char path[64];
  if (xtouch_history_cover_path_for_task_id(xtouch_history_tasks[row].task_id, path, sizeof(path)) != 0)
    return;
  if (xtouch_history_cover_load_path(row, path))
    lv_msg_send(XTOUCH_HISTORY_LIST_REFRESH, NULL);
}

static void xtouch_history_progress_timer_cb(lv_timer_t *tt)
{
  (void)tt;
  if (s_history_progress_next_row >= s_history_progress_target_row)
  {
    s_history_list_progress_done = true;
    for (int row = 0; row < XTOUCH_HISTORY_COVER_SLOTS; row++)
    {
      if (!s_history_cover_pending_ready[row])
        continue;
      s_history_cover_pending_ready[row] = 0;
      xtouch_history_apply_cover_row(row);
    }
    if (s_history_progress_timer)
    {
      lv_timer_del(s_history_progress_timer);
      s_history_progress_timer = NULL;
    }
    return;
  }
  s_history_progress_next_row++;
  ui_msg_send(XTOUCH_HISTORY_LIST_REFRESH, (unsigned long long)s_history_progress_next_row, 0);
}

typedef struct
{
  int retry_count;
  int append;
  char after[XTOUCH_HISTORY_TASK_ID_LEN];
} xtouch_history_fetch_req_t;

typedef struct
{
  int ok;
  int retry_count;
  int append;
  int prev_count;
  int fetched_count;
  int has_more;
  char next_after[XTOUCH_HISTORY_TASK_ID_LEN];
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

/** amsDetailMapping と現在の xtouch_history_reprint_printer_dd_slot に合わせてデフォルトの AMS 候補を埋める */
static void xtouch_history_reprint_recompute_default_picks(void)
{
  int cnt = xtouch_history_selected_ams_map_count;
  if (cnt <= 0)
    return;
  if (cnt > XTOUCH_HISTORY_AMS_MAP_MAX)
    cnt = XTOUCH_HISTORY_AMS_MAP_MAX;

  for (int i = 0; i < cnt; i++)
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
    long ams_bits = xtouch_reprint_ams_exist_bits();
    for (int ams_id = 0; ams_id < XTOUCH_BAMBU_AMS_UNITS; ams_id++)
    {
      if (((ams_bits >> ams_id) & 1) == 0)
        continue;
      for (int tray_id = 0; tray_id < XTOUCH_BAMBU_AMS_SLOTS_PER_UNIT; tray_id++)
      {
        uint32_t st = (uint32_t)get_tray_status_reprint((uint8_t)ams_id, (uint8_t)tray_id);
        if ((st & 1) == 0)
          continue;
        if (first_loaded_ams < 0)
        {
          first_loaded_ams = ams_id;
          first_loaded_tray = tray_id;
        }
        char *tt = get_tray_type_reprint((uint8_t)ams_id, (uint8_t)tray_id);
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
        const char *tc = get_tray_color_reprint((uint8_t)ams_id, (uint8_t)tray_id);
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

/* メインスレッド: 遅延後に fetch リクエストをキューへ投入 */
static void xtouch_history_enqueue_fetch_cb(lv_timer_t *t)
{
  int retry_count = (t && t->user_data) ? (int)(intptr_t)t->user_data : 0;
  if (s_history_fetch_queue)
    xQueueReset(s_history_fetch_queue);
  s_history_last_append_after[0] = '\0';
  xtouch_history_fetch_req_t req;
  memset(&req, 0, sizeof(req));
  req.retry_count = retry_count;
  req.append = 0;
  if (s_history_fetch_queue)
    xQueueSend(s_history_fetch_queue, &req, 0);
}

static bool xtouch_history_enqueue_fetch_append(const char *after)
{
  if (!s_history_fetch_queue || !after || !after[0])
    return false;
  if (strcmp(s_history_last_append_after, after) == 0)
    return false;
  xtouch_history_fetch_req_t req;
  memset(&req, 0, sizeof(req));
  req.retry_count = 0;
  req.append = 1;
  strncpy(req.after, after, sizeof(req.after) - 1);
  req.after[sizeof(req.after) - 1] = '\0';
  strncpy(s_history_last_append_after, req.after, sizeof(s_history_last_append_after) - 1);
  s_history_last_append_after[sizeof(s_history_last_append_after) - 1] = '\0';
  if (xQueueSend(s_history_fetch_queue, &req, 0) == pdTRUE)
    return true;
  /* 取りこぼし防止: キュー詰まり時は古い要求を捨てて最新 cursor で再投入 */
  xQueueReset(s_history_fetch_queue);
  return xQueueSend(s_history_fetch_queue, &req, 0) == pdTRUE;
}

/* task_id → path は net.h の getThumbPathForTaskId に集約（Home の getThumbPathForSlot と同一文字列になる）。戻り値: 0=OK */
static int xtouch_history_cover_path_for_task_id(const char *task_id, char *path, size_t path_size)
{
  if (!task_id || !path || path_size < 16)
    return -1;
  if (!getThumbPathForTaskId(task_id, path, path_size))
    return -1;
  return 0;
}

/** History 行のカバーを thumb_dl 共有キューへ投入（url/path はワーカーがそのまま使う） */
static void xtouch_history_enqueue_cover_row(int row)
{
  if (row < 0 || row >= XTOUCH_HISTORY_COVER_SLOTS)
    return;
  if (row >= xtouch_history_count || !xtouch_history_tasks[row].cover_url[0])
    return;
  char path[64];
  if (xtouch_history_cover_path_for_task_id(xtouch_history_tasks[row].task_id, path, sizeof(path)) != 0)
    return;
  (void)xtouch_thumbnail_enqueue_sd_download(XTOUCH_DL_KIND_HISTORY, row, xtouch_history_tasks[row].cover_url, path);
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
    const int limit = XTOUCH_HISTORY_FETCH_PAGE_LIMIT;
    xtouch_history_fetch_res_t res;
    memset(&res, 0, sizeof(res));
    res.retry_count = req.retry_count;
    res.append = req.append;
    res.prev_count = xtouch_history_count;
    res.has_more = 0;
    res.next_after[0] = '\0';
    if (req.append && req.after[0])
      res.ok = cloud.getMyTasks(limit, req.after, res.next_after, sizeof(res.next_after)) ? 1 : 0;
    else
      res.ok = cloud.getMyTasks(limit, nullptr, res.next_after, sizeof(res.next_after)) ? 1 : 0;
    res.has_more = res.next_after[0] ? 1 : 0;
    res.fetched_count = xtouch_history_count - res.prev_count;
    if (res.fetched_count < 0)
      res.fetched_count = 0;
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
    const char *tid = NULL;
    if (xtouch_history_reprint_task_id_valid)
      tid = xtouch_history_reprint_task_id;

    if (tid)
    {
      bool basic_ok = cloud.getMyTaskBasicForReprint(tid, &xtouch_history_reprint_task_basic);
      xtouch_history_reprint_task_basic_valid = basic_ok ? 1 : 0;

      /* サムネイルは Reprint 専用 descriptor にデコードするため、
       * まず SD 上に PNG を用意して、デコードはメインスレッド側で行う。 */
      if (!xTouchConfig.xTouchHideAllThumbnails &&
          xtouch_history_reprint_task_basic.cover_url[0])
      {
        char path[64];
        if (xtouch_history_cover_path_for_task_id(tid, path, sizeof(path)) == 0)
        {
          if (!xtouch_sdcard_exists(path))
            (void)downloadFileToSDCard(xtouch_history_reprint_task_basic.cover_url, path);
          /* 重いPNGデコードはワーカー側で実行し、UIスレッドのブロッキングを避ける */
          if (xtouch_sdcard_exists(path))
            (void)xtouch_history_reprint_cover_load_path(path);
          else
            xtouch_history_reprint_cover_clear();
        }
        else
        {
          xtouch_history_reprint_cover_clear();
        }
      }
      else
      {
        xtouch_history_reprint_cover_clear();
      }

      int cnt = cloud.getMyTaskAmsDetailMapping(tid, xtouch_history_selected_ams_map, XTOUCH_HISTORY_AMS_MAP_MAX);

      if (cnt >= 0)
      {
        if (cnt > XTOUCH_HISTORY_AMS_MAP_MAX)
          cnt = XTOUCH_HISTORY_AMS_MAP_MAX;
        xtouch_history_selected_ams_map_count = cnt;
        res.ok = 1;
        res.map_count = cnt;
        xtouch_history_reprint_recompute_default_picks();
      }
      else
      {
        xtouch_history_selected_ams_map_count = 0;
      }
    }
    else
    {
      xtouch_history_selected_ams_map_count = 0;
      res.ok = 0;
      res.map_count = 0;
    }
    
    if (s_history_detail_done_queue)
      xQueueSend(s_history_detail_done_queue, &res, 0);
  }
}

/* History 系画面外ではカバーを SD からデコードしない（遷移直後の done 残り・並行ヒープ消費を防ぐ） */
static void xtouch_history_on_cover_dl_cancel(lv_msg_t *m, void *user_data)
{
  (void)m;
  (void)user_data;
  if (s_history_done_queue)
    xQueueReset(s_history_done_queue);
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
  /* Reprint(16) へ遷移後は History 一覧オブジェクトが破棄済み。
   * ここで LIST_REFRESH を送ると、削除済み ui_historyListContainer へ通知が飛び LVGL が panic する。
   * 先頭行(row=0)のカバー完了が最も早く届くため「idx=0 だけ落ちる」に見えやすい。
   * Reprint 上半分のサムネは detail_done で xtouch_history_reprint_cover_dsc に載せる。 */
  if (xTouchConfig.currentScreenIndex != 15)
    return;
  if (!s_history_list_progress_done)
  {
    if (row >= 0 && row < XTOUCH_HISTORY_COVER_SLOTS)
      s_history_cover_pending_ready[row] = 1;
    return;
  }
  xtouch_history_apply_cover_row(row);
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
  int count_before = res.prev_count;
  if (count_before < 0)
    count_before = 0;
  if (count_before > XTOUCH_HISTORY_TASKS_MAX)
    count_before = XTOUCH_HISTORY_TASKS_MAX;
  int removed_dup = xtouch_history_dedup_tasks_inplace();
  int unique_added = xtouch_history_count - count_before;
  if (unique_added < 0)
    unique_added = 0;
  if (removed_dup > 0)
  {
    ConsoleVerbose.printf("[xPTouch][V][HIST] dedup removed=%d count=%d\n", removed_dup, xtouch_history_count);
  }
  if (!res.append)
  {
    xtouch_history_cover_clear();
    /* 初回ページ: 空から段階表示 */
    ui_msg_send(XTOUCH_HISTORY_LIST_REFRESH, 0, 0);
    s_history_progress_next_row = 0;
    s_history_progress_target_row = (xtouch_history_count < XTOUCH_HISTORY_COVER_SLOTS) ? xtouch_history_count : XTOUCH_HISTORY_COVER_SLOTS;
    memset(s_history_cover_pending_ready, 0, sizeof(s_history_cover_pending_ready));
    s_history_list_progress_done = (s_history_progress_target_row == 0);
    /* 描画進行とは独立してサムネDLを先行開始 */
    for (int row = 0; row < s_history_progress_target_row; row++)
    {
      if (!xtouch_history_tasks[row].cover_url[0])
        continue;
      xtouch_history_enqueue_cover_row(row);
    }
  }
  else if (unique_added > 0)
  {
    int old_target = s_history_progress_target_row;
    int new_target = (xtouch_history_count < XTOUCH_HISTORY_COVER_SLOTS) ? xtouch_history_count : XTOUCH_HISTORY_COVER_SLOTS;
    if (old_target < 0)
      old_target = 0;
    if (old_target > new_target)
      old_target = new_target;
    s_history_progress_next_row = old_target;
    s_history_progress_target_row = new_target;
    s_history_list_progress_done = (s_history_progress_next_row >= s_history_progress_target_row);
    for (int row = old_target; row < new_target; row++)
    {
      if (!xtouch_history_tasks[row].cover_url[0])
        continue;
      xtouch_history_enqueue_cover_row(row);
    }
  }
  if (s_history_progress_timer)
    lv_timer_del(s_history_progress_timer);
  if (s_history_progress_next_row < s_history_progress_target_row)
  {
    s_history_progress_timer = lv_timer_create(xtouch_history_progress_timer_cb, 40, NULL);
    lv_timer_set_repeat_count(s_history_progress_timer, -1);
  }
  else
  {
    s_history_list_progress_done = true;
    for (int row = 0; row < XTOUCH_HISTORY_COVER_SLOTS; row++)
    {
      if (!s_history_cover_pending_ready[row])
        continue;
      s_history_cover_pending_ready[row] = 0;
      xtouch_history_apply_cover_row(row);
    }
  }

  /* History は 1 回取得（limit=20）に固定。after の連鎖取得は行わない。 */
}

/* メインスレッド: Reprint 詳細取得完了を通知（画面側で再描画） */
static void xtouch_history_detail_done_timer_cb(lv_timer_t *t)
{
  (void)t;
  xtouch_history_detail_res_t res;
  if (xQueueReceive(s_history_detail_done_queue, &res, 0) != pdTRUE)
    return;
  /* サムネイルデコードはワーカー側で完了済み。ここではUI通知のみ。 */
  xtouch_history_reprint_detail_fetch_inflight = 0;
  xtouch_history_reprint_detail_fetch_done = 1;
  lv_msg_send(XTOUCH_HISTORY_REPRINT_DETAIL_READY, NULL);
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

  (void)payload; /* history_index ではなく task_id だけで detail を取得する */
  if (!xtouch_history_reprint_task_id_valid)
    return;
  if (xtouch_history_reprint_detail_fetch_inflight)
    return;

  xtouch_history_reprint_detail_fetch_inflight = 1;
  xtouch_history_reprint_task_basic_valid = 0;
  memset(&xtouch_history_reprint_task_basic, 0, sizeof(xtouch_history_reprint_task_basic));
  xtouch_history_reprint_cover_clear();
  memset(xtouch_history_reprint_pick_ams, 0, sizeof(xtouch_history_reprint_pick_ams));
  memset(xtouch_history_reprint_pick_tray, 0, sizeof(xtouch_history_reprint_pick_tray));
  xtouch_history_selected_ams_map_count = -1; /* loading */
  if (s_history_detail_queue)
  {
    xQueueReset(s_history_detail_queue);
    xtouch_history_detail_req_t req = { 0 };
    xQueueSend(s_history_detail_queue, &req, 0);
  }
}

/* 新しい History リプリント設定画面からの確定用。
 * ひとまず printer_slot / filament は Cloud 側の submitReprintTask に反映せず、
 * 既存の submitReprintTask と同じ挙動で再印刷だけ行う。 */
static void xtouch_history_on_reprint_printer_changed(void *user_data, lv_msg_t *m)
{
  (void)user_data;
  (void)m;
  if (xtouch_history_selected_ams_map_count <= 0)
    return;
  xtouch_history_reprint_recompute_default_picks();
  /* プリンタ変更時は選択先 AMS を画面へ反映する */
  lv_msg_send(XTOUCH_HISTORY_REPRINT_DETAIL_READY, NULL);
}

static void xtouch_history_on_reprint_with_options(void *user_data, lv_msg_t *m)
{
  (void)user_data;
  const struct XTOUCH_MESSAGE_DATA *payload =
      m ? (const struct XTOUCH_MESSAGE_DATA *)lv_msg_get_payload(m) : NULL;
  if (!payload)
    return;
  xtouch_history_reprint_printer_dd_slot = (int)payload->data2;
  bool ok = false;
  if (xtouch_history_reprint_task_id_valid)
    ok = cloud.submitReprintTaskByTaskId(xtouch_history_reprint_task_id);

  if (ok)
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
  }
  else
  {
    int n = (xtouch_history_count < XTOUCH_HISTORY_COVER_SLOTS) ? xtouch_history_count : XTOUCH_HISTORY_COVER_SLOTS;
    for (int row = 0; row < n; row++)
    {
      char path[64];
      if (xtouch_history_cover_path_for_task_id(xtouch_history_tasks[row].task_id, path, sizeof(path)) != 0)
        continue;
      if (xtouch_sdcard_exists(path))
        (void)xtouch_history_cover_load_path(row, path);
      else if (xtouch_history_tasks[row].cover_url[0])
        xtouch_history_enqueue_cover_row(row);
    }
  }
  struct XTOUCH_MESSAGE_DATA eventData = { 0, 0 };
  lv_msg_send(XTOUCH_HISTORY_LIST_REFRESH, &eventData);
  lv_msg_send(XTOUCH_HISTORY_REPRINT_DETAIL_READY, NULL);
}

static void xtouch_history_subscribe_events_impl(void)
{
  if (s_history_done_queue == NULL)
  {
    s_history_done_queue = xQueueCreate(XTOUCH_HISTORY_COVER_QUEUE_LEN, sizeof(int));
    s_history_fetch_queue = xQueueCreate(2, sizeof(xtouch_history_fetch_req_t));
    s_history_fetch_done_queue = xQueueCreate(2, sizeof(xtouch_history_fetch_res_t));
    s_history_detail_queue = xQueueCreate(1, sizeof(xtouch_history_detail_req_t));
    s_history_detail_done_queue = xQueueCreate(1, sizeof(xtouch_history_detail_res_t));
    if (s_history_done_queue && s_history_fetch_queue && s_history_fetch_done_queue && s_history_detail_queue &&
        s_history_detail_done_queue)
    {
      xtouch_thumbnail_register_history_cover_done_queue(s_history_done_queue);
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
  lv_msg_subscribe(XTOUCH_HISTORY_REPRINT_WITH_OPTIONS, (lv_msg_subscribe_cb_t)xtouch_history_on_reprint_with_options, NULL);
  lv_msg_subscribe(XTOUCH_HISTORY_REPRINT_DETAIL_FETCH, (lv_msg_subscribe_cb_t)xtouch_history_on_reprint_detail_fetch, NULL);
  lv_msg_subscribe(XTOUCH_HISTORY_REPRINT_SLOT_PICKED, (lv_msg_subscribe_cb_t)xtouch_history_on_reprint_slot_picked, NULL);
  lv_msg_subscribe(XTOUCH_HISTORY_REPRINT_PRINTER_CHANGED, (lv_msg_subscribe_cb_t)xtouch_history_on_reprint_printer_changed, NULL);
  lv_msg_subscribe(XTOUCH_HISTORY_COVER_DL_CANCEL, (lv_msg_subscribe_cb_t)xtouch_history_on_cover_dl_cancel, NULL);
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
