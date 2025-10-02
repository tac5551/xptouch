#ifndef _XLCD_LED_CONTROL
#define _XLCD_LED_CONTROL

#include "globals.h"
#include "debug.h"
#include "types.h"

#include <Adafruit_NeoPixel.h>
#if defined(__XTOUCH_SCREEN_28__)
#include "devices/2.8/screen.h"
#elif defined(__XTOUCH_SCREEN_50__)
#include "devices/5.0/screen.h"
#endif

#define PIXEL_COUNT 10

int led_red = 0;
int led_green = 0;
int led_blue = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, 27, NEO_GRB + NEO_KHZ800);

void xtouch_neo_pixel_round_trip(int count, int iRed, int iGreen, int iBlue, int weight = 50);
void xtouch_neo_pixel_on(int iRed, int iGreen, int iBlue, int weight = 50);
void xtouch_neo_pixel_off(int weight = 10);

int leds[3][PIXEL_COUNT];


void xtouch_led_set(int red, int green, int blue)
{
    // アクティブローのLEDのため、PWM値を反転（255-値）
    ledcWrite(0, 255 - red);    // 赤色LED（チャンネル0）
    ledcWrite(1, 255 - green);  // 緑色LED（チャンネル1）
    ledcWrite(2, 255 - blue);   // 青色LED（チャンネル2）
}

void xtouch_led_set_status(void)
{
    switch (bambuStatus.print_status)
    {
    case XTOUCH_PRINT_STATUS_PREPARE:
        xtouch_led_set(255, 255, 0);
        break;
    case XTOUCH_PRINT_STATUS_RUNNING:
        xtouch_led_set(0, 255, 0);
        break;
    case XTOUCH_PRINT_STATUS_PAUSED:
        xtouch_led_set(0, 0, 255);
        break;
    case XTOUCH_PRINT_STATUS_FINISHED:
        xtouch_led_set(0, 255, 0);
        break;
    case XTOUCH_PRINT_STATUS_IDLE:
        xtouch_led_set(0, 0, 0);
        break;
    case XTOUCH_PRINT_STATUS_FAILED:
        xtouch_led_set(255, 0, 0);
        break;
    }
    ConsoleDebug.print("[xPTouch][MQTT] ★★ mc_print_stage : " + String(bambuStatus.mc_print_stage));
    ConsoleDebug.print(" , print_status : " + String(bambuStatus.print_status));
    ConsoleDebug.print(" , print_gcode_action : " + String(bambuStatus.print_gcode_action));

    ConsoleDebug.print(" , mc_print_sub_stage : " + String(bambuStatus.mc_print_sub_stage));
    ConsoleDebug.println(", print_real_action : " + String(bambuStatus.print_real_action));
}

void xtouch_led_init(void)
{
    lgfx::boards::board_t board = tft.getBoard();

    if (board == lgfx::boards::board_t::board_Guition_ESP32_2432W328R || board == lgfx::boards::board_t::board_Guition_ESP32_2432W328C)
    {
        ConsoleDebug.println("LED GPIO 4, 16, 17");
        led_red = 4;
        led_green = 16;
        led_blue = 17;
    }
    else if (board == lgfx::boards::board_t::board_ESP32_ESP32E)
    {
        ConsoleDebug.println("LED GPIO 22, 16, 17");
        led_red = 22;
        led_green = 16;
        led_blue = 17;
    }

    ConsoleDebug.println("XTOUCH_LED_RED: " + String(led_red) + " GREEN: " + String(led_green) + " BLUE: " + String(led_blue));

    // LED用GPIOの初期化とオフ設定
    pinMode(led_red, OUTPUT);
    pinMode(led_blue, OUTPUT);
    pinMode(led_green, OUTPUT);

    // PWM設定（各LEDに専用チャンネルを割り当て）
    ledcSetup(0, 5000, 8);  // 赤色LED用チャンネル0
    ledcSetup(1, 5000, 8);  // 緑色LED用チャンネル1
    ledcSetup(2, 5000, 8);  // 青色LED用チャンネル2

    ledcAttachPin(led_red, 0);
    ledcAttachPin(led_green, 1);
    ledcAttachPin(led_blue, 2);

    xtouch_led_set(255, 255, 255);
    delay(100);
    xtouch_led_set(0, 0, 255);
    delay(100);
    xtouch_led_set(0, 255, 0);
    delay(100);
    xtouch_led_set(255, 0, 0);
    delay(100);
    xtouch_led_set(0, 0, 0);
    delay(100);
}

void xtouch_neo_pixel_init(void)
{
    lgfx::boards::board_t board = tft.getBoard();
    if (board == lgfx::boards::board_t::board_ESP32_ESP32E)
        return;

    ConsoleDebug.println("XTOUCH_NEO_PIXEL_INIT");
    for(int i=0;i<3;i++){
        for(int j=0;j<PIXEL_COUNT;j++){
            leds[i][j] = 0;
        }
    }

    if (board == lgfx::boards::board_t::board_Guition_ESP32_2432W328R || board == lgfx::boards::board_t::board_Guition_ESP32_2432W328C)
    {
        strip.setPin(21);
    }
    else if (board == lgfx::boards::board_t::board_Sunton_ESP32_2432S028_7789 || board == lgfx::boards::board_t::board_Sunton_ESP32_2432S028_9341)
    {
        strip.setPin(27);
    }

    strip.begin();

    xtouch_neo_pixel_round_trip(2, 255, 0, 0);
    xtouch_neo_pixel_on(255, 255, 255);

    xtouch_neo_pixel_off();
}

void xtouch_neo_pixel_off(int weight )
{
    lgfx::boards::board_t board = tft.getBoard();
    if (board == lgfx::boards::board_t::board_ESP32_ESP32E)
        return;
    ConsoleDebug.println("XTOUCH_NEO_PIXEL_OFF");
    int step = 10;
    for(int j=255;j>0;j=j-step){
        for(int i=0;i<PIXEL_COUNT;i++){
            leds[0][i] -= step;
            leds[1][i] -= step;
            leds[2][i] -= step;
            if(leds[0][i] < 0) leds[0][i] = 0;
            if(leds[1][i] < 0) leds[1][i] = 0;
            if(leds[2][i] < 0) leds[2][i] = 0;
            strip.setPixelColor(i, strip.Color(leds[0][i], leds[1][i], leds[2][i]));
        }
        strip.show();
        if (weight > 0) delay(weight);
    }
}

void xtouch_neo_pixel_on(int iRed, int iGreen, int iBlue, int weight)
{
    lgfx::boards::board_t board = tft.getBoard();
    if (board == lgfx::boards::board_t::board_ESP32_ESP32E)
        return;
    ConsoleDebug.println("XTOUCH_NEO_PIXEL_ROUND_TRIP");

    int idx = 0;
    // ひとつづつつけていく
    for (int j = 0; j < PIXEL_COUNT; j++)
    {
        leds[0][j] = 0;
        leds[1][j] = 0;
        leds[2][j] = 0;

        if (j == idx)
        {
            leds[0][j] = iRed;
            leds[1][j] = iGreen;
            leds[2][j] = iBlue;
        }

        strip.setPixelColor(j, strip.Color(leds[0][j], leds[1][j], leds[2][j]));
        strip.show();
        if (weight > 0) delay(weight);
        idx++;
    }
}

void xtouch_neo_pixel_round_trip(int count, int iRed, int iGreen, int iBlue, int weight)
{
    lgfx::boards::board_t board = tft.getBoard();
    if (board == lgfx::boards::board_t::board_ESP32_ESP32E)
        return;
    ConsoleDebug.println("XTOUCH_NEO_PIXEL_ROUND_TRIP");

    bool forward = true;
    int idx = 0;
    for (int i = 0; i < count * 2; i++)
    {
        for (int k = 0; k < PIXEL_COUNT; k++)
        {
            for (int j = 0; j < PIXEL_COUNT; j++)
            {
                leds[0][j] = 0;
                leds[1][j] = 0;
                leds[2][j] = 0;

                if (j == idx)
                {
                    leds[0][j] = iRed;
                    leds[1][j] = iGreen;
                    leds[2][j] = iBlue;
                }
                if (forward)
                {
                    if (j == idx - 1)
                    {
                        leds[0][j] = iRed / 2;
                        leds[1][j] = iGreen / 2;
                        leds[2][j] = iBlue / 2;
                    }
                }
                else
                {
                    if (j == idx + 1)
                    {
                        leds[0][j] = iRed / 2;
                        leds[1][j] = iGreen / 2;
                        leds[2][j] = iBlue / 2;
                    }
                }
                strip.setPixelColor(j, strip.Color(leds[0][j], leds[1][j], leds[2][j]));
            }
            strip.show();
            if (forward)
                idx++;
            else
                idx--;

            if (weight > 0) delay(weight);
        }

        if (forward)
            forward = false;
        else
            forward = true;
    }
}




#endif