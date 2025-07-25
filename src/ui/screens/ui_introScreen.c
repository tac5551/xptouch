#include "../ui.h"

void ui_introScreen_screen_init()
{
    introScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(introScreen, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM); /// Flags
    lv_obj_set_scrollbar_mode(introScreen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(introScreen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(introScreen, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_color(introScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(introScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(introScreen, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(introScreen, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(introScreen, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(introScreen, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(introScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(introScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    introScreenIcon = lv_img_create(introScreen);
    lv_obj_set_width(introScreenIcon, LV_SIZE_CONTENT);                                                                                                                                                                                                           ///? 100
    lv_obj_set_height(introScreenIcon, LV_SIZE_CONTENT);                                                                                                                                                                                                          /// 100
    lv_obj_clear_flag(introScreenIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(introScreenIcon, LV_SCROLLBAR_MODE_OFF);
    lv_img_set_src(introScreenIcon, &img_logo);

    introScreenCaption = lv_label_create(introScreen);
    lv_obj_set_width(introScreenCaption, LV_SIZE_CONTENT);  /// 100
    lv_obj_set_height(introScreenCaption, LV_SIZE_CONTENT); /// 100
    lv_label_set_text(introScreenCaption, LV_SYMBOL_SD_CARD);
    lv_obj_clear_flag(introScreenCaption, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(introScreenCaption, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(introScreenCaption, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(introScreenCaption, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);

#ifdef __XTOUCH_SCREEN_50__
    lv_obj_set_style_pad_top(introScreenCaption, 88, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_img_set_zoom(introScreenIcon, 384);
#else
    lv_obj_set_style_pad_top(introScreenCaption, 68, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif
}
