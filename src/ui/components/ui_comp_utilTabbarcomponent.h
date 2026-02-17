#ifndef _UI_COMP_UTILTABBARCOMPONENT_H
#define _UI_COMP_UTILTABBARCOMPONENT_H

#include "../ui.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Tab order = left menu order: TEMP(1), AXIS(2), HOTEND(3), UTIL(11) */
enum UtilTabbarTab {
    UI_UTILTABBAR_TAB_TEMP,
    UI_UTILTABBAR_TAB_AXIS,
    UI_UTILTABBAR_TAB_HOTEND,
    UI_UTILTABBAR_TAB_UTIL
};

enum UtilTabbarComponent {
    UI_COMP_UTILTABBARCOMPONENT_UTILTABBARCOMPONENT,
    UI_COMP_UTILTABBARCOMPONENT_BTN_TEMP,
    UI_COMP_UTILTABBARCOMPONENT_BTN_AXIS,
    UI_COMP_UTILTABBARCOMPONENT_BTN_HOTEND,
    UI_COMP_UTILTABBARCOMPONENT_BTN_UTIL,
    _UI_COMP_UTILTABBARCOMPONENT_NUM
};

lv_obj_t *ui_utilTabbarComponent_create(lv_obj_t *comp_parent, int active_tab_index);
void ui_utilTabbarComponent_set_active(lv_obj_t *comp, int index);

#ifdef __cplusplus
}
#endif

#endif
