#ifndef _XLCD_SETTINGS
#define _XLCD_SETTINGS

#include "types.h"
#include "xtouch/sdcard.h"

void xptouch_settings_save(bool onlyRoot = false)
{
    DynamicJsonDocument doc(256);
    doc["backlight"] = xPTouchConfig.xTouchBacklightLevel;
    doc["lightOff"] = xPTouchConfig.xTouchLEDOffValue; 
    doc["tftOff"] = xPTouchConfig.xTouchTFTOFFValue;
    doc["tftInvert"] = xPTouchConfig.xTouchTFTInvert;
    doc["ota"] = xPTouchConfig.xTouchOTAEnabled;
    doc["wop"] = xPTouchConfig.xTouchWakeOnPrint;
    doc["wdp"] = xPTouchConfig.xTouchWakeDuringPrint;
    doc["chamberLedOnWake"] = xPTouchConfig.xTouchChamberLedOnWake;
    doc["chamberTempDiff"] = xPTouchConfig.xTouchChamberSensorReadingDiff;

    doc["stackChanEnabled"] = xPTouchConfig.xTouchStackChanEnabled;
    doc["preheatEnabled"] = xPTouchConfig.xTouchPreheatEnabled;
    doc["multiPrinterMonitor"] = xPTouchConfig.xTouchMultiPrinterMonitorEnabled;
    doc["historyEnabled"] = xPTouchConfig.xTouchHistoryEnabled;
    doc["hideAllThumbnails"] = xPTouchConfig.xTouchHideAllThumbnails;
    doc["p1sCameraStreamEnabled"] = xPTouchConfig.xTouchP1sCameraStreamEnabled;
    doc["neoPixelBrightness"] = xPTouchConfig.xTouchNeoPixelBrightnessValue;
    doc["neoPixelNum"]= xPTouchConfig.xTouchNeoPixelNumValue;
    doc["neoPixelPin"] = xPTouchConfig.xTouchNeoPixelPinValue;
    doc["alarmTimeout"] = xPTouchConfig.xTouchAlarmTimeoutValue;
    doc["idleLEDEnabled"] = xPTouchConfig.xTouchIdleLEDEnabled;
    xptouch_filesystem_writeJson(xptouch_sdcard_fs(), xptouch_paths_settings, doc);

    if (onlyRoot)
    {
        return;
    }

    DynamicJsonDocument printersSettings(256);
    printersSettings["chamberTemp"] = xPTouchConfig.xTouchChamberSensorEnabled;
    printersSettings["auxFan"] = xPTouchConfig.xTouchAuxFanEnabled;
    printersSettings["chamberFan"] = xPTouchConfig.xTouchChamberFanEnabled;

    DynamicJsonDocument printers = cloud.loadPrinters();
    printers[xPTouchConfig.xTouchSerialNumber]["settings"] = printersSettings;
    xptouch_filesystem_writeJson(xptouch_sdcard_fs(), xptouch_paths_printers, printers);
}

void xptouch_settings_loadSettings()
{
    if (!xptouch_filesystem_exist(xptouch_sdcard_fs(), xptouch_paths_settings))
    {
        DynamicJsonDocument doc(256);
        xPTouchConfig.xTouchBacklightLevel = XPTOUCH_BACKLIGHT_SLIDER_DEFAULT;
        xPTouchConfig.xTouchTFTOFFValue = 15;
        xPTouchConfig.xTouchLEDOffValue = 15;
        xPTouchConfig.xTouchTFTInvert = false;
        xPTouchConfig.xTouchOTAEnabled = false;
        xPTouchConfig.xTouchWakeOnPrint = false;
        xPTouchConfig.xTouchWakeDuringPrint = false;
        xPTouchConfig.xTouchChamberLedOnWake = false;
        xPTouchConfig.xTouchChamberSensorReadingDiff = 0;

        xPTouchConfig.xTouchStackChanEnabled = false;
        xPTouchConfig.xTouchPreheatEnabled = false;
        xPTouchConfig.xTouchMultiPrinterMonitorEnabled = true;
        xPTouchConfig.xTouchHistoryEnabled = false;
        xPTouchConfig.xTouchP1sCameraStreamEnabled = false;
        xPTouchConfig.xTouchNeoPixelBrightnessValue = 25;
        xPTouchConfig.xTouchNeoPixelNumValue = 0;
        xPTouchConfig.xTouchNeoPixelPinValue = 0;

        xPTouchConfig.xTouchAlarmTimeoutValue = 1;
        xPTouchConfig.xTouchIdleLEDEnabled = false;
        xptouch_settings_save(true);
    }

    DynamicJsonDocument settings = xptouch_filesystem_readJson(xptouch_sdcard_fs(), xptouch_paths_settings);

    xPTouchConfig.xTouchBacklightLevel = settings.containsKey("backlight") ? settings["backlight"].as<int>() : XPTOUCH_BACKLIGHT_SLIDER_DEFAULT;
    if (xPTouchConfig.xTouchBacklightLevel < XPTOUCH_BACKLIGHT_SLIDER_MIN ||
        xPTouchConfig.xTouchBacklightLevel > XPTOUCH_BACKLIGHT_SLIDER_MAX)
        xPTouchConfig.xTouchBacklightLevel = XPTOUCH_BACKLIGHT_SLIDER_MAX;
    xPTouchConfig.xTouchTFTOFFValue = settings.containsKey("tftOff") ? settings["tftOff"].as<int>() : 15;
    xPTouchConfig.xTouchLEDOffValue = settings.containsKey("lightOff") ? settings["lightOff"].as<int>() : 15;
    xPTouchConfig.xTouchTFTInvert = settings.containsKey("tftInvert") ? settings["tftInvert"].as<bool>() : false;
    xPTouchConfig.xTouchOTAEnabled = settings.containsKey("ota") ? settings["ota"].as<bool>() : false;
    xPTouchConfig.xTouchWakeOnPrint = settings.containsKey("wop") ? settings["wop"].as<bool>() : false;
    xPTouchConfig.xTouchWakeDuringPrint = settings.containsKey("wdp") ? settings["wdp"].as<bool>() : false;
    xPTouchConfig.xTouchChamberLedOnWake = settings.containsKey("chamberLedOnWake") ? settings["chamberLedOnWake"].as<bool>() : false;
    xPTouchConfig.xTouchChamberSensorReadingDiff = settings.containsKey("chamberTempDiff") ? settings["chamberTempDiff"].as<int8_t>() : 0;

    xPTouchConfig.xTouchStackChanEnabled = settings.containsKey("stackChanEnabled") ? settings["stackChanEnabled"].as<bool>() : false;
    xPTouchConfig.xTouchPreheatEnabled = settings.containsKey("preheatEnabled") ? settings["preheatEnabled"].as<bool>() : false;
    xPTouchConfig.xTouchMultiPrinterMonitorEnabled = settings.containsKey("multiPrinterMonitor") ? settings["multiPrinterMonitor"].as<bool>() : true;
    xPTouchConfig.xTouchHistoryEnabled = settings.containsKey("historyEnabled") ? settings["historyEnabled"].as<bool>() : false;
    xPTouchConfig.xTouchHideAllThumbnails = settings.containsKey("hideAllThumbnails") ? settings["hideAllThumbnails"].as<bool>() : false;
    xPTouchConfig.xTouchP1sCameraStreamEnabled = settings.containsKey("p1sCameraStreamEnabled") ? settings["p1sCameraStreamEnabled"].as<bool>() : false;
    xPTouchConfig.xTouchNeoPixelBrightnessValue = settings.containsKey("neoPixelBrightness") ? settings["neoPixelBrightness"].as<int>() : 25;
    xPTouchConfig.xTouchNeoPixelNumValue = settings.containsKey("neoPixelNum") ? settings["neoPixelNum"].as<int>() : 10;
    xPTouchConfig.xTouchNeoPixelPinValue = settings.containsKey("neoPixelPin") ? settings["neoPixelPin"].as<int>() : 0;
    xPTouchConfig.xTouchAlarmTimeoutValue = settings.containsKey("alarmTimeout") ? settings["alarmTimeout"].as<int>() : 1;
    xPTouchConfig.xTouchIdleLEDEnabled = settings.containsKey("idleLEDEnabled") ? settings["idleLEDEnabled"].as<bool>() : false;

    if (cloud.isPaired())
    {
        cloud.loadPair();
        DynamicJsonDocument lp = cloud.loadPrinters();
        JsonObject root = lp.as<JsonObject>();
        JsonObject currentPrinterSettings;
        if (root.containsKey(xPTouchConfig.xTouchSerialNumber))
        {
            JsonObject dev = root[xPTouchConfig.xTouchSerialNumber].as<JsonObject>();
            if (!dev.isNull() && dev.containsKey("settings"))
                currentPrinterSettings = dev["settings"].as<JsonObject>();
        }
        xPTouchConfig.xTouchChamberSensorEnabled = (!currentPrinterSettings.isNull() && currentPrinterSettings.containsKey("chamberTemp")) ? currentPrinterSettings["chamberTemp"].as<bool>() : false;
        xPTouchConfig.xTouchAuxFanEnabled = (!currentPrinterSettings.isNull() && currentPrinterSettings.containsKey("auxFan")) ? currentPrinterSettings["auxFan"].as<bool>() : false;
        xPTouchConfig.xTouchChamberFanEnabled = (!currentPrinterSettings.isNull() && currentPrinterSettings.containsKey("chamberFan")) ? currentPrinterSettings["chamberFan"].as<bool>() : false;
    }
    else
    {
        xPTouchConfig.xTouchChamberSensorEnabled = false;
        xPTouchConfig.xTouchAuxFanEnabled = false;
        xPTouchConfig.xTouchChamberFanEnabled = false;
    }

    xptouch_screen_setupTFTFlip();
    xptouch_screen_setBrightness(xPTouchConfig.xTouchBacklightLevel);
    xptouch_neo_pixel_set_brightness(xPTouchConfig.xTouchNeoPixelBrightnessValue);
    xptouch_neo_pixel_set_num(xPTouchConfig.xTouchNeoPixelNumValue);
    xptouch_neo_pixel_set_alarm_timeout(xPTouchConfig.xTouchAlarmTimeoutValue);
    xptouch_neo_pixel_set_idle_led_enabled(xPTouchConfig.xTouchIdleLEDEnabled);
    xptouch_screen_invertColors();
}

#endif