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
    int16_t logical_w = (int16_t)tft.width();
    int16_t logical_h = (int16_t)tft.height();
    int16_t xCoord = round((x * x_touch_touchConfig.xCalM) + x_touch_touchConfig.xCalC);
    int16_t yCoord = round((y * x_touch_touchConfig.yCalM) + x_touch_touchConfig.yCalC);
    if (xCoord < 0)
        xCoord = 0;
    if (xCoord >= logical_w)
        xCoord = logical_w - 1;
    if (yCoord < 0)
        yCoord = 0;
    if (yCoord >= logical_h)
        yCoord = logical_h - 1;
    return (ScreenPoint(xCoord, yCoord));
}

void xtouch_loadTouchConfig(XTouchPanelConfig &config)
{
    // Open file for reading
    File file = xtouch_filesystem_open(xtouch_sdcard_fs(), xtouch_paths_touch);

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<512> doc;

    // Deserialize the JSON document
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
    DynamicJsonDocument doc(512); // Specify the size of the document
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
        int16_t max_x = (int16_t)tft.width();
        int16_t max_y = (int16_t)tft.height();
        int16_t min_dim = (max_x < max_y) ? max_x : max_y;
        int16_t edge_margin = min_dim / 12;
        if (edge_margin < 10) edge_margin = 10;
        if (edge_margin > 24) edge_margin = 24;
        int16_t cross_len = edge_margin * 2;

        lv_label_set_text(introScreenCaption, "Touch the  " LV_SYMBOL_PLUS "  with the stylus");
        lv_timer_handler();

        // wait for no touch
        uint16_t touchX, touchY;

        while (tft.getTouch(&touchX, &touchY))
            ;
        tft.drawFastHLine(0, edge_margin, cross_len, 0xFFFFFFU);
        tft.drawFastVLine(edge_margin, 0, cross_len, 0xFFFFFFU);
        while (!tft.getTouch(&touchX, &touchY))
            ;
        delay(50);
        ;
        x1 = touchX;
        y1 = touchY;
        tft.drawFastHLine(0, edge_margin, cross_len, 0x000000U);
        tft.drawFastVLine(edge_margin, 0, cross_len, 0x000000U);
        delay(500);

        while (tft.getTouch(&touchX, &touchY))
            ;
        tft.drawFastHLine(max_x - cross_len, max_y - edge_margin, cross_len, 0xFFFFFFU);
        tft.drawFastVLine(max_x - edge_margin, max_y - cross_len, cross_len, 0xFFFFFFU);

        while (!tft.getTouch(&touchX, &touchY))
            ;
        delay(50);

        x2 = touchX;
        y2 = touchY;
        tft.drawFastHLine(max_x - cross_len, max_y - edge_margin, cross_len, 0x000000U);
        tft.drawFastVLine(max_x - edge_margin, max_y - cross_len, cross_len, 0x000000U);

        int16_t xDist = max_x - (edge_margin * 2);
        int16_t yDist = max_y - (edge_margin * 2);

        x_touch_touchConfig.xCalM = (float)xDist / (float)(x2 - x1);
        x_touch_touchConfig.xCalC = (float)edge_margin - ((float)x1 * x_touch_touchConfig.xCalM);
        // y
        x_touch_touchConfig.yCalM = (float)yDist / (float)(y2 - y1);
        x_touch_touchConfig.yCalC = (float)edge_margin - ((float)y1 * x_touch_touchConfig.yCalM);

        xtouch_saveTouchConfig(x_touch_touchConfig);

        loadScreen(-1);
    }
}

#endif