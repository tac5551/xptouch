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
    /* 待機中（未接続スロットは行ごと非表示なのでここには来ない） */
    case XTOUCH_PRINT_STATUS_IDLE: return "IDLE";
    case XTOUCH_PRINT_STATUS_RUNNING: return "Running";
    case XTOUCH_PRINT_STATUS_PAUSED: return "Paused";
    case XTOUCH_PRINT_STATUS_FINISHED: return "Finished";
    case XTOUCH_PRINT_STATUS_PREPARE: return "Prepare";
    case XTOUCH_PRINT_STATUS_FAILED: return "Failed";
    default: return "-";
    }
}

#define PRINTERS_ROW_MAX 5

/* 1行: row の子は [0]=左サムネイル, [1]=右カラム(name/subtask/progress/layer), [2]=ボタンエリア(上段pause/stop/reprint, 下段select)。
 * ボタンは印刷中(RUNNING/PAUSED/PREPARE)のみ表示、終了時は非表示（スペースは維持）。 */
static void update_one_row(int slot, lv_obj_t *row)
{
    lv_obj_t *leftBox = lv_obj_get_child(row, 0);
    lv_obj_t *rightCol = lv_obj_get_child(row, 1);
    lv_obj_t *btnArea = lv_obj_get_child(row, 2);
    if (!leftBox || !rightCol || !btnArea)
        return;
    lv_obj_t *nameLabel = lv_obj_get_child(rightCol, 0);
    lv_obj_t *subtaskLabel = lv_obj_get_child(rightCol, 1);
    lv_obj_t *progressBar = lv_obj_get_child(rightCol, 2);
    lv_obj_t *layerLabel = lv_obj_get_child(rightCol, 3);
    lv_obj_t *topBtns = lv_obj_get_child(btnArea, 0);
    lv_obj_t *bottomBtns = lv_obj_get_child(btnArea, 1);
    lv_obj_t *pauseBtn = topBtns ? lv_obj_get_child(topBtns, 0) : NULL;
    lv_obj_t *stopBtn = topBtns ? lv_obj_get_child(topBtns, 1) : NULL;
    lv_obj_t *reprintBtn = topBtns ? lv_obj_get_child(topBtns, 2) : NULL;
    lv_obj_t *selectBtn = bottomBtns ? lv_obj_get_child(bottomBtns, 0) : NULL;
    if (!nameLabel || !subtaskLabel || !progressBar || !layerLabel || !topBtns || !bottomBtns || !pauseBtn || !stopBtn || !selectBtn || !reprintBtn)
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

    /* プリンタ名の下に subtask_name（ファイル名）の先頭を表示 */
    const char *subtask_src = "";
    if (slot == 0)
        subtask_src = bambuStatus.subtask_name[0] ? bambuStatus.subtask_name : "";
    else if (slot - 1 < xtouch_other_printer_count && otherPrinters[slot - 1].valid)
        subtask_src = otherPrinters[slot - 1].subtask_name[0] ? otherPrinters[slot - 1].subtask_name : "";
    {
        char subBuf[24];
        int n = 0;
        while (n < 20 && subtask_src[n] != '\0')
        {
            subBuf[n] = subtask_src[n];
            n++;
        }
        subBuf[n] = '\0';
        if (subtask_src[n] != '\0')
        {
            if (n > 3)
                n = 17;
            subBuf[n++] = '.';
            subBuf[n++] = '.';
            subBuf[n++] = '.';
            subBuf[n] = '\0';
        }
        lv_label_set_text(subtaskLabel, subBuf);
    }

    lv_slider_set_value(progressBar, percent, LV_ANIM_OFF);
    /* 印刷中(RUNNING/PAUSED/PREPARE)のみゲージ表示。それ以外(IDLE/FINISHED/FAILED 等)は非表示。 */
    if (status == XTOUCH_PRINT_STATUS_RUNNING ||
        status == XTOUCH_PRINT_STATUS_PAUSED ||
        status == XTOUCH_PRINT_STATUS_PREPARE)
        lv_obj_clear_flag(progressBar, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_add_flag(progressBar, LV_OBJ_FLAG_HIDDEN);

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

    /* 印刷中のみボタン表示、終了時は非表示（スペースはそのまま）。ボタンはコンテナで子がラベル */
    if (status == XTOUCH_PRINT_STATUS_RUNNING ||
        status == XTOUCH_PRINT_STATUS_PAUSED ||
        status == XTOUCH_PRINT_STATUS_PREPARE)
    {
        lv_obj_clear_flag(pauseBtn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(stopBtn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(reprintBtn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_t *pauseLbl = lv_obj_get_child(pauseBtn, 0);
        if (pauseLbl)
        {
            if (status == XTOUCH_PRINT_STATUS_PAUSED)
                lv_label_set_text(pauseLbl, "z");
            else
                lv_label_set_text(pauseLbl, "0");
        }
    }
    else
    {
        lv_obj_add_flag(pauseBtn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(stopBtn, LV_OBJ_FLAG_HIDDEN);
        const char *tid = "";
        if (slot == 0)
            tid = bambuStatus.task_id;
        else if (slot - 1 < xtouch_other_printer_count && otherPrinters[slot - 1].valid)
            tid = otherPrinters[slot - 1].task_id;
        if ((status == XTOUCH_PRINT_STATUS_FINISHED || status == XTOUCH_PRINT_STATUS_FAILED) &&
            tid && tid[0] && strcmp(tid, "0") != 0)
            lv_obj_clear_flag(reprintBtn, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(reprintBtn, LV_OBJ_FLAG_HIDDEN);
    }

    /* 一時切替中のみ先頭行に「SW」: ペア確定機へ戻す */
    if (slot == 0)
    {
        bool show_back_sw = (xTouchConfig.xTouchPairedSerialNumber[0] != '\0' &&
                             strcmp(xTouchConfig.xTouchSerialNumber, xTouchConfig.xTouchPairedSerialNumber) != 0);
        if (show_back_sw)
            lv_obj_clear_flag(selectBtn, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(selectBtn, LV_OBJ_FLAG_HIDDEN);
    }
    else
        lv_obj_clear_flag(selectBtn, LV_OBJ_FLAG_HIDDEN);
}

/* 指定スロットのサムネイル img を slot_dsc または path で即時描画 */
static void thumb_refresh_slot(int slot)
{
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
        ui_thumb_set_img_src_from_slot(img, slot);
        /* コールバック内で即反映させるため再描画を実行（次の DL ブロック前に画面更新） */
        lv_timer_handler();
    }
}

static void thumb_refresh_timer_cb(lv_timer_t *t)
{
    int slot = (int)(intptr_t)t->user_data;
    thumb_refresh_slot(slot);
}

void ui_printers_on_other_update(lv_msg_t *m, void *user_data)
{
    (void)user_data;
    if (ui_printersListContainer == NULL || xTouchConfig.currentScreenIndex != 6)
        return;

    const bool refresh_all_thumb_slots =
        (m == NULL || lv_msg_get_id(m) == XTOUCH_PRINTERS_LIST_REFRESH);

    /* サムネイルDL完了通知（XTOUCH_ON_OTHER_PRINTER_UPDATE）のとき payload のスロットを即描画。
     * 遅延しない: 送信側で DL→LGFX デコード済みなので、ここで即 set_src して次のスロット DL ブロック前に描画する。
     * payload: MQTT は &XTOUCH_MESSAGE_DATA (data = 行インデックス 0=メイン,1=他1台目…)、サムネは (void*)(slot+1)。 */
    if (m && lv_msg_get_id(m) == XTOUCH_ON_OTHER_PRINTER_UPDATE)
    {
        const void *p = lv_msg_get_payload(m);
        if (p)
        {
            int slot;
            if ((uintptr_t)p < 256)
                slot = (int)(intptr_t)p - 1;  /* サムネ側: slot+1 で送っている */
            else
                slot = (int)((const struct XTOUCH_MESSAGE_DATA *)p)->data;  /* MQTT: data = 行インデックス */
            if (slot >= 0 && slot < PRINTERS_ROW_MAX)
                thumb_refresh_slot(slot);
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

    if (refresh_all_thumb_slots)
    {
        for (int i = 0; i < show_count && i < PRINTERS_ROW_MAX; i++)
            thumb_refresh_slot(i);
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
        ui_msg_send(XTOUCH_PRINTERS_LIST_REFRESH, 0, 0);
    }
}

#else

void ui_printersScreen_screen_init(void)
{
    (void)0;
}

#endif
