#include "../ui.h"

// COMPONENT LightingPanel

void (*ui_LightingPanel_showOnYes)();
void (*ui_LightingPanel_showOnNo)();

void ui_LightingPanel_NOOP() {};
void ui_LightingPanel_show(const char *title, void (*onYES)(void))
{
    ui_LightingPanel_showOnYes = onYES;

    lv_obj_clear_flag(ui_lightingComponent, LV_OBJ_FLAG_HIDDEN); /// Flags
    lv_obj_t *titleLabel = ui_comp_get_child(ui_lightingComponent, UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER_CONFIRMPANELCAPTION);
    lv_label_set_text(titleLabel, title);

    // [](int a){
}

void ui_LightingPanel_show_with_no(const char *title, void (*onYES)(void), void (*onNO)(void))
{
    ui_LightingPanel_showOnNo = onNO;
    ui_LightingPanel_show(title, onYES);
}

void ui_LightingPanel_hide()
{
    printf("ui_LightingPanel_hide\n");
    lv_obj_add_flag(ui_lightingComponent, LV_OBJ_FLAG_HIDDEN); /// Flags
}

void ui_event_comp_LightingPanel_LightingPanelNO(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_LightingPanel = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        if (ui_LightingPanel_showOnNo != NULL)
        {
            ui_LightingPanel_showOnNo();
            ui_LightingPanel_showOnNo = NULL;
        }
        ui_LightingPanel_hide();
    }
}
void ui_event_comp_LightingPanel_LightingPanelYES(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_LightingPanel = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        if (ui_LightingPanel_showOnYes != NULL)
        {
            ui_LightingPanel_showOnYes();
            ui_LightingPanel_showOnYes = NULL;
        }
        ui_LightingPanel_hide();
    }
}

lv_obj_t *ui_LightingPanel_create(lv_obj_t *comp_parent)
{

    lv_obj_t *cui_LightingPanel;
    cui_LightingPanel = lv_obj_create(comp_parent);
    lv_obj_set_width(cui_LightingPanel, lv_pct(100));
    lv_obj_set_height(cui_LightingPanel, lv_pct(100));
    lv_obj_set_flex_flow(cui_LightingPanel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_LightingPanel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(cui_LightingPanel, LV_OBJ_FLAG_FLOATING);                                                                                                                                                                                                       /// Flags
    lv_obj_clear_flag(cui_LightingPanel, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_LightingPanel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_LightingPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_LightingPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_LightingPanel, 200, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_LightingPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_LightingPanelContainer;
    cui_LightingPanelContainer = lv_obj_create(cui_LightingPanel);
    lv_obj_set_width(cui_LightingPanelContainer, lv_pct(100));
    lv_obj_set_height(cui_LightingPanelContainer, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cui_LightingPanelContainer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_LightingPanelContainer, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_LightingPanelContainer, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_LightingPanelContainer, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_LightingPanelContainer, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_LightingPanelContainer, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_LightingPanelContainer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_LightingPanelCaption;
    cui_LightingPanelCaption = lv_label_create(cui_LightingPanelContainer);
    lv_obj_set_width(cui_LightingPanelCaption, lv_pct(100));      /// 1
    lv_obj_set_height(cui_LightingPanelCaption, LV_SIZE_CONTENT); /// 1
    lv_label_set_text(cui_LightingPanelCaption, "Are you sure?");
    lv_obj_set_style_text_font(cui_LightingPanelCaption, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(cui_LightingPanelCaption, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_LightingPanelCaption, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_left(cui_LightingPanelCaption, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_LightingPanelCaption, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_LightingPanelCaption, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_LightingPanelCaption, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_LightingPanelChamber;
    cui_LightingPanelChamber = lv_label_create(cui_LightingPanelContainer);
    lv_obj_set_height(cui_LightingPanelChamber, LV_SIZE_CONTENT); /// 1
    lv_obj_set_flex_grow(cui_LightingPanelChamber, 1);
    lv_obj_set_flex_flow(cui_LightingPanelChamber, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_LightingPanelChamber, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_label_set_text(cui_LightingPanelChamber, "r");
    lv_obj_add_flag(cui_LightingPanelChamber, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_LightingPanelChamber, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_LightingPanelChamber, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_LightingPanelChamber, &ui_font_xlcdmin, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_LightingPanelChamber, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_LightingPanelChamber, lv_color_hex(0xAA2A00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_LightingPanelChamber, lv_color_hex(0x552A00), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_LightingPanelChamber, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_LightingPanelChamber, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_LightingPanelChamber, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_LightingPanelChamber, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_LightingPanelChamber, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_LightingPanelNOLabel;
    cui_LightingPanelNOLabel = lv_label_create(cui_LightingPanelChamber);
    lv_obj_set_width(cui_LightingPanelNOLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_LightingPanelNOLabel, LV_SIZE_CONTENT); /// 1
    lv_label_set_text(cui_LightingPanelNOLabel, "Chamber Light");
    lv_obj_clear_flag(cui_LightingPanelNOLabel, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_LightingPanelNOLabel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_LightingPanelNOLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_LightingPanelNeoPixel;
    cui_LightingPanelNeoPixel = lv_label_create(cui_LightingPanelContainer);
    lv_obj_set_height(cui_LightingPanelNeoPixel, LV_SIZE_CONTENT); /// 1
    lv_obj_set_flex_grow(cui_LightingPanelNeoPixel, 1);
    lv_obj_set_flex_flow(cui_LightingPanelNeoPixel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_LightingPanelNeoPixel, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_label_set_text(cui_LightingPanelNeoPixel, "q");
    lv_obj_add_flag(cui_LightingPanelNeoPixel, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_LightingPanelNeoPixel, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_LightingPanelNeoPixel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_LightingPanelNeoPixel, &ui_font_xlcdmin, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_LightingPanelNeoPixel, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_LightingPanelNeoPixel, lv_color_hex(0x2AAA00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_LightingPanelNeoPixel, lv_color_hex(0x2A5500), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_LightingPanelNeoPixel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_LightingPanelNeoPixel, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_LightingPanelNeoPixel, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_LightingPanelNeoPixel, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_LightingPanelNeoPixel, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_LightingPanelYESLabel;
    cui_LightingPanelYESLabel = lv_label_create(cui_LightingPanelNeoPixel);
    lv_obj_set_width(cui_LightingPanelYESLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_LightingPanelYESLabel, LV_SIZE_CONTENT); /// 1
    lv_label_set_text(cui_LightingPanelYESLabel, "NeoPixel Light");
    lv_obj_clear_flag(cui_LightingPanelYESLabel, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_LightingPanelYESLabel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_LightingPanelYESLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t **children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_CONFIRMPANEL_NUM);
    children[UI_COMP_CONFIRMPANEL_CONFIRMPANEL] = cui_LightingPanel;
    children[UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER] = cui_LightingPanelContainer;
    children[UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER_CONFIRMPANELCAPTION] = cui_LightingPanelCaption;
    children[UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER_CONFIRMPANELNO] = cui_LightingPanelChamber;
    children[UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER_CONFIRMPANELNO1_CONFIRMPANELNOLABEL] = cui_LightingPanelNOLabel;
    children[UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER_CONFIRMPANELYES] = cui_LightingPanelNeoPixel;
    children[UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER_CONFIRMPANELYES_CONFIRMPANELYESLABEL] = cui_LightingPanelYESLabel;
    lv_obj_add_event_cb(cui_LightingPanel, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(cui_LightingPanel, del_component_child_event_cb, LV_EVENT_DELETE, children);
    lv_obj_add_event_cb(cui_LightingPanelChamber, ui_event_comp_LightingPanel_LightingPanelNO, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_LightingPanelNeoPixel, ui_event_comp_LightingPanel_LightingPanelYES, LV_EVENT_ALL, children);
    ui_comp_LightingPanel_create_hook(cui_LightingPanel);
    return cui_LightingPanel;
}