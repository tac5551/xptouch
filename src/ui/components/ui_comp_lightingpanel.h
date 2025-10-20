#ifndef _UI_COMP_LIGHTINGPANEL_H
#define _UI_COMP_LIGHTINGPANEL_H

#include "../ui.h"

#ifdef __cplusplus
extern "C"
{
#endif

// COMPONENT LightingPanel

enum LightingPanelComponents
{
UI_COMP_CONFIRMPANEL_LIGHTINGPANEL,
UI_COMP_CONFIRMPANEL_LIGHTINGPANELCONTAINER,
UI_COMP_CONFIRMPANEL_LIGHTINGPANELCONTAINER_CONFIRMPANELCAPTION,
UI_COMP_CONFIRMPANEL_LIGHTINGPANELCONTAINER_CONFIRMPANELNO,
UI_COMP_CONFIRMPANEL_LIGHTINGPANELCONTAINER_CONFIRMPANELNO1_CONFIRMPANELNOLABEL,
UI_COMP_CONFIRMPANEL_LIGHTINGPANELCONTAINER_CONFIRMPANELYES,
UI_COMP_CONFIRMPANEL_LIGHTINGPANELCONTAINER_CONFIRMPANELYES_CONFIRMPANELYESLABEL,
_UI_COMP_LIGHTINGPANEL_NUM
};
    lv_obj_t *ui_LightingPanel_create(lv_obj_t *comp_parent);
    void ui_LightingPanel_show(const char *title, void (*onYES)(void));
    void ui_LightingPanel_show_with_no(const char *title, void (*onYES)(void), void (*onNO)(void));
    void ui_LightingPanel_hide();
    void ui_LightingPanel_NOOP();
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif