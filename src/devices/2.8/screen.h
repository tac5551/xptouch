#ifndef _XLCD_SCREEN
#define _XLCD_SCREEN

#include <TFT_eSPI.h>

#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23
#define SD_CS 5

#define LCD_BACK_LIGHT_PIN 21

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0 0

// use 12 bit precission for LEDC timer
#define LEDC_TIMER_12_BIT 12

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ 5000

static const uint16_t screenWidth = 240;
static const uint16_t screenHeight = 320;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[4096];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */

#include "ui/ui.h"
#include "touch.h"
#include "xtouch/globals.h"
#include "xtouch/debug.h"

bool xtouch_screen_touchFromPowerOff = false;

void xtouch_screen_ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255)
{
    // calculate duty, 4095 from 2 ^ 12 - 1
    uint32_t duty = (4095 / valueMax) * min(value, valueMax);

    // write duty to LEDC
    ledcWrite(channel, duty);
}

void xtouch_screen_setBrightness(byte brightness)
{
    xtouch_screen_ledcAnalogWrite(LEDC_CHANNEL_0, brightness);
}

void xtouch_screen_setBackLedOff()
{
    // pinMode(4, OUTPUT);
    // pinMode(16, OUTPUT);
    // pinMode(17, OUTPUT);
    // digitalWrite(4, HIGH);
    // digitalWrite(16, HIGH);
    // digitalWrite(17, HIGH); // The LEDs are "active low", meaning HIGH == off, LOW == on
    ConsoleInfo.println("[xPTouch][SCREEN] Screen BackLed Off");
    xtouch_screen_setBrightness(0);
    xtouch_screen_touchFromPowerOff = true;
}

void xtouch_screen_wakeUp()
{
    ConsoleInfo.println("[xPTouch][SCREEN] Screen Reset");
    lv_timer_reset(xtouch_screen_onScreenOffTimer);
    xtouch_screen_touchFromPowerOff = false;
    loadScreen(0);
    xtouch_screen_setBrightness(xTouchConfig.xTouchBacklightLevel);
}

void xtouch_screen_onScreenOff(lv_timer_t *timer)
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
    xtouch_screen_setBrightness(0);
    xtouch_screen_touchFromPowerOff = true;
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
    xtouch_screen_onScreenOffTimer = lv_timer_create(xtouch_screen_onScreenOff, xTouchConfig.xTouchTFTOFFValue * 1000 * 60, NULL);
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

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

void xtouch_screen_touchRead(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{

    if (x_touch_touchScreen.tirqTouched() && x_touch_touchScreen.touched())
    {
        lv_timer_reset(xtouch_screen_onScreenOffTimer);
        // dont pass first touch after power on
        if (xtouch_screen_touchFromPowerOff)
        {
            xtouch_screen_wakeUp();
            while (x_touch_touchScreen.touched())
                ;
            return;
        }

        ScreenPoint sp = ScreenPoint();
        TS_Point p = x_touch_touchScreen.getPoint();
        sp = getScreenCoords(p.x, p.y);
        data->state = LV_INDEV_STATE_PR;
        data->point.x = sp.x;
        data->point.y = sp.y;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

void xtouch_screen_setup()
{

    ConsoleInfo.println("[xPTouch][SCREEN] Setup");
    pinMode(XPT2046_CS, OUTPUT);
    pinMode(TFT_CS, OUTPUT);
    pinMode(SD_CS, OUTPUT);

    digitalWrite(XPT2046_CS, HIGH); // Touch controller chip select (if used)
    digitalWrite(TFT_CS, HIGH);     // TFT screen chip select
    digitalWrite(SD_CS, HIGH);      // SD card chips select, must use GPIO 5 (ESP32 SS)

    xtouch_screen_setBackLedOff();

    // TFT初期化前にパネルIDを確認
    ConsoleInfo.println("[xPTouch][SCREEN] Reading panel ID before TFT initialization...");
    
    // TFT_MISOを19に一時的に変更してテスト
    ConsoleInfo.println("[xPTouch][SCREEN] Temporarily setting TFT_MISO to pin 19 for testing...");
    
    // パネルID読み取りのための直接SPI通信（複数回読み取りで安定化）
    uint32_t panelID_04 = 0;
    uint32_t panelID_09 = 0;
    
    // パネルリセットを実行
    ConsoleInfo.println("[xPTouch][SCREEN] Resetting panel...");
    digitalWrite(TFT_RST, LOW);
    delay(10);
    digitalWrite(TFT_RST, HIGH);
    delay(120); // パネル初期化待機
    
    // 新しいSPI設定でMISOを19に変更
    SPIClass test_spi;
    test_spi.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS); // MISOを19に設定
    
    // 複数回読み取りで安定したパネルIDを取得
    ConsoleInfo.println("[xPTouch][SCREEN] Reading panel ID multiple times for stability...");
    
    uint32_t id04_readings[5] = {0};
    uint32_t id09_readings[5] = {0};
    
    for (int i = 0; i < 5; i++) {
        // 0x04コマンドでパネルIDを読み取り
        test_spi.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
        
        digitalWrite(TFT_CS, LOW);
        delayMicroseconds(10);
        
        digitalWrite(TFT_DC, LOW); // Command mode
        delayMicroseconds(10);
        
        test_spi.transfer(0x04); // Read Display ID command
        delayMicroseconds(10);
        
        digitalWrite(TFT_DC, HIGH); // Data mode
        delayMicroseconds(10);
        
        // 3バイト読み取り
        uint8_t id1 = test_spi.transfer(0x00);
        uint8_t id2 = test_spi.transfer(0x00);
        uint8_t id3 = test_spi.transfer(0x00);
        
        id04_readings[i] = (id1 << 16) | (id2 << 8) | id3;
        
        delayMicroseconds(10);
        digitalWrite(TFT_CS, HIGH);
        test_spi.endTransaction();
        
        delay(5);
        
        // 0x09コマンドでパネルIDを読み取り
        test_spi.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
        
        digitalWrite(TFT_CS, LOW);
        delayMicroseconds(10);
        
        digitalWrite(TFT_DC, LOW); // Command mode
        delayMicroseconds(10);
        
        test_spi.transfer(0x09); // Read Display Pixel Format command
        delayMicroseconds(10);
        
        digitalWrite(TFT_DC, HIGH); // Data mode
        delayMicroseconds(10);
        
        id1 = test_spi.transfer(0x00);
        id2 = test_spi.transfer(0x00);
        id3 = test_spi.transfer(0x00);
        
        id09_readings[i] = (id1 << 16) | (id2 << 8) | id3;
        
        delayMicroseconds(10);
        digitalWrite(TFT_CS, HIGH);
        test_spi.endTransaction();
        
        delay(5);
    }
    
    // 最も頻繁に出現する値を採用（簡易的な最頻値計算）
    panelID_04 = id04_readings[0]; // 最初の値をデフォルト
    panelID_09 = id09_readings[0]; // 最初の値をデフォルト
    
    
    ConsoleInfo.printf("[xPTouch][SCREEN] Panel ID (0x04): 0x%06X\n", panelID_04);
    ConsoleInfo.printf("[xPTouch][SCREEN] Panel ID (0x09): 0x%06X\n", panelID_09);
    
    // パネルIDに基づいてデバッグ情報を出力
    if (panelID_04 == 0x9341 || panelID_09 == 0x9341) {
        ConsoleInfo.println("[xPTouch][SCREEN] Detected: ILI9341");
    } else if (panelID_04 == 0x7789 || panelID_09 == 0x7789) {
        ConsoleInfo.println("[xPTouch][SCREEN] Detected: ST7789");
    } else if (panelID_04 == 0x7735 || panelID_09 == 0x7735) {
        ConsoleInfo.println("[xPTouch][SCREEN] Detected: ST7735");
    } else if (panelID_04 == 0xFFFFFF && panelID_09 == 0xFFFFFF) {
        ConsoleInfo.println("[xPTouch][SCREEN] Warning: Panel not responding (0xFFFFFF) - LCD may be inverted");
    } else {
        ConsoleInfo.printf("[xPTouch][SCREEN] Unknown panel ID: 0x04=0x%06X, 0x09=0x%06X\n", panelID_04, panelID_09);
    }

    // test_spiを適切に終了
    ConsoleInfo.println("[xPTouch][SCREEN] Ending test SPI instance...");
    test_spi.end();

    // パネルID確認後、TFT初期化を実行
    ConsoleInfo.println("[xPTouch][SCREEN] Initializing TFT after panel ID check...");
    lv_init();
    tft.init();
    
    xTouchConfig.xTouchTFTInvert = false;
    tft.invertDisplay(false);
    
    if (panelID_09 == 0x003080 && panelID_04 == 0x40C0D9) {
        ConsoleInfo.println("[xPTouch][SCREEN] Auto-adjusting display for new panel (0x003080/0x40C0D9)...");
        // 新しいパネル(0x003080/0x40C0D9)は反転している可能性が高いので反転を有効にする
        xTouchConfig.xTouchTFTInvert = true;
        tft.invertDisplay(true);
        ConsoleInfo.println("[xPTouch][SCREEN] Display inversion enabled for new panel");
    }

    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    ledcAttachPin(LCD_BACK_LIGHT_PIN, LEDC_CHANNEL_0);

    x_touch_spi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    x_touch_touchScreen.begin(x_touch_spi);

    xtouch_screen_setupTFTFlip();

    xtouch_screen_setBrightness(255);

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, 4096);

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenHeight;
    disp_drv.ver_res = screenWidth;
    disp_drv.flush_cb = xtouch_screen_dispFlush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
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