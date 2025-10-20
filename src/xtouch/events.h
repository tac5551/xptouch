#ifndef _XLCD_CONFIG
#define _XLCD_CONFIG

#include "xtouch/webserver.h"
#include "xtouch/firmware.h"
#include "xtouch/types.h"
#include "ui/ui_loaders.h"

void xtouch_events_onResetDevice(lv_msg_t *m)
{
    ESP.restart();
}

void xtouch_events_onOtaUpdateNow(lv_msg_t *m)
{
    printf("xtouch_events_onOtaUpdateNow\n");

    printf("stop webserver\n");
    lv_label_set_text_fmt(introScreenCaption, " Stopping Webserver ");
    lv_timer_handler();
    lv_task_handler();
    xtouch_webserver_end();

    // MQTT接続を切断
    if (xtouch_pubSubClient.connected())
    {
        lv_label_set_text_fmt(introScreenCaption, " disconnect Mqtt ");
        lv_timer_handler();
        lv_task_handler();
        xtouch_pubSubClient.disconnect();
    }

    xTouchConfig.xTouchOTAEnabled = true;
    xtouch_firmware_checkOnlineFirmwareUpdate();
}

void xtouch_events_onUnPair(lv_msg_t *m)
{
    cloud.unpair();
}

void xtouch_events_onCloudSelect(lv_msg_t *m)
{
    xtouch_cloud_pair_loop_exit = true;
}

void xtouch_events_onBackLight(lv_msg_t *m)
{
    int32_t value = lv_slider_get_value(ui_settingsBackLightPanelSlider);
    xTouchConfig.xTouchBacklightLevel = value;
    xtouch_screen_setBrightness(value);
}

void xtouch_events_onBackLightSet(lv_msg_t *m)
{
    int32_t value = lv_slider_get_value(ui_settingsBackLightPanelSlider);
    DynamicJsonDocument settings = xtouch_filesystem_readJson(SD, xtouch_paths_settings);
    settings["backlight"] = value;
    xTouchConfig.xTouchBacklightLevel = value;
    xtouch_filesystem_writeJson(SD, xtouch_paths_settings, settings);
    xtouch_screen_setBrightness(value);
}

void xtouch_events_onTFTTimerSet(lv_msg_t *m)
{
    int32_t value = lv_slider_get_value(ui_settingsTFTOFFSlider);
    xtouch_screen_setScreenTimer(value * 1000 * 60);

    DynamicJsonDocument settings = xtouch_filesystem_readJson(SD, xtouch_paths_settings);
    settings["tftOff"] = value;
    xTouchConfig.xTouchTFTOFFValue = value;
    xtouch_filesystem_writeJson(SD, xtouch_paths_settings, settings);
}

void xtouch_events_onLEDOffTimerSet(lv_msg_t *m)
{
    int32_t value = lv_slider_get_value(ui_settingsLEDOFFSlider);
    xtouch_screen_setLEDOffTimer(value * 1000 * 60);

    DynamicJsonDocument settings = xtouch_filesystem_readJson(SD, xtouch_paths_settings);
    settings["lightOff"] = value;
    xTouchConfig.xTouchLEDOffValue = value;
    xtouch_filesystem_writeJson(SD, xtouch_paths_settings, settings);
}



void xtouch_events_onNeoPixelBlightnessSet(lv_msg_t *m)
{
    int32_t value = lv_slider_get_value(ui_optionalNeoPixelBlightnessSlider);
    DynamicJsonDocument settings = xtouch_filesystem_readJson(SD, xtouch_paths_settings);
    settings["neoPixelBlightness"] = value;
    xTouchConfig.xTouchNeoPixelBlightnessValue = value;
    xtouch_filesystem_writeJson(SD, xtouch_paths_settings, settings);
    xtouch_neo_pixel_set_brightness(value);
}

void xtouch_events_onAlarmTimeoutSet(lv_msg_t *m)
{
    int32_t value = lv_slider_get_value(ui_optionalAlarmTimeoutSlider);
    DynamicJsonDocument settings = xtouch_filesystem_readJson(SD, xtouch_paths_settings);
    settings["alarmTimeout"] = value;
    xTouchConfig.xTouchAlarmTimeoutValue = value;
    xtouch_filesystem_writeJson(SD, xtouch_paths_settings, settings);
}

void xtouch_events_onIdleLEDSwitch(lv_msg_t *m)
{
    bool value = lv_obj_has_state(ui_optional_Idle_ledSwitch, LV_STATE_CHECKED);
    DynamicJsonDocument settings = xtouch_filesystem_readJson(SD, xtouch_paths_settings);
    settings["idleLEDEnabled"] = value;
    xTouchConfig.xTouchIdleLEDEnabled = value;
    xtouch_neo_pixel_set_idle_led_enabled(value);
    xtouch_filesystem_writeJson(SD, xtouch_paths_settings, settings);
}

void xtouch_events_onTFTInvert(lv_msg_t *m)
{
    bool value = lv_obj_has_state(ui_settingsTFTInvertSwitch, LV_STATE_CHECKED);
    DynamicJsonDocument settings = xtouch_filesystem_readJson(SD, xtouch_paths_settings);
    settings["tftInvert"] = value ? true : false;
    xtouch_filesystem_writeJson(SD, xtouch_paths_settings, settings);
    xTouchConfig.xTouchTFTInvert = value;
    xtouch_screen_invertColors();
}

void xtouch_events_onSettingsSave(lv_msg_t *m)
{
    xtouch_settings_save();
}

void xtouch_events_onTFTFlip(lv_msg_t *m)
{
    xtouch_screen_toggleTFTFlip();
}

void xtouch_events_onResetTouch(lv_msg_t *m)
{
    xtouch_resetTouchConfig();
}

void xtouch_events_onNeoPixelNumSet(lv_msg_t *m)
{
   int32_t value = lv_slider_get_value(ui_optionalNeoPixelNumSlider);
   ConsoleDebug.printf("xtouch_events_onNeoPixelNumSet: %d", value);
   DynamicJsonDocument settings = xtouch_filesystem_readJson(SD, xtouch_paths_settings);
   settings["neoPixelNum"] = value;
   xtouch_filesystem_writeJson(SD, xtouch_paths_settings, settings);
   xTouchConfig.xTouchNeoPixelNumValue = value;

    if (xTouchConfig.xTouchNeoPixelNumValue > 0)
    {
        xtouch_neo_pixel_control_timer_start(xTouchConfig.xTouchNeoPixelPinValue);
        xtouch_neo_pixel_set_num(value);
        xtouch_neo_pixel_reset_all();
    }
    else
    {
        xtouch_neo_pixel_control_timer_stop();
    }
}


void xtouch_events_onChamberTempSwitch(lv_msg_t *m)
{
    if (xTouchConfig.xTouchChamberSensorEnabled)
    {
        xtouch_chamber_timer_start();
    }
    else
    {
        xtouch_chamber_timer_stop();
    }
}

void xtouch_setupGlobalEvents()
{
    lv_msg_subscribe(XTOUCH_SETTINGS_RESET_DEVICE, (lv_msg_subscribe_cb_t)xtouch_events_onResetDevice, NULL);
    lv_msg_subscribe(XTOUCH_SETTINGS_OTA_UPDATE_NOW, (lv_msg_subscribe_cb_t)xtouch_events_onOtaUpdateNow, NULL);
    lv_msg_subscribe(XTOUCH_SETTINGS_UNPAIR, (lv_msg_subscribe_cb_t)xtouch_events_onUnPair, NULL);
    lv_msg_subscribe(XTOUCH_ON_CLOUD_SELECT, (lv_msg_subscribe_cb_t)xtouch_events_onCloudSelect, NULL);
    lv_msg_subscribe(XTOUCH_SETTINGS_BACKLIGHT, (lv_msg_subscribe_cb_t)xtouch_events_onBackLight, NULL);
    lv_msg_subscribe(XTOUCH_SETTINGS_BACKLIGHT_SET, (lv_msg_subscribe_cb_t)xtouch_events_onBackLightSet, NULL);
    lv_msg_subscribe(XTOUCH_SETTINGS_TFTOFF_SET, (lv_msg_subscribe_cb_t)xtouch_events_onTFTTimerSet, NULL);
    lv_msg_subscribe(XTOUCH_SETTINGS_LEDOFF_SET, (lv_msg_subscribe_cb_t)xtouch_events_onLEDOffTimerSet, NULL);
    // lv_msg_subscribe(XTOUCH_SETTINGS_NEOPIXEL_NUM_SET, (lv_msg_subscribe_cb_t)xtouch_events_onNeoPixelNumSet, NULL);
    // lv_msg_subscribe(XTOUCH_SETTINGS_NEOPIXEL_SET, (lv_msg_subscribe_cb_t)xtouch_events_onNeoPixelBlightnessSet, NULL);
    lv_msg_subscribe(XTOUCH_SETTINGS_TFT_INVERT, (lv_msg_subscribe_cb_t)xtouch_events_onTFTInvert, NULL);
    lv_msg_subscribe(XTOUCH_SETTINGS_SAVE, (lv_msg_subscribe_cb_t)xtouch_events_onSettingsSave, NULL);
    // lv_msg_subscribe(XTOUCH_SETTINGS_CHAMBER_TEMP, (lv_msg_subscribe_cb_t)xtouch_events_onChamberTempSwitch, NULL);
    lv_msg_subscribe(XTOUCH_SETTINGS_TFT_FLIP, (lv_msg_subscribe_cb_t)xtouch_events_onTFTFlip, NULL);

    lv_msg_subscribe(XTOUCH_OPTIONAL_NEOPIXEL_NUM_SET, (lv_msg_subscribe_cb_t)xtouch_events_onNeoPixelNumSet, NULL);
    lv_msg_subscribe(XTOUCH_OPTIONAL_NEOPIXEL_SET, (lv_msg_subscribe_cb_t)xtouch_events_onNeoPixelBlightnessSet, NULL);
    lv_msg_subscribe(XTOUCH_OPTIONAL_CHAMBER_TEMP, (lv_msg_subscribe_cb_t)xtouch_events_onChamberTempSwitch, NULL);
    lv_msg_subscribe(XTOUCH_OPTIONAL_ALARM_TIMEOUT_SET, (lv_msg_subscribe_cb_t)xtouch_events_onAlarmTimeoutSet, NULL);
    lv_msg_subscribe(XTOUCH_OPTIONAL_IDLE_LED_SET, (lv_msg_subscribe_cb_t)xtouch_events_onIdleLEDSwitch, NULL);

}

#endif