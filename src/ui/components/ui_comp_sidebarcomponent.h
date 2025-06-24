#ifndef _UI_COMP_SIDEBARCOMPONENT_H
#define _UI_COMP_SIDEBARCOMPONENT_H

#include "../ui.h"

#ifdef __cplusplus
extern "C"
{
#endif

// COMPONENT sidebarComponent
#define UI_COMP_SIDEBARCOMPONENT_SIDEBARCOMPONENT 0
#define UI_COMP_SIDEBARCOMPONENT_SIDEBARHOMEBUTTON 1
#define UI_COMP_SIDEBARCOMPONENT_SIDEBARHOMEBUTTON_SIDEBARHOMEBUTTONICON 2
#define UI_COMP_SIDEBARCOMPONENT_SIDEBARTEMPBUTTON 3
#define UI_COMP_SIDEBARCOMPONENT_SIDEBARTEMPBUTTON_SIDEBARTEMPBUTTONICON 4
#define UI_COMP_SIDEBARCOMPONENT_SIDEBARCONTROLBUTTON 5
#define UI_COMP_SIDEBARCOMPONENT_SIDEBARCONTROLBUTTON_SIDEBARCONTROLBUTTONICON 6
#define UI_COMP_SIDEBARCOMPONENT_SIDEBARNOZZLEBUTTON 7
#define UI_COMP_SIDEBARCOMPONENT_SIDEBARNOZZLEBUTTON_SIDEBARNOZZLEBUTTONICON 8
#define UI_COMP_SIDEBARCOMPONENT_SIDEBARSETTINGSBUTTON 9
#define UI_COMP_SIDEBARCOMPONENT_SIDEBARSETTINGSBUTTON_SIDEBARSETTINGSBUTTONICON 10
#define _UI_COMP_SIDEBARCOMPONENT_NUM 11
    lv_obj_t *ui_sidebarComponent_create(lv_obj_t *comp_parent);
    void ui_event_comp_sidebarComponent_sidebarHomeButton(lv_event_t *e);
    void ui_event_comp_sidebarComponent_sidebarTempButton(lv_event_t *e);
    void ui_event_comp_sidebarComponent_sidebarControlButton(lv_event_t *e);
    void ui_event_comp_sidebarComponent_sidebarNozzleButton(lv_event_t *e);
    void ui_event_comp_sidebarComponent_sidebarSettingsButton(lv_event_t *e);
    void ui_sidebarComponent_set_active(int index);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif