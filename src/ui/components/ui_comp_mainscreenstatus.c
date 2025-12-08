#include "../ui.h"

// COMPONENT mainScreenStatus
void ui_event_comp_mainScreenStatusComponent_onPreHeatButton1Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onAMSSlot1_1Click\n");
        onPreHeatPLA(e);
    }
}

void ui_event_comp_mainScreenStatusComponent_onPreHeatButton2Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_mainScreenStatusComponent_onPreHeatButton2Click\n");
        onPreHeatABS(e);
    }
}

void ui_event_comp_mainScreenStatusComponent_onPreHeatButton3Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_mainScreenStatusComponent_onPreHeatButton3Click\n");
        onPreHeatOff(e);
    }
}

lv_obj_t *ui_mainScreenStatus_create(lv_obj_t *comp_parent)
{

    lv_obj_t *cui_mainScreenStatus;
    cui_mainScreenStatus = lv_obj_create(comp_parent);
    lv_obj_set_width(cui_mainScreenStatus, lv_pct(100));
    lv_obj_set_height(cui_mainScreenStatus, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenStatus, 3);
    lv_obj_set_x(cui_mainScreenStatus, 389);
    lv_obj_set_y(cui_mainScreenStatus, 177);
    lv_obj_set_flex_flow(cui_mainScreenStatus, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_mainScreenStatus, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_mainScreenStatus, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenStatus, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_mainScreenStatus, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenStatus, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenStatus, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenStatus, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenStatus, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenStatus, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenStatus, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenStatus, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenStatus, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenStatus, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenStatus, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_preHeatBox;
    cui_preHeatBox = lv_obj_create(cui_mainScreenStatus);
    lv_obj_set_width(cui_preHeatBox, lv_pct(100));
    lv_obj_set_height(cui_preHeatBox, lv_pct(50));
    lv_obj_set_flex_grow(cui_preHeatBox, 1);
    lv_obj_set_x(cui_preHeatBox, 386);
    lv_obj_set_y(cui_preHeatBox, 178);
    lv_obj_set_flex_flow(cui_preHeatBox, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_preHeatBox, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_preHeatBox, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_preHeatBox, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_preHeatBox, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_preHeatBox, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_preHeatBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_preHeatBox, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_preHeatBox, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_preHeatBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_preHeatBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_preHeatBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_preHeatBox, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_preHeatBox, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_preHeatBox, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_preHeatBox1;
    cui_preHeatBox1 = lv_obj_create(cui_mainScreenStatus);
    lv_obj_set_width(cui_preHeatBox1, lv_pct(100));
    lv_obj_set_height(cui_preHeatBox1, lv_pct(25));
    lv_obj_set_x(cui_preHeatBox1, 386);
    lv_obj_set_y(cui_preHeatBox1, 178);
    lv_obj_set_flex_flow(cui_preHeatBox1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_preHeatBox1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_preHeatBox1, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_preHeatBox1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_preHeatBox1, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_preHeatBox1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_preHeatBox1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_preHeatBox1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_preHeatBox1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_preHeatBox1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_preHeatBox1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_preHeatBox1, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_preHeatBox1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_preHeatBox1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_preHeatBox1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_preHeatBox2;
    cui_preHeatBox2 = lv_obj_create(cui_mainScreenStatus);
    lv_obj_set_width(cui_preHeatBox2, lv_pct(100));
    lv_obj_set_height(cui_preHeatBox2, lv_pct(25));
    lv_obj_set_x(cui_preHeatBox2, 386);
    lv_obj_set_y(cui_preHeatBox2, 178);
    lv_obj_set_flex_flow(cui_preHeatBox2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_preHeatBox2, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_preHeatBox2, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_preHeatBox2, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_preHeatBox2, lv_color_hex(0x550000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_preHeatBox2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_preHeatBox2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_preHeatBox2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_preHeatBox2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_preHeatBox2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_preHeatBox2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_preHeatBox2, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_preHeatBox2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_preHeatBox2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_preHeatBox2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_mainScreenStatusLogo;
    cui_mainScreenStatusLogo = lv_img_create(cui_preHeatBox);
    lv_obj_set_width(cui_mainScreenStatusLogo, 150);  ///? 100
    lv_obj_set_height(cui_mainScreenStatusLogo, LV_SIZE_CONTENT); /// 100
    lv_obj_clear_flag(cui_mainScreenStatusLogo, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_size(cui_mainScreenStatusLogo, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_scrollbar_mode(cui_mainScreenStatusLogo, LV_SCROLLBAR_MODE_OFF);
    lv_img_set_src(cui_mainScreenStatusLogo, &img_logo2);

    lv_obj_t *cui_mainScreenStatusCaption;
    cui_mainScreenStatusCaption = lv_label_create(cui_preHeatBox);
    lv_obj_set_width(cui_mainScreenStatusCaption, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_mainScreenStatusCaption, LV_SIZE_CONTENT); /// 1
    lv_label_set_text(cui_mainScreenStatusCaption, "N/A");
    lv_obj_set_style_text_font(cui_mainScreenStatusCaption, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *preHeatButton1;
    preHeatButton1 = lv_label_create(cui_preHeatBox1);
    lv_obj_set_width(preHeatButton1, lv_pct(37));
    lv_obj_set_height(preHeatButton1, lv_pct(100));
    lv_obj_set_flex_grow(preHeatButton1, 2);
    lv_obj_set_align(preHeatButton1, LV_ALIGN_CENTER);
    lv_label_set_text(preHeatButton1, "PLA");
    lv_obj_add_flag(preHeatButton1, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(preHeatButton1, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(preHeatButton1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(preHeatButton1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(preHeatButton1, lv_font_middle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(preHeatButton1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(preHeatButton1, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(preHeatButton1, lv_color_hex(0x007700), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(preHeatButton1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(preHeatButton1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(preHeatButton1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(preHeatButton1, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(preHeatButton1, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(preHeatButton1, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(preHeatButton1, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(preHeatButton1, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(preHeatButton1, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(preHeatButton1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *preHeatButton2;
    preHeatButton2 = lv_label_create(cui_preHeatBox1);
    lv_obj_set_width(preHeatButton2, lv_pct(37));
    lv_obj_set_height(preHeatButton2, lv_pct(100));
    lv_obj_set_flex_grow(preHeatButton2, 2);
    lv_obj_set_align(preHeatButton2, LV_ALIGN_CENTER);
    lv_label_set_text(preHeatButton2, "ABS");
    lv_obj_add_flag(preHeatButton2, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(preHeatButton2, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(preHeatButton2, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(preHeatButton2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(preHeatButton2, lv_font_middle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(preHeatButton2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(preHeatButton2, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(preHeatButton2, lv_color_hex(0x000077), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(preHeatButton2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(preHeatButton2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(preHeatButton2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(preHeatButton2, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(preHeatButton2, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(preHeatButton2, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(preHeatButton2, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(preHeatButton2, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(preHeatButton2, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(preHeatButton2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *preHeatButton3;
    preHeatButton3 = lv_label_create(cui_preHeatBox2);
    lv_obj_set_width(preHeatButton3, lv_pct(26));
    lv_obj_set_height(preHeatButton3, lv_pct(100));
    lv_obj_set_flex_grow(preHeatButton3, 2);
    lv_obj_set_align(preHeatButton3, LV_ALIGN_CENTER);
    lv_label_set_text(preHeatButton3, "Off");
    lv_obj_add_flag(preHeatButton3, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(preHeatButton3, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(preHeatButton3, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(preHeatButton3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(preHeatButton3, lv_font_middle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(preHeatButton3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(preHeatButton3, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(preHeatButton3, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(preHeatButton3, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(preHeatButton3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(preHeatButton3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(preHeatButton3, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(preHeatButton3, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(preHeatButton3, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(preHeatButton3, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(preHeatButton3, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(preHeatButton3, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(preHeatButton3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t **children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_MAINSCREENSTATUS_NUM);
    children[UI_COMP_MAINSCREENSTATUS_MAINSCREENSTATUS] = cui_mainScreenStatus;
    children[UI_COMP_MAINSCREENSTATUS_MAINSCREENSTATUSLOGO] = cui_mainScreenStatusLogo;
    children[UI_COMP_MAINSCREENSTATUS_MAINSCREENSTATUSCAPTION] = cui_mainScreenStatusCaption;
    children[UI_COMP_MAINSCREENSTATUS_MAINSCREENPREHEATBOX] = cui_preHeatBox;
    children[UI_COMP_MAINSCREENSTATUS_MAINSCREENPREHEATBOX1] = cui_preHeatBox1;
    children[UI_COMP_MAINSCREENSTATUS_MAINSCREENPREHEATBOX2] = cui_preHeatBox2;
    children[UI_COMP_MAINSCREENSTATUS_MAINSCREENPREHEATBUTTON1] = preHeatButton1;
    children[UI_COMP_MAINSCREENSTATUS_MAINSCREENPREHEATBUTTON2] = preHeatButton2;
    children[UI_COMP_MAINSCREENSTATUS_MAINSCREENPREHEATBUTTON3] = preHeatButton3;

    lv_obj_add_event_cb(preHeatButton1, ui_event_comp_mainScreenStatusComponent_onPreHeatButton1Click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(preHeatButton2, ui_event_comp_mainScreenStatusComponent_onPreHeatButton2Click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(preHeatButton3, ui_event_comp_mainScreenStatusComponent_onPreHeatButton3Click, LV_EVENT_CLICKED, NULL);

    lv_obj_add_event_cb(cui_mainScreenStatus, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(cui_mainScreenStatus, del_component_child_event_cb, LV_EVENT_DELETE, children);
    ui_comp_mainScreenStatus_create_hook(cui_mainScreenStatus);
    return cui_mainScreenStatus;
}