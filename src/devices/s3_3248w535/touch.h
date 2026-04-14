#ifndef _XLCD_TOUCH_H
#define _XLCD_TOUCH_H

#include "setting.h"

#include <Arduino_GFX_Library.h>
#include <ArduinoJson.h>

#include "jc3248_touch_chip.h"

#include "xtouch/filesystem.h"
#include "xtouch/paths.h"
#include "xtouch/debug.h"
#include "ui/ui.h"

XTouchPanelConfig x_touch_touchConfig;

class ScreenPoint
{
public:
    int16_t x;
    int16_t y;

    ScreenPoint() = default;

    ScreenPoint(int16_t xIn, int16_t yIn)
        : x(xIn)
        , y(yIn)
    {
    }
};

ScreenPoint getScreenCoords(int16_t x, int16_t y)
{
    int16_t xCoord = (int16_t)round((x * x_touch_touchConfig.xCalM) + x_touch_touchConfig.xCalC);
    int16_t yCoord = (int16_t)round((y * x_touch_touchConfig.yCalM) + x_touch_touchConfig.yCalC);
    if (xCoord < 0)
        xCoord = 0;
    if (xCoord >= screenWidth)
        xCoord = screenWidth - 1;
    if (yCoord < 0)
        yCoord = 0;
    if (yCoord >= screenHeight)
        yCoord = screenHeight - 1;
    return ScreenPoint(xCoord, yCoord);
}

void xtouch_loadTouchConfig(XTouchPanelConfig &config)
{
    File file = xtouch_filesystem_open(xtouch_sdcard_fs(), xtouch_paths_touch);
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        ConsoleError.println(F("[xPTouch][E][TOUCH] Failed to read touch config"));

    config.xCalM = doc["xCalM"].as<float>();
    config.yCalM = doc["yCalM"].as<float>();
    config.xCalC = doc["xCalC"].as<float>();
    config.yCalC = doc["yCalC"].as<float>();

    file.close();
}

void xtouch_saveTouchConfig(XTouchPanelConfig &config)
{
    DynamicJsonDocument doc(512);
    doc["xCalM"] = config.xCalM;
    doc["yCalM"] = config.yCalM;
    doc["xCalC"] = config.xCalC;
    doc["yCalC"] = config.yCalC;
    xtouch_filesystem_writeJson(xtouch_sdcard_fs(), xtouch_paths_touch, doc);
}

void xtouch_resetTouchConfig()
{
    ConsoleInfo.println(F("[xPTouch][I][TOUCH] Reset touch config"));
    xtouch_filesystem_deleteFile(xtouch_sdcard_fs(), xtouch_paths_touch);
    delay(500);
    ESP.restart();
}

bool hasTouchConfig()
{
    ConsoleInfo.println(F("[xPTouch][I][TOUCH] Checking for touch config"));
    return xtouch_filesystem_exist(xtouch_sdcard_fs(), xtouch_paths_touch);
}

static bool jc3248_poll_touch(uint16_t &x, uint16_t &y)
{
    JC3248TouchPoint p;
    if (s_touch.read(p) && p.touched)
    {
        x = p.x;
        y = p.y;
        return true;
    }
    return false;
}

void xtouch_touch_setup()
{
    if (hasTouchConfig())
    {
        ConsoleInfo.println(F("[xPTouch][I][TOUCH] Load touch config from SD card touch config"));
        xtouch_loadTouchConfig(x_touch_touchConfig);
    }
    else
    {
        ConsoleInfo.println(F("[xPTouch][I][TOUCH] Setup touch config from screen touch"));
        int16_t x1, y1, x2, y2;

        lv_label_set_text(introScreenCaption, "Touch the  " LV_SYMBOL_PLUS "  with the stylus");
        lv_timer_handler();

        uint16_t touchX = 0, touchY = 0;

        while (jc3248_poll_touch(touchX, touchY))
            ;
        s_canvas->drawFastHLine(0, 10, 20, 0xFFFFU);
        s_canvas->drawFastVLine(10, 0, 20, 0xFFFFU);
        s_canvas->flush();
        while (!jc3248_poll_touch(touchX, touchY))
            ;
        delay(50);
        x1 = (int16_t)touchX;
        y1 = (int16_t)touchY;
        s_canvas->drawFastHLine(0, 10, 20, 0x0000U);
        s_canvas->drawFastVLine(10, 0, 20, 0x0000U);
        s_canvas->flush();
        delay(500);

        while (jc3248_poll_touch(touchX, touchY))
            ;
        s_canvas->drawFastHLine(screenWidth - 20, screenHeight - 10, 20, 0xFFFFU);
        s_canvas->drawFastVLine(screenWidth - 10, screenHeight - 20, 20, 0xFFFFU);
        s_canvas->flush();
        while (!jc3248_poll_touch(touchX, touchY))
            ;
        delay(50);

        x2 = (int16_t)touchX;
        y2 = (int16_t)touchY;
        s_canvas->drawFastHLine(screenWidth - 20, screenHeight - 10, 20, 0x0000U);
        s_canvas->drawFastVLine(screenWidth - 10, screenHeight - 20, 20, 0x0000U);
        s_canvas->flush();

        int16_t xDist = screenWidth - 40;
        int16_t yDist = screenHeight - 40;

        x_touch_touchConfig.xCalM = (float)xDist / (float)(x2 - x1);
        x_touch_touchConfig.xCalC = 20.0f - ((float)x1 * x_touch_touchConfig.xCalM);
        x_touch_touchConfig.yCalM = (float)yDist / (float)(y2 - y1);
        x_touch_touchConfig.yCalC = 20.0f - ((float)y1 * x_touch_touchConfig.yCalM);

        xtouch_saveTouchConfig(x_touch_touchConfig);

        loadScreen(-1);
    }
}

#endif
