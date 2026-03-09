#include "ui_comp_printerscomponent.h"
#include "../ui_msgs.h"
#include "../ui_helpers.h"
#include "../ui_events.h"

#ifdef __XTOUCH_SCREEN_50__

/* サムネイルのサイズ（ui_printersScreen.c と揃える） */
#define ROW_LEFT_THUMB_W 150
#define ROW_LEFT_THUMB_H 150

lv_obj_t *ui_printersComponent_create(lv_obj_t *comp_parent)
{
    /* コンテンツ全体のパネル（従来の ui_printersContentPanel 相当） */
    lv_obj_t *cui_printersComponent = lv_obj_create(comp_parent);
    lv_obj_set_width(cui_printersComponent, lv_pct(90));
    lv_obj_set_height(cui_printersComponent, lv_pct(100));
    lv_obj_set_flex_grow(cui_printersComponent, 1);
    lv_obj_set_flex_flow(cui_printersComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_printersComponent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(cui_printersComponent, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_printersComponent, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_printersComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_printersComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_printersComponent, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_printersComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* プリンタ一覧リスト（スクロールコンテナ） */
    lv_obj_t *list = lv_obj_create(cui_printersComponent);
    lv_obj_set_width(list, lv_pct(100));
    lv_obj_set_flex_grow(list, 1);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_pad_row(list, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_printersContentPanel = cui_printersComponent;
    ui_printersListContainer = list;

    for (int i = 0; i < XTOUCH_MULTI_PRINTER_MAX; i++)
    {
        lv_obj_t *row = lv_obj_create(list);
        lv_obj_set_width(row, lv_pct(100));
        lv_obj_set_height(row, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_bg_color(row, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(row, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);

        /* 左: サムネイル用パネル */
        lv_obj_t *leftBox = lv_obj_create(row);
        lv_obj_set_width(leftBox, ROW_LEFT_THUMB_W);
        lv_obj_set_height(leftBox, ROW_LEFT_THUMB_H);
        lv_obj_set_style_bg_color(leftBox, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(leftBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(leftBox, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(leftBox, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t *img = lv_img_create(leftBox);
        lv_obj_center(img);
        lv_img_set_size_mode(img, LV_IMG_SIZE_MODE_REAL);
        lv_obj_set_size(img, ROW_LEFT_THUMB_W - 4, ROW_LEFT_THUMB_H - 4);

        /* 右: 名前 / 進捗バー / レイヤー・残り時間 */
        lv_obj_t *rightCol = lv_obj_create(row);
        lv_obj_set_flex_grow(rightCol, 1);
        lv_obj_set_height(rightCol, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(rightCol, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(rightCol, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_bg_opa(rightCol, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(rightCol, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_row(rightCol, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(rightCol, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *nameLabel = lv_label_create(rightCol);
        lv_label_set_text(nameLabel, "-");
        lv_obj_set_style_text_color(nameLabel, lv_color_hex(0xDDDDDD), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(nameLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *progressBar = lv_slider_create(rightCol);
        lv_slider_set_range(progressBar, 0, 100);
        lv_slider_set_value(progressBar, 0, LV_ANIM_OFF);
        lv_obj_set_width(progressBar, lv_pct(100));
        lv_obj_set_height(progressBar, 14);
        lv_obj_clear_flag(progressBar, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE);
        lv_obj_set_style_bg_color(progressBar, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(progressBar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(progressBar, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(progressBar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(progressBar, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

        lv_obj_t *layerLabel = lv_label_create(rightCol);
        lv_label_set_text(layerLabel, "Layer - | --");
        lv_obj_set_style_text_color(layerLabel, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(layerLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_height(layerLabel, LV_SIZE_CONTENT);
        lv_label_set_long_mode(layerLabel, LV_LABEL_LONG_WRAP);

        /* 右端: 一時停止・停止ボタン用エリア（スペース固定、印刷中のみボタン表示） */
        lv_obj_t *btnArea = lv_obj_create(row);
        lv_obj_set_width(btnArea, 120);
        lv_obj_set_height(btnArea, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(btnArea, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(btnArea, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(btnArea, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(btnArea, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(btnArea, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(btnArea, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *pauseBtn = lv_label_create(btnArea);
        lv_obj_set_width(pauseBtn, 56);
        lv_obj_set_height(pauseBtn, 40);
        lv_label_set_text(pauseBtn, "0");
        lv_obj_add_flag(pauseBtn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_text_align(pauseBtn, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(pauseBtn, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(pauseBtn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(pauseBtn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(pauseBtn, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(pauseBtn, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(pauseBtn, 255, LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_user_data(pauseBtn, (void *)(intptr_t)i);
        lv_obj_add_flag(pauseBtn, LV_OBJ_FLAG_HIDDEN);

        lv_obj_t *stopBtn = lv_label_create(btnArea);
        lv_obj_set_width(stopBtn, 56);
        lv_obj_set_height(stopBtn, 40);
        lv_label_set_text(stopBtn, "1");
        lv_obj_add_flag(stopBtn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_text_align(stopBtn, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(stopBtn, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(stopBtn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(stopBtn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(stopBtn, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(stopBtn, lv_color_hex(0xff682a), LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(stopBtn, 255, LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_user_data(stopBtn, (void *)(intptr_t)i);
        lv_obj_add_flag(stopBtn, LV_OBJ_FLAG_HIDDEN);

        lv_obj_add_event_cb(pauseBtn, onPrintersPause, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(stopBtn, onPrintersStop, LV_EVENT_CLICKED, NULL);
    }

    /* 購読・初期表示は画面側でイベント登録・送信。ここではサムネイル取得のみイベント送信 */
    {
        struct XTOUCH_MESSAGE_DATA eventData;
        eventData.data = 0;
        eventData.data2 = 0;
        lv_msg_send(XTOUCH_PRINTERS_SCHEDULE_THUMB_FETCH, &eventData);
    }
    return cui_printersComponent;
}

#else

lv_obj_t *ui_printersComponent_create(lv_obj_t *comp_parent)
{
    (void)comp_parent;
    return NULL;
}

#endif

