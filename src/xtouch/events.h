#ifndef _XLCD_CONFIG
#define _XLCD_CONFIG

#include "xtouch/webserver.h"
#include "xtouch/firmware.h"
#include "xtouch/sdcard.h"
#include "xtouch/types.h"
#include "xtouch/demo.h"
#include "ui/ui_loaders.h"

void xptouch_events_onResetDevice(lv_msg_t *m)
{
    ESP.restart();
}

void xptouch_events_onOtaUpdateNow(lv_msg_t *m)
{
    printf("xptouch_events_onOtaUpdateNow\n");

    printf("stop webserver\n");
    lv_label_set_text_fmt(introScreenCaption, " Stopping Webserver ");
    lv_timer_handler();
    lv_task_handler();
    xptouch_webserver_end();

    // MQTT接続を切断
    if (xptouch_pubSubClient.connected())
    {
        lv_label_set_text_fmt(introScreenCaption, " disconnect Mqtt ");
        lv_timer_handler();
        lv_task_handler();
        xptouch_pubSubClient.disconnect();
    }

    xptouch_firmware_checkOnlineFirmwareUpdate(true);
}

void xptouch_events_onUnPair(lv_msg_t *m)
{
    cloud.unpair();
}

void xptouch_events_onCloudSelect(lv_msg_t *m)
{
    xptouch_cloud_pair_loop_exit = true;
}

void xptouch_events_onBackLight(lv_msg_t *m)
{
    int32_t value = lv_slider_get_value(ui_settingsBackLightPanelSlider);
    xPTouchConfig.xTouchBacklightLevel = value;
    xptouch_screen_setBrightness(value);
}

void xptouch_events_onBackLightSet(lv_msg_t *m)
{
    int32_t value = lv_slider_get_value(ui_settingsBackLightPanelSlider);
    DynamicJsonDocument settings = xptouch_filesystem_readJson(xptouch_sdcard_fs(), xptouch_paths_settings);
    settings["backlight"] = value;
    xPTouchConfig.xTouchBacklightLevel = value;
    xptouch_filesystem_writeJson(xptouch_sdcard_fs(), xptouch_paths_settings, settings);
    xptouch_screen_setBrightness(value);
}

void xptouch_events_onTFTTimerSet(lv_msg_t *m)
{
    int32_t value = lv_slider_get_value(ui_settingsTFTOFFSlider);
    xptouch_screen_setScreenTimer(value * 1000 * 60);

    DynamicJsonDocument settings = xptouch_filesystem_readJson(xptouch_sdcard_fs(), xptouch_paths_settings);
    settings["tftOff"] = value;
    xPTouchConfig.xTouchTFTOFFValue = value;
    xptouch_filesystem_writeJson(xptouch_sdcard_fs(), xptouch_paths_settings, settings);
}

void xptouch_events_onLEDOffTimerSet(lv_msg_t *m)
{
    int32_t value = lv_slider_get_value(ui_settingsLEDOFFSlider);
    xptouch_screen_setLEDOffTimer(value * 1000 * 60);

    DynamicJsonDocument settings = xptouch_filesystem_readJson(xptouch_sdcard_fs(), xptouch_paths_settings);
    settings["lightOff"] = value;
    xPTouchConfig.xTouchLEDOffValue = value;
    xptouch_filesystem_writeJson(xptouch_sdcard_fs(), xptouch_paths_settings, settings);
}



void xptouch_events_onNeoPixelBrightnessSet(lv_msg_t *m)
{
    int32_t value = lv_slider_get_value(ui_optionalNeoPixelBrightnessSlider);
    DynamicJsonDocument settings = xptouch_filesystem_readJson(xptouch_sdcard_fs(), xptouch_paths_settings);
    settings["neoPixelBrightness"] = value;
    xPTouchConfig.xTouchNeoPixelBrightnessValue = value;
    xptouch_filesystem_writeJson(xptouch_sdcard_fs(), xptouch_paths_settings, settings);
    xptouch_neo_pixel_set_brightness(value);
}

void xptouch_events_onAlarmTimeoutSet(lv_msg_t *m)
{
    int32_t value = lv_slider_get_value(ui_optionalAlarmTimeoutSlider);
    DynamicJsonDocument settings = xptouch_filesystem_readJson(xptouch_sdcard_fs(), xptouch_paths_settings);
    settings["alarmTimeout"] = value;
    xPTouchConfig.xTouchAlarmTimeoutValue = value;
    xptouch_filesystem_writeJson(xptouch_sdcard_fs(), xptouch_paths_settings, settings);
}

void xptouch_events_onIdleLEDSwitch(lv_msg_t *m)
{
    bool value = lv_obj_has_state(ui_optional_Idle_ledSwitch, LV_STATE_CHECKED);
    DynamicJsonDocument settings = xptouch_filesystem_readJson(xptouch_sdcard_fs(), xptouch_paths_settings);
    settings["idleLEDEnabled"] = value;
    xPTouchConfig.xTouchIdleLEDEnabled = value;
    xptouch_neo_pixel_set_idle_led_enabled(value);
    xptouch_filesystem_writeJson(xptouch_sdcard_fs(), xptouch_paths_settings, settings);
}

void xptouch_events_onTFTInvert(lv_msg_t *m)
{
    bool value = lv_obj_has_state(ui_settingsTFTInvertSwitch, LV_STATE_CHECKED);
    DynamicJsonDocument settings = xptouch_filesystem_readJson(xptouch_sdcard_fs(), xptouch_paths_settings);
    settings["tftInvert"] = value ? true : false;
    xptouch_filesystem_writeJson(xptouch_sdcard_fs(), xptouch_paths_settings, settings);
    xPTouchConfig.xTouchTFTInvert = value;
    xptouch_screen_invertColors();
}

/* 下で定義するが、SettingsSave から先に呼ぶため前方宣言 */
void xptouch_events_onChamberTempSwitch(lv_msg_t *m);

void xptouch_events_onSettingsSave(lv_msg_t *m)
{
    xptouch_settings_save();
    /* 手動SAVE時にのみ反映。スイッチ操作直後の重い処理は避ける */
    xptouch_events_onChamberTempSwitch(m);
    xptouch_neo_pixel_set_idle_led_enabled(xPTouchConfig.xTouchIdleLEDEnabled);
    lv_msg_send(XPTOUCH_THUMBNAILS_HIDE_MODE_CHANGED, NULL);
}

void xptouch_events_onTFTFlip(lv_msg_t *m)
{
    xptouch_screen_toggleTFTFlip();
}

void xptouch_events_onResetTouch(lv_msg_t *m)
{
    xptouch_resetTouchConfig();
}

void xptouch_events_onNeoPixelNumSet(lv_msg_t *m)
{
   int32_t value = lv_slider_get_value(ui_optionalNeoPixelNumSlider);
   ConsoleDebug.printf("xptouch_events_onNeoPixelNumSet: %d", value);
   DynamicJsonDocument settings = xptouch_filesystem_readJson(xptouch_sdcard_fs(), xptouch_paths_settings);
   settings["neoPixelNum"] = value;
   xptouch_filesystem_writeJson(xptouch_sdcard_fs(), xptouch_paths_settings, settings);
   xPTouchConfig.xTouchNeoPixelNumValue = value;

    if (xPTouchConfig.xTouchNeoPixelNumValue > 0)
    {
        xptouch_neo_pixel_control_timer_start(xPTouchConfig.xTouchNeoPixelPinValue);
        xptouch_neo_pixel_set_num(value);
        xptouch_neo_pixel_reset_all();
    }
    else
    {
        xptouch_neo_pixel_control_timer_stop();
    }
}


void xptouch_events_onChamberTempSwitch(lv_msg_t *m)
{
    if (xPTouchConfig.xTouchChamberSensorEnabled)
    {
        xptouch_chamber_timer_start();
    }
    else
    {
        xptouch_chamber_timer_stop();
    }
}

void xptouch_events_onClearCache(lv_msg_t *m)
{
    (void)m;
#if defined(__XPTOUCH_PLATFORM_S3__)
    xptouch_thumbnail_clear_sd_cache();
#endif
}

void xptouch_events_onDemoModeToggle(lv_msg_t *m)
{
    (void)m;
    xptouch_demo_toggle_and_restart();
}

void xptouch_setupGlobalEvents()
{
    lv_msg_subscribe(XPTOUCH_SETTINGS_RESET_DEVICE, (lv_msg_subscribe_cb_t)xptouch_events_onResetDevice, NULL);
    lv_msg_subscribe(XPTOUCH_SETTINGS_DEMO_MODE_TOGGLE, (lv_msg_subscribe_cb_t)xptouch_events_onDemoModeToggle, NULL);
    lv_msg_subscribe(XPTOUCH_SETTINGS_OTA_UPDATE_NOW, (lv_msg_subscribe_cb_t)xptouch_events_onOtaUpdateNow, NULL);
    lv_msg_subscribe(XPTOUCH_SETTINGS_CLEAR_CACHE, (lv_msg_subscribe_cb_t)xptouch_events_onClearCache, NULL);
    lv_msg_subscribe(XPTOUCH_SETTINGS_UNPAIR, (lv_msg_subscribe_cb_t)xptouch_events_onUnPair, NULL);
    lv_msg_subscribe(XPTOUCH_ON_CLOUD_SELECT, (lv_msg_subscribe_cb_t)xptouch_events_onCloudSelect, NULL);
    lv_msg_subscribe(XPTOUCH_SETTINGS_BACKLIGHT, (lv_msg_subscribe_cb_t)xptouch_events_onBackLight, NULL);
    lv_msg_subscribe(XPTOUCH_SETTINGS_BACKLIGHT_SET, (lv_msg_subscribe_cb_t)xptouch_events_onBackLightSet, NULL);
    lv_msg_subscribe(XPTOUCH_SETTINGS_TFTOFF_SET, (lv_msg_subscribe_cb_t)xptouch_events_onTFTTimerSet, NULL);
    lv_msg_subscribe(XPTOUCH_SETTINGS_LEDOFF_SET, (lv_msg_subscribe_cb_t)xptouch_events_onLEDOffTimerSet, NULL);
    // lv_msg_subscribe(XPTOUCH_SETTINGS_NEOPIXEL_NUM_SET, (lv_msg_subscribe_cb_t)xptouch_events_onNeoPixelNumSet, NULL);
    // lv_msg_subscribe(XPTOUCH_SETTINGS_NEOPIXEL_SET, (lv_msg_subscribe_cb_t)xptouch_events_onNeoPixelBrightnessSet, NULL);
    lv_msg_subscribe(XPTOUCH_SETTINGS_TFT_INVERT, (lv_msg_subscribe_cb_t)xptouch_events_onTFTInvert, NULL);
    lv_msg_subscribe(XPTOUCH_SETTINGS_SAVE, (lv_msg_subscribe_cb_t)xptouch_events_onSettingsSave, NULL);
    // lv_msg_subscribe(XPTOUCH_SETTINGS_CHAMBER_TEMP, (lv_msg_subscribe_cb_t)xptouch_events_onChamberTempSwitch, NULL);
    lv_msg_subscribe(XPTOUCH_SETTINGS_TFT_FLIP, (lv_msg_subscribe_cb_t)xptouch_events_onTFTFlip, NULL);

    lv_msg_subscribe(XPTOUCH_OPTIONAL_NEOPIXEL_NUM_SET, (lv_msg_subscribe_cb_t)xptouch_events_onNeoPixelNumSet, NULL);
    lv_msg_subscribe(XPTOUCH_OPTIONAL_NEOPIXEL_SET, (lv_msg_subscribe_cb_t)xptouch_events_onNeoPixelBrightnessSet, NULL);
    lv_msg_subscribe(XPTOUCH_OPTIONAL_CHAMBER_TEMP, (lv_msg_subscribe_cb_t)xptouch_events_onChamberTempSwitch, NULL);
    lv_msg_subscribe(XPTOUCH_OPTIONAL_ALARM_TIMEOUT_SET, (lv_msg_subscribe_cb_t)xptouch_events_onAlarmTimeoutSet, NULL);
    lv_msg_subscribe(XPTOUCH_OPTIONAL_IDLE_LED_SET, (lv_msg_subscribe_cb_t)xptouch_events_onIdleLEDSwitch, NULL);

    lv_msg_subscribe(XPTOUCH_COMMAND_NEOPIXEL_TOGGLE, (lv_msg_subscribe_cb_t)xptouch_device_onNeoPixelToggleCommand, NULL);

    lv_msg_subscribe(XPTOUCH_PREHEAT_BUTTON1, (lv_msg_subscribe_cb_t)xptouch_device_onPreHeatPLACommand, NULL);
    lv_msg_subscribe(XPTOUCH_PREHEAT_BUTTON2, (lv_msg_subscribe_cb_t)xptouch_device_onPreHeatABSCommand, NULL);
    lv_msg_subscribe(XPTOUCH_PREHEAT_BUTTON3, (lv_msg_subscribe_cb_t)xptouch_device_onPreHeatOffCommand, NULL);

}

#endif