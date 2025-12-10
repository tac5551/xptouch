#include "../ui.h"
#include "../../xtouch/trays.h"
#include "../../ui/ui_msgs.h"


void ui_event_comp_UtilComponent_onButton1Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_UtilComponent_onButton1Click\n");
        onMoveNozzleScreen(e);
    }
}


void ui_event_comp_UtilComponent_onButton3Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_UtilComponent_onButton3Click\n");
        onMoveAmsViewScreen(e);
    }
}


void onXTouchUtilPrintStatus(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_utilComponent = lv_event_get_user_data(e);

    ui_confirmPanel_hide(); // hide confirm panel if new data comes

    lv_obj_t *button1 = comp_utilComponent[UI_COMP_UTILCOMPONENT_UTILCOMPONENTPRINTERCONFIG_BUTTON1];
    lv_obj_t *button2 = comp_utilComponent[UI_COMP_UTILCOMPONENT_UTILCOMPONENTPRINTERCONFIG_BUTTON2];
    lv_obj_t *button3 = comp_utilComponent[UI_COMP_UTILCOMPONENT_UTILCOMPONENTPRINTERCONFIG_BUTTON3];
    lv_obj_t *button4 = comp_utilComponent[UI_COMP_UTILCOMPONENT_UTILCOMPONENTPRINTERCONFIG_BUTTON4];

    switch (bambuStatus.print_status)
    {
    case XTOUCH_PRINT_STATUS_PAUSED:

        lv_obj_add_state(button1, LV_STATE_DISABLED);
        lv_obj_add_state(button2, LV_STATE_DISABLED);
        lv_obj_add_state(button3, LV_STATE_DISABLED);
        lv_obj_add_state(button4, LV_STATE_DISABLED);

        break;
    case XTOUCH_PRINT_STATUS_RUNNING:

        lv_obj_add_state(button1, LV_STATE_DISABLED);
        lv_obj_add_state(button2, LV_STATE_DISABLED);
        lv_obj_add_state(button3, LV_STATE_DISABLED);
        lv_obj_add_state(button4, LV_STATE_DISABLED);
        break;
    case XTOUCH_PRINT_STATUS_PREPARE:
        lv_obj_add_state(button1, LV_STATE_DISABLED);
        lv_obj_add_state(button2, LV_STATE_DISABLED);
        lv_obj_add_state(button3, LV_STATE_DISABLED);
        lv_obj_add_state(button4, LV_STATE_DISABLED);
        break;

    case XTOUCH_PRINT_STATUS_IDLE:
    case XTOUCH_PRINT_STATUS_FINISHED:
    case XTOUCH_PRINT_STATUS_FAILED:
        lv_obj_clear_state(button1, LV_STATE_DISABLED);
        lv_obj_clear_state(button2, LV_STATE_DISABLED);
        lv_obj_clear_state(button3, LV_STATE_DISABLED);
        lv_obj_clear_state(button4, LV_STATE_DISABLED);
        break;
    }
}


lv_obj_t *ui_utilComponent_create(lv_obj_t *comp_parent)
{
    lv_obj_t *cui_utilComponent;
    cui_utilComponent = lv_obj_create(comp_parent);
    lv_obj_set_height(cui_utilComponent, lv_pct(100));
    lv_obj_set_flex_grow(cui_utilComponent, 1);
    lv_obj_set_x(cui_utilComponent, 386);
    lv_obj_set_y(cui_utilComponent, 178);
    lv_obj_set_flex_flow(cui_utilComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_utilComponent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_utilComponent, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_utilComponent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_utilComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_utilComponent, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_utilComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_utilComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_utilComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_utilComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_utilComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_utilComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_utilComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_utilComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_utilComponent, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_utilComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_UtilLine1;
    cui_UtilLine1 = lv_obj_create(cui_utilComponent);
    lv_obj_set_width(cui_UtilLine1, lv_pct(100));
    lv_obj_set_height(cui_UtilLine1, lv_pct(50));
    lv_obj_set_flex_grow(cui_UtilLine1, 1);
    lv_obj_set_x(cui_UtilLine1, 386);
    lv_obj_set_y(cui_UtilLine1, 178);
    lv_obj_set_flex_flow(cui_UtilLine1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_UtilLine1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_UtilLine1, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_UtilLine1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_UtilLine1, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_UtilLine1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_UtilLine1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_UtilLine1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_UtilLine1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_UtilLine1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_UtilLine1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_UtilLine1, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_UtilLine1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_UtilLine1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_UtilLine1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_UtilLine2;
    cui_UtilLine2 = lv_obj_create(cui_utilComponent);
    lv_obj_set_width(cui_UtilLine2, lv_pct(100));
    lv_obj_set_height(cui_UtilLine2, lv_pct(50));
    lv_obj_set_flex_grow(cui_UtilLine2, 1);
    lv_obj_set_x(cui_UtilLine2, 386);
    lv_obj_set_y(cui_UtilLine2, 178);
    lv_obj_set_flex_flow(cui_UtilLine2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_UtilLine2, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_UtilLine2, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_UtilLine2, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_UtilLine2, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_UtilLine2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_UtilLine2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_UtilLine2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_UtilLine2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_UtilLine2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_UtilLine2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_UtilLine2, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_UtilLine2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_UtilLine2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_UtilLine2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *button1;
    button1 = lv_label_create(cui_UtilLine1);
    lv_obj_set_width(button1, lv_pct(50));
    lv_obj_set_height(button1, lv_pct(100));
    lv_obj_set_flex_grow(button1, 2);
    lv_obj_set_align(button1, LV_ALIGN_CENTER);
    lv_label_set_text(button1, "Nozzle\nSelect");
    lv_obj_add_flag(button1, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(button1, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(button1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(button1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(button1, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(button1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(button1, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(button1, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(button1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(button1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(button1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(button1, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(button1, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(button1, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(button1, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(button1, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(button1, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *button2;
    button2 = lv_label_create(cui_UtilLine1);
    lv_obj_set_width(button2, lv_pct(50));
    lv_obj_set_height(button2, lv_pct(100));
    lv_obj_set_flex_grow(button2, 2);
    lv_obj_set_align(button2, LV_ALIGN_CENTER);
    lv_label_set_text(button2, "Calibration");
    lv_obj_add_flag(button2, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(button2, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(button2, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(button2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(button2, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(button2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(button2, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(button2, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(button2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(button2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(button2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(button2, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(button2, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(button2, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(button2, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(button2, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(button2, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *button3;
    button3 = lv_label_create(cui_UtilLine2);
    lv_obj_set_width(button3, lv_pct(50));
    lv_obj_set_height(button3, lv_pct(100));
    lv_obj_set_flex_grow(button3, 2);
    lv_obj_set_align(button3, LV_ALIGN_CENTER);
    lv_label_set_text(button3, "AMS");
    lv_obj_add_flag(button3, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(button3, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(button3, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(button3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(button3, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(button3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(button3, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(button3, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(button3, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(button3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(button3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(button3, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(button3, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(button3, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(button3, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(button3, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(button3, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *button4;
    button4 = lv_label_create(cui_UtilLine2);
    lv_obj_set_width(button4, lv_pct(50));
    lv_obj_set_height(button4, lv_pct(100));
    lv_obj_set_flex_grow(button4, 2);
    lv_obj_set_align(button4, LV_ALIGN_CENTER);
    lv_label_set_text(button4, "Dummy4");
    lv_obj_add_flag(button4, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(button4, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(button4, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(button4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(button4, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(button4, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(button4, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(button4, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(button4, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(button4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(button4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(button4, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(button4, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(button4, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(button4, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(button4, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(button4, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_state(button4, LV_STATE_DISABLED);


    lv_obj_t **children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_UTILCOMPONENT_NUM);
    children[UI_COMP_UTILCOMPONENT_UTILCOMPONENT] = cui_utilComponent;
    children[UI_COMP_UTILCOMPONENT_UTILCOMPONENTPRINTERCONFIG_LINE1] = cui_UtilLine1;
    children[UI_COMP_UTILCOMPONENT_UTILCOMPONENTPRINTERCONFIG_LINE2] = cui_UtilLine2;
    children[UI_COMP_UTILCOMPONENT_UTILCOMPONENTPRINTERCONFIG_BUTTON1] = button1;    
    children[UI_COMP_UTILCOMPONENT_UTILCOMPONENTPRINTERCONFIG_BUTTON2] = button2;    
    children[UI_COMP_UTILCOMPONENT_UTILCOMPONENTPRINTERCONFIG_BUTTON3] = button3;    
    children[UI_COMP_UTILCOMPONENT_UTILCOMPONENTPRINTERCONFIG_BUTTON4] = button4;    

    lv_obj_add_event_cb(cui_utilComponent, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(cui_utilComponent, del_component_child_event_cb, LV_EVENT_DELETE, children);
    
    // for NozzleSelect
    lv_obj_add_event_cb(button1, ui_event_comp_UtilComponent_onButton1Click, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(button3, ui_event_comp_UtilComponent_onButton3Click, LV_EVENT_ALL, children);

 lv_obj_add_event_cb(cui_utilComponent, onXTouchUtilPrintStatus, LV_EVENT_MSG_RECEIVED, children);


    ui_comp_utilComponent_create_hook(cui_utilComponent);

    return cui_utilComponent;
}