#ifndef _XTOUCH_THUMBNAIL_H
#define _XTOUCH_THUMBNAIL_H

#include <stdio.h>
#include <string.h>
#include "ui/ui_msgs.h"
#ifdef __XTOUCH_SCREEN_50__
#include "xtouch/net.h"
#include <SD.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#endif

#define XTOUCH_THUMB_SLOT_MAX 5
#define XTOUCH_THUMB_LGFX_W 150
#define XTOUCH_THUMB_LGFX_H 150
/** DL 開始を遅らせる ms。この間は画面遷移など UI が応答する */
#define XTOUCH_THUMB_FETCH_DELAY_MS 200

static bool xtouch_load_thumb_slot_with_lgfx(int slot, int out_w, int out_h);       /* 実装は LGFX + pngle 内 */
static bool xtouch_load_logo_for_slot_with_lgfx(int slot, int out_w, int out_h);    /* /resource/logo.png 用（同じく pngle 経路） */

#ifndef XTOUCH_HISTORY_COVER_SLOTS
#define XTOUCH_HISTORY_COVER_SLOTS 10
#endif
/** History 画面 行別: SD 上の PNG をデコードして xtouch_history_cover_dsc[slot] に格納。path は "/tmp/history_cover_N.png" 等。 */
bool xtouch_history_cover_load_path(int slot, const char *path);
/** History cover を全スロットクリア（表示をプレースホルダに戻す）。 */
void xtouch_history_cover_clear(void);

/** サムネイル用パスは types.h の xtouch_thumbnail_slot_path[] を UI が参照。本ヘッダではタイマー等のみ。 */
/** サムネイル用タイマーを開始（Printers 画面用）。 */
void xtouch_thumbnail_timer_start(void);
/** サムネイル用タイマーを停止。 */
void xtouch_thumbnail_timer_stop(void);
/** 次回タイマーから全スロットを取得して上書きするようスケジュール（画面表示時に呼ぶ）。 */
void xtouch_thumbnail_schedule_fetch_all(void);

/** task_id に応じて xtouch_thumbnail_slot_path[slot] を更新（push_status 受信後などに呼ぶ）。
 *  ファイルが SD に存在する場合のみ path を設定。未 DL の場合は空にして open エラーを防ぐ。 */
inline void xtouch_thumbnail_update_path_for_slot(int slot)
{
    char path[64];
    getThumbPathForSlot(slot, path, sizeof(path));
    if (slot >= 0 && slot < XTOUCH_THUMB_SLOT_MAX)
    {
        if (SD.exists(path))
            snprintf(xtouch_thumbnail_slot_path[slot], XTOUCH_THUMB_PATH_LEN, "S:%s", path);
        else
            xtouch_thumbnail_slot_path[slot][0] = '\0';
    }
}

/* 以下は C++ のみ（main.cpp が include する想定）。実装は static 変数・コールバックのあとで extern "C" 定義 */
static lv_timer_t *s_thumb_timer = nullptr;
static int s_thumb_next_slot = 0;
static int s_thumb_force_fetch_slot = XTOUCH_THUMB_SLOT_MAX;
static bool s_thumb_exists[XTOUCH_THUMB_SLOT_MAX];
/** キャッシュから再描画を 1 回だけ行ったか（Home/Printers で SD に既にファイルがあるスロット用） */
static bool s_thumb_cache_refresh_done[XTOUCH_THUMB_SLOT_MAX];

#ifdef __XTOUCH_SCREEN_50__
#define XTOUCH_THUMB_DL_QUEUE_LEN 5
#define XTOUCH_THUMB_DL_TASK_STACK_WORDS 8192
struct thumb_dl_item
{
    int slot;
    char url[1024];
    char path[64];
};
static QueueHandle_t s_thumb_download_queue = nullptr;
static QueueHandle_t s_thumb_done_queue = nullptr;
#endif

/** types（bambuStatus / otherPrinters）だけを見てスロットに URL または task_id があるか判定。UI は呼ばない。 */
static bool thumbnail_slot_has_url_or_task(int slot, int cloud_logged_in)
{
    if (slot == 0)
    {
        if (bambuStatus.image_url[0])
            return true;
        if (cloud_logged_in && bambuStatus.task_id[0] && strcmp(bambuStatus.task_id, "0") != 0)
            return true;
        return false;
    }
    int idx = slot - 1;
    if (idx >= xtouch_other_printer_count || !otherPrinters[idx].valid)
        return false;
    if (otherPrinters[idx].image_url[0])
        return true;
    if (cloud_logged_in && otherPrinters[idx].task_id[0] && strcmp(otherPrinters[idx].task_id, "0") != 0)
        return true;
    return false;
}

static bool thumbnail_needs_download(int slot)
{
    if (slot < 0 || slot >= XTOUCH_THUMB_SLOT_MAX)
        return false;
    if (!thumbnail_slot_has_url_or_task(slot, cloud.loggedIn ? 1 : 0))
        return false;
    char path[64];
    getThumbPathForSlot(slot, path, sizeof(path));
    if (!path[0])
        return false;
    return !SD.exists(path);
}

#ifdef __XTOUCH_SCREEN_50__
/* ワーカータスク: DL キューから 1 件取り、downloadFileToSDCard のみ実行して done に投げる（メインをブロックしない） */
static void thumb_dl_task(void *pv)
{
    (void)pv;
    struct thumb_dl_item item;
    for (;;)
    {
        if (xQueueReceive(s_thumb_download_queue, &item, portMAX_DELAY) != pdTRUE)
            continue;
        if (item.slot < 0 || item.slot >= XTOUCH_THUMB_SLOT_MAX)
            continue;
        if (downloadFileToSDCard(item.url, item.path) != 0)
        {
            ConsoleDebug.print(F("[xPTouch][THUMB] dl_ng slot="));
            ConsoleDebug.println(item.slot);
            /* ダウンロード失敗時はこのスロットの URL / task_id をクリアし、以後はロゴにフォールバックさせる（リトライしない）。 */
            if (item.slot == 0)
            {
                bambuStatus.image_url[0] = '\0';
                bambuStatus.task_id[0] = '\0';
            }
            else
            {
                int idx = item.slot - 1;
                if (idx >= 0 && idx < xtouch_other_printer_count)
                {
                    otherPrinters[idx].image_url[0] = '\0';
                    otherPrinters[idx].task_id[0] = '\0';
                }
            }
            xQueueSend(s_thumb_done_queue, &item.slot, 0);
        }
        else
        {
            ConsoleDebug.print(F("[xPTouch][THUMB] dl_ok slot="));
            ConsoleDebug.print(item.slot);
            ConsoleDebug.print(F(" path="));
            ConsoleDebug.println(item.path);
            /* 成功時も UI 更新のため done キューに流す。
             * さらに、成功後は image_url を消しておき、今後このスロットでは SD 上の PNG のみを使うようにする。 */
            if (item.slot == 0)
            {
                bambuStatus.image_url[0] = '\0';
            }
            else
            {
                int idx = item.slot - 1;
                if (idx >= 0 && idx < xtouch_other_printer_count)
                {
                    otherPrinters[idx].image_url[0] = '\0';
                }
            }
            xQueueSend(s_thumb_done_queue, &item.slot, 0);
        }
    }
}

/* 200ms 遅延後に DL をキューに投入するだけ（実際の DL はワーカーが実行）。 */
static void thumbnail_do_slot_cb(lv_timer_t *t)
{
    int s = (int)(intptr_t)t->user_data;
    int idx = xTouchConfig.currentScreenIndex;
    if (idx != 6 && idx != 0)
        return;
    if (s < 0 || s >= XTOUCH_THUMB_SLOT_MAX || !s_thumb_download_queue || !s_thumb_done_queue)
        return;
    if (!thumbnail_needs_download(s))
        return;
    struct thumb_dl_item item;
    item.slot = s;
    if (!getThumbnailUrlAndPathForSlot(s, item.url, sizeof(item.url), item.path, sizeof(item.path)))
        return;
    if (xQueueSend(s_thumb_download_queue, &item, 0) != pdTRUE)
        return;
}
#else
static void thumbnail_do_slot_cb(lv_timer_t *t) { (void)t; }
#endif

#ifdef __XTOUCH_SCREEN_50__
/** 1 回だけ実行: サムネ更新メッセージを送る。タイマーコールバックの外で送って描画を確実に反映させる。 */
static void thumbnail_send_update_one_shot_cb(lv_timer_t *t)
{
    int slot_plus_one = (int)(intptr_t)t->user_data;
    if (slot_plus_one >= 1 && slot_plus_one <= XTOUCH_THUMB_SLOT_MAX)
    {
        ConsoleDebug.print(F("[xPTouch][THUMB] send_msg slot+1="));
        ConsoleDebug.println(slot_plus_one);
        lv_msg_send(XTOUCH_ON_OTHER_PRINTER_UPDATE, (void *)(intptr_t)slot_plus_one);
    }
}
#endif

static void thumbnail_timer_cb(lv_timer_t *t)
{
    (void)t;
#ifdef __XTOUCH_SCREEN_50__
    if (s_thumb_done_queue)
    {
        int slot;
        while (xQueueReceive(s_thumb_done_queue, &slot, 0) == pdTRUE)
        {
            if (slot >= 0 && slot < XTOUCH_THUMB_SLOT_MAX)
            {
                ConsoleDebug.print(F("[xPTouch][THUMB] done_pop slot="));
                ConsoleDebug.print(slot);
                ConsoleDebug.println(F(" → load"));
                xtouch_thumbnail_update_path_for_slot(slot);
                bool ok = xtouch_load_thumb_slot_with_lgfx(slot, XTOUCH_THUMB_LGFX_W, XTOUCH_THUMB_LGFX_H);
                if (!ok)
                {
                    /* DL 失敗・デコード失敗時はロゴを表示して終了（リトライしない）。 */
                    ok = xtouch_load_logo_for_slot_with_lgfx(slot, XTOUCH_THUMB_LGFX_W, XTOUCH_THUMB_LGFX_H);
                }
                s_thumb_exists[slot] = true;
                if (ok)
                {
                    ConsoleDebug.print(F("[xPTouch][THUMB] load_ok slot="));
                    ConsoleDebug.println(slot);
                    /* 同一コールバック内で lv_msg_send すると描画が追いつかないことがあるため、1 回だけのタイマーで次サイクルに送る */
                    lv_timer_t *once = lv_timer_create(thumbnail_send_update_one_shot_cb, 0, (void *)(intptr_t)(slot + 1));
                    lv_timer_set_repeat_count(once, 1);
                }
            }
        }
    }
#endif
    /* SD が無いときは、サムネイル（DL／ロゴ）処理を一切行わない。HTTP や SD エラーを防ぐため。 */
    if (SD.cardType() == CARD_NONE)
        return;

    int idx = xTouchConfig.currentScreenIndex;
    /* Home(0): URL/TaskID が無いスロットにはロゴを一度だけロード。
     * URL/TaskID があるスロットは 1 回だけ DL を試み、失敗したらロゴにフォールバック（リトライしない）。 */
    if (idx == 0)
    {
        for (int s = 0; s < XTOUCH_THUMB_SLOT_MAX; s++)
        {
            if (!s_thumb_exists[s] && !thumbnail_slot_has_url_or_task(s, cloud.loggedIn ? 1 : 0))
            {
                /* 起動直後など URL/TaskID がまだ無い場合でも、既に SD に
                 * /tmp/{task_id}.png が残っていればそれを優先して表示する。
                 * どちらも無いスロットだけロゴにフォールバック。 */
                char path[64];
                getThumbPathForSlot(s, path, sizeof(path));
                bool ok = false;
                if (path[0] && SD.exists(path))
                {
                    ConsoleDebug.print(F("[xPTouch][THUMB] home cached slot="));
                    ConsoleDebug.print(s);
                    ConsoleDebug.print(F(" path="));
                    ConsoleDebug.println(path);
                    ok = xtouch_load_thumb_slot_with_lgfx(s, XTOUCH_THUMB_LGFX_W, XTOUCH_THUMB_LGFX_H);
                }
                if (!ok)
                {
                    ConsoleDebug.print(F("[xPTouch][THUMB] home logo slot="));
                    ConsoleDebug.println(s);
                    ok = xtouch_load_logo_for_slot_with_lgfx(s, XTOUCH_THUMB_LGFX_W, XTOUCH_THUMB_LGFX_H);
                }
                if (ok)
                {
                    s_thumb_exists[s] = true;
                    lv_msg_send(XTOUCH_ON_OTHER_PRINTER_UPDATE, (void *)(intptr_t)(s + 1));
                }
            }
        }
        /* URL/TaskID はあるが DL 不要（SD に既にファイルがある）スロットは、そのファイルで 1 回だけ再描画する。 */
        for (int s = 0; s < XTOUCH_THUMB_SLOT_MAX; s++)
        {
            if (s_thumb_cache_refresh_done[s])
                continue;
            if (!thumbnail_slot_has_url_or_task(s, cloud.loggedIn ? 1 : 0))
                continue;
            char path[64];
            getThumbPathForSlot(s, path, sizeof(path));
            if (!path[0] || !SD.exists(path))
                continue;
            xtouch_thumbnail_update_path_for_slot(s);
            if (xtouch_load_thumb_slot_with_lgfx(s, XTOUCH_THUMB_LGFX_W, XTOUCH_THUMB_LGFX_H))
            {
                s_thumb_cache_refresh_done[s] = true;
                s_thumb_exists[s] = true;
                lv_msg_send(XTOUCH_ON_OTHER_PRINTER_UPDATE, (void *)(intptr_t)(s + 1));
            }
        }
        static uint32_t s_home_thumb_last = 0;
        uint32_t now = lv_tick_get();
        if (now - s_home_thumb_last >= 500u)
        {
            s_home_thumb_last = now;
            for (int i = 0; i < XTOUCH_THUMB_SLOT_MAX; i++)
            {
                int s = (s_thumb_next_slot + i) % XTOUCH_THUMB_SLOT_MAX;
                if (thumbnail_needs_download(s))
                {
                    lv_timer_t *once = lv_timer_create(thumbnail_do_slot_cb, XTOUCH_THUMB_FETCH_DELAY_MS, (void *)(intptr_t)s);
                    lv_timer_set_repeat_count(once, 1);
                    s_thumb_next_slot = (s + 1) % XTOUCH_THUMB_SLOT_MAX;
                    break;
                }
            }
        }
        return;
    }
    if (idx != 6)
        return;

    /* Printers画面でも、URL/TaskID が無いスロットにはデフォルトロゴを一度だけロードしておく。 */
    for (int s = 0; s < XTOUCH_THUMB_SLOT_MAX; s++)
    {
        if (!s_thumb_exists[s] && !thumbnail_slot_has_url_or_task(s, cloud.loggedIn ? 1 : 0))
        {
            /* Home と同様、まずは SD 上の /tmp/{task_id}.png を優先表示し、
             * 何も無いスロットだけロゴにフォールバックする。 */
            char path[64];
            getThumbPathForSlot(s, path, sizeof(path));
            bool ok = false;
            if (path[0] && SD.exists(path))
            {
                ConsoleDebug.print(F("[xPTouch][THUMB] printers cached slot="));
                ConsoleDebug.print(s);
                ConsoleDebug.print(F(" path="));
                ConsoleDebug.println(path);
                ok = xtouch_load_thumb_slot_with_lgfx(s, XTOUCH_THUMB_LGFX_W, XTOUCH_THUMB_LGFX_H);
            }
            if (!ok)
            {
                ConsoleDebug.print(F("[xPTouch][THUMB] printers logo slot="));
                ConsoleDebug.println(s);
                ok = xtouch_load_logo_for_slot_with_lgfx(s, XTOUCH_THUMB_LGFX_W, XTOUCH_THUMB_LGFX_H);
            }
            if (ok)
            {
                s_thumb_exists[s] = true;
                lv_msg_send(XTOUCH_ON_OTHER_PRINTER_UPDATE, (void *)(intptr_t)(s + 1));
            }
        }
    }
    /* Printers でも URL/TaskID はあるが SD に既にファイルがあるスロットは、そのファイルで 1 回だけ再描画する。 */
    for (int s = 0; s < XTOUCH_THUMB_SLOT_MAX; s++)
    {
        if (s_thumb_cache_refresh_done[s])
            continue;
        if (!thumbnail_slot_has_url_or_task(s, cloud.loggedIn ? 1 : 0))
            continue;
        char path[64];
        getThumbPathForSlot(s, path, sizeof(path));
        if (!path[0] || !SD.exists(path))
            continue;
        xtouch_thumbnail_update_path_for_slot(s);
        if (xtouch_load_thumb_slot_with_lgfx(s, XTOUCH_THUMB_LGFX_W, XTOUCH_THUMB_LGFX_H))
        {
            s_thumb_cache_refresh_done[s] = true;
            s_thumb_exists[s] = true;
            lv_msg_send(XTOUCH_ON_OTHER_PRINTER_UPDATE, (void *)(intptr_t)(s + 1));
        }
    }

    if (s_thumb_force_fetch_slot < XTOUCH_THUMB_SLOT_MAX)
    {
        int s = s_thumb_force_fetch_slot;
        if (!thumbnail_slot_has_url_or_task(s, cloud.loggedIn ? 1 : 0))
            return;
        s_thumb_force_fetch_slot++;
#ifdef XTOUCH_DEBUG
        ConsoleDebug.print(F("[xPTouch][THUMB] force fetch slot="));
        ConsoleDebug.println(s);
#endif
        lv_timer_t *once = lv_timer_create(thumbnail_do_slot_cb, XTOUCH_THUMB_FETCH_DELAY_MS, (void *)(intptr_t)s);
        lv_timer_set_repeat_count(once, 1);
        return;
    }

    for (int i = 0; i < XTOUCH_THUMB_SLOT_MAX; i++)
    {
        int s = (s_thumb_next_slot + i) % XTOUCH_THUMB_SLOT_MAX;
        if (thumbnail_needs_download(s))
        {
#ifdef XTOUCH_DEBUG
            ConsoleDebug.print(F("[xPTouch][THUMB] timer: start slot="));
            ConsoleDebug.println(s);
#endif
            s_thumb_next_slot = (s + 1) % XTOUCH_THUMB_SLOT_MAX;
            lv_timer_t *once = lv_timer_create(thumbnail_do_slot_cb, XTOUCH_THUMB_FETCH_DELAY_MS, (void *)(intptr_t)s);
            lv_timer_set_repeat_count(once, 1);
            break;
        }
    }
}

void xtouch_thumbnail_timer_start(void)
{
    if (s_thumb_timer)
        return;
    ConsoleDebug.println(F("[xPTouch][THUMB] timer_start"));
    /* ディレイほぼなし: 1スロット処理後すぐ次スロットを試行 */
    s_thumb_timer = lv_timer_create(thumbnail_timer_cb, 1, nullptr);
    lv_timer_set_repeat_count(s_thumb_timer, -1);
}

void xtouch_thumbnail_timer_stop(void)
{
    if (s_thumb_timer)
    {
        lv_timer_del(s_thumb_timer);
        s_thumb_timer = nullptr;
    }
    s_thumb_force_fetch_slot = XTOUCH_THUMB_SLOT_MAX;
    for (int i = 0; i < XTOUCH_THUMB_SLOT_MAX; i++)
    {
        s_thumb_exists[i] = false;
        s_thumb_cache_refresh_done[i] = false;
    }
}

void xtouch_thumbnail_schedule_fetch_all(void)
{
    s_thumb_force_fetch_slot = 0;
}

/* UI が送るイベントを xtouch 側で購読（mqtt.h の xtouch_mqtt_subscribe_commands と同じパターン） */
#ifdef __cplusplus
static void xtouch_thumbnail_on_schedule_fetch(lv_msg_t *m, void *user_data)
{
    (void)m;
    (void)user_data;
    xtouch_thumbnail_schedule_fetch_all();
}
static void xtouch_thumbnail_on_timer_start(lv_msg_t *m, void *user_data)
{
    (void)m;
    (void)user_data;
    xtouch_thumbnail_timer_start();
}
static void xtouch_thumbnail_on_timer_stop(lv_msg_t *m, void *user_data)
{
    (void)m;
    (void)user_data;
    xtouch_thumbnail_timer_stop();
}

/** Printers 画面用イベント購読を登録。main の setup で一度呼ぶ */
static void xtouch_thumbnail_subscribe_events(void)
{
#ifdef __XTOUCH_SCREEN_50__
    if (s_thumb_download_queue == nullptr)
    {
        s_thumb_download_queue = xQueueCreate(XTOUCH_THUMB_DL_QUEUE_LEN, sizeof(struct thumb_dl_item));
        s_thumb_done_queue = xQueueCreate(XTOUCH_THUMB_DL_QUEUE_LEN, sizeof(int));
        if (s_thumb_download_queue && s_thumb_done_queue)
            xTaskCreate(thumb_dl_task, "thumb_dl", XTOUCH_THUMB_DL_TASK_STACK_WORDS, NULL, 1, NULL);
    }
#endif
    lv_msg_subscribe(XTOUCH_PRINTERS_SCHEDULE_THUMB_FETCH, (lv_msg_subscribe_cb_t)xtouch_thumbnail_on_schedule_fetch, NULL);
    lv_msg_subscribe(XTOUCH_PRINTERS_THUMB_TIMER_START, (lv_msg_subscribe_cb_t)xtouch_thumbnail_on_timer_start, NULL);
    lv_msg_subscribe(XTOUCH_PRINTERS_THUMB_TIMER_STOP, (lv_msg_subscribe_cb_t)xtouch_thumbnail_on_timer_stop, NULL);
}
#endif

/* ========== LGFX で PNG をデコードして LVGL の lv_img 用バッファに出す ========== */
#include <esp32-hal-psram.h>
#include <LovyanGFX.h>
#include <SD.h>
#include "lgfx/utility/lgfx_pngle.h"

static lv_color_t *g_lgfx_thumb_buf = nullptr;
static int g_lgfx_thumb_buf_w = 0;
static int g_lgfx_thumb_buf_h = 0;
lv_img_dsc_t g_lgfx_thumb_dsc;

/* Printers 画面用: スロット毎のバッファと descriptor。xtouch_thumbnail_slot_dsc は types.h で extern（C から参照）。 */
static lv_color_t *g_lgfx_thumb_buf_slot[XTOUCH_THUMB_SLOT_MAX] = { nullptr };
static lv_img_dsc_t g_lgfx_thumb_dsc_slot[XTOUCH_THUMB_SLOT_MAX];
#define XTOUCH_HISTORY_COVER_W 150
#define XTOUCH_HISTORY_COVER_H 150

/* History 画面用: 行別 cover 画像用バッファと descriptor（最大 XTOUCH_HISTORY_COVER_SLOTS 件）。 */
static lv_color_t *g_history_cover_buf[XTOUCH_HISTORY_COVER_SLOTS] = { nullptr };
static lv_img_dsc_t g_history_cover_dsc[XTOUCH_HISTORY_COVER_SLOTS];

extern "C" {
void *xtouch_thumbnail_slot_dsc[XTOUCH_THUMB_SLOT_MAX] = { nullptr, nullptr, nullptr, nullptr, nullptr };
void *xtouch_history_cover_dsc[XTOUCH_HISTORY_COVER_SLOTS] = {};
}

struct xtouch_pngle_ctx_t
{
    File *file;
    lv_color_t *buf;
    int width;
    int height;
    uint32_t img_width;
    uint32_t img_height;
};

static uint32_t xtouch_pngle_read_cb(void *user_data, uint8_t *buf, uint32_t len)
{
    xtouch_pngle_ctx_t *ctx = (xtouch_pngle_ctx_t *)user_data;
    return (uint32_t)ctx->file->read(buf, len);
}

static void xtouch_pngle_draw_cb(void *user_data, uint32_t x, uint32_t y, uint_fast8_t div_x, size_t len, const uint8_t *argb)
{
    xtouch_pngle_ctx_t *ctx = (xtouch_pngle_ctx_t *)user_data;
    if (div_x != 1)
        return;
    if (ctx->img_width == 0 || ctx->img_height == 0)
        return;
    uint32_t iw = ctx->img_width;
    uint32_t ih = ctx->img_height;
    int ow = ctx->width;
    int oh = ctx->height;

    for (size_t i = 0; i < len; ++i, argb += 4)
    {
        uint32_t sx = x + (uint32_t)i;
        uint32_t sy = y;
        uint32_t dx_min = (sx * (uint32_t)ow) / iw;
        uint32_t dx_max = ((sx + 1) * (uint32_t)ow + iw - 1) / iw;
        uint32_t dy_min = (sy * (uint32_t)oh) / ih;
        uint32_t dy_max = ((sy + 1) * (uint32_t)oh + ih - 1) / ih;
        if (dx_max > (uint32_t)ow)
            dx_max = (uint32_t)ow;
        if (dy_max > (uint32_t)oh)
            dy_max = (uint32_t)oh;
        if (dx_min >= dx_max)
            dx_max = dx_min + 1;
        if (dy_min >= dy_max)
            dy_max = dy_min + 1;

        uint16_t rgb565 = (uint16_t)(((argb[1] & 0xF8) << 8) | ((argb[2] & 0xFC) << 3) | (argb[3] >> 3));
        for (uint32_t dy = dy_min; dy < dy_max; ++dy)
        {
            lv_color_t *row = &ctx->buf[dy * ow];
            for (uint32_t dx = dx_min; dx < dx_max; ++dx)
                row[dx].full = rgb565;
        }
    }
}

bool xtouch_load_thumb_with_lgfx(const char *path, int out_w, int out_h)
{
    if (out_w <= 0 || out_h <= 0)
        return false;

    size_t px_count = (size_t)out_w * (size_t)out_h;
    if (!g_lgfx_thumb_buf || g_lgfx_thumb_buf_w != out_w || g_lgfx_thumb_buf_h != out_h)
    {
        if (g_lgfx_thumb_buf)
            free(g_lgfx_thumb_buf);
        g_lgfx_thumb_buf = (lv_color_t *)ps_malloc(sizeof(lv_color_t) * px_count);
        if (!g_lgfx_thumb_buf)
        {
            Serial.println("[thumb] ps_malloc failed");
            g_lgfx_thumb_buf_w = g_lgfx_thumb_buf_h = 0;
            return false;
        }
        g_lgfx_thumb_buf_w = out_w;
        g_lgfx_thumb_buf_h = out_h;
    }
    memset(g_lgfx_thumb_buf, 0, sizeof(lv_color_t) * px_count);

    g_lgfx_thumb_dsc.header.always_zero = 0;
    g_lgfx_thumb_dsc.header.w = (lv_coord_t)out_w;
    g_lgfx_thumb_dsc.header.h = (lv_coord_t)out_h;
    g_lgfx_thumb_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
    g_lgfx_thumb_dsc.data = (const uint8_t *)g_lgfx_thumb_buf;
    g_lgfx_thumb_dsc.data_size = sizeof(lv_color_t) * px_count;

    File file = SD.open(path, "r");
    if (!file)
    {
        Serial.println("[thumb] SD.open failed");
        return false;
    }

    pngle_t *pngle = lgfx_pngle_new();
    if (!pngle)
    {
        file.close();
        Serial.println("[thumb] lgfx_pngle_new failed");
        return false;
    }

    xtouch_pngle_ctx_t pngle_ctx;
    pngle_ctx.file = &file;
    pngle_ctx.buf = g_lgfx_thumb_buf;
    pngle_ctx.width = out_w;
    pngle_ctx.height = out_h;
    pngle_ctx.img_width = 0;
    pngle_ctx.img_height = 0;

    if (lgfx_pngle_prepare(pngle, xtouch_pngle_read_cb, &pngle_ctx) < 0)
    {
        lgfx_pngle_destroy(pngle);
        file.close();
        Serial.println("[thumb] lgfx_pngle_prepare failed");
        return false;
    }

    pngle_ctx.img_width = lgfx_pngle_get_width(pngle);
    pngle_ctx.img_height = lgfx_pngle_get_height(pngle);
    if (pngle_ctx.img_width == 0 || pngle_ctx.img_height == 0)
    {
        lgfx_pngle_destroy(pngle);
        file.close();
        return false;
    }

    int res = lgfx_pngle_decomp(pngle, xtouch_pngle_draw_cb);
    file.close();
    lgfx_pngle_destroy(pngle);

    if (res < 0)
    {
        Serial.println("[thumb] lgfx_pngle_decomp failed");
        return false;
    }
    return true;
}

lv_img_dsc_t *xtouch_thumb_get_lgfx_dsc(void)
{
    return &g_lgfx_thumb_dsc;
}

/** 指定スロットの SD 上の PNG を LGFX でデコードし、Printers 画面用スロットバッファに格納。パスは task_id ベースの /tmp/{task_id}.png。 */
static bool xtouch_load_thumb_slot_with_lgfx(int slot, int out_w, int out_h)
{
    if (slot < 0 || slot >= XTOUCH_THUMB_SLOT_MAX || out_w <= 0 || out_h <= 0)
        return false;
    char path[64];
    getThumbPathForSlot(slot, path, sizeof(path));
    if (!path[0])
        return false;
    size_t px_count = (size_t)out_w * (size_t)out_h;
    lv_color_t *buf = g_lgfx_thumb_buf_slot[slot];
    if (!buf || g_lgfx_thumb_dsc_slot[slot].header.w != out_w || g_lgfx_thumb_dsc_slot[slot].header.h != out_h)
    {
        if (buf)
            free(buf);
        buf = (lv_color_t *)ps_malloc(sizeof(lv_color_t) * px_count);
        if (!buf)
        {
            xtouch_thumbnail_slot_dsc[slot] = nullptr;
            return false;
        }
        g_lgfx_thumb_buf_slot[slot] = buf;
    }
    memset(buf, 0, sizeof(lv_color_t) * px_count);
    g_lgfx_thumb_dsc_slot[slot].header.always_zero = 0;
    g_lgfx_thumb_dsc_slot[slot].header.w = (lv_coord_t)out_w;
    g_lgfx_thumb_dsc_slot[slot].header.h = (lv_coord_t)out_h;
    g_lgfx_thumb_dsc_slot[slot].header.cf = LV_IMG_CF_TRUE_COLOR;
    g_lgfx_thumb_dsc_slot[slot].data = (const uint8_t *)buf;
    g_lgfx_thumb_dsc_slot[slot].data_size = sizeof(lv_color_t) * px_count;

    xtouch_pngle_ctx_t pngle_ctx;
    File file = SD.open(path, "r");
    if (!file)
    {
        xtouch_thumbnail_slot_dsc[slot] = nullptr;
        return false;
    }
    pngle_t *pngle = lgfx_pngle_new();
    if (!pngle)
    {
        file.close();
        xtouch_thumbnail_slot_dsc[slot] = nullptr;
        return false;
    }
    pngle_ctx.file = &file;
    pngle_ctx.buf = buf;
    pngle_ctx.width = out_w;
    pngle_ctx.height = out_h;
    pngle_ctx.img_width = 0;
    pngle_ctx.img_height = 0;
    if (lgfx_pngle_prepare(pngle, xtouch_pngle_read_cb, &pngle_ctx) < 0)
    {
        lgfx_pngle_destroy(pngle);
        file.close();
        xtouch_thumbnail_slot_dsc[slot] = nullptr;
        return false;
    }
    pngle_ctx.img_width = lgfx_pngle_get_width(pngle);
    pngle_ctx.img_height = lgfx_pngle_get_height(pngle);
    if (pngle_ctx.img_width == 0 || pngle_ctx.img_height == 0)
    {
        lgfx_pngle_destroy(pngle);
        file.close();
        xtouch_thumbnail_slot_dsc[slot] = nullptr;
        return false;
    }
    int res = lgfx_pngle_decomp(pngle, xtouch_pngle_draw_cb);
    file.close();
    lgfx_pngle_destroy(pngle);
    if (res < 0)
    {
        xtouch_thumbnail_slot_dsc[slot] = nullptr;
        return false;
    }
    xtouch_thumbnail_slot_dsc[slot] = (void *)&g_lgfx_thumb_dsc_slot[slot];
    return true;
}

void xtouch_history_cover_clear(void)
{
    for (int i = 0; i < XTOUCH_HISTORY_COVER_SLOTS; i++)
        xtouch_history_cover_dsc[i] = nullptr;
}

bool xtouch_history_cover_load_path(int slot, const char *path)
{
    if (slot < 0 || slot >= XTOUCH_HISTORY_COVER_SLOTS || !path || !path[0] || SD.cardType() == CARD_NONE)
    {
        if (slot >= 0 && slot < XTOUCH_HISTORY_COVER_SLOTS)
            xtouch_history_cover_dsc[slot] = nullptr;
        return false;
    }
    const int out_w = XTOUCH_HISTORY_COVER_W;
    const int out_h = XTOUCH_HISTORY_COVER_H;
    size_t px_count = (size_t)out_w * (size_t)out_h;
    lv_color_t *buf = g_history_cover_buf[slot];
    if (!buf || g_history_cover_dsc[slot].header.w != (lv_coord_t)out_w || g_history_cover_dsc[slot].header.h != (lv_coord_t)out_h)
    {
        if (buf)
            free(buf);
        buf = (lv_color_t *)ps_malloc(sizeof(lv_color_t) * px_count);
        if (!buf)
        {
            xtouch_history_cover_dsc[slot] = nullptr;
            return false;
        }
        g_history_cover_buf[slot] = buf;
    }
    memset(buf, 0, sizeof(lv_color_t) * px_count);
    g_history_cover_dsc[slot].header.always_zero = 0;
    g_history_cover_dsc[slot].header.w = (lv_coord_t)out_w;
    g_history_cover_dsc[slot].header.h = (lv_coord_t)out_h;
    g_history_cover_dsc[slot].header.cf = LV_IMG_CF_TRUE_COLOR;
    g_history_cover_dsc[slot].data = (const uint8_t *)buf;
    g_history_cover_dsc[slot].data_size = sizeof(lv_color_t) * px_count;

    xtouch_pngle_ctx_t pngle_ctx;
    File file = SD.open(path, "r");
    if (!file)
    {
        xtouch_history_cover_dsc[slot] = nullptr;
        return false;
    }
    pngle_t *pngle = lgfx_pngle_new();
    if (!pngle)
    {
        file.close();
        xtouch_history_cover_dsc[slot] = nullptr;
        return false;
    }
    pngle_ctx.file = &file;
    pngle_ctx.buf = buf;
    pngle_ctx.width = out_w;
    pngle_ctx.height = out_h;
    pngle_ctx.img_width = 0;
    pngle_ctx.img_height = 0;
    if (lgfx_pngle_prepare(pngle, xtouch_pngle_read_cb, &pngle_ctx) < 0)
    {
        lgfx_pngle_destroy(pngle);
        file.close();
        xtouch_history_cover_dsc[slot] = nullptr;
        return false;
    }
    pngle_ctx.img_width = lgfx_pngle_get_width(pngle);
    pngle_ctx.img_height = lgfx_pngle_get_height(pngle);
    if (pngle_ctx.img_width == 0 || pngle_ctx.img_height == 0)
    {
        lgfx_pngle_destroy(pngle);
        file.close();
        xtouch_history_cover_dsc[slot] = nullptr;
        return false;
    }
    int res = lgfx_pngle_decomp(pngle, xtouch_pngle_draw_cb);
    file.close();
    lgfx_pngle_destroy(pngle);
    if (res < 0)
    {
        xtouch_history_cover_dsc[slot] = nullptr;
        return false;
    }
    xtouch_history_cover_dsc[slot] = (void *)&g_history_cover_dsc[slot];
    return true;
}

/** デフォルトロゴ (/resource/logo.png) を LGFX + pngle でデコードし、指定スロット用バッファに格納。 */
static bool xtouch_load_logo_for_slot_with_lgfx(int slot, int out_w, int out_h)
{
    if (slot < 0 || slot >= XTOUCH_THUMB_SLOT_MAX || out_w <= 0 || out_h <= 0)
        return false;
    /* SD 未挿入時は何もしない（無限に open を繰り返さないようにする） */
    if (SD.cardType() == CARD_NONE)
    {
        ConsoleDebug.println(F("[xPTouch][THUMB] logo: SD not present"));
        return false;
    }
    size_t px_count = (size_t)out_w * (size_t)out_h;
    lv_color_t *buf = g_lgfx_thumb_buf_slot[slot];
    if (!buf || g_lgfx_thumb_dsc_slot[slot].header.w != out_w || g_lgfx_thumb_dsc_slot[slot].header.h != out_h)
    {
        if (buf)
            free(buf);
        buf = (lv_color_t *)ps_malloc(sizeof(lv_color_t) * px_count);
        if (!buf)
        {
            xtouch_thumbnail_slot_dsc[slot] = nullptr;
            return false;
        }
        g_lgfx_thumb_buf_slot[slot] = buf;
    }
    memset(buf, 0, sizeof(lv_color_t) * px_count);
    g_lgfx_thumb_dsc_slot[slot].header.always_zero = 0;
    g_lgfx_thumb_dsc_slot[slot].header.w = (lv_coord_t)out_w;
    g_lgfx_thumb_dsc_slot[slot].header.h = (lv_coord_t)out_h;
    g_lgfx_thumb_dsc_slot[slot].header.cf = LV_IMG_CF_TRUE_COLOR;
    g_lgfx_thumb_dsc_slot[slot].data = (const uint8_t *)buf;
    g_lgfx_thumb_dsc_slot[slot].data_size = sizeof(lv_color_t) * px_count;

    const char *path = "/resource/logo.png";
    ConsoleDebug.print(F("[xPTouch][THUMB] logo: try slot="));
    ConsoleDebug.print(slot);
    ConsoleDebug.print(F(" path="));
    ConsoleDebug.println(path);
    xtouch_pngle_ctx_t pngle_ctx;
    File file = SD.open(path, "r");
    if (!file)
    {
        ConsoleDebug.print(F("[xPTouch][THUMB] logo: SD.open failed path="));
        ConsoleDebug.println(path);
        xtouch_thumbnail_slot_dsc[slot] = nullptr;
        return false;
    }
    pngle_t *pngle = lgfx_pngle_new();
    if (!pngle)
    {
        file.close();
        ConsoleDebug.println(F("[xPTouch][THUMB] logo: lgfx_pngle_new failed"));
        xtouch_thumbnail_slot_dsc[slot] = nullptr;
        return false;
    }
    pngle_ctx.file = &file;
    pngle_ctx.buf = buf;
    pngle_ctx.width = out_w;
    pngle_ctx.height = out_h;
    pngle_ctx.img_width = 0;
    pngle_ctx.img_height = 0;
    if (lgfx_pngle_prepare(pngle, xtouch_pngle_read_cb, &pngle_ctx) < 0)
    {
        lgfx_pngle_destroy(pngle);
        file.close();
        ConsoleDebug.println(F("[xPTouch][THUMB] logo: lgfx_pngle_prepare failed"));
        xtouch_thumbnail_slot_dsc[slot] = nullptr;
        return false;
    }
    pngle_ctx.img_width = lgfx_pngle_get_width(pngle);
    pngle_ctx.img_height = lgfx_pngle_get_height(pngle);
    if (pngle_ctx.img_width == 0 || pngle_ctx.img_height == 0)
    {
        lgfx_pngle_destroy(pngle);
        file.close();
        ConsoleDebug.println(F("[xPTouch][THUMB] logo: invalid size"));
        xtouch_thumbnail_slot_dsc[slot] = nullptr;
        return false;
    }
    int res = lgfx_pngle_decomp(pngle, xtouch_pngle_draw_cb);
    file.close();
    lgfx_pngle_destroy(pngle);
    if (res < 0)
    {
        ConsoleDebug.println(F("[xPTouch][THUMB] logo: lgfx_pngle_decomp failed"));
        xtouch_thumbnail_slot_dsc[slot] = nullptr;
        return false;
    }
    ConsoleDebug.print(F("[xPTouch][THUMB] logo: success slot="));
    ConsoleDebug.println(slot);
    xtouch_thumbnail_slot_dsc[slot] = (void *)&g_lgfx_thumb_dsc_slot[slot];
    return true;
}

#endif /* _XTOUCH_THUMBNAIL_H */
