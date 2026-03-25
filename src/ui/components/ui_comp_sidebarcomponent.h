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
#ifdef __XTOUCH_SCREEN_50__
    UI_COMP_SIDEBARCOMPONENT_SIDEBARPRINTERSBUTTON,
    UI_COMP_SIDEBARCOMPONENT_SIDEBARPRINTERSBUTTON_SIDEBARPRINTERSBUTTONICON,
    UI_COMP_SIDEBARCOMPONENT_SIDEBARHISTORYBUTTON,
    UI_COMP_SIDEBARCOMPONENT_SIDEBARHISTORYBUTTON_SIDEBARHISTORYBUTTONICON,
#endif
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
#ifdef __XTOUCH_SCREEN_50__
    void ui_event_comp_sidebarComponent_sidebarPrintersButton(lv_event_t *e);
    void ui_event_comp_sidebarComponent_sidebarHistoryButton(lv_event_t *e);
#endif
    void ui_event_comp_sidebarComponent_sidebarTempButton(lv_event_t *e);
    void ui_event_comp_sidebarComponent_sidebarAmsViewButton(lv_event_t *e);
    void ui_event_comp_sidebarComponent_sidebarSettingsButton(lv_event_t *e);
    void ui_sidebarComponent_set_active(int index);
#ifdef __XTOUCH_SCREEN_50__
    /** 設定保存後などに、Printers ボタンの表示を xTouchConfig に合わせて更新する */
    void ui_sidebarComponent_updatePrintersVisibility(void);
    /** History 無効時はサイドバーから History を隠す（LAN-only 時も非表示） */
    void ui_sidebarComponent_updateHistoryVisibility(void);
#endif
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif