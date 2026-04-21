#include "../ui.h"

#ifdef __XTOUCH_PLATFORM_S3__
void ui_videoScreen_screen_init(void)
{
    ui_videoScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_videoScreen, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(ui_videoScreen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(ui_videoScreen, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_videoScreen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(ui_videoScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_sidebarComponent = ui_sidebarComponent_create(ui_videoScreen);
    lv_obj_set_x(ui_sidebarComponent, 387);
    lv_obj_set_y(ui_sidebarComponent, 178);

    lv_obj_t *content_col = lv_obj_create(ui_videoScreen);
    lv_obj_set_width(content_col, lv_pct(90));
    lv_obj_set_height(content_col, lv_pct(100));
    lv_obj_set_flex_grow(content_col, 1);
    lv_obj_clear_flag(content_col, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(content_col, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(content_col, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(content_col, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_videoComponent = ui_videoComponent_create(content_col);
    lv_obj_set_width(ui_videoComponent, lv_pct(100));
    lv_obj_set_height(ui_videoComponent, lv_pct(100));
#if !defined(__XTOUCH_SCREEN_S3_050__)
    lv_obj_set_width(ui_sidebarComponent, 48);
#endif
}
#endif
