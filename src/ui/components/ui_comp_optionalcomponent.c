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

void ui_event_comp_optionalComponent_onNeoPixelBrightness(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        uint32_t value = lv_slider_get_value(ui_optionalNeoPixelBrightnessSlider);
        lv_label_set_text_fmt(ui_optionalNeoPixelBrightnessValue, "%d", value);
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

void ui_event_comp_optionalComponent_onPreheat(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        onOptionalPreheat(e);
    }
}

#ifdef __XTOUCH_PLATFORM_S3__
void ui_event_comp_optionalComponent_onMultiPrinterMonitor(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        onOptionalMultiPrinterMonitor(e);
    }
}
#endif

#ifdef __XTOUCH_PLATFORM_S3__
void ui_event_comp_optionalComponent_onHistory(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        onOptionalHistory(e);
    }
}

void ui_event_comp_optionalComponent_onHideThumbnails(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
        onOptionalHideAllThumbnails(e);
}
#endif

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
    /* flex 子で lv_pct(100) 高さと flex_grow を併用すると、親が column+grow のときレイアウト負荷・不具合の原因になり得る */
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

    //---Preheat Start------------------------------
    lv_obj_t *cui_optional_preheat;
    cui_optional_preheat = lv_obj_create(cui_optionalComponent);
    lv_obj_set_width(cui_optional_preheat, lv_pct(100));
    lv_obj_set_height(cui_optional_preheat, LV_SIZE_CONTENT); /// 50
    lv_obj_set_flex_flow(cui_optional_preheat, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_optional_preheat, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(cui_optional_preheat, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_optional_preheat, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_optional_preheat, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_optional_preheat, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optional_preheat, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optional_preheat, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optional_preheat, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optional_preheat, 16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_optional_preheatLabel;
    cui_optional_preheatLabel = lv_label_create(cui_optional_preheat);
    lv_obj_set_width(cui_optional_preheatLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_optional_preheatLabel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_style_text_font(cui_optional_preheatLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_optional_preheatLabel, "PreHeat button");
    lv_obj_set_scrollbar_mode(cui_optional_preheatLabel, LV_SCROLLBAR_MODE_OFF);

    ui_optional_preheatSwitch = lv_switch_create(cui_optional_preheat);
    lv_obj_set_width(ui_optional_preheatSwitch, 50);
    lv_obj_set_height(ui_optional_preheatSwitch, 25);

    lv_obj_set_style_bg_color(ui_optional_preheatSwitch, lv_color_hex(0x2AFF00), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_optional_preheatSwitch, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);

    lv_obj_set_style_bg_color(ui_optional_preheatSwitch, lv_color_hex(0x2AFF00), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optional_preheatSwitch, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_optional_preheatSwitch, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_optional_preheatSwitch, 255, LV_PART_KNOB | LV_STATE_CHECKED);

    if (xTouchConfig.xTouchPreheatEnabled)
    {
        lv_obj_add_state(ui_optional_preheatSwitch, LV_STATE_CHECKED);
    }
    //---Preheat End------------------------------

#if defined(__XTOUCH_PLATFORM_S3__)
    //---Multi Printer Monitor (Printers) Start------------------------------
    lv_obj_t *cui_optional_multiPrinterMonitor;
    cui_optional_multiPrinterMonitor = lv_obj_create(cui_optionalComponent);
    lv_obj_set_width(cui_optional_multiPrinterMonitor, lv_pct(100));
    lv_obj_set_height(cui_optional_multiPrinterMonitor, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cui_optional_multiPrinterMonitor, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_optional_multiPrinterMonitor, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(cui_optional_multiPrinterMonitor, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_optional_multiPrinterMonitor, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_optional_multiPrinterMonitor, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_optional_multiPrinterMonitor, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optional_multiPrinterMonitor, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optional_multiPrinterMonitor, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optional_multiPrinterMonitor, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optional_multiPrinterMonitor, 16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_optional_multiPrinterMonitorLabel;
    cui_optional_multiPrinterMonitorLabel = lv_label_create(cui_optional_multiPrinterMonitor);
    lv_obj_set_width(cui_optional_multiPrinterMonitorLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(cui_optional_multiPrinterMonitorLabel, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(cui_optional_multiPrinterMonitorLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_optional_multiPrinterMonitorLabel, "Multi Printer Monitor");
    lv_obj_set_scrollbar_mode(cui_optional_multiPrinterMonitorLabel, LV_SCROLLBAR_MODE_OFF);

    ui_optional_multiPrinterMonitorSwitch = lv_switch_create(cui_optional_multiPrinterMonitor);
    lv_obj_set_width(ui_optional_multiPrinterMonitorSwitch, 50);
    lv_obj_set_height(ui_optional_multiPrinterMonitorSwitch, 25);
    lv_obj_set_style_bg_color(ui_optional_multiPrinterMonitorSwitch, lv_color_hex(0x2AFF00), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_optional_multiPrinterMonitorSwitch, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui_optional_multiPrinterMonitorSwitch, lv_color_hex(0x2AFF00), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optional_multiPrinterMonitorSwitch, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_optional_multiPrinterMonitorSwitch, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_optional_multiPrinterMonitorSwitch, 255, LV_PART_KNOB | LV_STATE_CHECKED);
    if (xTouchConfig.xTouchMultiPrinterMonitorEnabled)
    {
        lv_obj_add_state(ui_optional_multiPrinterMonitorSwitch, LV_STATE_CHECKED);
    }
    //---Multi Printer Monitor End------------------------------

    //---History Screen Start------------------------------
    lv_obj_t *cui_optional_history;
    cui_optional_history = lv_obj_create(cui_optionalComponent);
    lv_obj_set_width(cui_optional_history, lv_pct(100));
    lv_obj_set_height(cui_optional_history, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cui_optional_history, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_optional_history, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(cui_optional_history, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_optional_history, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_optional_history, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_optional_history, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optional_history, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optional_history, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optional_history, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optional_history, 16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_optional_historyLabel;
    cui_optional_historyLabel = lv_label_create(cui_optional_history);
    lv_obj_set_width(cui_optional_historyLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(cui_optional_historyLabel, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(cui_optional_historyLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_optional_historyLabel, "History screen");
    lv_obj_set_scrollbar_mode(cui_optional_historyLabel, LV_SCROLLBAR_MODE_OFF);

    ui_optional_historySwitch = lv_switch_create(cui_optional_history);
    lv_obj_set_width(ui_optional_historySwitch, 50);
    lv_obj_set_height(ui_optional_historySwitch, 25);
    lv_obj_set_style_bg_color(ui_optional_historySwitch, lv_color_hex(0x2AFF00), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_optional_historySwitch, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui_optional_historySwitch, lv_color_hex(0x2AFF00), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optional_historySwitch, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_optional_historySwitch, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_optional_historySwitch, 255, LV_PART_KNOB | LV_STATE_CHECKED);
    if (xTouchConfig.xTouchHistoryEnabled)
    {
        lv_obj_add_state(ui_optional_historySwitch, LV_STATE_CHECKED);
    }
    //---History Screen End------------------------------

    //---Hide all thumbnails (Home / Printers / History) Start------------------------------
    lv_obj_t *cui_optional_hideThumbnails;
    cui_optional_hideThumbnails = lv_obj_create(cui_optionalComponent);
    lv_obj_set_width(cui_optional_hideThumbnails, lv_pct(100));
    lv_obj_set_height(cui_optional_hideThumbnails, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cui_optional_hideThumbnails, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_optional_hideThumbnails, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(cui_optional_hideThumbnails, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_optional_hideThumbnails, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_optional_hideThumbnails, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_optional_hideThumbnails, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optional_hideThumbnails, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optional_hideThumbnails, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optional_hideThumbnails, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optional_hideThumbnails, 16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_optional_hideThumbnailsLabel;
    cui_optional_hideThumbnailsLabel = lv_label_create(cui_optional_hideThumbnails);
    lv_obj_set_width(cui_optional_hideThumbnailsLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(cui_optional_hideThumbnailsLabel, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(cui_optional_hideThumbnailsLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_optional_hideThumbnailsLabel, "Hide all thumbnails");
    lv_obj_set_scrollbar_mode(cui_optional_hideThumbnailsLabel, LV_SCROLLBAR_MODE_OFF);

    ui_optional_hideThumbnailsSwitch = lv_switch_create(cui_optional_hideThumbnails);
    lv_obj_set_width(ui_optional_hideThumbnailsSwitch, 50);
    lv_obj_set_height(ui_optional_hideThumbnailsSwitch, 25);
    lv_obj_set_style_bg_color(ui_optional_hideThumbnailsSwitch, lv_color_hex(0x2AFF00), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_optional_hideThumbnailsSwitch, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui_optional_hideThumbnailsSwitch, lv_color_hex(0x2AFF00), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optional_hideThumbnailsSwitch, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_optional_hideThumbnailsSwitch, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_optional_hideThumbnailsSwitch, 255, LV_PART_KNOB | LV_STATE_CHECKED);
    if (xTouchConfig.xTouchHideAllThumbnails)
        lv_obj_add_state(ui_optional_hideThumbnailsSwitch, LV_STATE_CHECKED);
    //---Hide all thumbnails End------------------------------
#endif

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

    lv_obj_t *cui_optionalNeoPixelGpioHint;
    cui_optionalNeoPixelGpioHint = lv_label_create(cui_optionalComponent);
    lv_obj_set_width(cui_optionalNeoPixelGpioHint, lv_pct(100));
    lv_obj_set_height(cui_optionalNeoPixelGpioHint, LV_SIZE_CONTENT);
    {
        int pin = xTouchConfig.xTouchNeoPixelPinValue;
        if (pin <= 0)
            pin = 17;
        lv_label_set_text_fmt(cui_optionalNeoPixelGpioHint, "Connect to GPIO%d", pin);
    }
    lv_obj_set_scrollbar_mode(cui_optionalNeoPixelGpioHint, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_optionalNeoPixelGpioHint, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optionalNeoPixelGpioHint, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optionalNeoPixelGpioHint, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optionalNeoPixelGpioHint, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optionalNeoPixelGpioHint, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_optionalNeoPixelGpioHint, lv_color_hex(0xaaaaaa), LV_PART_MAIN | LV_STATE_DEFAULT);

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

    //---NEOPIXEL Brightness Start ------------------------------
    lv_obj_t *cui_optionalNeoPixelBrightnessPanel;
    cui_optionalNeoPixelBrightnessPanel = lv_obj_create(cui_optionalComponent);
    lv_obj_set_width(cui_optionalNeoPixelBrightnessPanel, lv_pct(100));
    lv_obj_set_height(cui_optionalNeoPixelBrightnessPanel, 70);
    lv_obj_set_flex_flow(cui_optionalNeoPixelBrightnessPanel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_optionalNeoPixelBrightnessPanel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_radius(cui_optionalNeoPixelBrightnessPanel, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_optionalNeoPixelBrightnessPanel, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_optionalNeoPixelBrightnessPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_optionalNeoPixelBrightnessPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optionalNeoPixelBrightnessPanel, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optionalNeoPixelBrightnessPanel, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optionalNeoPixelBrightnessPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optionalNeoPixelBrightnessPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_optionalNeoPixelBrightnessPanelLabel;
    cui_optionalNeoPixelBrightnessPanelLabel = lv_label_create(cui_optionalNeoPixelBrightnessPanel);
    lv_obj_set_width(cui_optionalNeoPixelBrightnessPanelLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_optionalNeoPixelBrightnessPanelLabel, LV_SIZE_CONTENT); /// 1
    lv_label_set_text(cui_optionalNeoPixelBrightnessPanelLabel, "Brightness");
    lv_obj_set_style_text_font(cui_optionalNeoPixelBrightnessPanelLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optionalNeoPixelBrightnessPanelLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optionalNeoPixelBrightnessPanelLabel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optionalNeoPixelBrightnessPanelLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optionalNeoPixelBrightnessPanelLabel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_optionalNeoPixelBrightnessSlider = lv_slider_create(cui_optionalNeoPixelBrightnessPanel);
    lv_slider_set_range(ui_optionalNeoPixelBrightnessSlider, 15, 255);
    lv_slider_set_value(ui_optionalNeoPixelBrightnessSlider, xTouchConfig.xTouchNeoPixelBrightnessValue, LV_ANIM_OFF);
    lv_obj_set_height(ui_optionalNeoPixelBrightnessSlider, 10);
    lv_obj_set_flex_grow(ui_optionalNeoPixelBrightnessSlider, 1);
    lv_obj_set_x(ui_optionalNeoPixelBrightnessSlider, 9);
    lv_obj_set_y(ui_optionalNeoPixelBrightnessSlider, 28);
    lv_obj_set_style_bg_color(ui_optionalNeoPixelBrightnessSlider, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optionalNeoPixelBrightnessSlider, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_optionalNeoPixelBrightnessSlider, lv_color_hex(0x00FF00), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optionalNeoPixelBrightnessSlider, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_optionalNeoPixelBrightnessSlider, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_optionalNeoPixelBrightnessSlider, 255, LV_PART_KNOB | LV_STATE_DEFAULT);

    ui_optionalNeoPixelBrightnessValue = lv_label_create(cui_optionalNeoPixelBrightnessPanel);
    lv_obj_set_width(ui_optionalNeoPixelBrightnessValue, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_optionalNeoPixelBrightnessValue, LV_SIZE_CONTENT); /// 1

    int32_t value3 = lv_slider_get_value(ui_optionalNeoPixelBrightnessSlider);
    lv_label_set_text_fmt(ui_optionalNeoPixelBrightnessValue, value3 < XTOUCH_LIGHT_MIN_SLEEP_TIME ? LV_SYMBOL_POWER : "%d", value3);
    lv_obj_set_style_text_font(ui_optionalNeoPixelBrightnessValue, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_optionalNeoPixelBrightnessValue, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_optionalNeoPixelBrightnessValue, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_optionalNeoPixelBrightnessValue, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_optionalNeoPixelBrightnessValue, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //---NEOPIXEL Brightness end------------------------------

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
    lv_label_set_text_fmt(ui_optionalAlarmTimeoutValue, value5 < 1 ? LV_SYMBOL_POWER : "%d", value5 * 30);
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

    //--- DS18B20 Sensor (Option) Title --------------------
    lv_obj_t *cui_optional_ds18b20Title;
    cui_optional_ds18b20Title = lv_label_create(cui_optionalComponent);
    lv_obj_set_width(cui_optional_ds18b20Title, lv_pct(100));
    lv_obj_set_height(cui_optional_ds18b20Title, LV_SIZE_CONTENT); /// 40
    lv_label_set_text(cui_optional_ds18b20Title, LV_SYMBOL_LIST " DS18B20 Sensor (Option)");
    lv_obj_set_scrollbar_mode(cui_optional_ds18b20Title, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_optional_ds18b20Title, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optional_ds18b20Title, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optional_ds18b20Title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optional_ds18b20Title, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optional_ds18b20Title, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_optional_ds18b20Title, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_optional_ds18b20Title, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_optional_ds18b20Title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_optional_ds18b20Title, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_optional_ds18b20Title, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

#if defined(__XTOUCH_SCREEN_S3_028__)
#define _UI_CHAMBER_TEMP_PIN 44
#elif defined(__XTOUCH_PLATFORM_S3__)
#define _UI_CHAMBER_TEMP_PIN 18
#else
#define _UI_CHAMBER_TEMP_PIN 22
#endif
    lv_obj_t *cui_optional_ds18b20GpioHint;
    cui_optional_ds18b20GpioHint = lv_label_create(cui_optionalComponent);
    lv_obj_set_width(cui_optional_ds18b20GpioHint, lv_pct(100));
    lv_obj_set_height(cui_optional_ds18b20GpioHint, LV_SIZE_CONTENT);
    lv_label_set_text_fmt(cui_optional_ds18b20GpioHint, "Connect to GPIO%d", _UI_CHAMBER_TEMP_PIN);
    lv_obj_set_scrollbar_mode(cui_optional_ds18b20GpioHint, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_optional_ds18b20GpioHint, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_optional_ds18b20GpioHint, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_optional_ds18b20GpioHint, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_optional_ds18b20GpioHint, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_optional_ds18b20GpioHint, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_optional_ds18b20GpioHint, lv_color_hex(0xaaaaaa), LV_PART_MAIN | LV_STATE_DEFAULT);
#undef _UI_CHAMBER_TEMP_PIN

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
        lv_obj_add_flag(cui_optional_ds18b20Title, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(cui_optional_ds18b20GpioHint, LV_OBJ_FLAG_HIDDEN);
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

    children[UI_COMP_OPTIONALCOMPONENT_NEOPIXEL] = cui_optionalNeoPixelBrightnessPanel;
    children[UI_COMP_OPTIONALCOMPONENT_NEOPIXEL_LABEL] = cui_optionalNeoPixelBrightnessPanelLabel;
    children[UI_COMP_OPTIONALCOMPONENT_NEOPIXEL_SLIDER] = ui_optionalNeoPixelBrightnessSlider;
    children[UI_COMP_OPTIONALCOMPONENT_NEOPIXEL_VALUE] = ui_optionalNeoPixelBrightnessValue;

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

    children[UI_COMP_OPTIONALCOMPONENT_PREHEAT] = cui_optional_preheat;
    children[UI_COMP_OPTIONALCOMPONENT_PREHEAT_LABEL] = cui_optional_preheatLabel;
    children[UI_COMP_OPTIONALCOMPONENT_PREHEAT_SWITCH] = ui_optional_preheatSwitch;

#if defined(__XTOUCH_PLATFORM_S3__)
    children[UI_COMP_OPTIONALCOMPONENT_MULTI_PRINTER_MONITOR] = cui_optional_multiPrinterMonitor;
    children[UI_COMP_OPTIONALCOMPONENT_MULTI_PRINTER_MONITOR_LABEL] = cui_optional_multiPrinterMonitorLabel;
    children[UI_COMP_OPTIONALCOMPONENT_MULTI_PRINTER_MONITOR_SWITCH] = ui_optional_multiPrinterMonitorSwitch;
    children[UI_COMP_OPTIONALCOMPONENT_HISTORY] = cui_optional_history;
    children[UI_COMP_OPTIONALCOMPONENT_HISTORY_LABEL] = cui_optional_historyLabel;
    children[UI_COMP_OPTIONALCOMPONENT_HISTORY_SWITCH] = ui_optional_historySwitch;
    children[UI_COMP_OPTIONALCOMPONENT_HIDE_THUMBNAILS] = cui_optional_hideThumbnails;
    children[UI_COMP_OPTIONALCOMPONENT_HIDE_THUMBNAILS_LABEL] = cui_optional_hideThumbnailsLabel;
    children[UI_COMP_OPTIONALCOMPONENT_HIDE_THUMBNAILS_SWITCH] = ui_optional_hideThumbnailsSwitch;
#else
    children[UI_COMP_OPTIONALCOMPONENT_MULTI_PRINTER_MONITOR] = NULL;
    children[UI_COMP_OPTIONALCOMPONENT_MULTI_PRINTER_MONITOR_LABEL] = NULL;
    children[UI_COMP_OPTIONALCOMPONENT_MULTI_PRINTER_MONITOR_SWITCH] = NULL;
    children[UI_COMP_OPTIONALCOMPONENT_HISTORY] = NULL;
    children[UI_COMP_OPTIONALCOMPONENT_HISTORY_LABEL] = NULL;
    children[UI_COMP_OPTIONALCOMPONENT_HISTORY_SWITCH] = NULL;
    children[UI_COMP_OPTIONALCOMPONENT_HIDE_THUMBNAILS] = NULL;
    children[UI_COMP_OPTIONALCOMPONENT_HIDE_THUMBNAILS_LABEL] = NULL;
    children[UI_COMP_OPTIONALCOMPONENT_HIDE_THUMBNAILS_SWITCH] = NULL;
#endif

   lv_obj_add_event_cb(cui_optionalComponent, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
   lv_obj_add_event_cb(cui_optionalComponent, del_component_child_event_cb, LV_EVENT_DELETE, children);

    lv_obj_add_event_cb(ui_optionalNeoPixelNumSlider, ui_event_comp_optionalComponent_onNeoPixelNum, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_optionalNeoPixelBrightnessSlider, ui_event_comp_optionalComponent_onNeoPixelBrightness, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_optionalAlarmTimeoutSlider, ui_event_comp_optionalComponent_onAlarmTimeout, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_optional_chamberSensorSwitch, ui_event_comp_optionalComponent_onChamberTemp, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_optional_stackChanSwitch, ui_event_comp_optionalComponent_onStackChan, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_optional_preheatSwitch, ui_event_comp_optionalComponent_onPreheat, LV_EVENT_VALUE_CHANGED, NULL);
#if defined(__XTOUCH_PLATFORM_S3__)
    lv_obj_add_event_cb(ui_optional_multiPrinterMonitorSwitch, ui_event_comp_optionalComponent_onMultiPrinterMonitor, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_optional_historySwitch, ui_event_comp_optionalComponent_onHistory, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_optional_hideThumbnailsSwitch, ui_event_comp_optionalComponent_onHideThumbnails, LV_EVENT_VALUE_CHANGED, NULL);
#endif
    lv_obj_add_event_cb(ui_optional_Idle_ledSwitch, ui_event_comp_optionalComponent_onIdleLED, LV_EVENT_VALUE_CHANGED, NULL);

    ui_comp_optionalComponent_create_hook(cui_optionalComponent);
    return cui_optionalComponent;
}
