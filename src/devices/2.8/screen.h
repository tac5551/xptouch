#ifndef _XLCD_SCREEN
#define _XLCD_SCREEN

#define LGFX_AUTODETECT
#include <LovyanGFX.hpp>

#include "setting.h"

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[4096];
LGFX tft;

#include "ui/ui.h"
#include "touch.h"
#include "xtouch/globals.h"
#include "xtouch/debug.h"

bool xptouch_screen_touchFromPowerOff = false;
bool xptouch_screen_neoPixelFromPowerOff = false;

void xptouch_screen_setBrightness(byte brightness)
{
    tft.setBrightness(brightness);
}



void xptouch_screen_sleep()
{
    xptouch_screen_touchFromPowerOff = true;
    if (xPTouchConfig.xTouchStackChanEnabled == true)
    {
         loadScreen(9);
    }
    else
    {
        xptouch_screen_setBrightness(0);
    }
}

void xptouch_screen_wakeUp()
{
    ConsoleInfo.println("[xPTouch][I][SCREEN] Wake Up");
    if (xptouch_screen_onScreenOffTimer != NULL) { // NULLチェックを追加
        lv_timer_reset(xptouch_screen_onScreenOffTimer);
    }
    xptouch_screen_touchFromPowerOff = false;
    loadScreen(0);
    xptouch_screen_setBrightness(xPTouchConfig.xTouchBacklightLevel);
    if (xPTouchConfig.xTouchChamberLedOnWake && !bambuStatus.chamberLed)
    {
        lv_msg_send(XPTOUCH_COMMAND_LIGHT_TOGGLE, NULL);
    }
}

void xptouch_screen_onScreenTimeout(lv_timer_t *timer)
{
    if (xPTouchConfig.currentScreenIndex == 17)
    {
        return;
    }

    if (bambuStatus.print_status == XPTOUCH_PRINT_STATUS_RUNNING && xPTouchConfig.xTouchWakeDuringPrint == true)
    {
        return;
    }

    if (xPTouchConfig.xTouchTFTOFFValue < XPTOUCH_LCD_MIN_SLEEP_TIME)
    {
        return;
    }

    ConsoleInfo.println("[xPTouch][I][SCREEN] Screen Off");
    xptouch_screen_sleep();

}

void xptouch_screen_onLEDOff(lv_timer_t *timer)
{

    if (bambuStatus.print_status == XPTOUCH_PRINT_STATUS_RUNNING && bambuStatus.camera_timelapse == true)
    {
        return;
    }
    if (bambuStatus.print_status == XPTOUCH_PRINT_STATUS_RUNNING && xPTouchConfig.xTouchWakeDuringPrint == true)
    {
        return;
    }
#ifdef __XPTOUCH_PLATFORM_S3__
    if (xPTouchConfig.currentScreenIndex == 17)
        return;
#endif

    if (xPTouchConfig.xTouchLEDOffValue < XPTOUCH_LIGHT_MIN_SLEEP_TIME || xPTouchConfig.xTouchLEDOffValue == 0)
    {
        return;
    }

    // if led On
    if (bambuStatus.chamberLed == true)
    {
        ConsoleInfo.println("[xPTouch][I][LED] LED Off");
        lv_msg_send(XPTOUCH_COMMAND_LIGHT_TOGGLE, NULL);
    }
}

void xptouch_screen_setupScreenTimer()
{
    xptouch_screen_onScreenOffTimer = lv_timer_create(xptouch_screen_onScreenTimeout, xPTouchConfig.xTouchTFTOFFValue * 1000 * 60, NULL);
    lv_timer_pause(xptouch_screen_onScreenOffTimer);
}

void xptouch_screen_startScreenTimer()
{
    ConsoleInfo.println("[xPTouch][I][SCREEN] Screen Resume");
    lv_timer_resume(xptouch_screen_onScreenOffTimer);
    lv_timer_reset(xptouch_screen_onScreenOffTimer);
}

void xptouch_screen_setScreenTimer(uint32_t period)
{
    ConsoleInfo.println("[xPTouch][I][SCREEN] Screen SetPeriod");
    lv_timer_set_period(xptouch_screen_onScreenOffTimer, period);
    lv_timer_reset(xptouch_screen_onScreenOffTimer);
}

void xptouch_screen_setupLEDOffTimer()
{
    xptouch_screen_onLEDOffTimer = lv_timer_create(xptouch_screen_onLEDOff, xPTouchConfig.xTouchLEDOffValue * 1000 * 60, NULL);
    lv_timer_pause(xptouch_screen_onLEDOffTimer);
}

void xptouch_screen_startLEDOffTimer()
{
    ConsoleInfo.println("[xPTouch][I][SCREEN] LED off Resume");
    lv_timer_resume(xptouch_screen_onLEDOffTimer);
    lv_timer_reset(xptouch_screen_onLEDOffTimer);
}

void xptouch_screen_stopLEDOffTimer()
{
    ConsoleInfo.println("[xPTouch][I][SCREEN] LED off Stop");
    lv_timer_pause(xptouch_screen_onLEDOffTimer);
    lv_timer_reset(xptouch_screen_onLEDOffTimer);
}
void xptouch_screen_setLEDOffTimer(uint32_t period)
{
    ConsoleInfo.println("[xPTouch][I][LED] LED off SetPeriod");
    lv_timer_set_period(xptouch_screen_onLEDOffTimer, period);
    lv_timer_reset(xptouch_screen_onLEDOffTimer);
}

void xptouch_screen_invertColors()
{
    tft.invertDisplay(xPTouchConfig.xTouchTFTInvert);
}

byte xptouch_screen_getTFTFlip()
{
    byte val = xptouch_eeprom_read(XPTOUCH_EEPROM_POS_TFTFLIP);
    ConsoleInfo.println("[xPTouch][I][SCREEN FLIP] " + String(val));
    xPTouchConfig.xTouchTFTFlip = val;
    return val;
}

void xptouch_screen_setTFTFlip(byte mode)
{
    xPTouchConfig.xTouchTFTFlip = mode;
    ConsoleInfo.println("[xPTouch][I][SCREEN FLIP] Set : " + String(mode));
    xptouch_eeprom_write(XPTOUCH_EEPROM_POS_TFTFLIP, mode);
}

void xptouch_screen_toggleTFTFlip()
{
    xptouch_screen_setTFTFlip(!xptouch_screen_getTFTFlip());
    xptouch_resetTouchConfig();
}

void xptouch_screen_setupTFTFlip()
{
    byte eepromTFTFlip = xptouch_screen_getTFTFlip();
    tft.setRotation(eepromTFTFlip == 1 ? 3 : 1);
}

void xptouch_screen_dispFlush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
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

void xptouch_screen_touchRead(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    uint16_t touchX, touchY;
    bool touched = tft.getTouch(&touchX, &touchY);
    if (touched)
    {
        if (xptouch_screen_onScreenOffTimer != NULL) { // NULLチェック
            lv_timer_reset(xptouch_screen_onScreenOffTimer);
        }

        // dont pass first touch after power on
        if (xptouch_screen_touchFromPowerOff)
        {
            xptouch_screen_wakeUp();
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

void xptouch_screen_setup()
{

    ConsoleInfo.println("[xPTouch][I][SCREEN] Setup");

    tft.begin();

    xptouch_screen_sleep();

    xptouch_screen_setupTFTFlip();

    xptouch_screen_setBrightness(255);
    xptouch_screen_touchFromPowerOff = false;

    // 実際の物理画面の解像度を取得
    screenWidth = tft.height();
    screenHeight = tft.width();
    ConsoleInfo.println("[xPTouch][I][SCREEN] Physical display size: " + String(screenWidth) + "x" + String(screenHeight));

    lv_init();

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, 4096);

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenHeight;
    disp_drv.ver_res = screenWidth;
    disp_drv.flush_cb = xptouch_screen_dispFlush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = xptouch_screen_touchRead;
    lv_indev_drv_register(&indev_drv);

    /*Initialize the graphics library */
    LV_EVENT_GET_COMP_CHILD = lv_event_register_id();

    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);

    initTopLayer();
}

#endif