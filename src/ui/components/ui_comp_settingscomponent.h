#ifndef _UI_COMP_SETTINGSCOMPONENT_H
#define _UI_COMP_SETTINGSCOMPONENT_H

#include "../ui.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // COMPONENT settingsComponent

    enum SettingsComponents
    {
        UI_COMP_SETTINGSCOMPONENT_SETTINGSCOMPONENT,
        UI_COMP_SETTINGSCOMPONENT_SCREEN_TITLE,
        UI_COMP_SETTINGSCOMPONENT_BACKLIGHT,
        UI_COMP_SETTINGSCOMPONENT_BACKLIGHT_LABEL,
        UI_COMP_SETTINGSCOMPONENT_BACKLIGHT_SLIDER,
        UI_COMP_SETTINGSCOMPONENT_TFTOFF,
        UI_COMP_SETTINGSCOMPONENT_TFTOFF_LABEL,
        UI_COMP_SETTINGSCOMPONENT_TFTOFF_SLIDER,
        UI_COMP_SETTINGSCOMPONENT_TFTOFF_VALUE,
        UI_COMP_SETTINGSCOMPONENT_TFT_INVERT,
        UI_COMP_SETTINGSCOMPONENT_TFT_INVERT_LABEL,
        UI_COMP_SETTINGSCOMPONENT_TFT_INVERT_INPUT,
        UI_COMP_SETTINGSCOMPONENT_TFT_WOP,
        UI_COMP_SETTINGSCOMPONENT_TFT_WOP_LABEL,
        UI_COMP_SETTINGSCOMPONENT_TFT_WOP_INPUT,
        UI_COMP_SETTINGSCOMPONENT_TFT_WDP,
        UI_COMP_SETTINGSCOMPONENT_TFT_WDP_LABEL,
        UI_COMP_SETTINGSCOMPONENT_TFT_WDP_INPUT,
        UI_COMP_SETTINGSCOMPONENT_TFT_FLIP,
        UI_COMP_SETTINGSCOMPONENT_TFT_FLIP_LABEL,
        UI_COMP_SETTINGSCOMPONENT_TFT_FLIP_INPUT,
        UI_COMP_SETTINGSCOMPONENT_LED_TITLE,
        UI_COMP_SETTINGSCOMPONENT_LEDOFF,
        UI_COMP_SETTINGSCOMPONENT_LEDOFF_LABEL,
        UI_COMP_SETTINGSCOMPONENT_LEDOFF_SLIDER,
        UI_COMP_SETTINGSCOMPONENT_LEDOFF_VALUE,
        UI_COMP_SETTINGSCOMPONENT_CHAMBERLEDWAKE,
        UI_COMP_SETTINGSCOMPONENT_CHAMBERLEDWAKE_LABEL,
        UI_COMP_SETTINGSCOMPONENT_CHAMBERLEDWAKE_SWITCH,
        UI_COMP_SETTINGSCOMPONENT_AUXFAN,
        UI_COMP_SETTINGSCOMPONENT_AUXFAN_LABEL,
        UI_COMP_SETTINGSCOMPONENT_AUXFAN_SWITCH,
        UI_COMP_SETTINGSCOMPONENT_CHAMBERFAN,
        UI_COMP_SETTINGSCOMPONENT_CHAMBERFAN_LABEL,
        UI_COMP_SETTINGSCOMPONENT_CHAMBERFAN_SWITCH,
        // System タブに移動: PAIR_TITLE, UNPAIRBUTTON, DEVICE_TITLE, OTA, RESETDEVICEBUTTON
        // UI_COMP_SETTINGSCOMPONENT_NEOPIXEL_NUM,
        // UI_COMP_SETTINGSCOMPONENT_NEOPIXEL_NUM_LABEL,
        // UI_COMP_SETTINGSCOMPONENT_NEOPIXEL_NUM_SLIDER,
        // UI_COMP_SETTINGSCOMPONENT_NEOPIXEL_NUM_VALUE,
        _UI_COMP_SETTINGSCOMPONENT_NUM
    };
    lv_obj_t *ui_settingsComponent_create(lv_obj_t *comp_parent);
    void ui_event_comp_settingsComponent_tftInvertInput(lv_event_t *e);
    void ui_event_comp_settingsComponent_resetTouchButton(lv_event_t *e);
    void ui_event_comp_settingsComponent_resetSettingsButton(lv_event_t *e);
    /* System component から参照 */
    void ui_event_comp_settingsComponent_unpairButton(lv_event_t *e);
    void ui_event_comp_settingsComponent_OtaUpdateNowButton(lv_event_t *e);
    void ui_event_comp_settingsComponent_resetDeviceButton(lv_event_t *e);
    void ui_event_comp_settingsComponent_clearCacheButton(lv_event_t *e);
    void ui_event_comp_settingsComponent_onAuxFan(lv_event_t *e);
    void ui_event_comp_settingsComponent_onChamberFan(lv_event_t *e);
    void ui_event_comp_settingsComponent_onChamberLedWake(lv_event_t *e);
    void ui_event_comp_settingsComponent_onOTA(lv_event_t *e);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif