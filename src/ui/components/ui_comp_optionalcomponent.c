#include <stdio.h>
#include "../ui.h"
#include "../ui_msgs.h"

void ui_event_comp_optionalComponent_onNeoPixelNum(lv_event_t *e)
{
   lv_event_code_t event_code = lv_event_get_code(e);
   if (event_code == LV_EVENT_VALUE_CHANGED)
   {
       uint32_t value = lv_slider_get_value(ui_optionalNeoPixelNumSlider);
       if (value < 1)
       {
           printf("ononNeoPixelNumLEDOFF OFFSIMBOLE");
           lv_label_set_text(ui_optionalNeoPixelNumValue, LV_SYMBOL_POWER);
       }
       else
       {
           lv_label_set_text_fmt(ui_optionalNeoPixelNumValue, "%d", value);
       }
   }
   if (event_code == LV_EVENT_RELEASED)
   {
       lv_msg_send(XTOUCH_OPTIONAL_NEOPIXEL_NUM_SET, NULL);
   }
}

void ui_event_comp_optionalComponent_onNeoPixelBlightness(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        uint32_t value = lv_slider_get_value(ui_optionalNeoPixelBlightnessSlider);
        lv_label_set_text_fmt(ui_optionalNeoPixelBlightnessValue, "%d", value);
    }
    if (event_code == LV_EVENT_RELEASED)
    {
        lv_msg_send(XTOUCH_OPTIONAL_NEOPIXEL_SET, NULL);
    }
}

void ui_event_comp_optionalComponent_onAlarmTimeout(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        uint32_t value = lv_slider_get_value(ui_optionalAlarmTimeoutSlider);
       if (value < 1)
       {
           printf("ononAlarmTimeoutLEDOFF OFFSIMBOLE");
           lv_label_set_text(ui_optionalAlarmTimeoutValue, LV_SYMBOL_POWER);
       }
       else
       {
           lv_label_set_text_fmt(ui_optionalAlarmTimeoutValue, "%d", value * 30);
       }
    }
    if (event_code == LV_EVENT_RELEASED)
    {
        lv_msg_send(XTOUCH_OPTIONAL_ALARM_TIMEOUT_SET, NULL);
    }
}

void ui_event_comp_optionalComponent_onChamberTemp(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        onOptionalChamberSensor(e);
    }
}

void ui_event_comp_optionalComponent_onStackChan(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        onOptionalStackChan(e);
    }
}

void ui_event_comp_optionalComponent_onIdleLED(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        onOptionalIdleLED(e);
    }
}

// COMPONENT optionalComponent

lv_obj_t *ui_optionalComponent_create(lv_obj_t *comp_parent)
{

    lv_obj_t *cui_optionalComponent;
    cui_optionalComponent = lv_obj_create(comp_parent);
    lv_obj_set_height(cui_optionalComponent, lv_pct(100));
    lv_obj_set_flex_grow(cui_optionalComponent, 1);
    lv_obj_set_x(cui_optionalComponent, 385);
    lv_obj_set_y(cui_optionalComponent, 178);
    lv_obj_set_flex_flow(cui_optionalComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_optionalComponent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_radius(cui_optionalComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_optionalComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optionalComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optionalComponent, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optionalComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optionalComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_optionalComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_optionalComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_optionalComponent, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_optionalComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(cui_optionalComponent, LV_SCROLLBAR_MODE_ACTIVE);

    lv_obj_t *cui_optionalTitle;
    cui_optionalTitle = lv_label_create(cui_optionalComponent);
    lv_obj_set_width(cui_optionalTitle, lv_pct(100));
    lv_obj_set_height(cui_optionalTitle, LV_SIZE_CONTENT); /// 40
    lv_label_set_text_fmt(cui_optionalTitle, LV_SYMBOL_SETTINGS " OPTIONAL");
    lv_obj_set_scrollbar_mode(cui_optionalTitle, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_optionalTitle, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optionalTitle, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optionalTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optionalTitle, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optionalTitle, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_optionalTitle, lv_color_hex(0x682AFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_optionalTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_optionalTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_optionalTitle, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_optionalTitle, lv_color_hex(0x000), LV_PART_MAIN | LV_STATE_DEFAULT);

    //---StackChan Mode Start------------------------------

    lv_obj_t *cui_optional_stackChan;
    cui_optional_stackChan = lv_obj_create(cui_optionalComponent);
    lv_obj_set_width(cui_optional_stackChan, lv_pct(100));
    lv_obj_set_height(cui_optional_stackChan, LV_SIZE_CONTENT); /// 50
    lv_obj_set_flex_flow(cui_optional_stackChan, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_optional_stackChan, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(cui_optional_stackChan, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_optional_stackChan, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_optional_stackChan, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_optional_stackChan, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optional_stackChan, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optional_stackChan, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optional_stackChan, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optional_stackChan, 16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_optional_stackChanLabel;
    cui_optional_stackChanLabel = lv_label_create(cui_optional_stackChan);
    lv_obj_set_width(cui_optional_stackChanLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_optional_stackChanLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_style_text_font(cui_optional_stackChanLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_optional_stackChanLabel, "Stack Chan Mode");
    lv_obj_set_scrollbar_mode(cui_optional_stackChanLabel, LV_SCROLLBAR_MODE_OFF);

    ui_optional_stackChanSwitch = lv_switch_create(cui_optional_stackChan);
    lv_obj_set_width(ui_optional_stackChanSwitch, 50);
    lv_obj_set_height(ui_optional_stackChanSwitch, 25);

    lv_obj_set_style_bg_color(ui_optional_stackChanSwitch, lv_color_hex(0x2AFF00), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_optional_stackChanSwitch, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);

    lv_obj_set_style_bg_color(ui_optional_stackChanSwitch, lv_color_hex(0x2AFF00), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optional_stackChanSwitch, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_optional_stackChanSwitch, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_optional_stackChanSwitch, 255, LV_PART_KNOB | LV_STATE_CHECKED);

    if (xTouchConfig.xTouchStackChanEnabled)
    {
        lv_obj_add_state(ui_optional_stackChanSwitch, LV_STATE_CHECKED);
    }
    //lv_obj_add_flag(cui_optional_stackChan, LV_OBJ_FLAG_HIDDEN);
    //---StackChan Mode End------------------------------

    //---NEOPIXEL Num Start------------------------------
    lv_obj_t *cui_optionalneoPixelTitle;
    cui_optionalneoPixelTitle = lv_label_create(cui_optionalComponent);
    lv_obj_set_width(cui_optionalneoPixelTitle, lv_pct(100));
    lv_obj_set_height(cui_optionalneoPixelTitle, LV_SIZE_CONTENT); /// 40
    lv_label_set_text(cui_optionalneoPixelTitle, LV_SYMBOL_LIST " NeoPixel(Optional)");
    lv_obj_set_scrollbar_mode(cui_optionalneoPixelTitle, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_optionalneoPixelTitle, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optionalneoPixelTitle, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optionalneoPixelTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optionalneoPixelTitle, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optionalneoPixelTitle, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_optionalneoPixelTitle, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_optionalneoPixelTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_optionalneoPixelTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_optionalneoPixelTitle, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_optionalneoPixelTitle, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_optionalNeoPixelNumPanel;
    cui_optionalNeoPixelNumPanel = lv_obj_create(cui_optionalComponent);
    lv_obj_set_width(cui_optionalNeoPixelNumPanel, lv_pct(100));
    lv_obj_set_height(cui_optionalNeoPixelNumPanel, 70);
    lv_obj_set_flex_flow(cui_optionalNeoPixelNumPanel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_optionalNeoPixelNumPanel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_radius(cui_optionalNeoPixelNumPanel, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_optionalNeoPixelNumPanel, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_optionalNeoPixelNumPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_optionalNeoPixelNumPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optionalNeoPixelNumPanel, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optionalNeoPixelNumPanel, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optionalNeoPixelNumPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optionalNeoPixelNumPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_optionalNeoPixelNumPanelLabel;
    cui_optionalNeoPixelNumPanelLabel = lv_label_create(cui_optionalNeoPixelNumPanel);
    lv_obj_set_width(cui_optionalNeoPixelNumPanelLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_optionalNeoPixelNumPanelLabel, LV_SIZE_CONTENT); /// 1
    lv_label_set_text(cui_optionalNeoPixelNumPanelLabel, "LEDs");
    lv_obj_set_style_text_font(cui_optionalNeoPixelNumPanelLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optionalNeoPixelNumPanelLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optionalNeoPixelNumPanelLabel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optionalNeoPixelNumPanelLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optionalNeoPixelNumPanelLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_optionalNeoPixelNumSlider = lv_slider_create(cui_optionalNeoPixelNumPanel);
    lv_slider_set_range(ui_optionalNeoPixelNumSlider, 0, 50);
    lv_slider_set_value(ui_optionalNeoPixelNumSlider, xTouchConfig.xTouchNeoPixelNumValue, LV_ANIM_OFF);
    lv_obj_set_height(ui_optionalNeoPixelNumSlider, 10);
    lv_obj_set_flex_grow(ui_optionalNeoPixelNumSlider, 1);
    lv_obj_set_x(ui_optionalNeoPixelNumSlider, 9);
    lv_obj_set_y(ui_optionalNeoPixelNumSlider, 28);
    lv_obj_set_style_bg_color(ui_optionalNeoPixelNumSlider, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optionalNeoPixelNumSlider, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_optionalNeoPixelNumSlider, lv_color_hex(0x00FF00), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optionalNeoPixelNumSlider, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_optionalNeoPixelNumSlider, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optionalNeoPixelNumSlider, 255, LV_PART_KNOB | LV_STATE_DEFAULT);

    ui_optionalNeoPixelNumValue = lv_label_create(cui_optionalNeoPixelNumPanel);
    lv_obj_set_width(ui_optionalNeoPixelNumValue, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_optionalNeoPixelNumValue, LV_SIZE_CONTENT); /// 1

    int32_t value4 = lv_slider_get_value(ui_optionalNeoPixelNumSlider);
    lv_label_set_text_fmt(ui_optionalNeoPixelNumValue,value4 < 1 ? LV_SYMBOL_POWER : "%d", value4);
    lv_obj_set_style_text_font(ui_optionalNeoPixelNumValue, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_optionalNeoPixelNumValue, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_optionalNeoPixelNumValue, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_optionalNeoPixelNumValue, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_optionalNeoPixelNumValue, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //---NEOPIXEL Num End------------------------------

    //---NEOPIXEL Blightness Start ------------------------------
    lv_obj_t *cui_optionalNeoPixelBlightnessPanel;
    cui_optionalNeoPixelBlightnessPanel = lv_obj_create(cui_optionalComponent);
    lv_obj_set_width(cui_optionalNeoPixelBlightnessPanel, lv_pct(100));
    lv_obj_set_height(cui_optionalNeoPixelBlightnessPanel, 70);
    lv_obj_set_flex_flow(cui_optionalNeoPixelBlightnessPanel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_optionalNeoPixelBlightnessPanel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_radius(cui_optionalNeoPixelBlightnessPanel, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_optionalNeoPixelBlightnessPanel, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_optionalNeoPixelBlightnessPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_optionalNeoPixelBlightnessPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optionalNeoPixelBlightnessPanel, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optionalNeoPixelBlightnessPanel, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optionalNeoPixelBlightnessPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optionalNeoPixelBlightnessPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_optionalNeoPixelBlightnessPanelLabel;
    cui_optionalNeoPixelBlightnessPanelLabel = lv_label_create(cui_optionalNeoPixelBlightnessPanel);
    lv_obj_set_width(cui_optionalNeoPixelBlightnessPanelLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_optionalNeoPixelBlightnessPanelLabel, LV_SIZE_CONTENT); /// 1
    lv_label_set_text(cui_optionalNeoPixelBlightnessPanelLabel, "Blightness");
    lv_obj_set_style_text_font(cui_optionalNeoPixelBlightnessPanelLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optionalNeoPixelBlightnessPanelLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optionalNeoPixelBlightnessPanelLabel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optionalNeoPixelBlightnessPanelLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optionalNeoPixelBlightnessPanelLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_optionalNeoPixelBlightnessSlider = lv_slider_create(cui_optionalNeoPixelBlightnessPanel);
    lv_slider_set_range(ui_optionalNeoPixelBlightnessSlider, 15, 255);
    lv_slider_set_value(ui_optionalNeoPixelBlightnessSlider, xTouchConfig.xTouchNeoPixelBlightnessValue, LV_ANIM_OFF);
    lv_obj_set_height(ui_optionalNeoPixelBlightnessSlider, 10);
    lv_obj_set_flex_grow(ui_optionalNeoPixelBlightnessSlider, 1);
    lv_obj_set_x(ui_optionalNeoPixelBlightnessSlider, 9);
    lv_obj_set_y(ui_optionalNeoPixelBlightnessSlider, 28);
    lv_obj_set_style_bg_color(ui_optionalNeoPixelBlightnessSlider, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optionalNeoPixelBlightnessSlider, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_optionalNeoPixelBlightnessSlider, lv_color_hex(0x00FF00), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optionalNeoPixelBlightnessSlider, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_optionalNeoPixelBlightnessSlider, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optionalNeoPixelBlightnessSlider, 255, LV_PART_KNOB | LV_STATE_DEFAULT);

    ui_optionalNeoPixelBlightnessValue = lv_label_create(cui_optionalNeoPixelBlightnessPanel);
    lv_obj_set_width(ui_optionalNeoPixelBlightnessValue, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_optionalNeoPixelBlightnessValue, LV_SIZE_CONTENT); /// 1

    int32_t value3 = lv_slider_get_value(ui_optionalNeoPixelBlightnessSlider);
    lv_label_set_text_fmt(ui_optionalNeoPixelBlightnessValue, value3 < XTOUCH_LIGHT_MIN_SLEEP_TIME ? LV_SYMBOL_POWER : "%d", value3);
    lv_obj_set_style_text_font(ui_optionalNeoPixelBlightnessValue, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_optionalNeoPixelBlightnessValue, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_optionalNeoPixelBlightnessValue, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_optionalNeoPixelBlightnessValue, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_optionalNeoPixelBlightnessValue, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //---NEOPIXEL Blightness end------------------------------

    //---NEOPIXEL Alarm timeout start------------------------------
    lv_obj_t *cui_optionalAlarmTimeoutPanel;
    cui_optionalAlarmTimeoutPanel = lv_obj_create(cui_optionalComponent);
    lv_obj_set_width(cui_optionalAlarmTimeoutPanel, lv_pct(100));
    lv_obj_set_height(cui_optionalAlarmTimeoutPanel, 70);
    lv_obj_set_flex_flow(cui_optionalAlarmTimeoutPanel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_optionalAlarmTimeoutPanel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_radius(cui_optionalAlarmTimeoutPanel, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_optionalAlarmTimeoutPanel, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_optionalAlarmTimeoutPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_optionalAlarmTimeoutPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optionalAlarmTimeoutPanel, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optionalAlarmTimeoutPanel, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optionalAlarmTimeoutPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optionalAlarmTimeoutPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_optionalAlarmTimeoutPanelLabel;
    cui_optionalAlarmTimeoutPanelLabel = lv_label_create(cui_optionalAlarmTimeoutPanel);
    lv_obj_set_width(cui_optionalAlarmTimeoutPanelLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_optionalAlarmTimeoutPanelLabel, LV_SIZE_CONTENT); /// 1
    lv_label_set_text(cui_optionalAlarmTimeoutPanelLabel, "Alarm Timeout");
    lv_obj_set_style_text_font(cui_optionalAlarmTimeoutPanelLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optionalAlarmTimeoutPanelLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optionalAlarmTimeoutPanelLabel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optionalAlarmTimeoutPanelLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optionalAlarmTimeoutPanelLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_optionalAlarmTimeoutSlider = lv_slider_create(cui_optionalAlarmTimeoutPanel);
    lv_slider_set_range(ui_optionalAlarmTimeoutSlider, 0, 20);
    lv_slider_set_value(ui_optionalAlarmTimeoutSlider, xTouchConfig.xTouchAlarmTimeoutValue, LV_ANIM_OFF);
    lv_obj_set_height(ui_optionalAlarmTimeoutSlider, 10);
    lv_obj_set_flex_grow(ui_optionalAlarmTimeoutSlider, 1);
    lv_obj_set_x(ui_optionalAlarmTimeoutSlider, 9);
    lv_obj_set_y(ui_optionalAlarmTimeoutSlider, 28);
    lv_obj_set_style_bg_color(ui_optionalAlarmTimeoutSlider, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optionalAlarmTimeoutSlider, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_optionalAlarmTimeoutSlider, lv_color_hex(0x00FF00), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optionalAlarmTimeoutSlider, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_optionalAlarmTimeoutSlider, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optionalAlarmTimeoutSlider, 255, LV_PART_KNOB | LV_STATE_DEFAULT);

    ui_optionalAlarmTimeoutValue = lv_label_create(cui_optionalAlarmTimeoutPanel);
    lv_obj_set_width(ui_optionalAlarmTimeoutValue, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_optionalAlarmTimeoutValue, LV_SIZE_CONTENT); /// 1

    int32_t value5 = lv_slider_get_value(ui_optionalAlarmTimeoutSlider);
    lv_label_set_text_fmt(ui_optionalAlarmTimeoutValue, value3 < 1 ? LV_SYMBOL_POWER : "%d", value5 * 30);
    lv_obj_set_style_text_font(ui_optionalAlarmTimeoutValue, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_optionalAlarmTimeoutValue, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_optionalAlarmTimeoutValue, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_optionalAlarmTimeoutValue, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_optionalAlarmTimeoutValue, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    //---NEOPIXEL Alarm timeout start------------------------------

    //---NEOPIXEL Idle LED Start-------------------------------
    lv_obj_t *cui_optional_Idle_led;
    cui_optional_Idle_led = lv_obj_create(cui_optionalComponent);
    lv_obj_set_width(cui_optional_Idle_led, lv_pct(100));
    lv_obj_set_height(cui_optional_Idle_led, LV_SIZE_CONTENT); /// 50
    lv_obj_set_flex_flow(cui_optional_Idle_led, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_optional_Idle_led, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(cui_optional_Idle_led, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_optional_Idle_led, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_optional_Idle_led, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_optional_Idle_led, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optional_Idle_led, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optional_Idle_led, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optional_Idle_led, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optional_Idle_led, 16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_optional_Idle_ledLabel;
    cui_optional_Idle_ledLabel = lv_label_create(cui_optional_Idle_led);
    lv_obj_set_width(cui_optional_Idle_ledLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_optional_Idle_ledLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_style_text_font(cui_optional_Idle_ledLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_optional_Idle_ledLabel, "Idle LED");
    lv_obj_set_scrollbar_mode(cui_optional_Idle_ledLabel, LV_SCROLLBAR_MODE_OFF);

    ui_optional_Idle_ledSwitch = lv_switch_create(cui_optional_Idle_led);
    lv_obj_set_width(ui_optional_Idle_ledSwitch, 50);
    lv_obj_set_height(ui_optional_Idle_ledSwitch, 25);

    lv_obj_set_style_bg_color(ui_optional_Idle_ledSwitch, lv_color_hex(0x2AFF00), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_optional_Idle_ledSwitch, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);

    lv_obj_set_style_bg_color(ui_optional_Idle_ledSwitch, lv_color_hex(0x2AFF00), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optional_Idle_ledSwitch, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_optional_Idle_ledSwitch, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_optional_Idle_ledSwitch, 255, LV_PART_KNOB | LV_STATE_CHECKED);

    if (xTouchConfig.xTouchIdleLEDEnabled)
    {
        lv_obj_add_state(ui_optional_Idle_ledSwitch, LV_STATE_CHECKED);
    }
    //---NEOPIXEL Idle LED End------------------------------

    //--- Chamber Sersor Start --------------------
    lv_obj_t *cui_optional_chamberSensor;
    cui_optional_chamberSensor = lv_obj_create(cui_optionalComponent);
    lv_obj_set_width(cui_optional_chamberSensor, lv_pct(100));
    lv_obj_set_height(cui_optional_chamberSensor, LV_SIZE_CONTENT); /// 50
    lv_obj_set_flex_flow(cui_optional_chamberSensor, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_optional_chamberSensor, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(cui_optional_chamberSensor, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_optional_chamberSensor, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_optional_chamberSensor, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_optional_chamberSensor, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optional_chamberSensor, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optional_chamberSensor, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optional_chamberSensor, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optional_chamberSensor, 16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_optional_chamberSensorLabel;
    cui_optional_chamberSensorLabel = lv_label_create(cui_optional_chamberSensor);
    lv_obj_set_width(cui_optional_chamberSensorLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_optional_chamberSensorLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_style_text_font(cui_optional_chamberSensorLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_optional_chamberSensorLabel, "CHAMBER TEMP");
    lv_obj_set_scrollbar_mode(cui_optional_chamberSensorLabel, LV_SCROLLBAR_MODE_OFF);

    ui_optional_chamberSensorSwitch = lv_switch_create(cui_optional_chamberSensor);
    lv_obj_set_width(ui_optional_chamberSensorSwitch, 50);
    lv_obj_set_height(ui_optional_chamberSensorSwitch, 25);

    lv_obj_set_style_bg_color(ui_optional_chamberSensorSwitch, lv_color_hex(0x2AFF00), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_optional_chamberSensorSwitch, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);

    lv_obj_set_style_bg_color(ui_optional_chamberSensorSwitch, lv_color_hex(0x2AFF00), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optional_chamberSensorSwitch, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_optional_chamberSensorSwitch, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_optional_chamberSensorSwitch, 255, LV_PART_KNOB | LV_STATE_CHECKED);

    if (!xtouch_bblp_is_p1Series())
    {
        lv_obj_add_flag(cui_optional_chamberSensor, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        if (xTouchConfig.xTouchChamberSensorEnabled)
        {
            lv_obj_add_state(ui_optional_chamberSensorSwitch, LV_STATE_CHECKED);
        }
    }
    //---Chamber Sensor End------------------------------

  

    lv_obj_t **children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_OPTIONALCOMPONENT_NUM);
    children[UI_COMP_OPTIONALCOMPONENT_OPTIONALCOMPONENT] = cui_optionalComponent;

    children[UI_COMP_OPTIONALCOMPONENT_CHAMBER_TEMP] = cui_optional_chamberSensor;
    children[UI_COMP_OPTIONALCOMPONENT_CHAMBER_TEMP_LABEL] = cui_optional_chamberSensorLabel;
    children[UI_COMP_OPTIONALCOMPONENT_CHAMBER_TEMP_SWITCH] = ui_optional_chamberSensorSwitch;

    children[UI_COMP_OPTIONALCOMPONENT_NEOPIXEL] = cui_optionalNeoPixelBlightnessPanel;
    children[UI_COMP_OPTIONALCOMPONENT_NEOPIXEL_LABEL] = cui_optionalNeoPixelBlightnessPanelLabel;
    children[UI_COMP_OPTIONALCOMPONENT_NEOPIXEL_SLIDER] = ui_optionalNeoPixelBlightnessSlider;
    children[UI_COMP_OPTIONALCOMPONENT_NEOPIXEL_VALUE] = ui_optionalNeoPixelBlightnessValue;

    children[UI_COMP_OPTIONALCOMPONENT_ALARM_TIMEOUT] = cui_optionalAlarmTimeoutPanel;
    children[UI_COMP_OPTIONALCOMPONENT_ALARM_TIMEOUT_LABEL] = cui_optionalAlarmTimeoutPanelLabel;
    children[UI_COMP_OPTIONALCOMPONENT_ALARM_TIMEOUT_SLIDER] = ui_optionalAlarmTimeoutSlider;
    children[UI_COMP_OPTIONALCOMPONENT_ALARM_TIMEOUT_VALUE] = ui_optionalAlarmTimeoutValue;

    children[UI_COMP_OPTIONALCOMPONENT_NEOPIXEL_NUM] = cui_optionalNeoPixelNumPanel;
    children[UI_COMP_OPTIONALCOMPONENT_NEOPIXEL_NUM_LABEL] = cui_optionalNeoPixelNumPanelLabel;
    children[UI_COMP_OPTIONALCOMPONENT_NEOPIXEL_NUM_SLIDER] = ui_optionalNeoPixelNumSlider;
    children[UI_COMP_OPTIONALCOMPONENT_NEOPIXEL_NUM_VALUE] = ui_optionalNeoPixelNumValue;

    children[UI_COMP_OPTIONALCOMPONENT_IDLE_LED] = cui_optional_Idle_led;
    children[UI_COMP_OPTIONALCOMPONENT_IDLE_LED_LABEL] = cui_optional_Idle_ledLabel;
    children[UI_COMP_OPTIONALCOMPONENT_IDLE_LED_SWITCH] = ui_optional_Idle_ledSwitch;

    children[UI_COMP_OPTIONALCOMPONENT_STACK_CHAN] = cui_optional_stackChan;
    children[UI_COMP_OPTIONALCOMPONENT_STACK_CHAN_LABEL] = cui_optional_stackChanLabel;
    children[UI_COMP_OPTIONALCOMPONENT_STACK_CHAN_SWITCH] = ui_optional_stackChanSwitch;
  
   lv_obj_add_event_cb(cui_optionalComponent, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
   lv_obj_add_event_cb(cui_optionalComponent, del_component_child_event_cb, LV_EVENT_DELETE, children);

    lv_obj_add_event_cb(ui_optionalNeoPixelNumSlider, ui_event_comp_optionalComponent_onNeoPixelNum, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_optionalNeoPixelBlightnessSlider, ui_event_comp_optionalComponent_onNeoPixelBlightness, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_optionalAlarmTimeoutSlider, ui_event_comp_optionalComponent_onAlarmTimeout, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_optional_chamberSensorSwitch, ui_event_comp_optionalComponent_onChamberTemp, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_optional_stackChanSwitch, ui_event_comp_optionalComponent_onStackChan, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_optional_Idle_ledSwitch, ui_event_comp_optionalComponent_onIdleLED, LV_EVENT_VALUE_CHANGED, NULL);

    ui_comp_optionalComponent_create_hook(cui_optionalComponent);
    return cui_optionalComponent;
}
