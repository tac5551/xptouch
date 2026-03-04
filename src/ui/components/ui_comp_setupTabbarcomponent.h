#ifndef _UI_COMP_SETUPTABBARCOMPONENT_H
#define _UI_COMP_SETUPTABBARCOMPONENT_H

#include "../ui.h"

#ifdef __cplusplus
extern "C" {
#endif

enum SetupTabbarTab {
    UI_SETUPTABBAR_TAB_GENERAL,
    UI_SETUPTABBAR_TAB_OPTION,
    UI_SETUPTABBAR_TAB_SYSTEM
};

enum SetupTabbarComponent {
    UI_COMP_SETUPTABBARCOMPONENT_SETUPTABBARCOMPONENT,
    UI_COMP_SETUPTABBARCOMPONENT_BTN_GENERAL,
    UI_COMP_SETUPTABBARCOMPONENT_BTN_OPTION,
    UI_COMP_SETUPTABBARCOMPONENT_BTN_SYSTEM,
    _UI_COMP_SETUPTABBARCOMPONENT_NUM
};

/** general_panel, option_panel, system_panel are shown/hidden on tab switch. active_tab_index 0=General, 1=Option, 2=System. */
lv_obj_t *ui_setupTabbarComponent_create(lv_obj_t *comp_parent, lv_obj_t *general_panel, lv_obj_t *option_panel, lv_obj_t *system_panel, int active_tab_index);
void ui_setupTabbarComponent_set_active(lv_obj_t *comp, int index);

#ifdef __cplusplus
}
#endif

#endif
