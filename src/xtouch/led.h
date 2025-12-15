#ifndef _XLCD_LED_CONTROL
#define _XLCD_LED_CONTROL

#include "globals.h"
#include "debug.h"
#include "types.h"

int led_red = 0;
int led_green = 0;
int led_blue = 0;

#if defined(__XTOUCH_SCREEN_28__)
#include "devices/2.8/screen.h"
#elif defined(__XTOUCH_SCREEN_50__)
#include "devices/5.0/screen.h"
#endif


void xtouch_led_set(int red, int green, int blue)
{
    // アクティブローのLEDのため、PWM値を反転（255-値）
    ledcWrite(0, 255 - red);   // 赤色LED（チャンネル0）
    ledcWrite(1, 255 - green); // 緑色LED（チャンネル1）
    ledcWrite(2, 255 - blue);  // 青色LED（チャンネル2）
}

void xtouch_led_init(void)
{
    lgfx::boards::board_t board = tft.getBoard();
#if defined(__XTOUCH_SCREEN_28__)
    if (board == lgfx::boards::board_t::board_Guition_ESP32_2432W328R || board == lgfx::boards::board_t::board_Guition_ESP32_2432W328C)
    {
        ConsoleDebug.println("LED GPIO 4, 16, 17");
        led_red = 4;
        led_green = 16;
        led_blue = 17;
    }
#endif
    ConsoleDebug.println("XTOUCH_LED_RED: " + String(led_red) + " GREEN: " + String(led_green) + " BLUE: " + String(led_blue));

    // LED用GPIOの初期化とオフ設定
    pinMode(led_red, OUTPUT);
    pinMode(led_blue, OUTPUT);
    pinMode(led_green, OUTPUT);

    // PWM設定（各LEDに専用チャンネルを割り当て）
    ledcSetup(0, 5000, 8); // 赤色LED用チャンネル0
    ledcSetup(1, 5000, 8); // 緑色LED用チャンネル1
    ledcSetup(2, 5000, 8); // 青色LED用チャンネル2

    ledcAttachPin(led_red, 0);
    ledcAttachPin(led_green, 1);
    ledcAttachPin(led_blue, 2);

    xtouch_led_set(0, 0, 0); // オフ
}
#endif