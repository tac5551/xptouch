#include "../ui.h"

void ui_characterScreen_screen_init(void)
{
printf("ui_characterScreen : lv_obj_create\n");
    ui_characterScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_characterScreen, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM); /// Flags
    lv_obj_set_scrollbar_mode(ui_characterScreen, LV_SCROLLBAR_MODE_OFF);
        
    lv_obj_set_style_pad_left(ui_characterScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_characterScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_characterScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_characterScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_characterScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_characterScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

printf("ui_characterScreen : ui_characterComponent_create\n");
    ui_characterComponent = ui_characterComponent_create(ui_characterScreen);

printf("ok1\n");
    xtouch_character_init();
}
