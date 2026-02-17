#ifndef _UI_COMP_HMSPANEL_H
#define _UI_COMP_HMSPANEL_H

#include "../ui.h"

#ifdef __cplusplus
extern "C"
{
#endif


enum XtouchDialogButtonStyle
{
    ONLY_CONFIRM = 0,
    CONFIRM_AND_CANCEL = 1,
    CONFIRM_AND_DONE = 2,
    CONFIRM_AND_RETRY = 3,
    DONE_AND_RETRY = 4,
    MAX_STYLE_NUM = 5
};


// COMPONENT hmsPanel
enum HmsPanelComponent {
    UI_COMP_HMSPANEL_HMSPANEL,
    UI_COMP_HMSPANEL_HMSPANELCONTAINER,
    UI_COMP_HMSPANEL_HMSPANELCONTAINER_HMSPANELCAPTION,
    UI_COMP_HMSPANEL_HMSPANELCONTAINER_HMSPANEL_RETRY_BUTTON,
    UI_COMP_HMSPANEL_HMSPANELCONTAINER_HMSPANEL_RETRY_BUTTON_LABEL,
    UI_COMP_HMSPANEL_HMSPANELCONTAINER_HMSPANEL_DONE_BUTTON,
    UI_COMP_HMSPANEL_HMSPANELCONTAINER_HMSPANEL_DONE_BUTTON_LABEL,
    UI_COMP_HMSPANEL_HMSPANELCONTAINER_HMSPANEL_CONFIRM_BUTTON,
    UI_COMP_HMSPANEL_HMSPANELCONTAINER_HMSPANEL_CONFIRM_BUTTON_LABEL,
    _UI_COMP_HMSPANEL_NUM
};

    lv_obj_t *ui_hmsPanel_create(lv_obj_t *comp_parent);
    void ui_hmsPanel_show(const char *title, enum XtouchDialogButtonStyle btn_style);
    void ui_hmsPanel_hide();
    const char *ui_hmsPanel_deviceHMSError(unsigned long long prefix);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif