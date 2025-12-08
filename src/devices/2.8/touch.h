#ifndef _XLCD_TOUCH
#define _XLCD_TOUCH

#include "setting.h"

XTouchPanelConfig x_touch_touchConfig;

class ScreenPoint
{
public:
    int16_t x;
    int16_t y;

    // default constructor
    ScreenPoint()
    {
    }

    ScreenPoint(int16_t xIn, int16_t yIn)
    {
        x = xIn;
        y = yIn;
    }
};

ScreenPoint getScreenCoords(int16_t x, int16_t y)
{
    int16_t xCoord = round((x * x_touch_touchConfig.xCalM) + x_touch_touchConfig.xCalC);
    int16_t yCoord = round((y * x_touch_touchConfig.yCalM) + x_touch_touchConfig.yCalC);
    if (xCoord < 0)
        xCoord = 0;
    if (xCoord >= screenHeight)
        xCoord = screenHeight - 1;
    if (yCoord < 0)
        yCoord = 0;
    if (yCoord >= screenWidth)
        yCoord = screenWidth - 1;
    return (ScreenPoint(xCoord, yCoord));
}

void xtouch_loadTouchConfig(XTouchPanelConfig &config)
{
    // Open file for reading
    File file = xtouch_filesystem_open(SD, xtouch_paths_touch);

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<512> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        ConsoleError.println(F("[xPTouch][Touch] Failed to read touch config"));

    config.xCalM = doc["xCalM"].as<float>();
    config.yCalM = doc["yCalM"].as<float>();
    config.xCalC = doc["xCalC"].as<float>();
    config.yCalC = doc["yCalC"].as<float>();

    file.close();
}

void xtouch_saveTouchConfig(XTouchPanelConfig &config)
{
    DynamicJsonDocument doc(512); // Specify the size of the document
    doc["xCalM"] = config.xCalM;
    doc["yCalM"] = config.yCalM;
    doc["xCalC"] = config.xCalC;
    doc["yCalC"] = config.yCalC;
    xtouch_filesystem_writeJson(SD, xtouch_paths_touch, doc);
}

void xtouch_resetTouchConfig()
{
    ConsoleInfo.println(F("[xPTouch][FS] Resetting touch config"));
    xtouch_filesystem_deleteFile(SD, xtouch_paths_touch);
    delay(500);
    ESP.restart();
}

bool hasTouchConfig()
{
    ConsoleInfo.println(F("[xPTouch][FS] Checking for touch config"));
    return xtouch_filesystem_exist(SD, xtouch_paths_touch);
}

void xtouch_touch_setup()
{
    if (hasTouchConfig())
    {
        ConsoleInfo.println(F("[xPTouch][TOUCH] Load from disk"));
        xtouch_loadTouchConfig(x_touch_touchConfig);
    }
    else
    {
        ConsoleInfo.println(F("[xPTouch][TOUCH] Touch Setup"));
        int16_t x1, y1, x2, y2;

        lv_label_set_text(introScreenCaption, "Touch the  " LV_SYMBOL_PLUS "  with the stylus");
        lv_timer_handler();

        // wait for no touch
        uint16_t touchX, touchY;

        while (tft.getTouch(&touchX, &touchY))
            ;
        tft.drawFastHLine(0, 10, 20, 0xFFFFFFU);
        tft.drawFastVLine(10, 0, 20, 0xFFFFFFU);
        while (!tft.getTouch(&touchX, &touchY))
            ;
        delay(50);
        ;
        x1 = touchX;
        y1 = touchY;
        tft.drawFastHLine(0, 10, 20, 0x000000U);
        tft.drawFastVLine(10, 0, 20, 0x000000U);
        delay(500);

        while (tft.getTouch(&touchX, &touchY))
            ;
        tft.drawFastHLine(screenHeight - 20, screenWidth - 10, 20, 0xFFFFFFU);
        tft.drawFastVLine(screenHeight - 10, screenWidth - 20, 20, 0xFFFFFFU);

        while (!tft.getTouch(&touchX, &touchY))
            ;
        delay(50);

        x2 = touchX;
        y2 = touchY;
        tft.drawFastHLine(screenHeight - 20, screenWidth - 10, 20, 0x000000U);
        tft.drawFastVLine(screenHeight - 10, screenWidth - 20, 20, 0x000000U);

        int16_t xDist = screenWidth - 40;
        int16_t yDist = screenHeight - 40;

        x_touch_touchConfig.xCalM = (float)xDist / (float)(x2 - x1);
        x_touch_touchConfig.xCalC = 20.0 - ((float)x1 * x_touch_touchConfig.xCalM);
        // y
        x_touch_touchConfig.yCalM = (float)yDist / (float)(y2 - y1);
        x_touch_touchConfig.yCalC = 20.0 - ((float)y1 * x_touch_touchConfig.yCalM);

        xtouch_saveTouchConfig(x_touch_touchConfig);

        loadScreen(-1);
    }
}

#endif