#include "../ui.h"

void ui_event_comp_sidebarComponent_sidebarHomeButton(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_sidebarComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onSidebarHome(e);
    }
}
#ifdef __XTOUCH_PLATFORM_S3__
void ui_event_comp_sidebarComponent_sidebarPrintersButton(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_sidebarComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onSidebarPrinters(e);
    }
}
void ui_event_comp_sidebarComponent_sidebarHistoryButton(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_sidebarComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onSidebarHistory(e);
    }
}
#endif
void ui_event_comp_sidebarComponent_sidebarTempButton(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_sidebarComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onSidebarTemp(e);
    }
}
void ui_event_comp_sidebarComponent_sidebarAmsViewButton(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_sidebarComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onSidebarAmsView(e);
    }
}

void ui_event_comp_sidebarComponent_sidebarSettingsButton(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_sidebarComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onSidebarSettings(e);
    }
}

#ifdef __XTOUCH_PLATFORM_S3__
void ui_sidebarComponent_updatePrintersVisibility(void)
{
    if (ui_sidebarComponent == NULL)
        return;
    lv_obj_t *btn = ui_comp_get_child(ui_sidebarComponent, UI_COMP_SIDEBARCOMPONENT_SIDEBARPRINTERSBUTTON);
    if (btn == NULL)
        return;
    if (xTouchConfig.xTouchLanOnlyMode || !xTouchConfig.xTouchMultiPrinterMonitorEnabled)
        lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_HIDDEN);
}

void ui_sidebarComponent_updateHistoryVisibility(void)
{
    if (ui_sidebarComponent == NULL)
        return;
    lv_obj_t *btn = ui_comp_get_child(ui_sidebarComponent, UI_COMP_SIDEBARCOMPONENT_SIDEBARHISTORYBUTTON);
    if (btn == NULL)
        return;
    if (xTouchConfig.xTouchLanOnlyMode || !xTouchConfig.xTouchHistoryEnabled)
        lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_HIDDEN);
}
#endif

void ui_sidebarComponent_set_active(int index)
{
    lv_obj_t *target;
#ifdef __XTOUCH_PLATFORM_S3__
    uint32_t indexes[6] = {
        UI_COMP_SIDEBARCOMPONENT_SIDEBARHOMEBUTTON,
        UI_COMP_SIDEBARCOMPONENT_SIDEBARPRINTERSBUTTON,
        UI_COMP_SIDEBARCOMPONENT_SIDEBARHISTORYBUTTON,
        UI_COMP_SIDEBARCOMPONENT_SIDEBARTEMPBUTTON,
        UI_COMP_SIDEBARCOMPONENT_SIDEBARAMSVIEWBUTTON,
        UI_COMP_SIDEBARCOMPONENT_SIDEBARSETTINGSBUTTON};
    const int n_buttons = 6;
#else
    uint32_t indexes[4] = {
        UI_COMP_SIDEBARCOMPONENT_SIDEBARHOMEBUTTON,
        UI_COMP_SIDEBARCOMPONENT_SIDEBARTEMPBUTTON,
        UI_COMP_SIDEBARCOMPONENT_SIDEBARAMSVIEWBUTTON,
        UI_COMP_SIDEBARCOMPONENT_SIDEBARSETTINGSBUTTON};
    const int n_buttons = 4;
#endif
    for (int i = 0; i < n_buttons; i++)
    {
        target = ui_comp_get_child(ui_sidebarComponent, indexes[i]);
        lv_obj_clear_state(target, LV_STATE_CHECKED);
    }
    if (index >= 0 && index < n_buttons)
    {
        uint32_t targetIndex = indexes[index];
        lv_obj_add_state(ui_comp_get_child(ui_sidebarComponent, targetIndex), LV_STATE_CHECKED);
    }
}
// COMPONENT sidebarComponent
lv_obj_t *ui_sidebarComponent_create(lv_obj_t *comp_parent)
{

    lv_obj_t *cui_sidebarComponent;
    cui_sidebarComponent = lv_obj_create(comp_parent);
    /* 5" のみ幅を割合で細く。2.8 等は従来どおり 48px（320×10% だと細すぎるため） */
#if defined(__XTOUCH_SCREEN_S3_050__)
    lv_obj_set_width(cui_sidebarComponent, lv_pct(10));
#else
    lv_obj_set_width(cui_sidebarComponent, 48);
#endif
    lv_obj_set_height(cui_sidebarComponent, lv_pct(100));
    lv_obj_set_x(cui_sidebarComponent, 387);
    lv_obj_set_y(cui_sidebarComponent, 178);
    lv_obj_set_flex_flow(cui_sidebarComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_sidebarComponent, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_sidebarComponent, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarComponent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_sidebarComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_sidebarComponent, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_sidebarComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_sidebarComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_sidebarComponent, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_sidebarComponent, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_sidebarComponent, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_sidebarComponent, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_sidebarComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_sidebarComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarComponent, lv_color_hex(0xDDDDDD), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_sidebarComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_sidebarHomeButton;
    cui_sidebarHomeButton = lv_obj_create(cui_sidebarComponent);
    lv_obj_set_width(cui_sidebarHomeButton, lv_pct(100));
    lv_obj_set_flex_grow(cui_sidebarHomeButton, 1);
    lv_obj_set_x(cui_sidebarHomeButton, 386);
    lv_obj_set_y(cui_sidebarHomeButton, 178);
    lv_obj_set_flex_flow(cui_sidebarHomeButton, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_sidebarHomeButton, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_sidebarHomeButton, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarHomeButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_sidebarHomeButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_sidebarHomeButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_sidebarHomeButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_sidebarHomeButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_sidebarHomeButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_sidebarHomeButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_sidebarHomeButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_sidebarHomeButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_sidebarHomeButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_sidebarHomeButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarHomeButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_sidebarHomeButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarHomeButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cui_sidebarHomeButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(cui_sidebarHomeButton, lv_color_hex(0x008800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_sidebarHomeButton, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_sidebarHomeButtonIcon;
    cui_sidebarHomeButtonIcon = lv_label_create(cui_sidebarHomeButton);
    lv_obj_set_width(cui_sidebarHomeButtonIcon, LV_SIZE_CONTENT);  /// 100
    lv_obj_set_height(cui_sidebarHomeButtonIcon, LV_SIZE_CONTENT); /// 24
    lv_label_set_text(cui_sidebarHomeButtonIcon, "a");
    lv_obj_clear_flag(cui_sidebarHomeButtonIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarHomeButtonIcon, LV_SCROLLBAR_MODE_OFF);
    /* 5" サイドバー: ui_font_xlcd48（他画面のスモールアイコンと同系） */
#if defined(__XTOUCH_SCREEN_S3_050__)
    lv_obj_set_style_text_font(cui_sidebarHomeButtonIcon, &ui_font_xlcd48, LV_PART_MAIN | LV_STATE_DEFAULT);
#else
    lv_obj_set_style_text_font(cui_sidebarHomeButtonIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif

#ifdef __XTOUCH_PLATFORM_S3__
    lv_obj_t *cui_sidebarPrintersButton;
    cui_sidebarPrintersButton = lv_obj_create(cui_sidebarComponent);
    lv_obj_set_width(cui_sidebarPrintersButton, lv_pct(100));
    lv_obj_set_flex_grow(cui_sidebarPrintersButton, 1);
    lv_obj_set_x(cui_sidebarPrintersButton, 386);
    lv_obj_set_y(cui_sidebarPrintersButton, 178);
    lv_obj_set_flex_flow(cui_sidebarPrintersButton, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_sidebarPrintersButton, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_sidebarPrintersButton, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(cui_sidebarPrintersButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_sidebarPrintersButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_sidebarPrintersButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_sidebarPrintersButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_sidebarPrintersButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_sidebarPrintersButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_sidebarPrintersButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_sidebarPrintersButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_sidebarPrintersButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_sidebarPrintersButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_sidebarPrintersButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarPrintersButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_sidebarPrintersButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarPrintersButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cui_sidebarPrintersButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(cui_sidebarPrintersButton, lv_color_hex(0x008800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_sidebarPrintersButton, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_sidebarPrintersButtonIcon;
    cui_sidebarPrintersButtonIcon = lv_label_create(cui_sidebarPrintersButton);
    lv_obj_set_width(cui_sidebarPrintersButtonIcon, LV_SIZE_CONTENT);
    lv_obj_set_height(cui_sidebarPrintersButtonIcon, LV_SIZE_CONTENT);
    lv_label_set_text(cui_sidebarPrintersButtonIcon, "B");
    lv_obj_clear_flag(cui_sidebarPrintersButtonIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(cui_sidebarPrintersButtonIcon, LV_SCROLLBAR_MODE_OFF);
 #if defined(__XTOUCH_SCREEN_S3_050__)
    lv_obj_set_style_text_font(cui_sidebarPrintersButtonIcon, &ui_font_xlcd48, LV_PART_MAIN | LV_STATE_DEFAULT);
 #else
    lv_obj_set_style_text_font(cui_sidebarPrintersButtonIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
 #endif
    if (xTouchConfig.xTouchLanOnlyMode || !xTouchConfig.xTouchMultiPrinterMonitorEnabled)
        lv_obj_add_flag(cui_sidebarPrintersButton, LV_OBJ_FLAG_HIDDEN);
#endif

#ifdef __XTOUCH_PLATFORM_S3__
    lv_obj_t *cui_sidebarHistoryButton;
    cui_sidebarHistoryButton = lv_obj_create(cui_sidebarComponent);
    lv_obj_set_width(cui_sidebarHistoryButton, lv_pct(100));
    lv_obj_set_flex_grow(cui_sidebarHistoryButton, 1);
    lv_obj_set_x(cui_sidebarHistoryButton, 386);
    lv_obj_set_y(cui_sidebarHistoryButton, 178);
    lv_obj_set_flex_flow(cui_sidebarHistoryButton, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_sidebarHistoryButton, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_sidebarHistoryButton, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(cui_sidebarHistoryButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_sidebarHistoryButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_sidebarHistoryButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_sidebarHistoryButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_sidebarHistoryButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_sidebarHistoryButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_sidebarHistoryButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_sidebarHistoryButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_sidebarHistoryButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_sidebarHistoryButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_sidebarHistoryButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarHistoryButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_sidebarHistoryButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarHistoryButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cui_sidebarHistoryButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(cui_sidebarHistoryButton, lv_color_hex(0x008800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_sidebarHistoryButton, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_t *cui_sidebarHistoryButtonIcon;
    cui_sidebarHistoryButtonIcon = lv_label_create(cui_sidebarHistoryButton);
    lv_obj_set_width(cui_sidebarHistoryButtonIcon, LV_SIZE_CONTENT);
    lv_obj_set_height(cui_sidebarHistoryButtonIcon, LV_SIZE_CONTENT);
    lv_label_set_text(cui_sidebarHistoryButtonIcon, "C");
    lv_obj_clear_flag(cui_sidebarHistoryButtonIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(cui_sidebarHistoryButtonIcon, LV_SCROLLBAR_MODE_OFF);
 #if defined(__XTOUCH_SCREEN_S3_050__)
    lv_obj_set_style_text_font(cui_sidebarHistoryButtonIcon, &ui_font_xlcd48, LV_PART_MAIN | LV_STATE_DEFAULT);
 #else
    lv_obj_set_style_text_font(cui_sidebarHistoryButtonIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
 #endif
    if (xTouchConfig.xTouchLanOnlyMode || !xTouchConfig.xTouchHistoryEnabled)
        lv_obj_add_flag(cui_sidebarHistoryButton, LV_OBJ_FLAG_HIDDEN);
#endif

    lv_obj_t *cui_sidebarTempButton;
    cui_sidebarTempButton = lv_obj_create(cui_sidebarComponent);
    lv_obj_set_width(cui_sidebarTempButton, lv_pct(100));
    lv_obj_set_flex_grow(cui_sidebarTempButton, 1);
    lv_obj_set_x(cui_sidebarTempButton, 386);
    lv_obj_set_y(cui_sidebarTempButton, 178);
    lv_obj_set_flex_flow(cui_sidebarTempButton, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_sidebarTempButton, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_sidebarTempButton, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarTempButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_sidebarTempButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_sidebarTempButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_sidebarTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_sidebarTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_sidebarTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_sidebarTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_sidebarTempButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_sidebarTempButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_sidebarTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_sidebarTempButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarTempButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_sidebarTempButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarTempButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cui_sidebarTempButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(cui_sidebarTempButton, lv_color_hex(0x008800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_sidebarTempButton, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_sidebarTempButtonIcon;
    cui_sidebarTempButtonIcon = lv_label_create(cui_sidebarTempButton);
    lv_obj_set_width(cui_sidebarTempButtonIcon, LV_SIZE_CONTENT);  /// 100
    lv_obj_set_height(cui_sidebarTempButtonIcon, LV_SIZE_CONTENT); /// 24
    lv_label_set_text(cui_sidebarTempButtonIcon, "p");
    lv_obj_clear_flag(cui_sidebarTempButtonIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarTempButtonIcon, LV_SCROLLBAR_MODE_OFF);
 #if defined(__XTOUCH_SCREEN_S3_050__)
    lv_obj_set_style_text_font(cui_sidebarTempButtonIcon, &ui_font_xlcd48, LV_PART_MAIN | LV_STATE_DEFAULT);
 #else
    lv_obj_set_style_text_font(cui_sidebarTempButtonIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
 #endif


    lv_obj_t *cui_sidebarAmsViewButton;
    cui_sidebarAmsViewButton = lv_obj_create(cui_sidebarComponent);
    lv_obj_set_width(cui_sidebarAmsViewButton, lv_pct(100));
    lv_obj_set_flex_grow(cui_sidebarAmsViewButton, 1);
    lv_obj_set_x(cui_sidebarAmsViewButton, 386);
    lv_obj_set_y(cui_sidebarAmsViewButton, 178);
    lv_obj_set_flex_flow(cui_sidebarAmsViewButton, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_sidebarAmsViewButton, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_sidebarAmsViewButton, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarAmsViewButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_sidebarAmsViewButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_sidebarAmsViewButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_sidebarAmsViewButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_sidebarAmsViewButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_sidebarAmsViewButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_sidebarAmsViewButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_sidebarAmsViewButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_sidebarAmsViewButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_sidebarAmsViewButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_sidebarAmsViewButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarAmsViewButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_sidebarAmsViewButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarAmsViewButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cui_sidebarAmsViewButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(cui_sidebarAmsViewButton, lv_color_hex(0x008800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_sidebarAmsViewButton, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_sidebarAmsViewButtonIcon;
    cui_sidebarAmsViewButtonIcon = lv_label_create(cui_sidebarAmsViewButton);
    lv_obj_set_width(cui_sidebarAmsViewButtonIcon, LV_SIZE_CONTENT);  /// 100
    lv_obj_set_height(cui_sidebarAmsViewButtonIcon, LV_SIZE_CONTENT); /// 24
    lv_label_set_text(cui_sidebarAmsViewButtonIcon, "n");
    lv_obj_clear_flag(cui_sidebarAmsViewButtonIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarAmsViewButtonIcon, LV_SCROLLBAR_MODE_OFF);
 #if defined(__XTOUCH_SCREEN_S3_050__)
    lv_obj_set_style_text_font(cui_sidebarAmsViewButtonIcon, &ui_font_xlcd48, LV_PART_MAIN | LV_STATE_DEFAULT);
 #else
    lv_obj_set_style_text_font(cui_sidebarAmsViewButtonIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
 #endif

    lv_obj_t *cui_sidebarSettingsButton;
    cui_sidebarSettingsButton = lv_obj_create(cui_sidebarComponent);
    lv_obj_set_width(cui_sidebarSettingsButton, lv_pct(100));
    lv_obj_set_flex_grow(cui_sidebarSettingsButton, 1);
    lv_obj_set_x(cui_sidebarSettingsButton, 386);
    lv_obj_set_y(cui_sidebarSettingsButton, 178);
    lv_obj_set_flex_flow(cui_sidebarSettingsButton, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_sidebarSettingsButton, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_sidebarSettingsButton, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarSettingsButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_sidebarSettingsButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_sidebarSettingsButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_sidebarSettingsButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_sidebarSettingsButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_sidebarSettingsButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_sidebarSettingsButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_sidebarSettingsButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_sidebarSettingsButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_sidebarSettingsButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_sidebarSettingsButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarSettingsButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_sidebarSettingsButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_sidebarSettingsButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cui_sidebarSettingsButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(cui_sidebarSettingsButton, lv_color_hex(0x008800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_sidebarSettingsButton, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_sidebarSettingsButtonIcon;
    cui_sidebarSettingsButtonIcon = lv_label_create(cui_sidebarSettingsButton);
    lv_obj_set_width(cui_sidebarSettingsButtonIcon, LV_SIZE_CONTENT);  /// 100
    lv_obj_set_height(cui_sidebarSettingsButtonIcon, LV_SIZE_CONTENT); /// 24
    lv_label_set_text(cui_sidebarSettingsButtonIcon, "d");
    lv_obj_clear_flag(cui_sidebarSettingsButtonIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_sidebarSettingsButtonIcon, LV_SCROLLBAR_MODE_OFF);
 #if defined(__XTOUCH_SCREEN_S3_050__)
    lv_obj_set_style_text_font(cui_sidebarSettingsButtonIcon, &ui_font_xlcd48, LV_PART_MAIN | LV_STATE_DEFAULT);
 #else
    lv_obj_set_style_text_font(cui_sidebarSettingsButtonIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
 #endif

    lv_obj_t **children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_SIDEBARCOMPONENT_NUM);
    children[UI_COMP_SIDEBARCOMPONENT_SIDEBARCOMPONENT] = cui_sidebarComponent;
    children[UI_COMP_SIDEBARCOMPONENT_SIDEBARHOMEBUTTON] = cui_sidebarHomeButton;
    children[UI_COMP_SIDEBARCOMPONENT_SIDEBARHOMEBUTTON_SIDEBARHOMEBUTTONICON] = cui_sidebarHomeButtonIcon;
#ifdef __XTOUCH_PLATFORM_S3__
    children[UI_COMP_SIDEBARCOMPONENT_SIDEBARPRINTERSBUTTON] = cui_sidebarPrintersButton;
    children[UI_COMP_SIDEBARCOMPONENT_SIDEBARPRINTERSBUTTON_SIDEBARPRINTERSBUTTONICON] = cui_sidebarPrintersButtonIcon;
    children[UI_COMP_SIDEBARCOMPONENT_SIDEBARHISTORYBUTTON] = cui_sidebarHistoryButton;
    children[UI_COMP_SIDEBARCOMPONENT_SIDEBARHISTORYBUTTON_SIDEBARHISTORYBUTTONICON] = cui_sidebarHistoryButtonIcon;
#endif
    children[UI_COMP_SIDEBARCOMPONENT_SIDEBARTEMPBUTTON] = cui_sidebarTempButton;
    children[UI_COMP_SIDEBARCOMPONENT_SIDEBARTEMPBUTTON_SIDEBARTEMPBUTTONICON] = cui_sidebarTempButtonIcon;
    children[UI_COMP_SIDEBARCOMPONENT_SIDEBARAMSVIEWBUTTON] = cui_sidebarAmsViewButton;
    children[UI_COMP_SIDEBARCOMPONENT_SIDEBARAMSVIEWBUTTON_SIDEBARAMSVIEWBUTTONICON] = cui_sidebarAmsViewButtonIcon;
    children[UI_COMP_SIDEBARCOMPONENT_SIDEBARSETTINGSBUTTON] = cui_sidebarSettingsButton;
    children[UI_COMP_SIDEBARCOMPONENT_SIDEBARSETTINGSBUTTON_SIDEBARSETTINGSBUTTONICON] = cui_sidebarSettingsButtonIcon;
    lv_obj_add_event_cb(cui_sidebarComponent, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(cui_sidebarComponent, del_component_child_event_cb, LV_EVENT_DELETE, children);
    lv_obj_add_event_cb(cui_sidebarHomeButton, ui_event_comp_sidebarComponent_sidebarHomeButton, LV_EVENT_ALL, children);
#ifdef __XTOUCH_PLATFORM_S3__
    lv_obj_add_event_cb(cui_sidebarPrintersButton, ui_event_comp_sidebarComponent_sidebarPrintersButton, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_sidebarHistoryButton, ui_event_comp_sidebarComponent_sidebarHistoryButton, LV_EVENT_ALL, children);
#endif
    lv_obj_add_event_cb(cui_sidebarTempButton, ui_event_comp_sidebarComponent_sidebarTempButton, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_sidebarAmsViewButton, ui_event_comp_sidebarComponent_sidebarAmsViewButton, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_sidebarSettingsButton, ui_event_comp_sidebarComponent_sidebarSettingsButton, LV_EVENT_ALL, children);
    ui_comp_sidebarComponent_create_hook(cui_sidebarComponent);

    lv_msg_subsribe_obj(XTOUCH_ON_AMS_BITS, cui_sidebarAmsViewButton, NULL);
    ui_msg_send(XTOUCH_ON_AMS_BITS, 0, 0);

    return cui_sidebarComponent;
}
