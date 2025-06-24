#include "../ui.h"
#include "../../xtouch/trays.h"
#include "../../ui/ui_msgs.h"

#define AMS_COUNT 4
#define SLOT_COUNT 5
#define AMS_BORDER 8



void ui_event_comp_amsViewComponent_onAmsHumidity(lv_event_t *e)
{

    //printf("onAmsHumidity %d\n", bambuStatus.ams_humidity);
    lv_obj_t *target = lv_event_get_target(e);
    char buffer[100];
    memset(buffer, 0, 100);
    sprintf(buffer, "H\n%d", bambuStatus.ams_humidity);
    lv_label_set_text(target, buffer);
}

void ui_event_comp_amsViewComponent_onAmsUpdate(lv_event_t *e)
{

    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);
    uintptr_t temp_user_data = (uintptr_t)lv_event_get_user_data(e);
    uint8_t user_data = (uint8_t)temp_user_data;
    //printf("onAmsUpdate %d\n", user_data);

    if (!(bambuStatus.ams_status_main == AMS_STATUS_MAIN_IDLE || bambuStatus.ams_status_main == AMS_STATUS_MAIN_ASSIST) || bambuStatus.print_status == XTOUCH_PRINT_STATUS_RUNNING)
    {
        lv_obj_add_state(target, LV_STATE_DISABLED);
    }
    else
    {
        lv_obj_clear_state(target, LV_STATE_DISABLED);
    }

    struct XTOUCH_MESSAGE_DATA *message = (struct XTOUCH_MESSAGE_DATA *)m->payload;

    uint32_t tray_status = get_tray_status(user_data);
    uint16_t tray_id = ((tray_status >> 4) & 0x0F);
    uint16_t loaded = ((tray_status) & 0x01);

    // lv_obj_t *unload = ui_comp_get_child(target, UI_COMP_amsViewComponent_FILAMENTSCREENFILAMENT_FILAMENTSCREENUNLOAD);

    if (user_data == tray_id)
    {
        lv_color_t color = lv_color_hex(tray_status >> 8);
        lv_color_t color_inv = lv_color_hex((0xFFFFFF - (tray_status >> 8)) & 0xFFFFFF);

        char buffer[100];
        memset(buffer, 0, 100);
        sprintf(buffer, "Slot %d\n%s", tray_id, get_tray_type(tray_id));
        lv_label_set_text(target, buffer);

        // printf(" tray_now: %d, tray_tar: %d, slot: %d, color: %06llX \n", bambuStatus.m_tray_now, bambuStatus.m_tray_tar, tray_id, message->data >> 8);

        lv_obj_set_style_bg_color(target, color, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(target, color_inv, LV_PART_MAIN | LV_STATE_DEFAULT);

        if (tray_id == 0)
            tray_id = 254 + 1;

        lv_obj_set_style_border_color(target, color, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(target, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        if (bambuStatus.m_tray_now + 1 == tray_id)
        {
            // lv_label_set_text(target, "L");
            lv_obj_set_style_border_color(target, color_inv, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(target, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        else if (bambuStatus.m_tray_pre + 1 == tray_id && bambuStatus.m_tray_pre != bambuStatus.m_tray_tar)
        {
            // lv_label_set_text(target, "U");
        }
        else if (!loaded)
        {
            // lv_label_set_text(target, "X");
        }
        else
        {
            // lv_label_set_text(target, "");
        }
    }
}

lv_obj_t *ui_amsViewComponent_create(lv_obj_t *comp_parent)
{
  lv_obj_t *cui_amsViewComponent;
    cui_amsViewComponent = lv_obj_create(comp_parent);
    lv_obj_set_height(cui_amsViewComponent, lv_pct(100));
    lv_obj_set_flex_grow(cui_amsViewComponent, 1);
    lv_obj_set_x(cui_amsViewComponent, 386);
    lv_obj_set_y(cui_amsViewComponent, 178);
    lv_obj_set_flex_flow(cui_amsViewComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_amsViewComponent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_amsViewComponent, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_amsViewComponent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_amsViewComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_amsViewComponent, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_amsViewComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_amsViewComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_amsViewComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_amsViewComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_amsViewComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_amsViewComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_amsViewComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_amsViewComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_amsViewComponent, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_amsViewComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsControl1;
    cui_AmsControl1 = lv_obj_create(cui_amsViewComponent);
    lv_obj_set_width(cui_AmsControl1, lv_pct(100));
    lv_obj_set_height(cui_AmsControl1, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsControl1, 1);
    lv_obj_set_x(cui_AmsControl1, 386);
    lv_obj_set_y(cui_AmsControl1, 178);
    lv_obj_set_flex_flow(cui_AmsControl1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_AmsControl1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_AmsControl1, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsControl1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_AmsControl1, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_AmsControl1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsControl1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsControl1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsControl1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsControl1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsControl1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_AmsControl1, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_AmsControl1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsControl1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_AmsControl1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsControl2;
    cui_AmsControl2 = lv_obj_create(cui_amsViewComponent);
    lv_obj_set_width(cui_AmsControl2, lv_pct(100));
    lv_obj_set_height(cui_AmsControl2, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsControl2, 1);
    lv_obj_set_x(cui_AmsControl2, 386);
    lv_obj_set_y(cui_AmsControl2, 178);
    lv_obj_set_flex_flow(cui_AmsControl2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_AmsControl2, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_AmsControl2, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsControl2, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_AmsControl2, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_AmsControl2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsControl2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsControl2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsControl2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsControl2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsControl2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_AmsControl2, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_AmsControl2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsControl2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_AmsControl2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsControl3;
    cui_AmsControl3 = lv_obj_create(cui_amsViewComponent);
    lv_obj_set_width(cui_AmsControl3, lv_pct(100));
    lv_obj_set_height(cui_AmsControl3, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsControl3, 1);
    lv_obj_set_x(cui_AmsControl3, 386);
    lv_obj_set_y(cui_AmsControl3, 178);
    lv_obj_set_flex_flow(cui_AmsControl3, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_AmsControl3, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_AmsControl3, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsControl3, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_AmsControl3, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_AmsControl3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsControl3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsControl3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsControl3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsControl3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsControl3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_AmsControl3, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_AmsControl3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsControl3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_AmsControl3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsControl4;
    cui_AmsControl4 = lv_obj_create(cui_amsViewComponent);
    lv_obj_set_width(cui_AmsControl4, lv_pct(100));
    lv_obj_set_height(cui_AmsControl4, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsControl4, 1);
    lv_obj_set_x(cui_AmsControl4, 386);
    lv_obj_set_y(cui_AmsControl4, 178);
    lv_obj_set_flex_flow(cui_AmsControl4, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_AmsControl4, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_AmsControl4, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsControl4, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_AmsControl4, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_AmsControl4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsControl4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsControl4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsControl4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsControl4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsControl4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_AmsControl4, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_AmsControl4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsControl4, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_AmsControl4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);



    lv_obj_t *cui_AmsHumid1;
    cui_AmsHumid1 = lv_label_create(cui_AmsControl1);
    lv_obj_set_width(cui_AmsHumid1, lv_pct(100));
    lv_obj_set_height(cui_AmsHumid1, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsHumid1, 1);
    lv_obj_set_align(cui_AmsHumid1, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsHumid1, "H\nX");
    lv_obj_clear_flag(cui_AmsHumid1, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsHumid1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsHumid1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsHumid1, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsHumid1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsHumid1, lv_color_hex(0x41ADDC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_AmsHumid1, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsHumid1, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_text_color(cui_AmsHumid1, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsHumid1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsHumid1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsHumid1, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsHumid1, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsHumid1, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsHumid1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsHumid1, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsHumid1, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsHumid1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    char buffer[100];
    memset(buffer, 0, 100);
    sprintf(buffer, "H\n%d", bambuStatus.ams_humidity);
    lv_label_set_text(cui_AmsHumid1, buffer);

    lv_obj_t *cui_AmsSlot1_1;
    cui_AmsSlot1_1 = lv_label_create(cui_AmsControl1);
    lv_obj_set_width(cui_AmsSlot1_1, lv_pct(100));
    lv_obj_set_height(cui_AmsSlot1_1, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsSlot1_1, 2);
    lv_obj_set_align(cui_AmsSlot1_1, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsSlot1_1, "Slot 1");
    lv_obj_add_flag(cui_AmsSlot1_1, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(cui_AmsSlot1_1, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsSlot1_1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsSlot1_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsSlot1_1, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsSlot1_1, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_AmsSlot1_1, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1_1, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsSlot1_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsSlot1_1, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsSlot1_1, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsSlot1_1, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsSlot1_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1_1, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1_1, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsSlot1_1, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsSlot1_2;
    cui_AmsSlot1_2 = lv_label_create(cui_AmsControl1);
    lv_obj_set_width(cui_AmsSlot1_2, lv_pct(100));
    lv_obj_set_height(cui_AmsSlot1_2, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsSlot1_2, 2);
    lv_obj_set_align(cui_AmsSlot1_2, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsSlot1_2, "Slot 2");
    lv_obj_add_flag(cui_AmsSlot1_2, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_AmsSlot1_2, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsSlot1_2, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsSlot1_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsSlot1_2, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsSlot1_2, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_AmsSlot1_2, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1_2, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsSlot1_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsSlot1_2, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsSlot1_2, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsSlot1_2, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsSlot1_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1_2, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1_2, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsSlot1_2, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsSlot1_3;
    cui_AmsSlot1_3 = lv_label_create(cui_AmsControl1);
    lv_obj_set_width(cui_AmsSlot1_3, lv_pct(100));
    lv_obj_set_height(cui_AmsSlot1_3, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsSlot1_3, 2);
    lv_obj_set_align(cui_AmsSlot1_3, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsSlot1_3, "Slot 3");
    lv_obj_add_flag(cui_AmsSlot1_3, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_AmsSlot1_3, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsSlot1_3, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsSlot1_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsSlot1_3, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsSlot1_3, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_AmsSlot1_3, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1_3, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsSlot1_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsSlot1_3, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsSlot1_3, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsSlot1_3, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsSlot1_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1_3, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1_3, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsSlot1_3, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsSlot1_4;
    cui_AmsSlot1_4 = lv_label_create(cui_AmsControl1);
    lv_obj_set_width(cui_AmsSlot1_4, lv_pct(100));
    lv_obj_set_height(cui_AmsSlot1_4, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsSlot1_4, 2);
    lv_obj_set_align(cui_AmsSlot1_4, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsSlot1_4, "Slot 4");
    lv_obj_add_flag(cui_AmsSlot1_4, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_AmsSlot1_4, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsSlot1_4, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsSlot1_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsSlot1_4, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsSlot1_4, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_AmsSlot1_4, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1_4, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1_4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsSlot1_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsSlot1_4, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsSlot1_4, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsSlot1_4, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsSlot1_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1_4, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1_4, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsSlot1_4, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_t *cui_AmsHumid2;
    cui_AmsHumid2 = lv_label_create(cui_AmsControl2);
    lv_obj_set_width(cui_AmsHumid2, lv_pct(100));
    lv_obj_set_height(cui_AmsHumid2, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsHumid2, 1);
    lv_obj_set_align(cui_AmsHumid2, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsHumid2, "H\nX");
    lv_obj_clear_flag(cui_AmsHumid2, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsHumid2, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsHumid2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsHumid2, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsHumid2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsHumid2, lv_color_hex(0x41ADDC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_AmsHumid2, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsHumid2, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_text_color(cui_AmsHumid2, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsHumid2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsHumid2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsHumid2, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsHumid2, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsHumid2, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsHumid2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsHumid2, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsHumid2, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsHumid2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsSlot2_1;
    cui_AmsSlot2_1 = lv_label_create(cui_AmsControl2);
    lv_obj_set_width(cui_AmsSlot2_1, lv_pct(100));
    lv_obj_set_height(cui_AmsSlot2_1, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsSlot2_1, 2);
    lv_obj_set_align(cui_AmsSlot2_1, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsSlot2_1, "Slot 1");
    lv_obj_add_flag(cui_AmsSlot2_1, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(cui_AmsSlot2_1, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsSlot2_1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsSlot2_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsSlot2_1, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsSlot2_1, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_AmsSlot2_1, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot2_1, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsSlot2_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsSlot2_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsSlot2_1, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsSlot2_1, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsSlot2_1, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsSlot2_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot2_1, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsSlot2_1, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsSlot2_1, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsSlot2_2;
    cui_AmsSlot2_2 = lv_label_create(cui_AmsControl2);
    lv_obj_set_width(cui_AmsSlot2_2, lv_pct(100));
    lv_obj_set_height(cui_AmsSlot2_2, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsSlot2_2, 2);
    lv_obj_set_align(cui_AmsSlot2_2, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsSlot2_2, "Slot 2");
    lv_obj_add_flag(cui_AmsSlot2_2, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_AmsSlot2_2, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsSlot2_2, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsSlot2_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsSlot2_2, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsSlot2_2, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_AmsSlot2_2, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot2_2, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsSlot2_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsSlot2_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsSlot2_2, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsSlot2_2, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsSlot2_2, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsSlot2_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot2_2, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsSlot2_2, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsSlot2_2, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsSlot2_3;
    cui_AmsSlot2_3 = lv_label_create(cui_AmsControl2);
    lv_obj_set_width(cui_AmsSlot2_3, lv_pct(100));
    lv_obj_set_height(cui_AmsSlot2_3, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsSlot2_3, 2);
    lv_obj_set_align(cui_AmsSlot2_3, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsSlot2_3, "Slot 3");
    lv_obj_add_flag(cui_AmsSlot2_3, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_AmsSlot2_3, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsSlot2_3, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsSlot2_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsSlot2_3, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsSlot2_3, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_AmsSlot2_3, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot2_3, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsSlot2_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsSlot2_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsSlot2_3, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsSlot2_3, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsSlot2_3, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsSlot2_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot2_3, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsSlot2_3, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsSlot2_3, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsSlot2_4;
    cui_AmsSlot2_4 = lv_label_create(cui_AmsControl2);
    lv_obj_set_width(cui_AmsSlot2_4, lv_pct(100));
    lv_obj_set_height(cui_AmsSlot2_4, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsSlot2_4, 2);
    lv_obj_set_align(cui_AmsSlot2_4, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsSlot2_4, "Slot 4");
    lv_obj_add_flag(cui_AmsSlot2_4, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_AmsSlot2_4, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsSlot2_4, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsSlot2_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsSlot2_4, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsSlot2_4, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_AmsSlot2_4, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot2_4, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsSlot2_4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsSlot2_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsSlot2_4, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsSlot2_4, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsSlot2_4, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsSlot2_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot2_4, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsSlot2_4, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsSlot2_4, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_add_event_cb(cui_AmsHumid1, ui_event_comp_amsViewComponent_onAmsHumidity, LV_EVENT_MSG_RECEIVED, (void *)NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_HUMIDITY_UPDATE, cui_AmsHumid1, (void *)NULL);

    lv_obj_add_event_cb(cui_AmsSlot1_1, ui_event_comp_amsViewComponent_onAmsUpdate, LV_EVENT_MSG_RECEIVED, (void *)1);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_AmsSlot1_1, (void *)1);

    lv_obj_add_event_cb(cui_AmsSlot1_2, ui_event_comp_amsViewComponent_onAmsUpdate, LV_EVENT_MSG_RECEIVED, (void *)2);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_AmsSlot1_2, (void *)2);

    lv_obj_add_event_cb(cui_AmsSlot1_3, ui_event_comp_amsViewComponent_onAmsUpdate, LV_EVENT_MSG_RECEIVED, (void *)3);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_AmsSlot1_3, (void *)3);

    lv_obj_add_event_cb(cui_AmsSlot1_4, ui_event_comp_amsViewComponent_onAmsUpdate, LV_EVENT_MSG_RECEIVED, (void *)4);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_AmsSlot1_4, (void *)4);

    lv_obj_add_event_cb(cui_AmsHumid2, ui_event_comp_amsViewComponent_onAmsHumidity, LV_EVENT_MSG_RECEIVED, (void *)NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_HUMIDITY_UPDATE, cui_AmsHumid2, (void *)NULL);

    lv_obj_add_event_cb(cui_AmsSlot2_1, ui_event_comp_amsViewComponent_onAmsUpdate, LV_EVENT_MSG_RECEIVED, (void *)5);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_AmsSlot2_1, (void *)5);

    lv_obj_add_event_cb(cui_AmsSlot2_2, ui_event_comp_amsViewComponent_onAmsUpdate, LV_EVENT_MSG_RECEIVED, (void *)6);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_AmsSlot2_2, (void *)6);

    lv_obj_add_event_cb(cui_AmsSlot2_3, ui_event_comp_amsViewComponent_onAmsUpdate, LV_EVENT_MSG_RECEIVED, (void *)7);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_AmsSlot2_3, (void *)7);

    lv_obj_add_event_cb(cui_AmsSlot2_4, ui_event_comp_amsViewComponent_onAmsUpdate, LV_EVENT_MSG_RECEIVED, (void *)8);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_AmsSlot2_4, (void *)8);



    ui_comp_amsViewComponent_create_hook(cui_amsViewComponent);

    struct XTOUCH_MESSAGE_DATA eventData;
    eventData.data = 0;
    lv_msg_send(XTOUCH_ON_AMS_SLOT_UPDATE, &eventData);

    return cui_amsViewComponent;
}