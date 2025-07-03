#ifndef _UI_COMP_CONFIRMPANEL_H
#define _UI_COMP_CONFIRMPANEL_H

#include "../ui.h"

#ifdef __cplusplus
extern "C"
{
#endif

// COMPONENT confirmPanel

enum ConfirmPanelComponents
{
UI_COMP_CONFIRMPANEL_CONFIRMPANEL,
UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER,
UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER_CONFIRMPANELCAPTION,
UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER_CONFIRMPANELNO,
UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER_CONFIRMPANELNO1_CONFIRMPANELNOLABEL,
UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER_CONFIRMPANELYES,
UI_COMP_CONFIRMPANEL_CONFIRMPANELCONTAINER_CONFIRMPANELYES_CONFIRMPANELYESLABEL,
_UI_COMP_CONFIRMPANEL_NUM
};
    lv_obj_t *ui_confirmPanel_create(lv_obj_t *comp_parent);
    void ui_confirmPanel_show(const char *title, void (*onYES)(void));
    void ui_confirmPanel_show_with_no(const char *title, void (*onYES)(void), void (*onNO)(void));
    void ui_confirmPanel_hide();
    void ui_confirmPanel_NOOP();
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif