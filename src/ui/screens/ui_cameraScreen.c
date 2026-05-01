#include "../ui.h"

#ifdef __XTOUCH_PLATFORM_S3__

void ui_cameraScreen_screen_init(void)
{
    ui_cameraScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_cameraScreen, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_set_scrollbar_mode(ui_cameraScreen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(ui_cameraScreen, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_cameraScreen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(ui_cameraScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_cameraScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_cameraScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_cameraScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_cameraScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_cameraScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_sidebarComponent = ui_sidebarComponent_create(ui_cameraScreen);
    lv_obj_set_x(ui_sidebarComponent, 387);
    lv_obj_set_y(ui_sidebarComponent, 178);

    lv_obj_t *content_col = lv_obj_create(ui_cameraScreen);
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

    ui_cameraComponent = ui_cameraComponent_create(content_col);
    lv_obj_set_width(ui_cameraComponent, lv_pct(100));
    lv_obj_set_height(ui_cameraComponent, lv_pct(100));
    lv_obj_set_flex_grow(ui_cameraComponent, 1);
}

#else

void ui_cameraScreen_screen_init(void)
{
    (void)0;
}

#endif
