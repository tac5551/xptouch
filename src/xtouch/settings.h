#ifndef _XLCD_SETTINGS
#define _XLCD_SETTINGS

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
    doc["chamberTempDiff"] = xTouchConfig.xTouchChamberSensorReadingDiff;

    doc["stackChanEnabled"] = xTouchConfig.xTouchStackChanEnabled;
    doc["neoPixelBlightness"]= xTouchConfig.xTouchNeoPixelBlightnessValue ;
    doc["neoPixelNum"]= xTouchConfig.xTouchNeoPixelNumValue;
    doc["neoPixelPin"] = xTouchConfig.xTouchNeoPixelPinValue;
    doc["alarmTimeout"] = xTouchConfig.xTouchAlarmTimeoutValue;
    doc["idleLEDEnabled"] = xTouchConfig.xTouchIdleLEDEnabled;
    xtouch_filesystem_writeJson(SD, xtouch_paths_settings, doc);

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
    xtouch_filesystem_writeJson(SD, xtouch_paths_printers, printers);
}

void xtouch_settings_loadSettings()
{
    if (!xtouch_filesystem_exist(SD, xtouch_paths_settings))
    {
        DynamicJsonDocument doc(256);
        xTouchConfig.xTouchBacklightLevel = 128;
        xTouchConfig.xTouchTFTOFFValue = 15;
        xTouchConfig.xTouchLEDOffValue = 15;
        xTouchConfig.xTouchTFTInvert = false;
        xTouchConfig.xTouchOTAEnabled = false;
        xTouchConfig.xTouchWakeOnPrint = true;
        xTouchConfig.xTouchWakeDuringPrint = true;
        xTouchConfig.xTouchChamberSensorReadingDiff = 0;

        xTouchConfig.xTouchStackChanEnabled = false;
        xTouchConfig.xTouchNeoPixelBlightnessValue = 25;
        xTouchConfig.xTouchNeoPixelNumValue = 0;
        xTouchConfig.xTouchNeoPixelPinValue = 0;

        xTouchConfig.xTouchAlarmTimeoutValue = 1;
        xTouchConfig.xTouchIdleLEDEnabled = false;
        xtouch_settings_save(true);
    }

    DynamicJsonDocument settings = xtouch_filesystem_readJson(SD, xtouch_paths_settings);

    xTouchConfig.xTouchBacklightLevel = settings.containsKey("backlight") ? settings["backlight"].as<int>() : 128;
    xTouchConfig.xTouchTFTOFFValue = settings.containsKey("tftOff") ? settings["tftOff"].as<int>() : 15;
    xTouchConfig.xTouchLEDOffValue = settings.containsKey("lightOff") ? settings["lightOff"].as<int>() : 15;
    xTouchConfig.xTouchTFTInvert = settings.containsKey("tftInvert") ? settings["tftInvert"].as<bool>() : false;
    xTouchConfig.xTouchOTAEnabled = settings.containsKey("ota") ? settings["ota"].as<bool>() : true;
    xTouchConfig.xTouchWakeOnPrint = settings.containsKey("wop") ? settings["wop"].as<bool>() : true;
    xTouchConfig.xTouchWakeDuringPrint = settings.containsKey("wdp") ? settings["wdp"].as<bool>() : true;
    xTouchConfig.xTouchChamberSensorReadingDiff = settings.containsKey("chamberTempDiff") ? settings["chamberTempDiff"].as<int8_t>() : 0;

    xTouchConfig.xTouchStackChanEnabled = settings.containsKey("stackChanEnabled") ? settings["stackChanEnabled"].as<bool>() : true;
    xTouchConfig.xTouchNeoPixelBlightnessValue = settings.containsKey("neoPixelBlightness") ? settings["neoPixelBlightness"].as<int>() : 128;
    xTouchConfig.xTouchNeoPixelNumValue = settings.containsKey("neoPixelNum") ? settings["neoPixelNum"].as<int>() : 10;
    xTouchConfig.xTouchNeoPixelPinValue = settings.containsKey("neoPixelPin") ? settings["neoPixelPin"].as<int>() : 0;
    xTouchConfig.xTouchAlarmTimeoutValue = settings.containsKey("alarmTimeout") ? settings["alarmTimeout"].as<int>() : 1;
    xTouchConfig.xTouchIdleLEDEnabled = settings.containsKey("idleLEDEnabled") ? settings["idleLEDEnabled"].as<bool>() : true;

    if (cloud.isPaired())
    {
        cloud.loadPair();
        JsonObject currentPrinterSettings = cloud.loadPrinters()[xTouchConfig.xTouchSerialNumber]["settings"];
        xTouchConfig.xTouchChamberSensorEnabled = currentPrinterSettings.containsKey("chamberTemp") ? currentPrinterSettings["chamberTemp"].as<bool>() : false;
        xTouchConfig.xTouchAuxFanEnabled = currentPrinterSettings.containsKey("auxFan") ? currentPrinterSettings["auxFan"].as<bool>() : false;
        xTouchConfig.xTouchChamberFanEnabled = currentPrinterSettings.containsKey("chamberFan") ? currentPrinterSettings["chamberFan"].as<bool>() : false;
    }
    else
    {
        xTouchConfig.xTouchChamberSensorEnabled = false;
        xTouchConfig.xTouchAuxFanEnabled = false;
        xTouchConfig.xTouchChamberFanEnabled = false;
    }

    xtouch_screen_setupTFTFlip();
    xtouch_screen_setBrightness(xTouchConfig.xTouchBacklightLevel);
    xtouch_neo_pixel_set_brightness(xTouchConfig.xTouchNeoPixelBlightnessValue);
    xtouch_neo_pixel_set_num(xTouchConfig.xTouchNeoPixelNumValue);
    xtouch_neo_pixel_set_alarm_timeout(xTouchConfig.xTouchAlarmTimeoutValue);
    xtouch_neo_pixel_set_idle_led_enabled(xTouchConfig.xTouchIdleLEDEnabled);
    xtouch_screen_invertColors();
}

#endif