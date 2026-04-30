#ifndef _XLCD_SETTINGS
#define _XLCD_SETTINGS

#include "types.h"
#include "xtouch/sdcard.h"

void xtouch_settings_save(bool onlyRoot = false)
{
    DynamicJsonDocument doc(256);
    doc["backlight"] = xTouchConfig.xTouchBacklightLevel;
    doc["lightOff"] = xTouchConfig.xTouchLEDOffValue; 
    doc["tftOff"] = xTouchConfig.xTouchTFTOFFValue;
    doc["tftInvert"] = xTouchConfig.xTouchTFTInvert;
    doc["ota"] = xTouchConfig.xTouchOTAEnabled;
    doc["wop"] = xTouchConfig.xTouchWakeOnPrint;
    doc["wdp"] = xTouchConfig.xTouchWakeDuringPrint;
    doc["chamberLedOnWake"] = xTouchConfig.xTouchChamberLedOnWake;
    doc["chamberTempDiff"] = xTouchConfig.xTouchChamberSensorReadingDiff;

    doc["stackChanEnabled"] = xTouchConfig.xTouchStackChanEnabled;
    doc["preheatEnabled"] = xTouchConfig.xTouchPreheatEnabled;
    doc["multiPrinterMonitor"] = xTouchConfig.xTouchMultiPrinterMonitorEnabled;
    doc["historyEnabled"] = xTouchConfig.xTouchHistoryEnabled;
    doc["hideAllThumbnails"] = xTouchConfig.xTouchHideAllThumbnails;
    doc["p1sCameraStreamEnabled"] = xTouchConfig.xTouchP1sCameraStreamEnabled;
    doc["neoPixelBrightness"] = xTouchConfig.xTouchNeoPixelBrightnessValue;
    doc["neoPixelNum"]= xTouchConfig.xTouchNeoPixelNumValue;
    doc["neoPixelPin"] = xTouchConfig.xTouchNeoPixelPinValue;
    doc["alarmTimeout"] = xTouchConfig.xTouchAlarmTimeoutValue;
    doc["idleLEDEnabled"] = xTouchConfig.xTouchIdleLEDEnabled;
    xtouch_filesystem_writeJson(xtouch_sdcard_fs(), xtouch_paths_settings, doc);

    if (onlyRoot)
    {
        return;
    }

    DynamicJsonDocument printersSettings(256);
    printersSettings["chamberTemp"] = xTouchConfig.xTouchChamberSensorEnabled;
    printersSettings["auxFan"] = xTouchConfig.xTouchAuxFanEnabled;
    printersSettings["chamberFan"] = xTouchConfig.xTouchChamberFanEnabled;

    DynamicJsonDocument printers = cloud.loadPrinters();
    printers[xTouchConfig.xTouchSerialNumber]["settings"] = printersSettings;
    xtouch_filesystem_writeJson(xtouch_sdcard_fs(), xtouch_paths_printers, printers);
}

void xtouch_settings_loadSettings()
{
    if (!xtouch_filesystem_exist(xtouch_sdcard_fs(), xtouch_paths_settings))
    {
        DynamicJsonDocument doc(256);
        xTouchConfig.xTouchBacklightLevel = XTOUCH_BACKLIGHT_SLIDER_DEFAULT;
        xTouchConfig.xTouchTFTOFFValue = 15;
        xTouchConfig.xTouchLEDOffValue = 15;
        xTouchConfig.xTouchTFTInvert = false;
        xTouchConfig.xTouchOTAEnabled = false;
        xTouchConfig.xTouchWakeOnPrint = false;
        xTouchConfig.xTouchWakeDuringPrint = false;
        xTouchConfig.xTouchChamberLedOnWake = false;
        xTouchConfig.xTouchChamberSensorReadingDiff = 0;

        xTouchConfig.xTouchStackChanEnabled = false;
        xTouchConfig.xTouchPreheatEnabled = false;
        xTouchConfig.xTouchMultiPrinterMonitorEnabled = true;
        xTouchConfig.xTouchHistoryEnabled = false;
        xTouchConfig.xTouchP1sCameraStreamEnabled = false;
        xTouchConfig.xTouchNeoPixelBrightnessValue = 25;
        xTouchConfig.xTouchNeoPixelNumValue = 0;
        xTouchConfig.xTouchNeoPixelPinValue = 0;

        xTouchConfig.xTouchAlarmTimeoutValue = 1;
        xTouchConfig.xTouchIdleLEDEnabled = false;
        xtouch_settings_save(true);
    }

    DynamicJsonDocument settings = xtouch_filesystem_readJson(xtouch_sdcard_fs(), xtouch_paths_settings);

    xTouchConfig.xTouchBacklightLevel = settings.containsKey("backlight") ? settings["backlight"].as<int>() : XTOUCH_BACKLIGHT_SLIDER_DEFAULT;
    if (xTouchConfig.xTouchBacklightLevel < XTOUCH_BACKLIGHT_SLIDER_MIN ||
        xTouchConfig.xTouchBacklightLevel > XTOUCH_BACKLIGHT_SLIDER_MAX)
        xTouchConfig.xTouchBacklightLevel = XTOUCH_BACKLIGHT_SLIDER_MAX;
    xTouchConfig.xTouchTFTOFFValue = settings.containsKey("tftOff") ? settings["tftOff"].as<int>() : 15;
    xTouchConfig.xTouchLEDOffValue = settings.containsKey("lightOff") ? settings["lightOff"].as<int>() : 15;
    xTouchConfig.xTouchTFTInvert = settings.containsKey("tftInvert") ? settings["tftInvert"].as<bool>() : false;
    xTouchConfig.xTouchOTAEnabled = settings.containsKey("ota") ? settings["ota"].as<bool>() : false;
    xTouchConfig.xTouchWakeOnPrint = settings.containsKey("wop") ? settings["wop"].as<bool>() : false;
    xTouchConfig.xTouchWakeDuringPrint = settings.containsKey("wdp") ? settings["wdp"].as<bool>() : false;
    xTouchConfig.xTouchChamberLedOnWake = settings.containsKey("chamberLedOnWake") ? settings["chamberLedOnWake"].as<bool>() : false;
    xTouchConfig.xTouchChamberSensorReadingDiff = settings.containsKey("chamberTempDiff") ? settings["chamberTempDiff"].as<int8_t>() : 0;

    xTouchConfig.xTouchStackChanEnabled = settings.containsKey("stackChanEnabled") ? settings["stackChanEnabled"].as<bool>() : false;
    xTouchConfig.xTouchPreheatEnabled = settings.containsKey("preheatEnabled") ? settings["preheatEnabled"].as<bool>() : false;
    xTouchConfig.xTouchMultiPrinterMonitorEnabled = settings.containsKey("multiPrinterMonitor") ? settings["multiPrinterMonitor"].as<bool>() : true;
    xTouchConfig.xTouchHistoryEnabled = settings.containsKey("historyEnabled") ? settings["historyEnabled"].as<bool>() : false;
    xTouchConfig.xTouchHideAllThumbnails = settings.containsKey("hideAllThumbnails") ? settings["hideAllThumbnails"].as<bool>() : false;
    xTouchConfig.xTouchP1sCameraStreamEnabled = settings.containsKey("p1sCameraStreamEnabled") ? settings["p1sCameraStreamEnabled"].as<bool>() : false;
    xTouchConfig.xTouchNeoPixelBrightnessValue = settings.containsKey("neoPixelBrightness") ? settings["neoPixelBrightness"].as<int>() : 25;
    xTouchConfig.xTouchNeoPixelNumValue = settings.containsKey("neoPixelNum") ? settings["neoPixelNum"].as<int>() : 10;
    xTouchConfig.xTouchNeoPixelPinValue = settings.containsKey("neoPixelPin") ? settings["neoPixelPin"].as<int>() : 0;
    xTouchConfig.xTouchAlarmTimeoutValue = settings.containsKey("alarmTimeout") ? settings["alarmTimeout"].as<int>() : 1;
    xTouchConfig.xTouchIdleLEDEnabled = settings.containsKey("idleLEDEnabled") ? settings["idleLEDEnabled"].as<bool>() : false;

    if (cloud.isPaired())
    {
        cloud.loadPair();
        DynamicJsonDocument lp = cloud.loadPrinters();
        JsonObject root = lp.as<JsonObject>();
        JsonObject currentPrinterSettings;
        if (root.containsKey(xTouchConfig.xTouchSerialNumber))
        {
            JsonObject dev = root[xTouchConfig.xTouchSerialNumber].as<JsonObject>();
            if (!dev.isNull() && dev.containsKey("settings"))
                currentPrinterSettings = dev["settings"].as<JsonObject>();
        }
        xTouchConfig.xTouchChamberSensorEnabled = (!currentPrinterSettings.isNull() && currentPrinterSettings.containsKey("chamberTemp")) ? currentPrinterSettings["chamberTemp"].as<bool>() : false;
        xTouchConfig.xTouchAuxFanEnabled = (!currentPrinterSettings.isNull() && currentPrinterSettings.containsKey("auxFan")) ? currentPrinterSettings["auxFan"].as<bool>() : false;
        xTouchConfig.xTouchChamberFanEnabled = (!currentPrinterSettings.isNull() && currentPrinterSettings.containsKey("chamberFan")) ? currentPrinterSettings["chamberFan"].as<bool>() : false;
    }
    else
    {
        xTouchConfig.xTouchChamberSensorEnabled = false;
        xTouchConfig.xTouchAuxFanEnabled = false;
        xTouchConfig.xTouchChamberFanEnabled = false;
    }

    xtouch_screen_setupTFTFlip();
    xtouch_screen_setBrightness(xTouchConfig.xTouchBacklightLevel);
    xtouch_neo_pixel_set_brightness(xTouchConfig.xTouchNeoPixelBrightnessValue);
    xtouch_neo_pixel_set_num(xTouchConfig.xTouchNeoPixelNumValue);
    xtouch_neo_pixel_set_alarm_timeout(xTouchConfig.xTouchAlarmTimeoutValue);
    xtouch_neo_pixel_set_idle_led_enabled(xTouchConfig.xTouchIdleLEDEnabled);
    xtouch_screen_invertColors();
}

#endif