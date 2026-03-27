#include "ui_comp_historycomponent.h"
#include "../ui_msgs.h"
#include "../ui_helpers.h"
#include "../ui_events.h"

#ifdef __XTOUCH_SCREEN_50__

#define ROW_LEFT_W 150
#define ROW_LEFT_H 150

lv_obj_t *ui_historyComponent_create(lv_obj_t *comp_parent)
{
    lv_obj_t *cui_historyComponent = lv_obj_create(comp_parent);
    lv_obj_set_width(cui_historyComponent, lv_pct(90));
    lv_obj_set_height(cui_historyComponent, lv_pct(100));
    lv_obj_set_flex_grow(cui_historyComponent, 1);
    lv_obj_set_flex_flow(cui_historyComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_historyComponent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(cui_historyComponent, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_historyComponent, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_historyComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_historyComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_historyComponent, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_historyComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *list = lv_obj_create(cui_historyComponent);
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

    ui_historyContentPanel = cui_historyComponent;
    ui_historyListContainer = list;

    for (int i = 0; i < XTOUCH_HISTORY_TASKS_MAX; i++)
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

        lv_obj_t *leftBox = lv_obj_create(row);
        lv_obj_set_width(leftBox, ROW_LEFT_W);
        lv_obj_set_height(leftBox, ROW_LEFT_H);
        lv_obj_set_style_bg_color(leftBox, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(leftBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(leftBox, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(leftBox, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t *coverImg = lv_img_create(leftBox);
        lv_obj_center(coverImg);
        lv_img_set_size_mode(coverImg, LV_IMG_SIZE_MODE_REAL);
        lv_obj_set_size(coverImg, ROW_LEFT_W, ROW_LEFT_H);
        lv_obj_add_flag(coverImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_t *placeLabel = lv_label_create(leftBox);
        lv_label_set_text(placeLabel, "-");
        lv_obj_center(placeLabel);
        lv_obj_set_style_text_color(placeLabel, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *rightCol = lv_obj_create(row);
        lv_obj_set_flex_grow(rightCol, 1);
        lv_obj_set_height(rightCol, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(rightCol, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(rightCol, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_bg_opa(rightCol, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(rightCol, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_row(rightCol, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(rightCol, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *titleLabel = lv_label_create(rightCol);
        lv_label_set_text(titleLabel, "-");
        lv_obj_set_style_text_color(titleLabel, lv_color_hex(0xDDDDDD), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(titleLabel, &lv_font_notosans_28, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_long_mode(titleLabel, LV_LABEL_LONG_CLIP);

        lv_obj_t *printerLabel = lv_label_create(rightCol);
        lv_label_set_text(printerLabel, "");
        lv_obj_set_style_text_color(printerLabel, lv_color_hex(0xAAAAAA), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(printerLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_long_mode(printerLabel, LV_LABEL_LONG_CLIP);

        lv_obj_t *dateLabel = lv_label_create(rightCol);
        lv_label_set_text(dateLabel, "");
        lv_obj_set_style_text_color(dateLabel, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(dateLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *statusLabel = lv_label_create(rightCol);
        lv_label_set_text(statusLabel, "");
        lv_obj_set_style_text_color(statusLabel, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(statusLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *reprintBtn = lv_obj_create(row);
        lv_obj_set_width(reprintBtn, 120);
        lv_obj_set_height(reprintBtn, 75);
        lv_obj_set_flex_flow(reprintBtn, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(reprintBtn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_add_flag(reprintBtn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(reprintBtn, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(reprintBtn, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_style_radius(reprintBtn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(reprintBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(reprintBtn, lv_color_hex(0x2a552a), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(reprintBtn, lv_color_hex(0x008800), LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(reprintBtn, 255, LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_user_data(reprintBtn, (void *)(intptr_t)i);
        lv_obj_t *reprintLbl = lv_label_create(reprintBtn);
        lv_label_set_text(reprintLbl, "Reprint");
        lv_obj_set_style_text_font(reprintLbl, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_add_event_cb(reprintBtn, onHistoryReprint, LV_EVENT_CLICKED, NULL);
    }

    {
        ui_msg_send(XTOUCH_HISTORY_FETCH, 0, 0);
    }
    return cui_historyComponent;
}

#else

lv_obj_t *ui_historyComponent_create(lv_obj_t *comp_parent)
{
    (void)comp_parent;
    return NULL;
}

#endif
