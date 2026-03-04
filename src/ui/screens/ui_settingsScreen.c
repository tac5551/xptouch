#include "../ui.h"

void ui_settingsScreen_screen_init(void)
{
    ui_settingsScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_settingsScreen, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM); /// Flags
    lv_obj_set_scrollbar_mode(ui_settingsScreen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(ui_settingsScreen, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_settingsScreen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(ui_settingsScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_settingsScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_settingsScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_settingsScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_settingsScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_settingsScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_sidebarComponent = ui_sidebarComponent_create(ui_settingsScreen);
    lv_obj_set_x(ui_sidebarComponent, 387);
    lv_obj_set_y(ui_sidebarComponent, 178);

    lv_obj_t *content_col = lv_obj_create(ui_settingsScreen);
    lv_obj_set_width(content_col, lv_pct(90));
    lv_obj_set_height(content_col, lv_pct(100));
    lv_obj_set_flex_grow(content_col, 1);
    lv_obj_set_flex_flow(content_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(content_col, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_left(content_col, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(content_col, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(content_col, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(content_col, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(content_col, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(content_col, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(content_col, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(content_col, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *switcher = lv_obj_create(content_col);
    lv_obj_set_width(switcher, lv_pct(100));
    lv_obj_set_flex_grow(switcher, 1);
    lv_obj_set_flex_flow(switcher, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(switcher, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(switcher, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(switcher, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(switcher, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *general_panel = lv_obj_create(switcher);
    lv_obj_set_width(general_panel, lv_pct(100));
    lv_obj_set_flex_grow(general_panel, 1);
    lv_obj_set_flex_flow(general_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(general_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(general_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(general_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(general_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *option_panel = lv_obj_create(switcher);
    lv_obj_set_width(option_panel, lv_pct(100));
    lv_obj_set_flex_grow(option_panel, 1);
    lv_obj_set_flex_flow(option_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(option_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(option_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(option_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(option_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *system_panel = lv_obj_create(switcher);
    lv_obj_set_width(system_panel, lv_pct(100));
    lv_obj_set_flex_grow(system_panel, 1);
    lv_obj_set_flex_flow(system_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(system_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(system_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(system_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(system_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *setup_tabbar = ui_setupTabbarComponent_create(content_col, general_panel, option_panel, system_panel, UI_SETUPTABBAR_TAB_GENERAL);
    lv_obj_move_to_index(setup_tabbar, 0);

    ui_settingsComponent = ui_settingsComponent_create(general_panel);
    lv_obj_set_width(ui_settingsComponent, lv_pct(100));
    lv_obj_set_flex_grow(ui_settingsComponent, 1);
    lv_obj_set_x(ui_settingsComponent, 0);
    lv_obj_set_y(ui_settingsComponent, 0);

    ui_optionalComponent = ui_optionalComponent_create(option_panel);
    lv_obj_set_width(ui_optionalComponent, lv_pct(100));
    lv_obj_set_flex_grow(ui_optionalComponent, 1);
    lv_obj_set_x(ui_optionalComponent, 0);
    lv_obj_set_y(ui_optionalComponent, 0);

    lv_obj_t *ui_systemComponent = ui_systemComponent_create(system_panel);
    lv_obj_set_width(ui_systemComponent, lv_pct(100));
    lv_obj_set_flex_grow(ui_systemComponent, 1);
    lv_obj_set_x(ui_systemComponent, 0);
    lv_obj_set_y(ui_systemComponent, 0);

#ifdef __XTOUCH_SCREEN_28__
    lv_obj_set_width(ui_sidebarComponent, 48);
#endif
#ifdef __XTOUCH_SCREEN_3248__
    lv_obj_set_width(ui_sidebarComponent, 48);
#endif
}
