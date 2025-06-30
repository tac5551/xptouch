#include "../ui.h"
void ui_amsViewScreen_screen_init(void)  {
    ui_amsViewScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_amsViewScreen, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM); /// Flags
    lv_obj_set_scrollbar_mode(ui_amsViewScreen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(ui_amsViewScreen, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_amsViewScreen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(ui_amsViewScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_amsViewScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_amsViewScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_amsViewScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_amsViewScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_amsViewScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_sidebarComponent = ui_sidebarComponent_create(ui_homeScreen);
    lv_obj_set_x(ui_sidebarComponent, 387);
    lv_obj_set_y(ui_sidebarComponent, 178);

    ui_amsViewComponent = ui_amsViewComponent_create(ui_amsViewScreen);
    lv_obj_set_x(ui_amsViewComponent, 386);
    lv_obj_set_y(ui_amsViewComponent, 178);
#ifdef __XTOUCH_SCREEN_28__
    lv_obj_set_width(ui_sidebarComponent, 48);
#endif

}