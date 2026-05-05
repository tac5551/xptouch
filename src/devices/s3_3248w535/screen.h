#ifndef _XLCD_SCREEN
#define _XLCD_SCREEN

#include "setting.h"

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <esp_heap_caps.h>
#include <lvgl.h>

#include "jc3248_touch_chip.h"

static Arduino_ESP32QSPI *s_bus = nullptr;
static Arduino_AXS15231B *s_panel = nullptr;
Arduino_Canvas *s_canvas = nullptr;
JC3248Touch s_touch;

#include "touch.h"

#include "ui/ui.h"
#include "xtouch/eeprom.h"
#include "xtouch/globals.h"
#include "xtouch/debug.h"

static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf1;
static lv_color_t *disp_draw_buf2;

/* Keep BL on a dedicated PWM channel. */
static const uint8_t s_bl_ledc_channel = 7;
static bool s_bl_pwm_inited = false;

static void jc3248_backlight_pwm_init()
{
    if (s_bl_pwm_inited)
    {
        return;
    }
    ledcSetup(s_bl_ledc_channel, 5000, 8);
    ledcAttachPin(JC3248_TFT_BL, s_bl_ledc_channel);
    s_bl_pwm_inited = true;
}

bool xptouch_screen_touchFromPowerOff = false;
bool xptouch_screen_neoPixelFromPowerOff = false;

void xptouch_screen_setBrightness(byte brightness)
{
    jc3248_backlight_pwm_init();
#if JC3248_TFT_BL_ACTIVE_LOW
    ledcWrite(s_bl_ledc_channel, (uint32_t)(255 - brightness));
#else
    ledcWrite(s_bl_ledc_channel, brightness);
#endif
}

static void jc3248_gfx_begin(uint8_t rotation)
{
    pinMode(JC3248_TFT_BL, OUTPUT);
    xptouch_screen_setBrightness(255);

    s_bus = new Arduino_ESP32QSPI(
        JC3248_LCD_CS,
        JC3248_LCD_SCLK,
        JC3248_LCD_SDIO0,
        JC3248_LCD_SDIO1,
        JC3248_LCD_SDIO2,
        JC3248_LCD_SDIO3);

    s_panel = new Arduino_AXS15231B(
        s_bus,
        JC3248_LCD_RST,
        0,
        false,
        JC3248_LCD_NATIVE_W,
        JC3248_LCD_NATIVE_H);

    s_canvas = new Arduino_Canvas(JC3248_LCD_NATIVE_W, JC3248_LCD_NATIVE_H, s_panel);
    s_canvas->begin();
    s_canvas->setRotation(rotation);
    s_touch.begin();
    s_touch.setRotation(s_canvas->getRotation(), s_canvas->width(), s_canvas->height());
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
    if (xptouch_screen_onScreenOffTimer != NULL)
    {
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
    (void)timer;
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
    (void)timer;

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
    if (s_panel != nullptr)
    {
        s_panel->invertDisplay(xPTouchConfig.xTouchTFTInvert);
    }
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
    uint8_t rot = (eepromTFTFlip == 1) ? 3 : 1;
    if (s_canvas != nullptr)
    {
        s_canvas->setRotation(rot);
        s_touch.setRotation(s_canvas->getRotation(), s_canvas->width(), s_canvas->height());
        screenWidth = s_canvas->width();
        screenHeight = s_canvas->height();
    }
}

void xptouch_screen_dispFlush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    auto *cv = (Arduino_Canvas *)disp->user_data;
    uint32_t w = (uint32_t)(area->x2 - area->x1 + 1);
    uint32_t h = (uint32_t)(area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
    cv->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, (int16_t)w, (int16_t)h);
#else
    cv->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, (int16_t)w, (int16_t)h);
#endif
    cv->flush();
    lv_disp_flush_ready(disp);
}

void xptouch_screen_touchRead(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    (void)indev_driver;
    JC3248TouchPoint p;
    if (s_touch.read(p) && p.touched)
    {
        if (xptouch_screen_onScreenOffTimer != NULL)
        {
            lv_timer_reset(xptouch_screen_onScreenOffTimer);
        }

        if (xptouch_screen_touchFromPowerOff)
        {
            xptouch_screen_wakeUp();
            while (s_touch.read(p) && p.touched)
            {
            }
            return;
        }

        data->state = LV_INDEV_STATE_PR;
        data->point.x = (lv_coord_t)p.x;
        data->point.y = (lv_coord_t)p.y;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

void xptouch_screen_setup()
{
    ConsoleInfo.println("[xPTouch][I][SCREEN] Setup screen (JC3248W535 / Arduino_GFX)");

    jc3248_gfx_begin((uint8_t)XPTOUCH_TFT_DEFAULT_ROTATION);

    screenWidth = s_canvas->width();
    screenHeight = s_canvas->height();

    /* lv_init ?? xptouch_screen_sleep() ???E��EtackChan ???�E BL=0 ??????�E????????????E
     * ?�E????????�E????????�E intro ???�E???????E*/
    xptouch_screen_touchFromPowerOff = false;

    xptouch_screen_setupTFTFlip();

    xptouch_screen_setBrightness(255);

    if (s_canvas != nullptr)
    {
        s_canvas->fillScreen(0x0000U);
        s_canvas->flush();
    }

    lv_init();

    int buf_size = screenWidth * screenHeight / XPTOUCH_LVGL_DRAW_BUF_DENOM;
    disp_draw_buf1 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * buf_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    /* 3248 ? History ?????????????????????? 1 ???????? */
    disp_draw_buf2 = nullptr;
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf1, disp_draw_buf2, buf_size);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = xptouch_screen_dispFlush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.user_data = s_canvas;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = xptouch_screen_touchRead;
    lv_indev_drv_register(&indev_drv);

    LV_EVENT_GET_COMP_CHILD = lv_event_register_id();

    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);

    initTopLayer();
}

#endif


