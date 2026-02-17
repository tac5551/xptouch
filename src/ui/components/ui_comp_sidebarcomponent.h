#ifndef _UI_COMP_SIDEBARCOMPONENT_H
#define _UI_COMP_SIDEBARCOMPONENT_H

#include "../ui.h"

#ifdef __cplusplus
extern "C"
{
#endif

// COMPONENT sidebarComponent
enum SidebarComponent {
    UI_COMP_SIDEBARCOMPONENT_SIDEBARCOMPONENT,
    UI_COMP_SIDEBARCOMPONENT_SIDEBARHOMEBUTTON,
    UI_COMP_SIDEBARCOMPONENT_SIDEBARHOMEBUTTON_SIDEBARHOMEBUTTONICON,
    UI_COMP_SIDEBARCOMPONENT_SIDEBARTEMPBUTTON,
    UI_COMP_SIDEBARCOMPONENT_SIDEBARTEMPBUTTON_SIDEBARTEMPBUTTONICON,
    UI_COMP_SIDEBARCOMPONENT_SIDEBARAMSVIEWBUTTON,
    UI_COMP_SIDEBARCOMPONENT_SIDEBARAMSVIEWBUTTON_SIDEBARAMSVIEWBUTTONICON,
    UI_COMP_SIDEBARCOMPONENT_SIDEBARSETTINGSBUTTON,
    UI_COMP_SIDEBARCOMPONENT_SIDEBARSETTINGSBUTTON_SIDEBARSETTINGSBUTTONICON,
    _UI_COMP_SIDEBARCOMPONENT_NUM
};

    lv_obj_t *ui_sidebarComponent_create(lv_obj_t *comp_parent);
    void ui_event_comp_sidebarComponent_sidebarHomeButton(lv_event_t *e);
    void ui_event_comp_sidebarComponent_sidebarTempButton(lv_event_t *e);
    void ui_event_comp_sidebarComponent_sidebarAmsViewButton(lv_event_t *e);
    void ui_event_comp_sidebarComponent_sidebarSettingsButton(lv_event_t *e);
    void ui_sidebarComponent_set_active(int index);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif