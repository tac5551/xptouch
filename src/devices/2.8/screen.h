#ifndef _XLCD_SCREEN
#define _XLCD_SCREEN

#define LGFX_AUTODETECT
#include <LovyanGFX.hpp>
#include <string.h>

#include "setting.h"

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[4096];
LGFX tft;

#include "ui/ui.h"
#include "touch.h"
#include "xtouch/globals.h"
#include "xtouch/debug.h"

bool xtouch_screen_touchFromPowerOff = false;
bool xtouch_screen_neoPixelFromPowerOff = false;

void xtouch_screen_setBrightness(byte brightness)
{
    tft.setBrightness(brightness);
}



void xtouch_screen_sleep()
{
    xtouch_screen_touchFromPowerOff = true;
    if (xTouchConfig.xTouchStackChanEnabled == true)
    {
         loadScreen(9);
    }
    else
    {
        xtouch_screen_setBrightness(0);
    }
}

void xtouch_screen_wakeUp()
{
    ConsoleInfo.println("[xPTouch][SCREEN] Screen Reset");
    if (xtouch_screen_onScreenOffTimer != NULL) { // NULLチェックを追加
        lv_timer_reset(xtouch_screen_onScreenOffTimer);
    }
    xtouch_screen_touchFromPowerOff = false;
    loadScreen(0);
    xtouch_screen_setBrightness(xTouchConfig.xTouchBacklightLevel);
}

void xtouch_screen_onScreenTimeout(lv_timer_t *timer)
{
    if (bambuStatus.print_status == XTOUCH_PRINT_STATUS_RUNNING && xTouchConfig.xTouchWakeDuringPrint == true)
    {
        return;
    }

    if (xTouchConfig.xTouchTFTOFFValue < XTOUCH_LCD_MIN_SLEEP_TIME)
    {
        return;
    }

    ConsoleInfo.println("[xPTouch][SCREEN] Screen Off");
    xtouch_screen_sleep();

}

void xtouch_screen_onLEDOff(lv_timer_t *timer)
{

    if (bambuStatus.print_status == XTOUCH_PRINT_STATUS_RUNNING && bambuStatus.camera_timelapse == true)
    {
        return;
    }

    if (xTouchConfig.xTouchLEDOffValue < XTOUCH_LIGHT_MIN_SLEEP_TIME || xTouchConfig.xTouchLEDOffValue == 0)
    {
        return;
    }

    // if led On
    if (bambuStatus.chamberLed == true)
    {
        ConsoleInfo.println("[xPTouch][LED] LED Off");
        lv_msg_send(XTOUCH_COMMAND_LIGHT_TOGGLE, NULL);
    }
}

void xtouch_screen_setupScreenTimer()
{
    xtouch_screen_onScreenOffTimer = lv_timer_create(xtouch_screen_onScreenTimeout, xTouchConfig.xTouchTFTOFFValue * 1000 * 60, NULL);
    lv_timer_pause(xtouch_screen_onScreenOffTimer);
}

void xtouch_screen_startScreenTimer()
{
    ConsoleInfo.println("[xPTouch][SCREEN] Screen Resume");
    lv_timer_resume(xtouch_screen_onScreenOffTimer);
    lv_timer_reset(xtouch_screen_onScreenOffTimer);
}

void xtouch_screen_setScreenTimer(uint32_t period)
{
    ConsoleInfo.println("[xPTouch][SCREEN] Screen SetPeriod");
    lv_timer_set_period(xtouch_screen_onScreenOffTimer, period);
    lv_timer_reset(xtouch_screen_onScreenOffTimer);
}

void xtouch_screen_setupLEDOffTimer()
{
    xtouch_screen_onLEDOffTimer = lv_timer_create(xtouch_screen_onLEDOff, xTouchConfig.xTouchLEDOffValue * 1000 * 60, NULL);
    lv_timer_pause(xtouch_screen_onLEDOffTimer);
}

void xtouch_screen_startLEDOffTimer()
{
    ConsoleInfo.println("[xPTouch][SCREEN] LED off Resume");
    lv_timer_resume(xtouch_screen_onLEDOffTimer);
    lv_timer_reset(xtouch_screen_onLEDOffTimer);
}

void xtouch_screen_stopLEDOffTimer()
{
    ConsoleInfo.println("[xPTouch][SCREEN] LED off Stop");
    lv_timer_pause(xtouch_screen_onLEDOffTimer);
    lv_timer_reset(xtouch_screen_onLEDOffTimer);
}
void xtouch_screen_setLEDOffTimer(uint32_t period)
{
    ConsoleInfo.println("[xPTouch][LED] LED off SetPeriod");
    lv_timer_set_period(xtouch_screen_onLEDOffTimer, period);
    lv_timer_reset(xtouch_screen_onLEDOffTimer);
}

void xtouch_screen_invertColors()
{
    tft.invertDisplay(xTouchConfig.xTouchTFTInvert);
}

byte xtouch_screen_getTFTFlip()
{
    byte val = xtouch_eeprom_read(XTOUCH_EEPROM_POS_TFTFLIP);
    ConsoleInfo.println("[xPTouch][SCREEN FLIP] " + String(val));
    xTouchConfig.xTouchTFTFlip = val;
    return val;
}

void xtouch_screen_setTFTFlip(byte mode)
{
    xTouchConfig.xTouchTFTFlip = mode;
    ConsoleInfo.println("[xPTouch][SCREEN FLIP] Set : " + String(mode));
    xtouch_eeprom_write(XTOUCH_EEPROM_POS_TFTFLIP, mode);
}

void xtouch_screen_toggleTFTFlip()
{
    xtouch_screen_setTFTFlip(!xtouch_screen_getTFTFlip());
    xtouch_resetTouchConfig();
}

void xtouch_screen_setupTFTFlip()
{
    byte eepromTFTFlip = xtouch_screen_getTFTFlip();
    tft.setRotation(eepromTFTFlip == 1 ? 3 : 1);
}

void xtouch_screen_dispFlush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    if (tft.getStartCount() == 0)
    {
        tft.startWrite();
    }
    tft.setAddrWindow(area->x1, area->y1, w, h);
    // tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.pushImage(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (lgfx::rgb565_t *)&color_p->full);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

void xtouch_screen_touchRead(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    uint16_t touchX, touchY;
    bool touched = tft.getTouch(&touchX, &touchY);
    if (touched)
    {
        if (xtouch_screen_onScreenOffTimer != NULL) { // NULLチェック
            lv_timer_reset(xtouch_screen_onScreenOffTimer);
        }

        // dont pass first touch after power on
        if (xtouch_screen_touchFromPowerOff)
        {
            xtouch_screen_wakeUp();
            while (tft.getTouch(&touchX, &touchY))
                ;
            return;
        }

        data->state = LV_INDEV_STATE_PR;
        data->point.x = touchX;
        data->point.y = touchY;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

void xtouch_screen_setup()
{

    ConsoleInfo.println("[xPTouch][SCREEN] Setup");

    xtouch_screen_sleep();

    tft.begin();

    xtouch_screen_setupTFTFlip();

    xtouch_screen_setBrightness(255);
    xtouch_screen_touchFromPowerOff = false;

    // 実際の物理画面の解像度を取得
    screenWidth = tft.height();
    screenHeight = tft.width();
    ConsoleInfo.println("[xPTouch][SCREEN] Physical display size: " + String(screenWidth) + "x" + String(screenHeight));

    lv_init();

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, 4096);

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenHeight;
    disp_drv.ver_res = screenWidth;
    disp_drv.flush_cb = xtouch_screen_dispFlush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = xtouch_screen_touchRead;
    lv_indev_drv_register(&indev_drv);

    /*Initialize the graphics library */
    LV_EVENT_GET_COMP_CHILD = lv_event_register_id();

    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);

    initTopLayer();
}

#endif