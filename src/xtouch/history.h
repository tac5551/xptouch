/**
 * History 画面用: Cloud 履歴取得・再印刷のイベント購読。
 * UI は XPTOUCH_HISTORY_FETCH / XPTOUCH_HISTORY_REPRINT_WITH_OPTIONS を送信し、
 * ここで購読して cloud.getMyTasks / cloud.submitReprintTask を呼ぶ。
 * 一覧テキストは常に一度に描画し、その後サムネのみ SD キャッシュ読み or DL 完了のたびに 1 行ずつ LIST_REFRESH + lv_refr_now で反映する。
 * loadScreen(15) のたびに XPTOUCH_HISTORY_FETCH で getMyTasks し一覧を更新する（描画パターンは同じ）。
 */
#if defined(__XPTOUCH_PLATFORM_S3__)

#include "lvgl.h"
#include "ui/ui_msgs.h"
#include "types.h"
#include "cloud.hpp"
#include "xtouch/net.h"
#include "xtouch/thumbnail.h"
#include "xtouch/trays.h"
#include "xtouch/sdcard.h"
#include "demo.h"
#include "paths.h"
#include "filesystem.h"
#include "sdcard.h"
#include <cstdio>
#include <cstring>
#include <strings.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

static QueueHandle_t s_history_done_queue = NULL;
static lv_timer_t *s_history_done_timer = NULL;
static QueueHandle_t s_history_fetch_queue = NULL;
static QueueHandle_t s_history_fetch_done_queue = NULL;
static lv_timer_t *s_history_fetch_done_timer = NULL;
static QueueHandle_t s_history_detail_queue = NULL;
static QueueHandle_t s_history_detail_done_queue = NULL;
static lv_timer_t *s_history_detail_done_timer = NULL;
static char s_history_last_append_after[XPTOUCH_HISTORY_TASK_ID_LEN] = {0};
static void xptouch_history_enqueue_cover_row(int row);
static int xptouch_history_cover_path_for_task_id(const char *task_id, char *path, size_t path_size);

/** 一覧を全行まとめて再描画（テキスト・プレースホルダー）。サムネは別途 1 行ずつ。 */
static inline void xptouch_history_list_refresh_full(void)
{
  ui_msg_send(XPTOUCH_HISTORY_LIST_REFRESH, 0, 0);
}

/** 1 行ぶんの dsc 更新後に UI へ通知し、LVGL に即フラッシュ（同一タイマー内の連続処理で描画が溜まるのを防ぐ） */
static inline void xptouch_history_cover_ui_refresh_now(void)
{
  ui_msg_send(XPTOUCH_HISTORY_LIST_REFRESH, 0, 0);
  lv_disp_t *disp = lv_disp_get_default();
  if (disp)
    lv_refr_now(disp);
}

static bool s_hist_cover_draining = false;
static void xptouch_history_cover_drain_one_cb(lv_timer_t *t);

static lv_timer_t *s_history_cover_defer_timer = NULL;
enum
{
  XPTOUCH_HISTORY_DEFER_COVER_RETRY = 0,
  XPTOUCH_HISTORY_DEFER_THUMBS_SHOW = 1,
  XPTOUCH_HISTORY_DEFER_FETCH_ENQUEUE_FIRST = 2,
  XPTOUCH_HISTORY_DEFER_FETCH_ENQUEUE_APPEND = 3
};
static void xptouch_history_cover_defer_cancel(void);
static void xptouch_history_cover_retry_work_body(void);
static void xptouch_history_thumbs_show_work_body(void);
static void xptouch_history_fetch_enqueue_first_work_body(void);
static void xptouch_history_fetch_enqueue_append_work_body(void);
static void xptouch_history_cover_defer_work_cb(lv_timer_t *t);
static void xptouch_history_schedule_cover_work_deferred(int kind);
static int s_hist_fetch_defer_prev;
static int s_hist_fetch_defer_new_n;

static bool xptouch_history_task_id_equals(const char *a, const char *b)
{
  if (!a || !b)
    return false;
  if (!a[0] || !b[0])
    return false;
  return strcmp(a, b) == 0;
}

/* 同じ task_id が混入したときに後ろ側を落として先頭優先で一意化する */
static int xptouch_history_dedup_tasks_inplace(void)
{
  int removed = 0;
  if (xptouch_history_count <= 1)
    return 0;
  int write_idx = 0;
  for (int i = 0; i < xptouch_history_count; i++)
  {
    bool dup = false;
    for (int j = 0; j < write_idx; j++)
    {
      if (xptouch_history_task_id_equals(xptouch_history_tasks[j].task_id, xptouch_history_tasks[i].task_id))
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
      memcpy(&xptouch_history_tasks[write_idx], &xptouch_history_tasks[i], sizeof(xptouch_history_tasks[0]));
    write_idx++;
  }
  for (int i = write_idx; i < xptouch_history_count; i++)
    memset(&xptouch_history_tasks[i], 0, sizeof(xptouch_history_tasks[0]));
  xptouch_history_count = write_idx;
  return removed;
}

static void xptouch_history_apply_cover_row(int row)
{
  if (xPTouchConfig.xTouchHideAllThumbnails)
    return;
  if (row < 0 || row >= xptouch_history_count)
    return;
  if (xPTouchConfig.currentScreenIndex != 15)
    return;
  char path[64];
  if (xptouch_history_cover_path_for_task_id(xptouch_history_tasks[row].task_id, path, sizeof(path)) != 0)
    return;
  if (xptouch_history_cover_load_path(row, path))
    xptouch_history_cover_ui_refresh_now();
}

static void xptouch_history_cover_drain_chain_next(void)
{
  if (s_history_done_queue && uxQueueMessagesWaiting(s_history_done_queue) > 0)
  {
    lv_timer_t *next = lv_timer_create(xptouch_history_cover_drain_one_cb, 1, NULL);
    lv_timer_set_repeat_count(next, 1);
  }
  else
    s_hist_cover_draining = false;
}

static void xptouch_history_cover_drain_one_cb(lv_timer_t *t)
{
  (void)t;
  int row;
  if (!s_history_done_queue || xQueueReceive(s_history_done_queue, &row, 0) != pdTRUE)
  {
    s_hist_cover_draining = false;
    return;
  }
  if (xPTouchConfig.xTouchHideAllThumbnails)
  {
    xptouch_history_cover_drain_chain_next();
    return;
  }
  if (row < 0 || row >= xptouch_history_count)
  {
    xptouch_history_cover_drain_chain_next();
    return;
  }
  if (xPTouchConfig.currentScreenIndex != 15)
  {
    xptouch_history_cover_drain_chain_next();
    return;
  }
  xptouch_history_apply_cover_row(row);
  xptouch_history_cover_drain_chain_next();
}

static void xptouch_history_cover_defer_cancel(void)
{
  if (s_history_cover_defer_timer)
  {
    lv_timer_del(s_history_cover_defer_timer);
    s_history_cover_defer_timer = nullptr;
  }
}

static void xptouch_history_cover_retry_work_body(void)
{
  if (xPTouchConfig.xTouchHideAllThumbnails)
    return;
  if (xPTouchConfig.currentScreenIndex != 15)
    return;
  int n = (xptouch_history_count < XPTOUCH_HISTORY_COVER_SLOTS) ? xptouch_history_count : XPTOUCH_HISTORY_COVER_SLOTS;
  for (int row = 0; row < n; row++)
  {
    if (!xptouch_history_tasks[row].valid)
      continue;
    char path[64];
    if (xptouch_history_cover_path_for_task_id(xptouch_history_tasks[row].task_id, path, sizeof(path)) != 0)
      continue;
    if (xptouch_sdcard_exists(path))
      xptouch_history_apply_cover_row(row);
    else if (!xPTouchConfig.xTouchDemoMode && xptouch_history_tasks[row].cover_url[0])
      xptouch_history_enqueue_cover_row(row);
  }
}

static void xptouch_history_thumbs_show_work_body(void)
{
  if (xPTouchConfig.xTouchHideAllThumbnails)
    return;
  if (xPTouchConfig.currentScreenIndex != 15)
    return;
  int n = (xptouch_history_count < XPTOUCH_HISTORY_COVER_SLOTS) ? xptouch_history_count : XPTOUCH_HISTORY_COVER_SLOTS;
  for (int row = 0; row < n; row++)
  {
    char path[64];
    if (xptouch_history_cover_path_for_task_id(xptouch_history_tasks[row].task_id, path, sizeof(path)) != 0)
      continue;
    if (xptouch_sdcard_exists(path))
      xptouch_history_apply_cover_row(row);
    else if (!xPTouchConfig.xTouchDemoMode && xptouch_history_tasks[row].cover_url[0])
      xptouch_history_enqueue_cover_row(row);
  }
}

static void xptouch_history_fetch_enqueue_first_work_body(void)
{
  if (xPTouchConfig.xTouchHideAllThumbnails)
    return;
  int lim = (xptouch_history_count < XPTOUCH_HISTORY_COVER_SLOTS) ? xptouch_history_count : XPTOUCH_HISTORY_COVER_SLOTS;
  if (xPTouchConfig.xTouchDemoMode)
  {
    for (int row = 0; row < lim; row++)
    {
      if (!xptouch_history_tasks[row].valid)
        continue;
      char path[64];
      if (xptouch_history_cover_path_for_task_id(xptouch_history_tasks[row].task_id, path, sizeof(path)) != 0)
        continue;
      if (xptouch_sdcard_exists(path))
        xptouch_history_apply_cover_row(row);
    }
    return;
  }
  for (int row = 0; row < lim; row++)
  {
    if (!xptouch_history_tasks[row].cover_url[0])
      continue;
    xptouch_history_enqueue_cover_row(row);
  }
}

static void xptouch_history_fetch_enqueue_append_work_body(void)
{
  if (xPTouchConfig.xTouchHideAllThumbnails)
    return;
  for (int row = s_hist_fetch_defer_prev; row < s_hist_fetch_defer_new_n; row++)
  {
    if (!xptouch_history_tasks[row].cover_url[0])
      continue;
    xptouch_history_enqueue_cover_row(row);
  }
}

static void xptouch_history_cover_defer_work_cb(lv_timer_t *t)
{
  s_history_cover_defer_timer = nullptr;
  int kind = (int)(intptr_t)(t ? t->user_data : 0);
  switch (kind)
  {
  case XPTOUCH_HISTORY_DEFER_THUMBS_SHOW:
    xptouch_history_thumbs_show_work_body();
    break;
  case XPTOUCH_HISTORY_DEFER_FETCH_ENQUEUE_FIRST:
    xptouch_history_fetch_enqueue_first_work_body();
    break;
  case XPTOUCH_HISTORY_DEFER_FETCH_ENQUEUE_APPEND:
    xptouch_history_fetch_enqueue_append_work_body();
    break;
  default:
    xptouch_history_cover_retry_work_body();
    break;
  }
}

static void xptouch_history_schedule_cover_work_deferred(int kind)
{
  xptouch_history_cover_defer_cancel();
  lv_timer_t *once =
      lv_timer_create(xptouch_history_cover_defer_work_cb, XPTOUCH_HISTORY_COVER_DEFER_AFTER_LIST_MS, (void *)(intptr_t)kind);
  lv_timer_set_repeat_count(once, 1);
  s_history_cover_defer_timer = once;
}

typedef struct
{
  int retry_count;
  int append;
  char after[XPTOUCH_HISTORY_TASK_ID_LEN];
} xptouch_history_fetch_req_t;

typedef struct
{
  int ok;
  int retry_count;
  int append;
  int prev_count;
  int fetched_count;
  int has_more;
  char next_after[XPTOUCH_HISTORY_TASK_ID_LEN];
} xptouch_history_fetch_res_t;

typedef struct
{
  int history_index;
} xptouch_history_detail_req_t;

typedef struct
{
  int ok;
  int history_index;
  int map_count;
} xptouch_history_detail_res_t;

static unsigned xptouch_color_dist_sq_rrggbbaa(const char *a, const char *b)
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

/** amsDetailMapping と現在の xptouch_history_reprint_printer_dd_slot に合わせてデフォルトの AMS 候補を埋める */
static void xptouch_history_reprint_recompute_default_picks(void)
{
  int cnt = xptouch_history_selected_ams_map_count;
  if (cnt <= 0)
    return;
  if (cnt > XPTOUCH_HISTORY_AMS_MAP_MAX)
    cnt = XPTOUCH_HISTORY_AMS_MAP_MAX;

  for (int i = 0; i < cnt; i++)
  {
    const xptouch_history_ams_map_t *m = &xptouch_history_selected_ams_map[i];
    const char *want_type = (m->filamentType[0] && strcmp(m->filamentType, "null") != 0) ? m->filamentType : NULL;
    unsigned best_d = 0xFFFFFFFFu;
    int best_ams = -1;
    int best_tray = -1;
    int first_loaded_ams = -1;
    int first_loaded_tray = -1;
    int first_type_loaded_ams = -1;
    int first_type_loaded_tray = -1;
    long ams_bits = xptouch_reprint_ams_exist_bits();
    for (int ams_id = 0; ams_id < XPTOUCH_BAMBU_AMS_UNITS; ams_id++)
    {
      if (((ams_bits >> ams_id) & 1) == 0)
        continue;
      for (int tray_id = 0; tray_id < XPTOUCH_BAMBU_AMS_SLOTS_PER_UNIT; tray_id++)
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
        unsigned d = xptouch_color_dist_sq_rrggbbaa(m->sourceColor, tc);
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
      xptouch_history_reprint_pick_ams[i] = (uint8_t)best_ams;
      xptouch_history_reprint_pick_tray[i] = (uint8_t)best_tray;
    }
    else if (first_type_loaded_ams >= 0)
    {
      xptouch_history_reprint_pick_ams[i] = (uint8_t)first_type_loaded_ams;
      xptouch_history_reprint_pick_tray[i] = (uint8_t)first_type_loaded_tray;
    }
    else if (first_loaded_ams >= 0)
    {
      xptouch_history_reprint_pick_ams[i] = (uint8_t)first_loaded_ams;
      xptouch_history_reprint_pick_tray[i] = (uint8_t)first_loaded_tray;
    }
    else if (m->amsId >= 0 && m->amsId < XPTOUCH_BAMBU_AMS_UNITS)
    {
      xptouch_history_reprint_pick_ams[i] = (uint8_t)m->amsId;
      xptouch_history_reprint_pick_tray[i] = (uint8_t)(m->slotId & 0xFF);
    }
    else
    {
      xptouch_history_reprint_pick_ams[i] = 0;
      xptouch_history_reprint_pick_tray[i] = 0;
    }
  }
}

/* メインスレッド: 遅延後に fetch リクエストをキューへ投入 */
static void xptouch_history_enqueue_fetch_cb(lv_timer_t *t)
{
  int retry_count = (t && t->user_data) ? (int)(intptr_t)t->user_data : 0;
  if (s_history_fetch_queue)
    xQueueReset(s_history_fetch_queue);
  s_history_last_append_after[0] = '\0';
  xptouch_history_fetch_req_t req;
  memset(&req, 0, sizeof(req));
  req.retry_count = retry_count;
  req.append = 0;
  if (s_history_fetch_queue)
    xQueueSend(s_history_fetch_queue, &req, 0);
}

static bool xptouch_history_enqueue_fetch_append(const char *after)
{
  if (!s_history_fetch_queue || !after || !after[0])
    return false;
  if (strcmp(s_history_last_append_after, after) == 0)
    return false;
  xptouch_history_fetch_req_t req;
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
static int xptouch_history_cover_path_for_task_id(const char *task_id, char *path, size_t path_size)
{
  if (!task_id || !path || path_size < 16)
    return -1;

  char tmp_path[64];
  if (!getThumbPathForTaskId(task_id, tmp_path, sizeof(tmp_path)))
    return -1;

  if (xptouch_sdcard_exists(tmp_path))
  {
    strncpy(path, tmp_path, path_size - 1);
    path[path_size - 1] = '\0';
    return 0;
  }

  /* デモバックアップ: R:\demo\*.png → SD /demo/{task_id}.png */
  if (xPTouchConfig.xTouchDemoMode && strncmp(tmp_path, "/tmp/", 5) == 0)
  {
    char demo_path[64];
    snprintf(demo_path, sizeof(demo_path), "%s/%s", xptouch_paths_demo_dir, tmp_path + 5);
    if (xptouch_sdcard_exists(demo_path))
    {
      strncpy(path, demo_path, path_size - 1);
      path[path_size - 1] = '\0';
      return 0;
    }
  }

  strncpy(path, tmp_path, path_size - 1);
  path[path_size - 1] = '\0';
  return 0;
}

/** History 行のカバーを thumb_dl 共有キューへ投入（url/path はワーカーがそのまま使う） */
static void xptouch_history_enqueue_cover_row(int row)
{
  if (row < 0 || row >= XPTOUCH_HISTORY_COVER_SLOTS)
    return;
  if (row >= xptouch_history_count || !xptouch_history_tasks[row].cover_url[0])
    return;
  char path[64];
  if (xptouch_history_cover_path_for_task_id(xptouch_history_tasks[row].task_id, path, sizeof(path)) != 0)
    return;
  (void)xptouch_thumbnail_enqueue_sd_download(XPTOUCH_DL_KIND_HISTORY, row, xptouch_history_tasks[row].cover_url, path);
}

/* ワーカータスク: fetch キューから要求を取り、Cloud から履歴を取得して結果を done キューに投げる（UI/LVGL をブロックしない） */
static void xptouch_history_fetch_task(void *pv)
{
  (void)pv;
  xptouch_history_fetch_req_t req;
  for (;;)
  {
    if (xQueueReceive(s_history_fetch_queue, &req, portMAX_DELAY) != pdTRUE)
      continue;
    const int limit = XPTOUCH_HISTORY_FETCH_PAGE_LIMIT;
    xptouch_history_fetch_res_t res;
    memset(&res, 0, sizeof(res));
    res.retry_count = req.retry_count;
    res.append = req.append;
    res.prev_count = xptouch_history_count;
    res.has_more = 0;
    res.next_after[0] = '\0';
    if (req.append && req.after[0])
      res.ok = cloud.getMyTasks(limit, req.after, res.next_after, sizeof(res.next_after)) ? 1 : 0;
    else
      res.ok = cloud.getMyTasks(limit, nullptr, res.next_after, sizeof(res.next_after)) ? 1 : 0;
    res.has_more = res.next_after[0] ? 1 : 0;
    res.fetched_count = xptouch_history_count - res.prev_count;
    if (res.fetched_count < 0)
      res.fetched_count = 0;
    if (s_history_fetch_done_queue)
      xQueueSend(s_history_fetch_done_queue, &res, 0);
  }
}

/* ワーカータスク: Reprint 用に 1タスクだけ詳細取得（GET /my/task/<id> の filaments[] を擬似マッピングに展開） */
static void xptouch_history_detail_task(void *pv)
{
  (void)pv;
  xptouch_history_detail_req_t req;
  for (;;)
  {
    if (xQueueReceive(s_history_detail_queue, &req, portMAX_DELAY) != pdTRUE)
      continue;
    xptouch_history_detail_res_t res;
    res.history_index = req.history_index;
    res.ok = 0;
    res.map_count = 0;
    const char *tid = NULL;
    if (xptouch_history_reprint_task_id_valid)
      tid = xptouch_history_reprint_task_id;

    if (tid)
    {
      bool basic_ok = cloud.getMyTaskBasicForReprint(tid, &xptouch_history_reprint_task_basic);
      xptouch_history_reprint_task_basic_valid = basic_ok ? 1 : 0;

      /* サムネイルは Reprint 専用 descriptor にデコードするため、
       * まず SD 上に PNG を用意して、デコードはメインスレッド側で行う。 */
      if (!xPTouchConfig.xTouchHideAllThumbnails &&
          xptouch_history_reprint_task_basic.cover_url[0])
      {
        char path[64];
        if (xptouch_history_cover_path_for_task_id(tid, path, sizeof(path)) == 0)
        {
          if (!xptouch_sdcard_exists(path))
            (void)downloadFileToSDCard(xptouch_history_reprint_task_basic.cover_url, path);
          /* 重いPNGデコードはワーカー側で実行し、UIスレッドのブロッキングを避ける */
          if (xptouch_sdcard_exists(path))
            (void)xptouch_history_reprint_cover_load_path(path);
          else
            xptouch_history_reprint_cover_clear();
        }
        else
        {
          xptouch_history_reprint_cover_clear();
        }
      }
      else
      {
        xptouch_history_reprint_cover_clear();
      }

      int cnt = cloud.getMyTaskAmsDetailMapping(tid, xptouch_history_selected_ams_map, XPTOUCH_HISTORY_AMS_MAP_MAX);

      if (cnt >= 0)
      {
        if (cnt > XPTOUCH_HISTORY_AMS_MAP_MAX)
          cnt = XPTOUCH_HISTORY_AMS_MAP_MAX;
        xptouch_history_selected_ams_map_count = cnt;
        res.ok = 1;
        res.map_count = cnt;
        xptouch_history_reprint_recompute_default_picks();
      }
      else
      {
        xptouch_history_selected_ams_map_count = 0;
      }
    }
    else
    {
      xptouch_history_selected_ams_map_count = 0;
      res.ok = 0;
      res.map_count = 0;
    }
    
    if (s_history_detail_done_queue)
      xQueueSend(s_history_detail_done_queue, &res, 0);
  }
}

/* History 系画面外ではカバーを SD からデコードしない（遷移直後の done 残り・並行ヒープ消費を防ぐ） */
static void xptouch_history_on_cover_dl_cancel(lv_msg_t *m, void *user_data)
{
  (void)m;
  (void)user_data;
  if (s_history_done_queue)
    xQueueReset(s_history_done_queue);
  s_hist_cover_draining = false;
  xptouch_history_cover_defer_cancel();
}

/* メインスレッド: DL 完了キューは 1 件ずつデコード→UI 通知→lv_refr_now を挟み、次は 1ms 後に続行（全件を同一コールバックで捌かない） */
static void xptouch_history_cover_done_timer_cb(lv_timer_t *t)
{
  (void)t;
  if (s_hist_cover_draining)
    return;
  if (!s_history_done_queue || uxQueueMessagesWaiting(s_history_done_queue) == 0)
    return;
  s_hist_cover_draining = true;
  lv_timer_t *once = lv_timer_create(xptouch_history_cover_drain_one_cb, 0, NULL);
  lv_timer_set_repeat_count(once, 1);
}

/* メインスレッド: fetch 結果を受け取り UI 更新 or リトライをスケジュール */
static void xptouch_history_fetch_done_timer_cb(lv_timer_t *t)
{
  (void)t;
  xptouch_history_fetch_res_t res;
  if (xQueueReceive(s_history_fetch_done_queue, &res, 0) != pdTRUE)
    return;
  if (!res.ok)
  {
    if (res.retry_count < XPTOUCH_HISTORY_FETCH_RETRY_MAX - 1)
    {
      lv_timer_t *again =
          lv_timer_create(xptouch_history_enqueue_fetch_cb, XPTOUCH_HISTORY_FETCH_RETRY_DELAY_MS, (void *)(intptr_t)(res.retry_count + 1));
      lv_timer_set_repeat_count(again, 1);
    }
    return;
  }
  int count_before = res.prev_count;
  if (count_before < 0)
    count_before = 0;
  if (count_before > XPTOUCH_HISTORY_TASKS_MAX)
    count_before = XPTOUCH_HISTORY_TASKS_MAX;
  int removed_dup = xptouch_history_dedup_tasks_inplace();
  int unique_added = xptouch_history_count - count_before;
  if (unique_added < 0)
    unique_added = 0;
  if (removed_dup > 0)
  {
    ConsoleVerbose.printf("[xPTouch][V][HIST] dedup removed=%d count=%d\n", removed_dup, xptouch_history_count);
  }
  if (!res.append)
  {
    xptouch_history_cover_clear();
    xptouch_history_list_refresh_full();
    {
      lv_disp_t *disp = lv_disp_get_default();
      if (disp)
        lv_refr_now(disp);
    }
    xptouch_history_schedule_cover_work_deferred(XPTOUCH_HISTORY_DEFER_FETCH_ENQUEUE_FIRST);
  }
  else if (unique_added > 0)
  {
    int prev = count_before;
    if (prev < 0)
      prev = 0;
    if (prev > XPTOUCH_HISTORY_COVER_SLOTS)
      prev = XPTOUCH_HISTORY_COVER_SLOTS;
    int new_n = (xptouch_history_count < XPTOUCH_HISTORY_COVER_SLOTS) ? xptouch_history_count : XPTOUCH_HISTORY_COVER_SLOTS;
    xptouch_history_list_refresh_full();
    {
      lv_disp_t *disp = lv_disp_get_default();
      if (disp)
        lv_refr_now(disp);
    }
    s_hist_fetch_defer_prev = prev;
    s_hist_fetch_defer_new_n = new_n;
    xptouch_history_schedule_cover_work_deferred(XPTOUCH_HISTORY_DEFER_FETCH_ENQUEUE_APPEND);
  }

  /* History は 1 回取得（limit=20）に固定。after の連鎖取得は行わない。 */
}

/* メインスレッド: Reprint 詳細取得完了を通知（画面側で再描画） */
static void xptouch_history_detail_done_timer_cb(lv_timer_t *t)
{
  (void)t;
  xptouch_history_detail_res_t res;
  if (xQueueReceive(s_history_detail_done_queue, &res, 0) != pdTRUE)
    return;
  /* サムネイルデコードはワーカー側で完了済み。ここではUI通知のみ。 */
  xptouch_history_reprint_detail_fetch_inflight = 0;
  xptouch_history_reprint_detail_fetch_done = 1;
  lv_msg_send(XPTOUCH_HISTORY_REPRINT_DETAIL_READY, NULL);
}

static void xptouch_history_finish_demo_fetch_ui(void)
{
  xptouch_history_cover_clear();
  xptouch_history_list_refresh_full();
  lv_disp_t *disp = lv_disp_get_default();
  if (disp)
    lv_refr_now(disp);
  xptouch_history_schedule_cover_work_deferred(XPTOUCH_HISTORY_DEFER_FETCH_ENQUEUE_FIRST);
}

/** デモ Reprint: /demo/task_{id}.json または /tmp/task_{id}.json を読む */
static bool xptouch_demo_read_task_detail_json(const char *task_id, char *buf, size_t buf_len, size_t *out_len)
{
  if (!task_id || !task_id[0] || !buf || buf_len < 64 || !out_len)
    return false;
  *out_len = 0;

  char path_my[56];
  char path_demo[56];
  char path_tmp[56];
  snprintf(path_my, sizeof(path_my), "%s/my_task_%s.json", xptouch_paths_demo_dir, task_id);
  snprintf(path_demo, sizeof(path_demo), "%s/task_%s.json", xptouch_paths_demo_dir, task_id);
  snprintf(path_tmp, sizeof(path_tmp), "/tmp/task_%s.json", task_id);
  const char *paths[3] = { path_my, path_demo, path_tmp };

  for (int pi = 0; pi < 3; pi++)
  {
    if (!xptouch_filesystem_exist(xptouch_sdcard_fs(), paths[pi]))
      continue;
    File f = xptouch_filesystem_open(xptouch_sdcard_fs(), paths[pi]);
    if (!f)
      continue;
    const size_t sz = f.size();
    if (sz == 0 || sz >= buf_len)
    {
      f.close();
      continue;
    }
    const size_t n = f.read((uint8_t *)buf, sz);
    f.close();
    if (n != sz)
      continue;
    buf[n] = '\0';
    *out_len = n;
    ConsoleInfo.printf("[xPTouch][I][DEMO] reprint detail json %s (%u bytes)\n", paths[pi], (unsigned)n);
    return true;
  }
  return false;
}

static const xptouch_history_task_t *xptouch_demo_find_history_task(const char *task_id)
{
  if (!task_id || !task_id[0])
    return nullptr;
  for (int i = 0; i < xptouch_history_count; i++)
  {
    if (xptouch_history_tasks[i].valid && strcmp(xptouch_history_tasks[i].task_id, task_id) == 0)
      return &xptouch_history_tasks[i];
  }
  return nullptr;
}

/** cloud_parse_json_str_key は未検出時に out を空にする。history マージ用に値があるときだけ上書き。 */
static void xptouch_demo_merge_json_str(const char *json, size_t len, const char *key, char *dst, size_t dst_len)
{
  if (!json || !key || !dst || dst_len == 0)
    return;
  char tmp[128];
  cloud_parse_json_str_key(json, len, key, tmp, sizeof(tmp));
  if (tmp[0])
  {
    strncpy(dst, tmp, dst_len - 1);
    dst[dst_len - 1] = '\0';
  }
}

/** デモ Reprint 詳細（Cloud GET /my/task の代替） */
static bool xptouch_demo_load_reprint_detail(const char *task_id)
{
  if (!task_id || !task_id[0])
    return false;

  if (xptouch_history_count <= 0)
    (void)xptouch_demo_load_history_from_sd();

  xptouch_history_reprint_cover_clear();
  memset(xptouch_history_reprint_pick_ams, 0, sizeof(xptouch_history_reprint_pick_ams));
  memset(xptouch_history_reprint_pick_tray, 0, sizeof(xptouch_history_reprint_pick_tray));
  memset(&xptouch_history_reprint_task_basic, 0, sizeof(xptouch_history_reprint_task_basic));
  xptouch_history_reprint_task_basic_valid = 0;
  memset(xptouch_history_selected_ams_map, 0, sizeof(xptouch_history_selected_ams_map));
  xptouch_history_selected_ams_map_count = 0;

  const xptouch_history_task_t *hist = xptouch_demo_find_history_task(task_id);
  if (hist)
  {
    memcpy(&xptouch_history_reprint_task_basic, hist, sizeof(xptouch_history_reprint_task_basic));
    xptouch_history_reprint_task_basic_valid = 1;
  }

  xptouch_history_task_t *out = &xptouch_history_reprint_task_basic;
  strncpy(out->task_id, task_id, sizeof(out->task_id) - 1);
  out->task_id[sizeof(out->task_id) - 1] = '\0';

  static char json_buf[XPTOUCH_DEMO_TASK_JSON_CAP];
  size_t json_len = 0;
  if (xptouch_demo_read_task_detail_json(task_id, json_buf, sizeof(json_buf), &json_len))
  {
    /* /tmp/task_*.json は getTaskThumbnailUrl（iot-service）のダンプが多い。my/task 形式とキーが異なる */
    xptouch_demo_merge_json_str(json_buf, json_len, "modelId", out->model_id, sizeof(out->model_id));
    cloud_parse_json_scalar_key_as_string(json_buf, json_len, "modelId", out->model_id, sizeof(out->model_id));
    xptouch_demo_merge_json_str(json_buf, json_len, "title", out->title, sizeof(out->title));
    xptouch_demo_merge_json_str(json_buf, json_len, "subtaskName", out->title, sizeof(out->title));
    xptouch_demo_merge_json_str(json_buf, json_len, "subtask_name", out->title, sizeof(out->title));
    xptouch_demo_merge_json_str(json_buf, json_len, "name", out->title, sizeof(out->title));
    xptouch_demo_merge_json_str(json_buf, json_len, "projectName", out->title, sizeof(out->title));
    xptouch_demo_merge_json_str(json_buf, json_len, "cover", out->cover_url, sizeof(out->cover_url));
    xptouch_demo_merge_json_str(json_buf, json_len, "deviceName", out->device_name, sizeof(out->device_name));
    xptouch_demo_merge_json_str(json_buf, json_len, "deviceModel", out->device_model, sizeof(out->device_model));
    xptouch_demo_merge_json_str(json_buf, json_len, "startTime", out->start_time, sizeof(out->start_time));
    xptouch_demo_merge_json_str(json_buf, json_len, "endTime", out->end_time, sizeof(out->end_time));
    {
      const int prof = cloud_parse_json_int_key(json_buf, json_len, "profileId");
      if (prof != 0)
        out->profile_id = prof;
      const int plate = cloud_parse_json_int_key(json_buf, json_len, "plateIndex");
      if (plate != 0)
        out->plate_index = plate;
      const int st = cloud_parse_json_int_key(json_buf, json_len, "status");
      if (st != 0)
        out->status = st;
    }
    if (cloud_parse_json_bool_key(json_buf, json_len, "isPrintable"))
      out->is_printable = 1;
    out->has_ams_mapping =
        (strstr(json_buf, "\"amsDetailMapping\"") != nullptr || strstr(json_buf, "\"filaments\"") != nullptr) ? 1 : 0;
    out->valid = 1;

    int cnt = cloud_parse_ams_detail_mapping(json_buf, json_len, xptouch_history_selected_ams_map, XPTOUCH_HISTORY_AMS_MAP_MAX);
    if (cnt <= 0)
      cnt = cloud_parse_reprint_mapping_from_buffer(json_buf, json_len, xptouch_history_selected_ams_map, XPTOUCH_HISTORY_AMS_MAP_MAX);
    if (cnt < 0)
      cnt = 0;
    if (cnt > XPTOUCH_HISTORY_AMS_MAP_MAX)
      cnt = XPTOUCH_HISTORY_AMS_MAP_MAX;
    xptouch_history_selected_ams_map_count = cnt;
  }

  if (!out->title[0] && bambuStatus.task_id[0] && strcmp(bambuStatus.task_id, task_id) == 0 && bambuStatus.subtask_name[0])
  {
    strncpy(out->title, bambuStatus.subtask_name, sizeof(out->title) - 1);
    out->title[sizeof(out->title) - 1] = '\0';
  }

  if (out->title[0] || out->device_name[0] || out->model_id[0] || out->device_model[0])
  {
    out->valid = 1;
    xptouch_history_reprint_task_basic_valid = 1;
  }

  if (!xPTouchConfig.xTouchHideAllThumbnails)
  {
    char thumb_path[64];
    if (xptouch_history_cover_path_for_task_id(task_id, thumb_path, sizeof(thumb_path)) == 0 &&
        xptouch_sdcard_exists(thumb_path))
      (void)xptouch_history_reprint_cover_load_path(thumb_path);
  }

  if (xptouch_history_selected_ams_map_count > 0)
    xptouch_history_reprint_recompute_default_picks();

  ConsoleInfo.printf("[xPTouch][I][DEMO] reprint detail basic=%d maps=%d cover=%d\n",
                     xptouch_history_reprint_task_basic_valid,
                     xptouch_history_selected_ams_map_count,
                     xptouch_history_reprint_cover_dsc != nullptr ? 1 : 0);
  return xptouch_history_reprint_task_basic_valid != 0;
}

/* XPTOUCH_HISTORY_FETCH 受信時は即 getMyTasks せず、遅延タイマーで非同期実行（Printers 直後の SSL メモリ競合を避ける）。user_data はリトライ回数（初回は 0）。 */
static void xptouch_history_on_fetch(lv_msg_t *m, void *user_data)
{
  (void)m;
  (void)user_data;
  if (xPTouchConfig.xTouchDemoMode)
  {
    if (xptouch_demo_load_history_from_sd())
      xptouch_history_finish_demo_fetch_ui();
    return;
  }
  lv_timer_t *once =
      lv_timer_create(xptouch_history_enqueue_fetch_cb, XPTOUCH_HISTORY_FETCH_DELAY_MS, (void *)(intptr_t)0);
  lv_timer_set_repeat_count(once, 1);
}

static void xptouch_history_on_reprint_slot_picked(void *user_data, lv_msg_t *m)
{
  (void)user_data;
  const struct XPTOUCH_MESSAGE_DATA *payload =
      m ? (const struct XPTOUCH_MESSAGE_DATA *)lv_msg_get_payload(m) : NULL;
  if (!payload)
    return;
  int mi = (int)payload->data;
  unsigned long long d2 = payload->data2;
  uint8_t ams = (uint8_t)(d2 & 0xFFu);
  uint8_t tray = (uint8_t)((d2 >> 8) & 0xFFu);
  if (mi < 0 || mi >= xptouch_history_selected_ams_map_count || mi >= XPTOUCH_HISTORY_AMS_MAP_MAX)
    return;
  xptouch_history_reprint_pick_ams[mi] = ams;
  xptouch_history_reprint_pick_tray[mi] = tray;
}

/* UI から Reprint 画面が開かれたタイミングで 1件の詳細を取りに行く */
static void xptouch_history_on_reprint_detail_fetch(void *user_data, lv_msg_t *m)
{
  (void)user_data;
  const struct XPTOUCH_MESSAGE_DATA *payload =
      m ? (const struct XPTOUCH_MESSAGE_DATA *)lv_msg_get_payload(m) : NULL;
  if (!payload)
    return;

  if (xPTouchConfig.xTouchDemoMode)
  {
    if (!xptouch_history_reprint_task_id_valid)
      return;
    xptouch_history_reprint_detail_fetch_inflight = 1;
    xptouch_history_selected_ams_map_count = -1;
    (void)xptouch_demo_load_reprint_detail(xptouch_history_reprint_task_id);
    xptouch_history_reprint_detail_fetch_inflight = 0;
    xptouch_history_reprint_detail_fetch_done = 1;
    lv_msg_send(XPTOUCH_HISTORY_REPRINT_DETAIL_READY, NULL);
    return;
  }

  (void)payload; /* history_index ではなく task_id だけで detail を取得する */
  if (!xptouch_history_reprint_task_id_valid)
    return;
  if (xptouch_history_reprint_detail_fetch_inflight)
    return;

  xptouch_history_reprint_detail_fetch_inflight = 1;
  xptouch_history_reprint_task_basic_valid = 0;
  memset(&xptouch_history_reprint_task_basic, 0, sizeof(xptouch_history_reprint_task_basic));
  xptouch_history_reprint_cover_clear();
  memset(xptouch_history_reprint_pick_ams, 0, sizeof(xptouch_history_reprint_pick_ams));
  memset(xptouch_history_reprint_pick_tray, 0, sizeof(xptouch_history_reprint_pick_tray));
  xptouch_history_selected_ams_map_count = -1; /* loading */
  if (s_history_detail_queue)
  {
    xQueueReset(s_history_detail_queue);
    xptouch_history_detail_req_t req = { 0 };
    xQueueSend(s_history_detail_queue, &req, 0);
  }
}

/* 新しい History リプリント設定画面からの確定用。
 * ひとまず printer_slot / filament は Cloud 側の submitReprintTask に反映せず、
 * 既存の submitReprintTask と同じ挙動で再印刷だけ行う。 */
static void xptouch_history_on_reprint_printer_changed(void *user_data, lv_msg_t *m)
{
  (void)user_data;
  (void)m;
  /* -1=詳細未取得のときだけスキップ。0件マッピングでもプリンタ切替後の AMS 表示更新が必要 */
  if (xptouch_history_selected_ams_map_count < 0)
    return;
  xptouch_history_reprint_recompute_default_picks();
  /* プリンタ変更時は選択先 AMS を画面へ反映する */
  lv_msg_send(XPTOUCH_HISTORY_REPRINT_DETAIL_READY, NULL);
}

static void xptouch_history_on_reprint_with_options(void *user_data, lv_msg_t *m)
{
  (void)user_data;
  const struct XPTOUCH_MESSAGE_DATA *payload =
      m ? (const struct XPTOUCH_MESSAGE_DATA *)lv_msg_get_payload(m) : NULL;
  if (!payload)
    return;
  xptouch_history_reprint_printer_dd_slot = (int)payload->data2;
  if (xPTouchConfig.xTouchDemoMode)
  {
    struct XPTOUCH_MESSAGE_DATA done = { 0, 0 };
    lv_msg_send(XPTOUCH_HISTORY_REPRINT_DONE, &done);
    return;
  }
  bool ok = false;
  if (xptouch_history_reprint_task_id_valid)
    ok = cloud.submitReprintTaskByTaskId(xptouch_history_reprint_task_id);

  if (ok)
  {
    struct XPTOUCH_MESSAGE_DATA done = { 0, 0 };
    lv_msg_send(XPTOUCH_HISTORY_REPRINT_DONE, &done);
  }
}

static void xptouch_history_on_thumbnails_hide_changed(lv_msg_t *m, void *user_data)
{
  (void)m;
  (void)user_data;
  if (xPTouchConfig.xTouchHideAllThumbnails)
  {
    xptouch_history_cover_clear();
    xptouch_history_list_refresh_full();
  }
  else
  {
    xptouch_history_list_refresh_full();
    {
      lv_disp_t *disp = lv_disp_get_default();
      if (disp)
        lv_refr_now(disp);
    }
    xptouch_history_schedule_cover_work_deferred(XPTOUCH_HISTORY_DEFER_THUMBS_SHOW);
  }
  lv_msg_send(XPTOUCH_HISTORY_REPRINT_DETAIL_READY, NULL);
}

/** History 再入室時: 一覧はメモリにある。LIST_REFRESH のみ（lv_refr_now は load_scr 前に呼ぶとアクティブ画面不一致で Panic し得る）。サムネは遅延タイマーで開始。 */
static void xptouch_history_on_cover_retry(lv_msg_t *m, void *user_data)
{
  (void)m;
  (void)user_data;
  if (xPTouchConfig.xTouchHideAllThumbnails)
    return;
  xptouch_history_list_refresh_full();
  xptouch_history_schedule_cover_work_deferred(XPTOUCH_HISTORY_DEFER_COVER_RETRY);
}

static void xptouch_history_subscribe_events_impl(void)
{
  if (s_history_done_queue == NULL)
  {
    s_history_done_queue = xQueueCreate(XPTOUCH_HISTORY_COVER_QUEUE_LEN, sizeof(int));
    s_history_fetch_queue = xQueueCreate(2, sizeof(xptouch_history_fetch_req_t));
    s_history_fetch_done_queue = xQueueCreate(2, sizeof(xptouch_history_fetch_res_t));
    s_history_detail_queue = xQueueCreate(1, sizeof(xptouch_history_detail_req_t));
    s_history_detail_done_queue = xQueueCreate(1, sizeof(xptouch_history_detail_res_t));
    if (s_history_done_queue && s_history_fetch_queue && s_history_fetch_done_queue && s_history_detail_queue &&
        s_history_detail_done_queue)
    {
      xptouch_thumbnail_register_history_cover_done_queue(s_history_done_queue);
      s_history_done_timer = lv_timer_create(xptouch_history_cover_done_timer_cb, XPTOUCH_HISTORY_COVER_POLL_MS, NULL);
      lv_timer_set_repeat_count(s_history_done_timer, -1);
      xTaskCreate(xptouch_history_fetch_task, "hist_fetch", XPTOUCH_HISTORY_COVER_TASK_STACK_WORDS, NULL, 1, NULL);
      s_history_fetch_done_timer = lv_timer_create(xptouch_history_fetch_done_timer_cb, XPTOUCH_HISTORY_COVER_POLL_MS, NULL);
      lv_timer_set_repeat_count(s_history_fetch_done_timer, -1);
      xTaskCreate(xptouch_history_detail_task, "hist_detail", XPTOUCH_HISTORY_COVER_TASK_STACK_WORDS, NULL, 1, NULL);
      s_history_detail_done_timer = lv_timer_create(xptouch_history_detail_done_timer_cb, XPTOUCH_HISTORY_COVER_POLL_MS, NULL);
      lv_timer_set_repeat_count(s_history_detail_done_timer, -1);
    }
  }
  lv_msg_subscribe(XPTOUCH_HISTORY_FETCH, (lv_msg_subscribe_cb_t)xptouch_history_on_fetch, NULL);
  lv_msg_subscribe(XPTOUCH_HISTORY_COVER_RETRY, (lv_msg_subscribe_cb_t)xptouch_history_on_cover_retry, NULL);
  lv_msg_subscribe(XPTOUCH_HISTORY_REPRINT_WITH_OPTIONS, (lv_msg_subscribe_cb_t)xptouch_history_on_reprint_with_options, NULL);
  lv_msg_subscribe(XPTOUCH_HISTORY_REPRINT_DETAIL_FETCH, (lv_msg_subscribe_cb_t)xptouch_history_on_reprint_detail_fetch, NULL);
  lv_msg_subscribe(XPTOUCH_HISTORY_REPRINT_SLOT_PICKED, (lv_msg_subscribe_cb_t)xptouch_history_on_reprint_slot_picked, NULL);
  lv_msg_subscribe(XPTOUCH_HISTORY_REPRINT_PRINTER_CHANGED, (lv_msg_subscribe_cb_t)xptouch_history_on_reprint_printer_changed, NULL);
  lv_msg_subscribe(XPTOUCH_HISTORY_COVER_DL_CANCEL, (lv_msg_subscribe_cb_t)xptouch_history_on_cover_dl_cancel, NULL);
  lv_msg_subscribe(XPTOUCH_THUMBNAILS_HIDE_MODE_CHANGED, (lv_msg_subscribe_cb_t)xptouch_history_on_thumbnails_hide_changed, NULL);
}

extern "C" void xptouch_history_clear_tasks_on_leave_c(void)
{
  xptouch_history_count = 0;
  memset(xptouch_history_tasks, 0, sizeof(xptouch_history_tasks));
  s_history_last_append_after[0] = '\0';
  xptouch_history_cover_clear();
}

void xptouch_history_subscribe_events(void)
{
  xptouch_history_subscribe_events_impl();
}

#else

void xptouch_history_subscribe_events(void)
{
  (void)0;
}

#endif
