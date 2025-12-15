#include "../ui.h"

void ui_utilNozzleChangeScreen_screen_init(void)
{
    ui_utilNozzleChangeScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_utilNozzleChangeScreen, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM); /// Flags
    lv_obj_set_scrollbar_mode(ui_utilNozzleChangeScreen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(ui_utilNozzleChangeScreen, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_utilNozzleChangeScreen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(ui_utilNozzleChangeScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_utilNozzleChangeScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_utilNozzleChangeScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_utilNozzleChangeScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_utilNozzleChangeScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_utilNozzleChangeScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_sidebarComponent = ui_sidebarComponent_create(ui_homeScreen);
    lv_obj_set_x(ui_sidebarComponent, 387);
    lv_obj_set_y(ui_sidebarComponent, 178);

    ui_utilNozzleChagneComponent = ui_utilNozzleChagneComponent_create(ui_utilNozzleChangeScreen);
    lv_obj_set_x(ui_utilNozzleChagneComponent, 385);
    lv_obj_set_y(ui_utilNozzleChagneComponent, 178);
#ifdef __XTOUCH_SCREEN_28__
    lv_obj_set_width(ui_sidebarComponent, 48);
#endif
}
