#ifndef _UI_EVENTS_H
#define _UI_EVENTS_H

#ifdef __cplusplus
extern "C"
{
#endif

    void initialActions(lv_event_t *e);

    /* sidebar */
    void onSidebarHome(lv_event_t *e);
    void onSidebarTemp(lv_event_t *e);
    void onSidebarControl(lv_event_t *e);
    void onSidebarNozzle(lv_event_t *e);
    void onSidebarSettings(lv_event_t *e);
    void onSidebarAmsView(lv_event_t *e);
    /* home */

    void onHomeControllerPlayPause(lv_event_t *e);
    void onHomeControllerStop(lv_event_t *e);
    void onHomeSpeedSelection(lv_event_t *e);
    void onHomeLight(lv_event_t *e);
    void onHomeLCD(lv_event_t *e);
    void onHomeNeoPixel(lv_event_t *e);
    void onHomeBedTemp(lv_event_t *e);
    void onHomeNozzleTemp(lv_event_t *e, int backToScreen);

    /* temps */

    void onTemperatureBed(const char *value);
    void onTemperatureNozzle(int value);
    void onTemperaturePart(lv_event_t *e);
    void onTemperatureAux(lv_event_t *e);
    void onTemperatureKeypad(lv_event_t *e);

    /* control */
    void onControlRange(lv_event_t *e);
    void onControlLeft(lv_event_t *e);
    void onControlUp(lv_event_t *e);
    void onControlHome(lv_event_t *e);
    void onControlDown(lv_event_t *e);
    void onControlBedUp(lv_event_t *e);
    void onControlBedDown(lv_event_t *e);
    void onControlAxis(lv_event_t *e);
    void onControlRight(lv_event_t *e);

    /* Settings */
    void onSettingsOtaUpdateNow(lv_event_t *e);
    void onSettingsMoveOptional(lv_event_t *e);
    void onSettingsResetDevice(lv_event_t *e);
    void onSettingsClearAccesCodeCache(lv_event_t *e);
    void onSettingsAuxFan(lv_event_t *e);
    void onSettingsChamberFan(lv_event_t *e);
    void onSettingsTFTFlip(lv_event_t *e);
    void onSettingsUnPair(lv_event_t *e);
    void onSettingsOTA(lv_event_t *e);
    void onSettingsWOP(lv_event_t *e);
    void onSettingsWDP(lv_event_t *e);

    void onOptionalChamberSensor(lv_event_t *e);
    void onOptionalStackChan(lv_event_t *e);
    void onOptionalIdleLED(lv_event_t *e);

    void onTouchStackChan(lv_event_t *e);

    void onMoveNozzleScreen(lv_event_t *e);
    void onMoveHomeScreen(lv_event_t *e);
    void onMoveAmsViewScreen(lv_event_t *e);

    /* -----------Confirm-------------- */
    void onConfirmPanelNO(lv_event_t *e);
    void onConfirmPanelYES(lv_event_t *e);

    /* filament */
    void onNozzleUp(lv_event_t *e);
    void onNozzleDown(lv_event_t *e);
    void onFilamentLoad(lv_event_t *e);
    void onFilamentUnload(lv_event_t *e);

    void onAmsSlotLoad(lv_event_t *e, int slot);

    /* pre heat */
    void onPreHeatPLA(lv_event_t *e);
    void onPreHeatABS(lv_event_t *e);
    void onPreHeatOff(lv_event_t *e);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
