#ifndef _UI_COMP_AMSEDITCOMPONENT_H
#define _UI_COMP_AMSEDITCOMPONENT_H

#include "../ui.h"

#ifdef __cplusplus
extern "C" {
#endif

enum AmsEditComponentChildren {
    UI_COMP_AMSEDITCOMPONENT_AMSEDITCOMPONENT,
    UI_COMP_AMSEDITCOMPONENT_TITLE,
    UI_COMP_AMSEDITCOMPONENT_CONTENT,
    UI_COMP_AMSEDITCOMPONENT_DROPDOWN_BRAND,
    UI_COMP_AMSEDITCOMPONENT_DROPDOWN_TYPE,
    UI_COMP_AMSEDITCOMPONENT_COLOR_PANEL,
    UI_COMP_AMSEDITCOMPONENT_COLOR_BTN,
    UI_COMP_AMSEDITCOMPONENT_FOOTER_BACK,
    UI_COMP_AMSEDITCOMPONENT_FOOTER_RESET,
    UI_COMP_AMSEDITCOMPONENT_FOOTER_SAVE,
    _UI_COMP_AMSEDITCOMPONENT_NUM
};
lv_obj_t *ui_amsEditComponent_create(lv_obj_t *comp_parent);
/** 画面表示時に Brand/Type ドロップダウンを構造体の最新内容で更新する。loadScreen(13) のあとで呼ぶ。 */
void ui_amsEditComponent_refresh_filament_options(void);

#ifdef __cplusplus
}
#endif

#endif
