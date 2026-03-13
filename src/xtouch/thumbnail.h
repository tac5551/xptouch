#ifndef _XTOUCH_THUMBNAIL_H
#define _XTOUCH_THUMBNAIL_H

#include <stdio.h>
#include <string.h>
#include "ui/ui_msgs.h"
#ifdef __XTOUCH_SCREEN_50__
#include "xtouch/net.h"
#include <SD.h>
#endif

#define XTOUCH_THUMB_PATH_PREFIX "/tmp/pthumb_"
#define XTOUCH_THUMB_PATH_SUFFIX ".png"
#define XTOUCH_THUMB_SLOT_MAX 5
#define XTOUCH_THUMB_LGFX_W 150
#define XTOUCH_THUMB_LGFX_H 150
/** DL 開始を遅らせる ms。この間は画面遷移など UI が応答する */
#define XTOUCH_THUMB_FETCH_DELAY_MS 200

static bool xtouch_load_thumb_slot_with_lgfx(int slot, int out_w, int out_h);       /* 実装は LGFX ブロック内 */
static bool xtouch_load_logo_for_slot_with_lgfx(int slot, int out_w, int out_h);    /* /resource/logo.png 用 */

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
    if (!thumbnail_slot_has_url_or_task(slot, cloud.loggedIn ? 1 : 0))
        return false;
    char path[64];
    getThumbPathForSlot(slot, path, sizeof(path));
    bool exists = SD.exists(path);
    return !exists;
}

/* 200ms 遅延後に 1 スロットだけ DL→LGFX→通知。ブロックするのでメインタイマーでは呼ばずワンショットで実行 */
static void thumbnail_do_slot_cb(lv_timer_t *t)
{
    int s = (int)(intptr_t)t->user_data;
    int idx = xTouchConfig.currentScreenIndex;
    if (idx != 6 && idx != 0)
        return;
    if (downloadThumbnailForSlot(s) && s < XTOUCH_THUMB_SLOT_MAX)
    {
        s_thumb_exists[s] = true;
        xtouch_load_thumb_slot_with_lgfx(s, XTOUCH_THUMB_LGFX_W, XTOUCH_THUMB_LGFX_H);
        lv_msg_send(XTOUCH_ON_OTHER_PRINTER_UPDATE, (void *)(intptr_t)(s + 1));
    }
}

static void thumbnail_timer_cb(lv_timer_t *t)
{
    (void)t;
    /* SD が無いときは、サムネイル（DL／ロゴ）処理を一切行わない。HTTP や SD エラーを防ぐため。 */
    if (SD.cardType() == CARD_NONE)
        return;

    int idx = xTouchConfig.currentScreenIndex;
    /* Home(0): 全スロットを順番にキャッシュ（Printers へ遷移時にすぐ表示できるよう）。
     * 起動直後印刷中の場合、SCHEDULE_THUMB_FETCH で force_fetch_slot=0 が設定されるのでスロット0を優先取得。 */
    if (idx == 0)
    {
        /* サムネイルURLもTaskIDも無いスロットには、デフォルトロゴを一度だけロードしておく（Home画面Idle用）。 */
        for (int s = 0; s < XTOUCH_THUMB_SLOT_MAX; s++)
        {
            if (!s_thumb_exists[s] && !thumbnail_slot_has_url_or_task(s, cloud.loggedIn ? 1 : 0))
            {
                ConsoleDebug.print(F("[xPTouch][THUMB] home logo slot="));
                ConsoleDebug.println(s);
                bool ok = xtouch_load_logo_for_slot_with_lgfx(s, XTOUCH_THUMB_LGFX_W, XTOUCH_THUMB_LGFX_H);
                s_thumb_exists[s] = true; /* 成功/失敗にかかわらず一度だけ試す（SD 連続アクセス防止） */
                if (ok)
                    lv_msg_send(XTOUCH_ON_OTHER_PRINTER_UPDATE, (void *)(intptr_t)(s + 1));
            }
        }
        if (s_thumb_force_fetch_slot < XTOUCH_THUMB_SLOT_MAX)
        {
            int s = s_thumb_force_fetch_slot;
            if (thumbnail_slot_has_url_or_task(s, cloud.loggedIn ? 1 : 0) && thumbnail_needs_download(s))
            {
                s_thumb_force_fetch_slot++;
                lv_timer_t *once = lv_timer_create(thumbnail_do_slot_cb, XTOUCH_THUMB_FETCH_DELAY_MS, (void *)(intptr_t)s);
                lv_timer_set_repeat_count(once, 1);
                return;
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
                if (thumbnail_slot_has_url_or_task(s, cloud.loggedIn ? 1 : 0) && thumbnail_needs_download(s))
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
            ConsoleDebug.print(F("[xPTouch][THUMB] printers logo slot="));
            ConsoleDebug.println(s);
            bool ok = xtouch_load_logo_for_slot_with_lgfx(s, XTOUCH_THUMB_LGFX_W, XTOUCH_THUMB_LGFX_H);
            s_thumb_exists[s] = true; /* 成功/失敗にかかわらず一度だけ試す（SD 連続アクセス防止） */
            if (ok)
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
        s_thumb_exists[i] = false;
}

/** 現在の一覧（スロット 0..4 の task_id または pthumb_N）にない /tmp/*.png を削除。一覧取得後に呼ぶ。 */
static void cleanup_thumbnail_files_not_in_list(void)
{
    char path[64];
    char allowed[XTOUCH_THUMB_SLOT_MAX][32];
    int n = 0;
    for (int s = 0; s < XTOUCH_THUMB_SLOT_MAX; s++)
    {
        getThumbPathForSlot(s, path, sizeof(path));
        size_t plen = strlen(path);
        if (plen > 9 && strcmp(path + plen - 4, ".png") == 0)
        {
            size_t stem_len = plen - 5 - 4;
            if (stem_len >= sizeof(allowed[0]))
                stem_len = sizeof(allowed[0]) - 1;
            memcpy(allowed[n], path + 5, stem_len);
            allowed[n][stem_len] = '\0';
            n++;
        }
    }
    File root = SD.open("/tmp");
    if (!root || !root.isDirectory())
        return;
    while (true)
    {
        File entry = root.openNextFile();
        if (!entry)
            break;
        String name = entry.name();
        entry.close();
        if (name.endsWith(".png"))
        {
            String stem = name.substring(0, name.length() - 4);
            bool keep = false;
            for (int i = 0; i < n; i++)
                if (stem.equals(allowed[i]))
                {
                    keep = true;
                    break;
                }
            if (!keep)
            {
                char full[64];
                snprintf(full, sizeof(full), "/tmp/%s", name.c_str());
                SD.remove(full);
            }
        }
    }
    root.close();
}

void xtouch_thumbnail_schedule_fetch_all(void)
{
    cleanup_thumbnail_files_not_in_list();
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
extern "C" {
void *xtouch_thumbnail_slot_dsc[XTOUCH_THUMB_SLOT_MAX] = { nullptr, nullptr, nullptr, nullptr, nullptr };
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

/** 指定スロットの SD 上の PNG を LGFX でデコードし、Printers 画面用スロットバッファに格納。パスは /tmp/pthumb_N.png。 */
static bool xtouch_load_thumb_slot_with_lgfx(int slot, int out_w, int out_h)
{
    if (slot < 0 || slot >= XTOUCH_THUMB_SLOT_MAX || out_w <= 0 || out_h <= 0)
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

    char path[64];
    getThumbPathForSlot(slot, path, sizeof(path));
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

/** デフォルトロゴ (/resource/logo.png) を LGFX の PNG デコーダ経由で読み込み、指定スロット用バッファにコピー。 */
static bool xtouch_load_logo_for_slot_with_lgfx(int slot, int out_w, int out_h)
{
    if (slot < 0 || slot >= XTOUCH_THUMB_SLOT_MAX || out_w <= 0 || out_h <= 0)
        return false;
    /* SD 未挿入時は何もしない（無限に open を繰り返さないようにする） */
    if (SD.cardType() == CARD_NONE)
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

    /* LGFX のスプライトに logo.png を描画し、そのバッファを LVGL バッファにコピーする */
    extern LGFX tft;
    LGFX_Sprite spr(&tft);
    spr.setColorDepth(16); /* RGB565 */
    if (!spr.createSprite(out_w, out_h))
    {
        xtouch_thumbnail_slot_dsc[slot] = nullptr;
        return false;
    }
    spr.fillScreen(0x0000);
    spr.drawPngFile(SD, "/resource/logo.png", 0, 0);

    void *spr_buf = spr.getBuffer();
    if (!spr_buf)
    {
        spr.deleteSprite();
        xtouch_thumbnail_slot_dsc[slot] = nullptr;
        return false;
    }
    /* LGFX スプライトのバッファは RGB565(uint16_t) 想定。lv_color_t とサイズを合わせてコピー。 */
    memcpy(buf, spr_buf, sizeof(lv_color_t) * px_count);
    spr.deleteSprite();

    g_lgfx_thumb_dsc_slot[slot].header.always_zero = 0;
    g_lgfx_thumb_dsc_slot[slot].header.w = (lv_coord_t)out_w;
    g_lgfx_thumb_dsc_slot[slot].header.h = (lv_coord_t)out_h;
    g_lgfx_thumb_dsc_slot[slot].header.cf = LV_IMG_CF_TRUE_COLOR;
    g_lgfx_thumb_dsc_slot[slot].data = (const uint8_t *)buf;
    g_lgfx_thumb_dsc_slot[slot].data_size = sizeof(lv_color_t) * px_count;

    xtouch_thumbnail_slot_dsc[slot] = (void *)&g_lgfx_thumb_dsc_slot[slot];
    return true;
}

#endif /* _XTOUCH_THUMBNAIL_H */
