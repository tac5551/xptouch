#ifndef _XTOUCH_THUMBNAIL_H
#define _XTOUCH_THUMBNAIL_H

#include <stdio.h>
#include <string.h>
#include "ui/ui_msgs.h"

#define XTOUCH_THUMB_PATH_PREFIX "/tmp/pthumb_"
#define XTOUCH_THUMB_PATH_SUFFIX ".png"
#define XTOUCH_THUMB_SLOT_MAX 5

/** サムネイル用パスは types.h の xtouch_thumbnail_slot_path[] を UI が参照。本ヘッダではタイマー等のみ。 */
/** サムネイル用タイマーを開始（Printers 画面用）。 */
void xtouch_thumbnail_timer_start(void);
/** サムネイル用タイマーを停止。 */
void xtouch_thumbnail_timer_stop(void);
/** 次回タイマーから全スロットを取得して上書きするようスケジュール（画面表示時に呼ぶ）。 */
void xtouch_thumbnail_schedule_fetch_all(void);

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
    bool exists = (slot < XTOUCH_THUMB_SLOT_MAX && s_thumb_exists[slot]);
#ifdef XTOUCH_DEBUG
    ConsoleDebug.print(F("[xPTouch][THUMB] needs? slot="));
    ConsoleDebug.print(slot);
    ConsoleDebug.print(F(" exists="));
    ConsoleDebug.println(exists ? 1 : 0);
#endif
    return !exists;
}

static void thumbnail_timer_cb(lv_timer_t *t)
{
    (void)t;
    if (xTouchConfig.currentScreenIndex != 6)
        return;

    if (s_thumb_force_fetch_slot < XTOUCH_THUMB_SLOT_MAX)
    {
        int s = s_thumb_force_fetch_slot;
        if (!thumbnail_slot_has_url_or_task(s, cloud.loggedIn ? 1 : 0))
        {
            /* task_id 等がまだ届いていない場合は進めない。次ティックで同じスロットを再試行 */
            return;
        }
        s_thumb_force_fetch_slot++;
#ifdef XTOUCH_DEBUG
        ConsoleDebug.print(F("[xPTouch][THUMB] force fetch slot="));
        ConsoleDebug.println(s);
#endif
        if (downloadThumbnailForSlot(s) && s < XTOUCH_THUMB_SLOT_MAX)
        {
            s_thumb_exists[s] = true;
            lv_msg_send(XTOUCH_ON_OTHER_PRINTER_UPDATE, (void *)(intptr_t)(s + 1));
        }
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
            if (downloadThumbnailForSlot(s) && s < XTOUCH_THUMB_SLOT_MAX)
            {
                s_thumb_exists[s] = true;
                lv_msg_send(XTOUCH_ON_OTHER_PRINTER_UPDATE, (void *)(intptr_t)(s + 1));
            }
            s_thumb_next_slot = (s + 1) % XTOUCH_THUMB_SLOT_MAX;
            break;
        }
    }
}

void xtouch_thumbnail_timer_start(void)
{
    if (s_thumb_timer)
        return;
    s_thumb_timer = lv_timer_create(thumbnail_timer_cb, 3000, nullptr);
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
    lv_msg_subscribe(XTOUCH_PRINTERS_SCHEDULE_THUMB_FETCH, (lv_msg_subscribe_cb_t)xtouch_thumbnail_on_schedule_fetch, NULL);
    lv_msg_subscribe(XTOUCH_PRINTERS_THUMB_TIMER_START, (lv_msg_subscribe_cb_t)xtouch_thumbnail_on_timer_start, NULL);
    lv_msg_subscribe(XTOUCH_PRINTERS_THUMB_TIMER_STOP, (lv_msg_subscribe_cb_t)xtouch_thumbnail_on_timer_stop, NULL);
}
#endif

#endif /* _XTOUCH_THUMBNAIL_H */
