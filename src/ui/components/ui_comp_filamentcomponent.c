#include "../ui.h"
#include "../../xtouch/trays.h"
#include "../../ui/ui_msgs.h"

#define SLOT_COUNT 5
#define AMS_BORDER 8

void ui_event_comp_filamentComponent_onNozzleTemp(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);

    struct XTOUCH_MESSAGE_DATA *message = (struct XTOUCH_MESSAGE_DATA *)m->payload;

    char value[10];
    itoa(message->data, value, 10);
    lv_label_set_text(target, value);
    lv_obj_set_style_text_color(target, message->data < 170 ? lv_color_hex(0x39a1fd) : lv_color_hex(0xfaa61e), LV_PART_MAIN | LV_STATE_DEFAULT);
}

void ui_event_comp_filamentComponent_filamentScreenNozzleUpClick(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_filamentComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onNozzleUp(e);
    }
}
void ui_event_comp_filamentComponent_filamentScreenNozzleDownCick(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_filamentComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onNozzleDown(e);
    }
}
void ui_event_comp_filamentComponent_filamentScreenUnloadClick(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_filamentComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onFilamentUnload(e);
    }
}

void ui_event_comp_filamentComponent_filamentScreenLoadClick(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_filamentComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onFilamentLoad(e);
    }
}

void ui_event_comp_filamentComponent_onAMSSlot1Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_filamentComponent_onAMSSlot1Click\n");
        onAmsSlotLoad(e, 1);
    }
}

void ui_event_comp_filamentComponent_onAMSSlot2Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_filamentComponent_onAMSSlot2Click\n");
        onAmsSlotLoad(e, 2);
    }
}

void ui_event_comp_filamentComponent_onAMSSlot3Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_filamentComponent_onAMSSlot3Click\n");
        onAmsSlotLoad(e, 3);
    }
}

void ui_event_comp_filamentComponent_onAMSSlot4Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_filamentComponent_onAMSSlot4Click\n");
        onAmsSlotLoad(e, 4);
    }
}

void onAmsLoad(lv_event_t *e)
{
    void *user_data = lv_event_get_user_data(e);
    lv_msg_send(XTOUCH_COMMAND_AMS_LOAD_SLOT, user_data);
}

void ui_event_comp_filamentComponent_amsLoad(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onAmsLoad(e);
    }
}

void ui_filamentComponent_onAMSBitsSlot(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);

    if (bambuStatus.ams_exist_bits == 0)
    {
        lv_obj_add_flag(target, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_clear_flag(target, LV_OBJ_FLAG_HIDDEN);
    }
}
void ui_filamentComponent_onAMSBits(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);

    lv_obj_t *unload = ui_comp_get_child(target, UI_COMP_FILAMENTCOMPONENT_FILAMENTSCREENFILAMENT_FILAMENTSCREENUNLOAD);

    if (bambuStatus.ams_exist_bits != 0 && bambuStatus.ams_status_main == AMS_STATUS_MAIN_IDLE && bambuStatus.hw_switch_state == 1 && bambuStatus.m_tray_now == 255)
    {
        lv_obj_clear_state(unload, LV_STATE_DISABLED);
    }
    else
    {
        lv_obj_add_state(unload, LV_STATE_DISABLED);
    }
}

void ui_event_comp_filamentComponent_onNozzleTempClick(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onHomeNozzleTemp(e, 3);
    }
}

void ui_event_comp_filamentComponent_onAmsState(lv_event_t *e)
{

    printf("onAmsState\n");
    lv_obj_t *target = lv_event_get_target(e);

    if (!(bambuStatus.ams_status_main == AMS_STATUS_MAIN_IDLE || bambuStatus.ams_status_main == AMS_STATUS_MAIN_ASSIST))
    {
        lv_obj_add_state(target, LV_STATE_DISABLED);
    }
    else
    {
        lv_obj_clear_state(target, LV_STATE_DISABLED);
    }
}

void ui_event_comp_filamentComponent_onAmsHumidity(lv_event_t *e)
{

    printf("onAmsHumidity %d\n", bambuStatus.ams_humidity);
    lv_obj_t *target = lv_event_get_target(e);
    char buffer[100];
    memset(buffer, 0, 100);
    sprintf(buffer, "H\n%d", bambuStatus.ams_humidity);
    lv_label_set_text(target, buffer);
}

void ui_event_comp_filamentComponent_onAmsUpdate(lv_event_t *e)
{

    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);
    uintptr_t temp_user_data = (uintptr_t)lv_event_get_user_data(e);
    uint8_t user_data = (uint8_t)temp_user_data;
    printf("onAmsUpdate %d\n", user_data);

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

    // lv_obj_t *unload = ui_comp_get_child(target, UI_COMP_FILAMENTCOMPONENT_FILAMENTSCREENFILAMENT_FILAMENTSCREENUNLOAD);

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

// COMPONENT filamentComponent

lv_obj_t *ui_filamentComponent_create(lv_obj_t *comp_parent)
{

    lv_obj_t *cui_filamentComponent;
    cui_filamentComponent = lv_obj_create(comp_parent);
    lv_obj_set_height(cui_filamentComponent, lv_pct(100));
    lv_obj_set_flex_grow(cui_filamentComponent, 1);
    lv_obj_set_x(cui_filamentComponent, 386);
    lv_obj_set_y(cui_filamentComponent, 178);
    lv_obj_set_flex_flow(cui_filamentComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_filamentComponent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_filamentComponent, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentComponent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_filamentComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentComponent, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_filamentComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_filamentComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_filamentComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_filamentComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_filamentComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_filamentComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_filamentComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_filamentComponent, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_filamentComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsControl;
    cui_AmsControl = lv_obj_create(cui_filamentComponent);
    lv_obj_set_width(cui_AmsControl, lv_pct(100));
    lv_obj_set_height(cui_AmsControl, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsControl, 1);
    lv_obj_set_x(cui_AmsControl, 386);
    lv_obj_set_y(cui_AmsControl, 178);
    lv_obj_set_flex_flow(cui_AmsControl, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_AmsControl, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_AmsControl, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsControl, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_AmsControl, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_AmsControl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsControl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsControl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsControl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsControl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsControl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_AmsControl, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_AmsControl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsControl, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_AmsControl, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_filamentControlComponent;
    cui_filamentControlComponent = lv_obj_create(cui_filamentComponent);
    lv_obj_set_height(cui_filamentControlComponent, lv_pct(100));
    lv_obj_set_width(cui_filamentControlComponent, lv_pct(100));
    lv_obj_set_flex_grow(cui_filamentControlComponent, 4);
    lv_obj_set_x(cui_filamentControlComponent, 386);
    lv_obj_set_y(cui_filamentControlComponent, 178);
    lv_obj_set_flex_flow(cui_filamentControlComponent, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_filamentControlComponent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_filamentControlComponent, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentControlComponent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_filamentControlComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentControlComponent, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_filamentControlComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_filamentControlComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_filamentControlComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_filamentControlComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentControlComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_filamentControlComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_filamentControlComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_filamentControlComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_filamentControlComponent, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_filamentControlComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsHumid;
    cui_AmsHumid = lv_label_create(cui_AmsControl);
    lv_obj_set_width(cui_AmsHumid, lv_pct(100));
    lv_obj_set_height(cui_AmsHumid, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsHumid, 1);
    lv_obj_set_align(cui_AmsHumid, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsHumid, "H\nX");
    lv_obj_clear_flag(cui_AmsHumid, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsHumid, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsHumid, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsHumid, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsHumid, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsHumid, lv_color_hex(0x41ADDC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_AmsHumid, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsHumid, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_text_color(cui_AmsHumid, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsHumid, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsHumid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsHumid, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsHumid, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsHumid, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsHumid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsHumid, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsHumid, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsHumid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    char buffer[100];
    memset(buffer, 0, 100);
    sprintf(buffer, "H\n%d", bambuStatus.ams_humidity);
    lv_label_set_text(cui_AmsHumid, buffer);

    lv_obj_t *cui_AmsSlot1;
    cui_AmsSlot1 = lv_label_create(cui_AmsControl);
    lv_obj_set_width(cui_AmsSlot1, lv_pct(100));
    lv_obj_set_height(cui_AmsSlot1, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsSlot1, 2);
    lv_obj_set_align(cui_AmsSlot1, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsSlot1, "Slot 1");
    lv_obj_add_flag(cui_AmsSlot1, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(cui_AmsSlot1, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsSlot1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsSlot1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsSlot1, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsSlot1, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_AmsSlot1, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsSlot1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsSlot1, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsSlot1, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsSlot1, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsSlot1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsSlot1, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsSlot2;
    cui_AmsSlot2 = lv_label_create(cui_AmsControl);
    lv_obj_set_width(cui_AmsSlot2, lv_pct(100));
    lv_obj_set_height(cui_AmsSlot2, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsSlot2, 2);
    lv_obj_set_align(cui_AmsSlot2, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsSlot2, "Slot 2");
    lv_obj_add_flag(cui_AmsSlot2, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_AmsSlot2, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsSlot2, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsSlot2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsSlot2, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsSlot2, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_AmsSlot2, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot2, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsSlot2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsSlot2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsSlot2, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsSlot2, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsSlot2, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsSlot2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot2, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsSlot2, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsSlot2, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsSlot3;
    cui_AmsSlot3 = lv_label_create(cui_AmsControl);
    lv_obj_set_width(cui_AmsSlot3, lv_pct(100));
    lv_obj_set_height(cui_AmsSlot3, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsSlot3, 2);
    lv_obj_set_align(cui_AmsSlot3, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsSlot3, "Slot 3");
    lv_obj_add_flag(cui_AmsSlot3, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_AmsSlot3, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsSlot3, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsSlot3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsSlot3, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsSlot3, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_AmsSlot3, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot3, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsSlot3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsSlot3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsSlot3, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsSlot3, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsSlot3, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsSlot3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot3, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsSlot3, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsSlot3, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsSlot4;
    cui_AmsSlot4 = lv_label_create(cui_AmsControl);
    lv_obj_set_width(cui_AmsSlot4, lv_pct(100));
    lv_obj_set_height(cui_AmsSlot4, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsSlot4, 2);
    lv_obj_set_align(cui_AmsSlot4, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsSlot4, "Slot 4");
    lv_obj_add_flag(cui_AmsSlot4, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_AmsSlot4, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsSlot4, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsSlot4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsSlot4, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsSlot4, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_AmsSlot4, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot4, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsSlot4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsSlot4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsSlot4, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsSlot4, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsSlot4, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsSlot4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot4, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsSlot4, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsSlot4, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_filamentScreenNozzle;
    cui_filamentScreenNozzle = lv_obj_create(cui_filamentControlComponent);
    lv_obj_set_height(cui_filamentScreenNozzle, lv_pct(100));
    lv_obj_set_flex_grow(cui_filamentScreenNozzle, 2);
    lv_obj_set_x(cui_filamentScreenNozzle, 386);
    lv_obj_set_y(cui_filamentScreenNozzle, 178);
    lv_obj_set_flex_flow(cui_filamentScreenNozzle, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_filamentScreenNozzle, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_filamentScreenNozzle, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenNozzle, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_filamentScreenNozzle, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_filamentScreenNozzle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_filamentScreenNozzle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_filamentScreenNozzle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_filamentScreenNozzle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentScreenNozzle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_filamentScreenNozzle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_filamentScreenNozzle, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_filamentScreenNozzle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_filamentScreenNozzle, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_filamentScreenNozzle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_filamentScreenNozzleUp;
    cui_filamentScreenNozzleUp = lv_label_create(cui_filamentScreenNozzle);
    lv_obj_set_width(cui_filamentScreenNozzleUp, lv_pct(100));
    lv_obj_set_flex_grow(cui_filamentScreenNozzleUp, 2);
    lv_obj_set_align(cui_filamentScreenNozzleUp, LV_ALIGN_CENTER);
    lv_label_set_text(cui_filamentScreenNozzleUp, "s");
    lv_obj_add_flag(cui_filamentScreenNozzleUp, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_filamentScreenNozzleUp, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenNozzleUp, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_filamentScreenNozzleUp, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_filamentScreenNozzleUp, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_filamentScreenNozzleUp, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenNozzleUp, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_filamentScreenNozzleUp, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_filamentScreenNozzleUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_filamentScreenNozzleUp, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_filamentScreenNozzleUp, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentScreenNozzleUp, 14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_filamentScreenNozzleUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenNozzleUp, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_filamentScreenNozzleUp, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_filamentScreenNozzleIcon;
    cui_filamentScreenNozzleIcon = lv_label_create(cui_filamentScreenNozzle);
    lv_obj_set_width(cui_filamentScreenNozzleIcon, lv_pct(100));
    lv_obj_set_flex_grow(cui_filamentScreenNozzleIcon, 1);
    lv_obj_set_align(cui_filamentScreenNozzleIcon, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(cui_filamentScreenNozzleIcon, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_filamentScreenNozzleIcon, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_label_set_text(cui_filamentScreenNozzleIcon, "p");
    lv_obj_clear_flag(cui_filamentScreenNozzleIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenNozzleIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_filamentScreenNozzleIcon, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_filamentScreenNozzleIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_filamentScreenNozzleIcon, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenNozzleIcon, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_filamentScreenNozzleIcon, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_filamentScreenNozzleIcon, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_filamentScreenNozzleIcon, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_filamentScreenNozzleIcon, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentScreenNozzleIcon, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_filamentScreenNozzleIcon, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenNozzleIcon, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_filamentScreenNozzleIcon, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_add_flag(cui_filamentScreenNozzleIcon, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *cui_filamentScreenNozzleTemp;
    cui_filamentScreenNozzleTemp = lv_label_create(cui_filamentScreenNozzleIcon);
    lv_label_set_text(cui_filamentScreenNozzleTemp, "");
    lv_obj_set_style_text_font(cui_filamentScreenNozzleTemp, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentScreenNozzleTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_filamentScreenNozzleDown;
    cui_filamentScreenNozzleDown = lv_label_create(cui_filamentScreenNozzle);
    lv_obj_set_width(cui_filamentScreenNozzleDown, lv_pct(100));
    lv_obj_set_flex_grow(cui_filamentScreenNozzleDown, 2);
    lv_obj_set_align(cui_filamentScreenNozzleDown, LV_ALIGN_CENTER);
    lv_label_set_text(cui_filamentScreenNozzleDown, "t");
    lv_obj_add_flag(cui_filamentScreenNozzleDown, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_filamentScreenNozzleDown, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenNozzleDown, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_filamentScreenNozzleDown, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_filamentScreenNozzleDown, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_filamentScreenNozzleDown, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenNozzleDown, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_filamentScreenNozzleDown, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_filamentScreenNozzleDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_filamentScreenNozzleDown, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_filamentScreenNozzleDown, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentScreenNozzleDown, 14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_filamentScreenNozzleDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenNozzleDown, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_filamentScreenNozzleDown, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_filamentScreenFilament;
    cui_filamentScreenFilament = lv_obj_create(cui_filamentControlComponent);
    lv_obj_set_height(cui_filamentScreenFilament, lv_pct(100));
    lv_obj_set_flex_grow(cui_filamentScreenFilament, 2);
    lv_obj_set_x(cui_filamentScreenFilament, 386);
    lv_obj_set_y(cui_filamentScreenFilament, 178);
    lv_obj_set_flex_flow(cui_filamentScreenFilament, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_filamentScreenFilament, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_filamentScreenFilament, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenFilament, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_filamentScreenFilament, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_filamentScreenFilament, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_filamentScreenFilament, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_filamentScreenFilament, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_filamentScreenFilament, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentScreenFilament, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_filamentScreenFilament, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_filamentScreenFilament, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_filamentScreenFilament, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_filamentScreenFilament, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_filamentScreenFilament, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_filamentScreenUnload;
    cui_filamentScreenUnload = lv_label_create(cui_filamentScreenFilament);
    lv_obj_set_width(cui_filamentScreenUnload, lv_pct(100));
    lv_obj_set_flex_grow(cui_filamentScreenUnload, 2);
    lv_obj_set_align(cui_filamentScreenUnload, LV_ALIGN_CENTER);
    lv_label_set_text(cui_filamentScreenUnload, "UNLOAD");
    lv_obj_add_flag(cui_filamentScreenUnload, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_filamentScreenUnload, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenUnload, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_filamentScreenUnload, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_filamentScreenUnload, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_filamentScreenUnload, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_filamentScreenUnload, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenUnload, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenUnload, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_filamentScreenUnload, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_filamentScreenUnload, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_filamentScreenUnload, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_filamentScreenUnload, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentScreenUnload, 28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_filamentScreenUnload, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenUnload, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_filamentScreenUnload, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_filamentScreenFilamentIcon;
    cui_filamentScreenFilamentIcon = lv_label_create(cui_filamentScreenFilament);
    lv_obj_set_width(cui_filamentScreenFilamentIcon, lv_pct(100));
    lv_obj_set_flex_grow(cui_filamentScreenFilamentIcon, 1);
    lv_obj_set_align(cui_filamentScreenFilamentIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_filamentScreenFilamentIcon, "n");
    lv_obj_clear_flag(cui_filamentScreenFilamentIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenFilamentIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_filamentScreenFilamentIcon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_filamentScreenFilamentIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_filamentScreenFilamentIcon, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenFilamentIcon, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_filamentScreenFilamentIcon, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_filamentScreenFilamentIcon, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_filamentScreenFilamentIcon, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_filamentScreenFilamentIcon, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentScreenFilamentIcon, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_filamentScreenFilamentIcon, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenFilamentIcon, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_filamentScreenFilamentIcon, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_filamentScreenLoad;
    cui_filamentScreenLoad = lv_label_create(cui_filamentScreenFilament);
    lv_obj_set_width(cui_filamentScreenLoad, lv_pct(100));
    lv_obj_set_flex_grow(cui_filamentScreenLoad, 2);
    lv_obj_set_align(cui_filamentScreenLoad, LV_ALIGN_CENTER);
    lv_label_set_text(cui_filamentScreenLoad, "LOAD");
    lv_obj_add_flag(cui_filamentScreenLoad, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_filamentScreenLoad, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenLoad, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_filamentScreenLoad, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_filamentScreenLoad, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_filamentScreenLoad, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_filamentScreenLoad, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenLoad, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenLoad, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_filamentScreenLoad, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_filamentScreenLoad, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_filamentScreenLoad, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_filamentScreenLoad, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentScreenLoad, 28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_filamentScreenLoad, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenLoad, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_filamentScreenLoad, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t **children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_FILAMENTCOMPONENT_NUM);
    children[UI_COMP_FILAMENTCOMPONENT_FILAMENTCOMPONENT] = cui_filamentControlComponent;
    children[UI_COMP_FILAMENTCOMPONENT_FILAMENTSCREENNOZZLE] = cui_filamentScreenNozzle;
    children[UI_COMP_FILAMENTCOMPONENT_FILAMENTSCREENNOZZLE_FILAMENTSCREENNOZZLEUP] = cui_filamentScreenNozzleUp;
    children[UI_COMP_FILAMENTCOMPONENT_FILAMENTSCREENNOZZLE_FILAMENTSCREENNOZZLEICON] = cui_filamentScreenNozzleIcon;
    children[UI_COMP_FILAMENTCOMPONENT_FILAMENTSCREENNOZZLE_FILAMENTSCREENNOZZLETEMP] = cui_filamentScreenNozzleTemp;
    children[UI_COMP_FILAMENTCOMPONENT_FILAMENTSCREENNOZZLE_FILAMENTSCREENNOZZLEDOWN] = cui_filamentScreenNozzleDown;
    children[UI_COMP_FILAMENTCOMPONENT_FILAMENTSCREENFILAMENT] = cui_filamentScreenFilament;
    children[UI_COMP_FILAMENTCOMPONENT_FILAMENTSCREENFILAMENT_FILAMENTSCREENUNLOAD] = cui_filamentScreenUnload;
    children[UI_COMP_FILAMENTCOMPONENT_FILAMENTSCREENFILAMENT_FILAMENTSCREENFILAMENTICON] = cui_filamentScreenFilamentIcon;
    children[UI_COMP_FILAMENTCOMPONENT_FILAMENTSCREENFILAMENT_FILAMENTSCREENLOAD] = cui_filamentScreenLoad;
    children[UI_COMP_FILAMENTCOMPONENT_FILAMENTSCREENFILAMENT_FILAMENTSCREENAMSSLOT1] = cui_AmsSlot1;
    children[UI_COMP_FILAMENTCOMPONENT_FILAMENTSCREENFILAMENT_FILAMENTSCREENAMSSLOT2] = cui_AmsSlot2;
    children[UI_COMP_FILAMENTCOMPONENT_FILAMENTSCREENFILAMENT_FILAMENTSCREENAMSSLOT3] = cui_AmsSlot3;
    children[UI_COMP_FILAMENTCOMPONENT_FILAMENTSCREENFILAMENT_FILAMENTSCREENAMSSLOT4] = cui_AmsSlot4;

    lv_obj_add_event_cb(cui_filamentControlComponent, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(cui_filamentControlComponent, del_component_child_event_cb, LV_EVENT_DELETE, children);

    // button click e
    lv_obj_add_event_cb(cui_filamentScreenNozzleUp, ui_event_comp_filamentComponent_filamentScreenNozzleUpClick, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_filamentScreenNozzleDown, ui_event_comp_filamentComponent_filamentScreenNozzleDownCick, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_filamentScreenUnload, ui_event_comp_filamentComponent_filamentScreenUnloadClick, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_filamentScreenLoad, ui_event_comp_filamentComponent_filamentScreenLoadClick, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_filamentScreenNozzleIcon, ui_event_comp_filamentComponent_onNozzleTempClick, LV_EVENT_ALL, children);

    lv_obj_add_event_cb(cui_AmsSlot1, ui_event_comp_filamentComponent_onAMSSlot1Click, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_AmsSlot2, ui_event_comp_filamentComponent_onAMSSlot2Click, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_AmsSlot3, ui_event_comp_filamentComponent_onAMSSlot3Click, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_AmsSlot4, ui_event_comp_filamentComponent_onAMSSlot4Click, LV_EVENT_ALL, children);

    lv_obj_add_event_cb(cui_AmsHumid, ui_event_comp_filamentComponent_onAmsHumidity, LV_EVENT_MSG_RECEIVED, (void *)NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_HUMIDITY_UPDATE, cui_AmsHumid, (void *)NULL);

    lv_obj_add_event_cb(cui_AmsSlot1, ui_event_comp_filamentComponent_onAmsUpdate, LV_EVENT_MSG_RECEIVED, (void *)1);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_AmsSlot1, (void *)1);

    lv_obj_add_event_cb(cui_AmsSlot2, ui_event_comp_filamentComponent_onAmsUpdate, LV_EVENT_MSG_RECEIVED, (void *)2);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_AmsSlot2, (void *)2);

    lv_obj_add_event_cb(cui_AmsSlot3, ui_event_comp_filamentComponent_onAmsUpdate, LV_EVENT_MSG_RECEIVED, (void *)3);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_AmsSlot3, (void *)3);

    lv_obj_add_event_cb(cui_AmsSlot4, ui_event_comp_filamentComponent_onAmsUpdate, LV_EVENT_MSG_RECEIVED, (void *)4);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_AmsSlot4, (void *)4);

    lv_obj_add_event_cb(cui_filamentScreenUnload, ui_event_comp_filamentComponent_onAmsState, LV_EVENT_MSG_RECEIVED, (void *)NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_STATE_UPDATE, cui_filamentScreenUnload, (void *)NULL);

    lv_obj_add_event_cb(cui_filamentScreenLoad, ui_event_comp_filamentComponent_onAmsState, LV_EVENT_MSG_RECEIVED, (void *)NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_STATE_UPDATE, cui_filamentScreenLoad, (void *)NULL);

    lv_obj_add_event_cb(cui_AmsControl, ui_filamentComponent_onAMSBitsSlot, LV_EVENT_MSG_RECEIVED, (void *)NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_BITS, cui_AmsControl, (void *)NULL);

    lv_obj_add_event_cb(cui_AmsSlot1, ui_filamentComponent_onAMSBitsSlot, LV_EVENT_MSG_RECEIVED, (void *)NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_BITS, cui_AmsSlot1, (void *)NULL);

    lv_obj_add_event_cb(cui_AmsSlot2, ui_filamentComponent_onAMSBitsSlot, LV_EVENT_MSG_RECEIVED, (void *)NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_BITS, cui_AmsSlot2, (void *)NULL);

    lv_obj_add_event_cb(cui_AmsSlot3, ui_filamentComponent_onAMSBitsSlot, LV_EVENT_MSG_RECEIVED, (void *)NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_BITS, cui_AmsSlot3, (void *)NULL);

    lv_obj_add_event_cb(cui_AmsSlot4, ui_filamentComponent_onAMSBitsSlot, LV_EVENT_MSG_RECEIVED, (void *)NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_BITS, cui_AmsSlot4, (void *)NULL);

    lv_obj_add_event_cb(cui_AmsControl, ui_event_comp_filamentComponent_onAmsUpdate, LV_EVENT_MSG_RECEIVED, children);
    lv_msg_subsribe_obj(XTOUCH_ON_PRINT_STATUS, cui_AmsControl, (void *)NULL);

    lv_obj_add_event_cb(cui_AmsSlot1, ui_event_comp_filamentComponent_onAmsUpdate, LV_EVENT_MSG_RECEIVED, children);
    lv_msg_subsribe_obj(XTOUCH_ON_PRINT_STATUS, cui_AmsSlot1, (void *)NULL);

    lv_obj_add_event_cb(cui_AmsSlot2, ui_event_comp_filamentComponent_onAmsUpdate, LV_EVENT_MSG_RECEIVED, children);
    lv_msg_subsribe_obj(XTOUCH_ON_PRINT_STATUS, cui_AmsSlot2, (void *)NULL);

    lv_obj_add_event_cb(cui_AmsSlot3, ui_event_comp_filamentComponent_onAmsUpdate, LV_EVENT_MSG_RECEIVED, children);
    lv_msg_subsribe_obj(XTOUCH_ON_PRINT_STATUS, cui_AmsSlot3, (void *)NULL);

    lv_obj_add_event_cb(cui_AmsSlot4, ui_filamentComponent_onAMSBitsSlot, LV_EVENT_MSG_RECEIVED, children);
    lv_msg_subsribe_obj(XTOUCH_ON_PRINT_STATUS, cui_AmsSlot4, (void *)NULL);

    lv_obj_add_event_cb(cui_filamentControlComponent, ui_filamentComponent_onAMSBits, LV_EVENT_MSG_RECEIVED, (void *)NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_BITS, cui_filamentControlComponent, (void *)NULL);

    lv_obj_add_event_cb(cui_filamentScreenNozzleTemp, ui_event_comp_filamentComponent_onNozzleTemp, LV_EVENT_MSG_RECEIVED, (void *)NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_NOZZLE_TEMP, cui_filamentScreenNozzleTemp, (void *)NULL);

    ui_comp_filamentComponent_create_hook(cui_filamentControlComponent);

    struct XTOUCH_MESSAGE_DATA eventData;
    eventData.data = 0;
    lv_msg_send(XTOUCH_ON_AMS_SLOT_UPDATE, &eventData);

    return cui_filamentControlComponent;
}