#include <stdio.h>
#include <string.h>
#include "../ui.h"
#include "../ui_msgs.h"
#include "../ui_helpers.h"

#ifdef __XTOUCH_SCREEN_50__

#define ROW_LEFT_THUMB_W 150
#define ROW_LEFT_THUMB_H 150

static const char *print_status_str(int s)
{
    switch (s)
    {
    /* この画面の初期表示では未接続に見えるよう、IDLE は Disconnected 表記にする */
    case XTOUCH_PRINT_STATUS_IDLE: return "Disconnected";
    case XTOUCH_PRINT_STATUS_RUNNING: return "Running";
    case XTOUCH_PRINT_STATUS_PAUSED: return "Paused";
    case XTOUCH_PRINT_STATUS_FINISHED: return "Finished";
    case XTOUCH_PRINT_STATUS_PREPARE: return "Prepare";
    case XTOUCH_PRINT_STATUS_FAILED: return "Failed";
    default: return "-";
    }
}

#define PRINTERS_ROW_MAX 5

/* 1行: row の子は [0]=左サムネイル用パネル(中にimg), [1]=右カラム(中に name, progress, layer)。
 * サムネイル画像はイベント通知時のみ set_src する（SD を参照しない）。 */
static void update_one_row(int slot, lv_obj_t *row)
{
    lv_obj_t *leftBox = lv_obj_get_child(row, 0);
    lv_obj_t *rightCol = lv_obj_get_child(row, 1);
    if (!leftBox || !rightCol)
        return;
    lv_obj_t *nameLabel = lv_obj_get_child(rightCol, 0);
    lv_obj_t *progressBar = lv_obj_get_child(rightCol, 1);
    lv_obj_t *layerLabel = lv_obj_get_child(rightCol, 2);
    if (!nameLabel || !progressBar || !layerLabel)
        return;

    const char *name = "-";
    int percent = 0;
    int left_time = 0;
    int cur_layer = 0;
    int tot_layers = 0;
    int status = XTOUCH_PRINT_STATUS_IDLE;

    if (slot == 0)
    {
        name = xTouchConfig.xTouchPrinterName[0] ? xTouchConfig.xTouchPrinterName : xTouchConfig.xTouchSerialNumber;
        percent = bambuStatus.mc_print_percent;
        left_time = bambuStatus.mc_left_time;
        cur_layer = bambuStatus.current_layer;
        tot_layers = bambuStatus.total_layers;
        status = bambuStatus.print_status;
    }
    else if (slot - 1 < xtouch_other_printer_count && otherPrinters[slot - 1].valid)
    {
        name = otherPrinters[slot - 1].name[0] ? otherPrinters[slot - 1].name : otherPrinters[slot - 1].dev_id;
        percent = otherPrinters[slot - 1].mc_print_percent;
        left_time = otherPrinters[slot - 1].mc_left_time;
        cur_layer = otherPrinters[slot - 1].current_layer;
        tot_layers = otherPrinters[slot - 1].total_layers;
        status = otherPrinters[slot - 1].print_status;
    }

    lv_label_set_text(nameLabel, name);
    lv_slider_set_value(progressBar, percent, LV_ANIM_OFF);

    char layerBuf[96];
    char timeBuf[48];
    _ui_seconds_to_timeleft((uint32_t)(left_time > 0 ? left_time : 0), timeBuf);

    /* Home と同様のイメージで、印刷中だけ Layer + 残り時間、それ以外はステータス文字を表示 */
    if (status == XTOUCH_PRINT_STATUS_RUNNING ||
        status == XTOUCH_PRINT_STATUS_PAUSED ||
        status == XTOUCH_PRINT_STATUS_PREPARE)
    {
        if (tot_layers > 0)
            snprintf(layerBuf, sizeof(layerBuf), "Layer %d/%d | %s", cur_layer, tot_layers, timeBuf);
        else
            snprintf(layerBuf, sizeof(layerBuf), "Layer - | %s", timeBuf);
    }
    else
    {
        snprintf(layerBuf, sizeof(layerBuf), "%s", print_status_str(status));
    }
    lv_label_set_text(layerLabel, layerBuf);
    lv_obj_clear_flag(layerLabel, LV_OBJ_FLAG_HIDDEN);
}

/* 次の LVGL サイクルでサムネイルを set_src する（メッセージ配送中だと FS open が失敗しうるため遅延） */
static void thumb_refresh_timer_cb(lv_timer_t *t)
{
    int slot = (int)(intptr_t)t->user_data;
    if (ui_printersListContainer == NULL || xTouchConfig.currentScreenIndex != 6)
        return;
    lv_obj_t *row = lv_obj_get_child(ui_printersListContainer, slot);
    if (!row)
        return;
    lv_obj_t *leftBox = lv_obj_get_child(row, 0);
    if (!leftBox)
        return;
    lv_obj_t *img = lv_obj_get_child(leftBox, 0);
    if (!img)
        return;
    if (slot >= 0 && slot < XTOUCH_THUMB_SLOT_MAX)
    {
        lv_img_set_src(img, xtouch_thumbnail_slot_path[slot]);
        lv_obj_invalidate(img);
    }
}

void ui_printers_on_other_update(lv_msg_t *m, void *user_data)
{
    (void)user_data;
    if (ui_printersListContainer == NULL || xTouchConfig.currentScreenIndex != 6)
        return;

    /* サムネイルDL完了通知（XTOUCH_ON_OTHER_PRINTER_UPDATE）のときだけ payload をスロットとして使う */
    if (m && lv_msg_get_id(m) == XTOUCH_ON_OTHER_PRINTER_UPDATE)
    {
        const void *p = lv_msg_get_payload(m);
        if (p)
        {
            int slot = (int)(intptr_t)p - 1;  /* 送信側で +1 しているので戻す */
            if (slot >= 0 && slot < PRINTERS_ROW_MAX)
            {
                /* 50ms 遅延: ダウンロード close 直後だと FS がまだ同期中の場合がある */
                lv_timer_t *t = lv_timer_create(thumb_refresh_timer_cb, 50, (void *)(intptr_t)slot);
                lv_timer_set_repeat_count(t, 1);
            }
        }
    }

    /* 表示行数: 現在機1 + 他機数、最大5。名前・進捗・レイヤは常に update_one_row で更新 */
    int show_count = 1 + xtouch_other_printer_count;
    if (show_count > XTOUCH_MULTI_PRINTER_MAX)
        show_count = XTOUCH_MULTI_PRINTER_MAX;
    for (int i = 0; i < XTOUCH_MULTI_PRINTER_MAX; i++)
    {
        lv_obj_t *row = lv_obj_get_child(ui_printersListContainer, i);
        if (row == NULL)
            continue;
        if (i < show_count)
        {
            lv_obj_clear_flag(row, LV_OBJ_FLAG_HIDDEN);
            update_one_row(i, row);
        }
        else
            lv_obj_add_flag(row, LV_OBJ_FLAG_HIDDEN);
    }
}

/* lv_msg_subsribe_obj 経由で LV_EVENT_MSG_RECEIVED を受けたときに呼ばれるブリッジ（画面内で static） */
static void ui_event_printers_on_other_update(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_MSG_RECEIVED)
        return;
    lv_msg_t *m = lv_event_get_msg(e);
    ui_printers_on_other_update(m, NULL);
}

void ui_printersScreen_screen_init(void)
{
    ui_printersScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_printersScreen, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_set_scrollbar_mode(ui_printersScreen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(ui_printersScreen, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_printersScreen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(ui_printersScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_printersScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_printersScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_printersScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_printersScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_printersScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_sidebarComponent = ui_sidebarComponent_create(ui_printersScreen);
    lv_obj_set_x(ui_sidebarComponent, 387);
    lv_obj_set_y(ui_sidebarComponent, 178);

    /* コンテンツ部分はコンポーネントに委譲 */
    ui_printersComponent_create(ui_printersScreen);
    /* 一覧の購読・コールバックは画面側で登録。初期表示もイベントで依頼 */
    lv_msg_subsribe_obj(XTOUCH_ON_OTHER_PRINTER_UPDATE, ui_printersListContainer, NULL);
    lv_msg_subsribe_obj(XTOUCH_PRINTERS_LIST_REFRESH, ui_printersListContainer, NULL);
    lv_obj_add_event_cb(ui_printersListContainer, ui_event_printers_on_other_update, LV_EVENT_MSG_RECEIVED, NULL);
    {
        struct XTOUCH_MESSAGE_DATA eventData;
        eventData.data = 0;
        eventData.data2 = 0;
        lv_msg_send(XTOUCH_PRINTERS_LIST_REFRESH, &eventData);
    }
}

#else

void ui_printersScreen_screen_init(void)
{
    (void)0;
}

#endif
