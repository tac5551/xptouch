#include "../ui.h"
#include "../../xtouch/trays.h"
#include "../../ui/ui_msgs.h"

#define SLOT_COUNT 5
#define AMS_BORDER 3

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
    uintptr_t temp_user_data = (uintptr_t)lv_event_get_user_data(e);
    uint8_t user_data = (uint8_t)temp_user_data;

    uint8_t ams_id = user_data - 1;

    uint8_t check_bit = 0;
    if (ams_id == 0)
        check_bit = 0b00000001;
    if (ams_id == 1)
        check_bit = 0b00000010;
    if (ams_id == 2)
        check_bit = 0b00000100;
    if (ams_id == 3)
        check_bit = 0b00001000;

    // printf("onAMSBitsSlot user_data:%d ams_id:%d   ams_exist_bits:%08b = checkbit:%08b => %d\n", user_data, ams_id, bambuStatus.ams_exist_bits, check_bit, bambuStatus.ams_exist_bits & check_bit);

    if ((bambuStatus.ams_exist_bits & check_bit) == 0)
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

    // printf("onAmsHumidity %d\n", bambuStatus.ams_humidity);
    lv_obj_t *target = lv_event_get_target(e);
    uintptr_t temp_user_data = (uintptr_t)lv_event_get_user_data(e);
    uint8_t user_data = (uint8_t)temp_user_data;
    uint8_t ams_id = user_data - 1;
    char buffer[100];
    memset(buffer, 0, 100);
    sprintf(buffer, "H\n%d", bambuStatus.ams_humidity[ams_id]);
    // printf("onAmsHumidity %d %d\n", ams_id, bambuStatus.ams_humidity[ams_id]);

    lv_label_set_text(target, buffer);
}

void ui_event_comp_filamentComponent_onAmsUpdate(lv_event_t *e)
{

    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);
    uintptr_t temp_user_data = (uintptr_t)lv_event_get_user_data(e);
    uint8_t user_data = (uint8_t)temp_user_data;

    if (!(bambuStatus.ams_status_main == AMS_STATUS_MAIN_IDLE || bambuStatus.ams_status_main == AMS_STATUS_MAIN_ASSIST) || bambuStatus.print_status == XTOUCH_PRINT_STATUS_RUNNING)
    {
        lv_obj_add_state(target, LV_STATE_DISABLED);
    }
    else
    {
        lv_obj_clear_state(target, LV_STATE_DISABLED);
    }

    struct XTOUCH_MESSAGE_DATA *message = (struct XTOUCH_MESSAGE_DATA *)m->payload;

    uint8_t tmp_ams_id = user_data / 100;
    uint8_t tmp_tray_id = user_data % 100;

    // printf("onAmsUpdate %d %d %d\n", tmp_ams_id, tmp_tray_id, user_data);

    uint32_t tray_status = get_tray_status(tmp_ams_id, tmp_tray_id);
    uint16_t tray_id = ((tray_status >> 4) & 0x0F);
    uint16_t loaded = ((tray_status) & 0x01);
    char *tray_type = get_tray_type(tmp_ams_id, tmp_tray_id);
    // lv_obj_t *unload = ui_comp_get_child(target, UI_COMP_FILAMENTCOMPONENT_FILAMENTSCREENFILAMENT_FILAMENTSCREENUNLOAD);

    if (tmp_tray_id == tray_id)
    {
        lv_color_t color = lv_color_hex(tray_status >> 8);
        lv_color_t color_inv = lv_color_hex((0xFFFFFF - (tray_status >> 8)) & 0xFFFFFF);

        char buffer[100];
        memset(buffer, 0, 100);
   
        // tray_typeがnullポインタでなく、空文字列でもなく、文字列"null"でもない場合、その文字列を設定（優先）
        if (tray_type[0] != '\0' && strcmp(tray_type, "null") != 0) {
            lv_label_set_text(target, tray_type);
        } else {
            lv_label_set_text(target, "x");
        }

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
    lv_obj_set_width(cui_filamentComponent, lv_pct(100));
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

    lv_obj_t *cui_filamentControlComponent;
    cui_filamentControlComponent = lv_obj_create(cui_filamentComponent);
    lv_obj_set_width(cui_filamentControlComponent, lv_pct(100));
    lv_obj_set_flex_grow(cui_filamentControlComponent, 1);
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

    // left box
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

    // nozzle up
    lv_obj_t *cui_filamentScreenNozzleUp;
    cui_filamentScreenNozzleUp = lv_obj_create(cui_filamentScreenNozzle);
    lv_obj_set_height(cui_filamentScreenNozzleUp, lv_pct(40));
    lv_obj_set_width(cui_filamentScreenNozzleUp, lv_pct(100));
    lv_obj_set_flex_grow(cui_filamentScreenNozzleUp, 2);
    lv_obj_clear_flag(cui_filamentScreenNozzleUp, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenNozzleUp, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_filamentScreenNozzleUp, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_filamentScreenNozzleUp, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_filamentScreenNozzleUp, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenNozzleUp, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_filamentScreenNozzleUp, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_filamentScreenNozzleUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_filamentScreenNozzleUp, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_filamentScreenNozzleUp, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentScreenNozzleUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_filamentScreenNozzleUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenNozzleUp, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_filamentScreenNozzleUp, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_filamentScreenNozzleUpIcon;
    cui_filamentScreenNozzleUpIcon = lv_label_create(cui_filamentScreenNozzleUp);
    lv_obj_set_width(cui_filamentScreenNozzleUpIcon, LV_SIZE_CONTENT);  /// 100
    lv_obj_set_height(cui_filamentScreenNozzleUpIcon, LV_SIZE_CONTENT); /// 24
    lv_obj_set_align(cui_filamentScreenNozzleUpIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_filamentScreenNozzleUpIcon, "s");
    lv_obj_clear_flag(cui_filamentScreenNozzleUpIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenNozzleUpIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_filamentScreenNozzleUpIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    // nozzle temp
    lv_obj_t *cui_filamentScreenNozzleIcon;
    cui_filamentScreenNozzleIcon = lv_obj_create(cui_filamentScreenNozzle);
    lv_obj_set_height(cui_filamentScreenNozzleIcon, lv_pct(20));
    lv_obj_set_width(cui_filamentScreenNozzleIcon, lv_pct(100));
    lv_obj_set_flex_grow(cui_filamentScreenNozzleIcon, 1);
    lv_obj_clear_flag(cui_filamentScreenNozzleIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenNozzleIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_filamentScreenNozzleIcon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_filamentScreenNozzleIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_filamentScreenNozzleIcon, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenNozzleIcon, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_filamentScreenNozzleIcon, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_filamentScreenNozzleIcon, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_filamentScreenNozzleIcon, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_filamentScreenNozzleIcon, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentScreenNozzleIcon, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_filamentScreenNozzleIcon, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenNozzleIcon, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_filamentScreenNozzleIcon, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_add_flag(cui_filamentScreenNozzleIcon, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *cui_filamentScreenNozzleIcon2;
    cui_filamentScreenNozzleIcon2 = lv_label_create(cui_filamentScreenNozzleIcon);
    lv_label_set_text(cui_filamentScreenNozzleIcon2, "p");
    lv_obj_set_align(cui_filamentScreenNozzleIcon, LV_ALIGN_LEFT_MID);
    lv_obj_set_style_text_font(cui_filamentScreenNozzleIcon2, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentScreenNozzleIcon2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_filamentScreenNozzleTemp;
    cui_filamentScreenNozzleTemp = lv_label_create(cui_filamentScreenNozzleIcon);
    lv_label_set_text(cui_filamentScreenNozzleTemp, "");
    lv_obj_set_align(cui_filamentScreenNozzleTemp, LV_ALIGN_RIGHT_MID);
    lv_obj_set_style_text_font(cui_filamentScreenNozzleTemp, lv_font_big, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentScreenNozzleTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_filamentScreenNozzleDown;
    cui_filamentScreenNozzleDown = lv_obj_create(cui_filamentScreenNozzle);
    lv_obj_set_height(cui_filamentScreenNozzleDown, lv_pct(40));
    lv_obj_set_width(cui_filamentScreenNozzleDown, lv_pct(100));
    lv_obj_set_flex_grow(cui_filamentScreenNozzleDown, 2);
    lv_obj_clear_flag(cui_filamentScreenNozzleDown, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenNozzleDown, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_filamentScreenNozzleDown, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_filamentScreenNozzleDown, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_filamentScreenNozzleDown, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenNozzleDown, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_filamentScreenNozzleDown, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_filamentScreenNozzleDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_filamentScreenNozzleDown, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_filamentScreenNozzleDown, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentScreenNozzleDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_filamentScreenNozzleDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenNozzleDown, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_filamentScreenNozzleDown, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_filamentScreenNozzleDownIcon;
    cui_filamentScreenNozzleDownIcon = lv_label_create(cui_filamentScreenNozzleDown);
    lv_obj_set_width(cui_filamentScreenNozzleDownIcon, LV_SIZE_CONTENT);  /// 100
    lv_obj_set_height(cui_filamentScreenNozzleDownIcon, LV_SIZE_CONTENT); /// 24
    lv_obj_set_align(cui_filamentScreenNozzleDownIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_filamentScreenNozzleDownIcon, "t");
    lv_obj_clear_flag(cui_filamentScreenNozzleDownIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenNozzleDownIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_filamentScreenNozzleDownIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

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
    cui_filamentScreenUnload = lv_obj_create(cui_filamentScreenFilament);
    lv_obj_set_width(cui_filamentScreenUnload, lv_pct(100));
    lv_obj_set_flex_grow(cui_filamentScreenUnload, 2);
    lv_obj_clear_flag(cui_filamentScreenUnload, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenUnload, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_filamentScreenUnload, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_filamentScreenUnload, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_filamentScreenUnload, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenUnload, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_filamentScreenUnload, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_filamentScreenUnload, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_filamentScreenUnload, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_filamentScreenUnload, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentScreenUnload, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_filamentScreenUnload, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenUnload, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_filamentScreenUnload, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_filamentScreenUnloadIcon;
    cui_filamentScreenUnloadIcon = lv_label_create(cui_filamentScreenUnload);
    lv_obj_set_width(cui_filamentScreenUnloadIcon, LV_SIZE_CONTENT);  /// 100
    lv_obj_set_height(cui_filamentScreenUnloadIcon, LV_SIZE_CONTENT); /// 24
    lv_obj_set_align(cui_filamentScreenUnloadIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_filamentScreenUnloadIcon, "UNLOAD");
    lv_obj_clear_flag(cui_filamentScreenUnloadIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenUnloadIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_filamentScreenUnloadIcon, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_filamentScreenFilamentIcon;
    cui_filamentScreenFilamentIcon = lv_label_create(cui_filamentScreenFilament);
    lv_obj_set_width(cui_filamentScreenFilamentIcon, lv_pct(100));
    lv_obj_set_flex_grow(cui_filamentScreenFilamentIcon, 1);
    lv_obj_set_align(cui_filamentScreenFilamentIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_filamentScreenFilamentIcon, "n");
    lv_obj_clear_flag(cui_filamentScreenFilamentIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenFilamentIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_filamentScreenFilamentIcon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_filamentScreenFilamentIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
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
    cui_filamentScreenLoad = lv_obj_create(cui_filamentScreenFilament);
    lv_obj_set_width(cui_filamentScreenLoad, lv_pct(100));
    lv_obj_set_flex_grow(cui_filamentScreenLoad, 2);
    lv_obj_clear_flag(cui_filamentScreenLoad, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenLoad, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_filamentScreenLoad, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_filamentScreenLoad, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_filamentScreenLoad, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenLoad, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_filamentScreenLoad, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_filamentScreenLoad, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_filamentScreenLoad, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_filamentScreenLoad, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_filamentScreenLoad, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_filamentScreenLoad, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_filamentScreenLoad, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_filamentScreenLoad, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_filamentScreenLoadIcon;
    cui_filamentScreenLoadIcon = lv_label_create(cui_filamentScreenLoad);
    lv_obj_set_width(cui_filamentScreenLoadIcon, LV_SIZE_CONTENT);  /// 100
    lv_obj_set_height(cui_filamentScreenLoadIcon, LV_SIZE_CONTENT); /// 24
    lv_obj_set_align(cui_filamentScreenLoadIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_filamentScreenLoadIcon, "LOAD");
    lv_obj_clear_flag(cui_filamentScreenLoadIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_filamentScreenLoadIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_filamentScreenLoadIcon, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

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

    lv_obj_add_event_cb(cui_filamentControlComponent, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(cui_filamentControlComponent, del_component_child_event_cb, LV_EVENT_DELETE, children);

    // button click e
    lv_obj_add_event_cb(cui_filamentScreenNozzleUp, ui_event_comp_filamentComponent_filamentScreenNozzleUpClick, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_filamentScreenNozzleDown, ui_event_comp_filamentComponent_filamentScreenNozzleDownCick, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_filamentScreenUnload, ui_event_comp_filamentComponent_filamentScreenUnloadClick, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_filamentScreenLoad, ui_event_comp_filamentComponent_filamentScreenLoadClick, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_filamentScreenNozzleIcon, ui_event_comp_filamentComponent_onNozzleTempClick, LV_EVENT_ALL, children);

    // Contraler
    lv_obj_add_event_cb(cui_filamentScreenUnload, ui_event_comp_filamentComponent_onAmsState, LV_EVENT_MSG_RECEIVED, (void *)NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_STATE_UPDATE, cui_filamentScreenUnload, (void *)NULL);

    lv_obj_add_event_cb(cui_filamentScreenLoad, ui_event_comp_filamentComponent_onAmsState, LV_EVENT_MSG_RECEIVED, (void *)NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_STATE_UPDATE, cui_filamentScreenLoad, (void *)NULL);

    lv_obj_add_event_cb(cui_filamentScreenNozzleTemp, ui_event_comp_filamentComponent_onNozzleTemp, LV_EVENT_MSG_RECEIVED, (void *)NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_NOZZLE_TEMP, cui_filamentScreenNozzleTemp, (void *)NULL);

    ui_comp_filamentComponent_create_hook(cui_filamentControlComponent);

    return cui_filamentControlComponent;
}