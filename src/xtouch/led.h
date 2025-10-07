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

#define NeoPixelCount 45

int led_red = 0;
int led_green = 0;
int led_blue = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NeoPixelCount, 27, NEO_GRB + NEO_KHZ800);

void xtouch_neo_pixel_round_trip(int count, int iRed, int iGreen, int iBlue, int weight = 50);
void xtouch_neo_pixel_on(int iRed, int iGreen, int iBlue, int weight = 50);
void xtouch_neo_pixel_off(int weight = 10);

int leds[3][NeoPixelCount];

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
    ledcSetup(0, 5000, 8); // 赤色LED用チャンネル0
    ledcSetup(1, 5000, 8); // 緑色LED用チャンネル1
    ledcSetup(2, 5000, 8); // 青色LED用チャンネル2

    ledcAttachPin(led_red, 0);
    ledcAttachPin(led_green, 1);
    ledcAttachPin(led_blue, 2);

    // // オレンジ色のLEDテスト
    // ConsoleDebug.println("LED Test - Orange");
    // xtouch_led_set(255, 165, 0); // オレンジ色（赤255, 緑165, 青0）
    // delay(1000);
    xtouch_led_set(0, 0, 0); // オフ
    // delay(100);
}

void xtouch_neo_pixel_init(void)
{
    lgfx::boards::board_t board = tft.getBoard();
    if (board == lgfx::boards::board_t::board_ESP32_ESP32E)
        return;

    ConsoleDebug.println("XTOUCH_NEO_PIXEL_INIT");
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < NeoPixelCount; j++)
        {
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

    //xtouch_neo_pixel_round_trip(2, 255, 0, 0);
    //xtouch_neo_pixel_on(255, 255, 255);

    //xtouch_neo_pixel_off();
}

void xtouch_neo_pixel_off(int weight)
{
    lgfx::boards::board_t board = tft.getBoard();
    if (board == lgfx::boards::board_t::board_ESP32_ESP32E)
        return;
    int step = 10;
    for (int j = 255; j > 0; j = j - step)
    {
        for (int i = 0; i < NeoPixelCount; i++)
        {
            leds[0][i] -= step;
            leds[1][i] -= step;
            leds[2][i] -= step;
            if (leds[0][i] < 0)
                leds[0][i] = 0;
            if (leds[1][i] < 0)
                leds[1][i] = 0;
            if (leds[2][i] < 0)
                leds[2][i] = 0;
            strip.setPixelColor(i, strip.Color(leds[0][i], leds[1][i], leds[2][i]));
        }
        strip.show();
        if (weight > 0)
            delay(weight);
    }
}

void xtouch_neo_pixel_on(int iRed, int iGreen, int iBlue, int weight)
{
    lgfx::boards::board_t board = tft.getBoard();
    if (board == lgfx::boards::board_t::board_ESP32_ESP32E)
        return;

    int idx = 0;
    // ひとつづつつけていく
    for (int j = 0; j < NeoPixelCount; j++)
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
        if (weight > 0)
            delay(weight);
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
        for (int k = 0; k < NeoPixelCount; k++)
        {
            for (int j = 0; j < NeoPixelCount; j++)
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

            if (weight > 0)
                delay(weight);
        }

        if (forward)
            forward = false;
        else
            forward = true;
    }
}

void xtouch_neo_pixel_control_timer_stop();
void xtouch_neo_pixel_control_timer_start();
void xtouch_neo_pixel_control_timer_create();
void xtouch_neo_pixel_set_led_color(int red, int green, int blue);
void xtouch_neo_pixel_set_pettern(int pattern);
void xtouch_neo_pixel_set_status_timeout(int timeout);
void xtouch_neo_pixel_control_timer_handler(lv_timer_t *timer);

lv_timer_t *xtouch_neo_pixel_control_timer;
bool xtouch_neo_pixel_control_started = false;
int last_print_status = 0;
bool print_status_changed = false;
int last_print_gcode_action = 0;
bool print_gcode_action_changed = false;
int status_timeout = -1; // -1:無期限 x:x秒でステータスにかかわらずIDLEに戻す
int led_color[3] = {0, 0, 0};
int pettern = 0; // 0:on hold 1:fade slow 2:fade fast 3:flowing light

// アニメーション制御用変数
int animation_frame = 0;     // 現在のフレーム
int animation_direction = 1; // 流れる光の方向（1: 前進, -1: 後退）
bool blink_state = false;    // 点滅状態
int blink_counter = 0;       // 点滅カウンター
int flowing_position = 0;    // 流れる光の位置
int fade_counter = 0;        // フェードカウンター
int fade_direction = 1;      // フェード方向（1: フェードイン, -1: フェードアウト）


int blightness_max = 32;
int blightness_min = 2;
int flowing_speed = 8; // より滑らかな移動のため増加
const int timer_tick = 5;

void xtouch_neo_pixel_control_timer_create()
{
    xtouch_neo_pixel_control_timer = lv_timer_create(xtouch_neo_pixel_control_timer_handler, timer_tick, NULL);
    lv_timer_set_repeat_count(xtouch_neo_pixel_control_timer, -1); // 無限ループ
}

void xtouch_neo_pixel_control_timer_start()
{
    if (!xtouch_neo_pixel_control_started)
    {
        xtouch_led_init();
        xtouch_neo_pixel_init();
        xtouch_neo_pixel_control_started = true;
    }
    xtouch_neo_pixel_control_timer_create();
}

void xtouch_neo_pixel_control_timer_stop()
{
    lv_timer_pause(xtouch_neo_pixel_control_timer);
}

void xtouch_led_timer_init()
{

    // if (xTouchConfig.xTouchNeoPixelEnabled)
    // {
    xtouch_neo_pixel_control_timer_start();
    // }
    // else
    // {
    //     if (xtouch_neo_pixel_control_started)
    //     {

    //         xtouch_neo_pixel_control_timer_stop();
    //     }
    // }
}

void xtouch_neo_pixel_set_led_color(int red, int green, int blue)
{
    led_color[0] = red;
    led_color[1] = green;
    led_color[2] = blue;
}

void xtouch_neopixel_set_num(int num)
{
    int backup  =NeoPixelCount;
    //NeoPixelCount = num;

    // 範囲外を消灯
    for (int i = NeoPixelCount; i < 100; i++)
    {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.show();
}

void xtouch_neopixel_set_brightness(int brightness)
{
    blightness_max = brightness;
}

void xtouch_neo_pixel_set_pettern(int pattern)
{
    pettern = pattern;
}
void xtouch_neo_pixel_set_status_timeout(int timeout)
{
    status_timeout = timeout;
}

void xtouch_neo_pixel_control_timer_handler(lv_timer_t *timer)
{
    // status_timeoutのカウントダウン処理
    if (status_timeout > 0)
    {
        status_timeout -= timer_tick * 10;//main delay 10ms
        if (status_timeout <= 0)
        {
            ConsoleDebug.println("[xPTouch][LED] print_status : " + String(bambuStatus.print_status) + " , print_gcode_action : " + String(bambuStatus.print_gcode_action) + "percent : " + String(bambuStatus.mc_print_percent)) ;
            status_timeout = 0;
        }
    }
    
    // ステータスを確認してLEDを制御
    if (last_print_status != bambuStatus.print_status)
    {
        last_print_status = bambuStatus.print_status;
        print_status_changed = true;
        // ステータス変更時にアニメーションをリセット
        animation_frame = 0;
        blink_state = false;
        blink_counter = 0;
        flowing_position = 0;
        fade_counter = 0;
        fade_direction = 1;
    }

    if (last_print_gcode_action != bambuStatus.print_gcode_action)
    {
        last_print_gcode_action = bambuStatus.print_gcode_action;
        print_gcode_action_changed = true;
    }

    if (print_status_changed || print_gcode_action_changed)
    {

        if (bambuStatus.print_status == XTOUCH_PRINT_STATUS_PREPARE)
        {
            xtouch_neo_pixel_set_led_color(255, 80, 0); // オレンジ色
            xtouch_neo_pixel_set_pettern(4);
        }
        else if (bambuStatus.print_status == XTOUCH_PRINT_STATUS_RUNNING)
        {
            if (bambuStatus.print_gcode_action == 0) // 0	turn off light and wait extrude temperature
            {
                xtouch_neo_pixel_set_led_color(0, 0, 255); // 青色
                xtouch_neo_pixel_set_pettern(4); // 進捗表示パターン
            }
            else if (bambuStatus.print_gcode_action == 1) // 1	bed leveling
            {
                xtouch_neo_pixel_set_led_color(255, 110, 0); // オレンジ色
                xtouch_neo_pixel_set_pettern(4);
            }
            else if (bambuStatus.print_gcode_action == 2) // 2	heatbet preheat
            {
                xtouch_neo_pixel_set_led_color(255, 60, 0); // オレンジ色
                xtouch_neo_pixel_set_pettern(3);
            }
            else if (bambuStatus.print_gcode_action == 8) // 8	draw extrinsic para cali pain
            {
                xtouch_neo_pixel_set_led_color(255, 130, 0); // オレンジ色
                xtouch_neo_pixel_set_pettern(4);
            }
            else if (bambuStatus.print_gcode_action == 13) // 13	home after wipe mouth
            {
                xtouch_neo_pixel_set_led_color(255, 140, 0); // オレンジ色
                xtouch_neo_pixel_set_pettern(4);
            }
            else if (bambuStatus.print_gcode_action == 14) // 14	nozzle wipe
            {
                xtouch_neo_pixel_set_led_color(255, 150, 0); // オレンジ色
                xtouch_neo_pixel_set_pettern(4);
            }
            else if (bambuStatus.print_gcode_action == 255) // 255	unknown
            {
                xtouch_neo_pixel_set_led_color(0, 160, 255); // シアン（水色）
                xtouch_neo_pixel_set_pettern(3);
            }
            else
            {
                xtouch_neo_pixel_set_led_color(0, 0, 0); // オフ
            }
        }
        else if (bambuStatus.print_status == XTOUCH_PRINT_STATUS_PAUSED)
        {
            xtouch_neo_pixel_set_led_color(128, 128, 255); // 白色
            xtouch_neo_pixel_set_pettern(1);
        }
        else if (bambuStatus.print_status == XTOUCH_PRINT_STATUS_FINISHED)
        {
            xtouch_neo_pixel_set_led_color(0, 255, 0);
            xtouch_neo_pixel_set_pettern(1);
            xtouch_neo_pixel_set_status_timeout(30000);
        }
        else if (bambuStatus.print_status == XTOUCH_PRINT_STATUS_FAILED)
        {
            xtouch_neo_pixel_set_led_color(64, 0, 0); // 赤色の点滅
            xtouch_neo_pixel_set_pettern(6);
            xtouch_neo_pixel_set_status_timeout(120000);
        }
        else if (bambuStatus.print_status == XTOUCH_PRINT_STATUS_IDLE)
        {
            xtouch_neo_pixel_set_led_color(32, 0, 0);
            xtouch_neo_pixel_set_pettern(5);
        }
        else
        {
            xtouch_neo_pixel_set_led_color(0, 0, 0);
            xtouch_neo_pixel_set_pettern(0);
        }
        ConsoleDebug.println("[xPTouch][LED] print_status : " + String(bambuStatus.print_status) + " , print_gcode_action : " + String(bambuStatus.print_gcode_action) + " , percent : " + String(bambuStatus.mc_print_percent)) ;
        ConsoleDebug.println("[xPTouch][LED] pettern : " + String(pettern) + " , led_color : " + String(led_color[0]) + " , " + String(led_color[1]) + " , " + String(led_color[2]) +" status_timeout : " + String(status_timeout)) ;
    }

    //タイムアウト時は無条件でIDLE状態に設定
    if(status_timeout == 0 ){
        ConsoleDebug.println("[xPTouch][LED] status_timeout : " + String(status_timeout)) ;
        xtouch_neo_pixel_set_led_color(32, 0, 0);
        xtouch_neo_pixel_set_pettern(5);
        print_status_changed = true; //ステータス変更状態とする
        status_timeout = -1; //タイムアウトを無制限に。
    }
    // ステータスが変わっていない場合のみアニメーション処理を実行
    // パターンに応じたアニメーション処理
    int current_red = led_color[0];
    int current_green = led_color[1];
    int current_blue = led_color[2];
    int fade_progress = 0; // 変数宣言をswitch文の外に移動
    int brightness_range = 0; // 変数宣言をswitch文の外に移動
    int current_brightness = 0; // 変数宣言をswitch文の外に移動
    int progress_leds = 0; // 進捗表示用変数をswitch文の外に移動
    float smooth_position = 0.0; // 流れる光用変数をswitch文の外に移動
    int progress_percent = 0; // 進捗率用変数をswitch文の外に移動
    int fade_interval = 1000;

    //Setting の明るさが0の場合は無条件でOFF
    if(xTouchConfig.xTouchNeoPixelBlightnessValue <= 5){
        xtouch_neo_pixel_off(0);
        // ステータス変更フラグをリセット
        print_status_changed = false;
        print_gcode_action_changed = false;
        return;
    }

    switch (pettern)
    {
    case 0: // 点灯保持
            // 何もしない（既に設定された色で点灯し続ける）
        if (print_status_changed || print_gcode_action_changed)
        {
            xtouch_neo_pixel_on(current_red, current_green, current_blue, 0);
        }
        break;

     case 1: // ゆっくりフェード（2秒周期でフェードアウト/フェードイン）
         fade_counter += timer_tick;
        fade_interval = 200;
        if (fade_counter >= fade_interval) 
        {
            fade_direction = -fade_direction; // 方向を反転
            fade_counter = 0;
        }

        // フェード計算（0-255の範囲）
        fade_progress = (fade_counter * 255) / fade_interval; // 0-255
        if (fade_direction == -1)
        {
            fade_progress = 255 - fade_progress; // フェードアウト
        }

        // フェード計算（blightness_min から blightness_max の範囲）
        brightness_range = blightness_max - blightness_min;
        current_brightness = blightness_min + (fade_progress * brightness_range) / 255;
        
        current_red = (led_color[0] * current_brightness) / 255;
        current_green = (led_color[1] * current_brightness) / 255;
        current_blue = (led_color[2] * current_brightness) / 255;
        xtouch_neo_pixel_on(current_red, current_green, current_blue, 0);
        break;

     case 2: // 速いフェード（1秒周期でフェードアウト/フェードイン）
        fade_counter += timer_tick;
        fade_interval = 100;
        if (fade_counter >= fade_interval) 
        {
            fade_direction = -fade_direction; // 方向を反転
            fade_counter = 0;
        }

        // フェード計算（0-255の範囲）
        fade_progress = (fade_counter * 255) / fade_interval; // 0-255
         if (fade_direction == -1)
         {
             fade_progress = 255 - fade_progress; // フェードアウト
         }

         // フェード計算（blightness_min から blightness_max の範囲）
         brightness_range = blightness_max - blightness_min;
         current_brightness = blightness_min + (fade_progress * brightness_range) / 255;
         
         current_red = (led_color[0] * current_brightness) / 255;
         current_green = (led_color[1] * current_brightness) / 255;
         current_blue = (led_color[2] * current_brightness) / 255;
         xtouch_neo_pixel_on(current_red, current_green, current_blue, 0);
         break;

    case 3: // 流れる光（滑らか、ゆっくり）
        // 流れる光の位置を更新（一方向のみ、滑らか）
        flowing_position++;
        if (flowing_position >= NeoPixelCount * flowing_speed)
        {
            flowing_position = 0; // 最初に戻る
        }
        
        // 滑らかな位置計算（小数点以下も考慮）
        smooth_position = (float)flowing_position / flowing_speed;

        // 各LEDの明度を滑らかに計算
        for (int i = 0; i < NeoPixelCount; i++)
        {
            // 距離を滑らかに計算（ループを考慮）
            float distance1 = fabs(i - smooth_position);
            float distance2 = fabs(i - (smooth_position - NeoPixelCount));
            float distance3 = fabs(i - (smooth_position + NeoPixelCount));
            float distance = min(distance1, min(distance2, distance3));
            
            // 滑らかなグラデーション計算
            float brightness_factor = 1.0;
            if (distance <= 1.0)
            {
                // 中心付近：滑らかな減衰
                brightness_factor = 1.0 - (distance * 0.3);
            }
            else if (distance <= 2.0)
            {
                // 中距離：さらに滑らかな減衰
                brightness_factor = 0.7 - ((distance - 1.0) * 0.4);
            }
            else if (distance <= 3.0)
            {
                // 中遠距離：さらに滑らかな減衰
                brightness_factor = 0.3 - ((distance - 2.0) * 0.2);
            }
            else
            {
                // 遠距離：最小明度
                brightness_factor = 0.1;
            }
            
            // 明度を計算（blightness_minからblightness_maxの範囲）
            int brightness = blightness_min + (int)((blightness_max - blightness_min) * brightness_factor);
            if (brightness < blightness_min) brightness = blightness_min;
            if (brightness > blightness_max) brightness = blightness_max;

            int r = (current_red * brightness) / 255;
            int g = (current_green * brightness) / 255;
            int b = (current_blue * brightness) / 255;
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        strip.show();
        break; 

    case 4: // 進捗表示（印刷中）- 流れる光付き
        // 進捗率を10%ごとに切り上げ（最低1個は点灯）
        progress_percent = bambuStatus.mc_print_percent;
        if (progress_percent > 0 && progress_percent < 100) {
            progress_percent = ((progress_percent + 9) / 10) * 10; // 10%ごとに切り上げ
        }
        if (progress_percent > 0 && progress_percent < 10) {
            progress_percent = 10; // 最低10%として1個点灯
        }
        
        // 進捗率に基づいてLEDの個数を計算
        progress_leds = (progress_percent * NeoPixelCount) / 100;
        if (progress_leds > NeoPixelCount) progress_leds = NeoPixelCount;
        if (progress_leds < 1 && progress_percent > 0) progress_leds = 1; // 最低1個は点灯
        
        // 流れる光の位置を更新
        flowing_position++;
        if (flowing_position >= NeoPixelCount * flowing_speed)
        {
            flowing_position = 0; // 最初に戻る
        }
        
        // 滑らかな位置計算
        smooth_position = (float)flowing_position / flowing_speed;
        
        // 全LEDをblightness_minの明るさで点灯
        for (int i = 0; i < NeoPixelCount; i++)
        {
            int r = (current_red * blightness_min) / 255;
            int g = (current_green * blightness_min) / 255;
            int b = (current_blue * blightness_min) / 255;
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        
        // 進捗分のLEDを明るく点灯（流れる光効果付き）
        for (int i = 0; i < progress_leds; i++)
        {
            // 距離を滑らかに計算（ループを考慮）
            float distance1 = fabs(i - smooth_position);
            float distance2 = fabs(i - (smooth_position - NeoPixelCount));
            float distance3 = fabs(i - (smooth_position + NeoPixelCount));
            float distance = min(distance1, min(distance2, distance3));
            
            // 滑らかなグラデーション計算
            float brightness_factor = 1.0;
            if (distance <= 1.0)
            {
                brightness_factor = 1.0 - (distance * 0.3);
            }
            else if (distance <= 2.0)
            {
                brightness_factor = 0.7 - ((distance - 1.0) * 0.4);
            }
            else if (distance <= 3.0)
            {
                brightness_factor = 0.3 - ((distance - 2.0) * 0.2);
            }
            else
            {
                brightness_factor = 0.1;
            }
            
            // 明度を計算（blightness_minからblightness_maxの範囲）
            int brightness = blightness_min + (int)((blightness_max - blightness_min) * brightness_factor);
            if (brightness < blightness_min) brightness = blightness_min;
            if (brightness > blightness_max) brightness = blightness_max;

            int r = (current_red * brightness) / 255;
            int g = (current_green * brightness) / 255;
            int b = (current_blue * brightness) / 255;
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        
        // 進捗分より後ろのLEDを明示的に消す
        for (int i = progress_leds; i < NeoPixelCount; i++)
        {
            strip.setPixelColor(i, strip.Color(0, 0, 0));
        }
        
        strip.show();
        break;

    case 5: // 流れる光（往復バージョン）
        // 流れる光の位置を更新（往復）
        flowing_position += animation_direction;
        if (flowing_position >= NeoPixelCount * flowing_speed)
        {
            flowing_position = NeoPixelCount * flowing_speed;
            animation_direction = -1; // 逆方向に変更
        }
        else if (flowing_position <= 0)
        {
            flowing_position = 0;
            animation_direction = 1; // 正方向に変更
        }
        
        // 滑らかな位置計算
        smooth_position = (float)flowing_position / flowing_speed;
        
        // 全LEDをblightness_minの明るさで点灯
        for (int i = 0; i < NeoPixelCount; i++)
        {
            int r = (current_red * blightness_min) / 255;
            int g = (current_green * blightness_min) / 255;
            int b = (current_blue * blightness_min) / 255;
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        
        // 流れる光の効果を適用
        for (int i = 0; i < NeoPixelCount; i++)
        {
            // 距離を滑らかに計算（往復バージョンではループ処理不要）
            float distance = fabs(i - smooth_position);
            
            // 滑らかなグラデーション計算
            float brightness_factor = 1.0;
            if (distance <= 1.0)
            {
                brightness_factor = 1.0 - (distance * 0.3);
            }
            else if (distance <= 2.0)
            {
                brightness_factor = 0.7 - ((distance - 1.0) * 0.4);
            }
            else if (distance <= 3.0)
            {
                brightness_factor = 0.3 - ((distance - 2.0) * 0.2);
            }
            else
            {
                brightness_factor = 0.1;
            }
            
            // 明度を計算（blightness_minからblightness_maxの範囲）
            int brightness = blightness_min + (int)((blightness_max - blightness_min) * brightness_factor);
            if (brightness < blightness_min) brightness = blightness_min;
            if (brightness > blightness_max) brightness = blightness_max;

            int r = (current_red * brightness) / 255;
            int g = (current_green * brightness) / 255;
            int b = (current_blue * brightness) / 255;
            strip.setPixelColor(i, strip.Color(r, g, b));
        }
        
        strip.show();
        break;

    case 6: // パッパ、パッパ点滅
        // パッパ、パッパのタイミング制御
        fade_counter += timer_tick;
        int on1 = 10;
        int off1 = on1 + 10;
        int on2 = off1 + 10;
        int off2 = on2 + 10;
        int on3 = off2 + 10;
        int off3 = on2 + 120;

        
        // パッパ、パッパの周期: 800ms (400ms点灯 + 400ms消灯)
        if (fade_counter >= off3)
        {
            fade_counter = 0;
        }
        
        // パッパ、パッパのパターン
        if (fade_counter < on1) // 100ms 点灯 
        {
            xtouch_neo_pixel_on(current_red, current_green, current_blue, 0);
        }
        else if (fade_counter < off1) // 200ms 消灯
        {
            xtouch_neo_pixel_off(0);
        }
        else if (fade_counter < on2) // 100ms点灯
        {
            xtouch_neo_pixel_on(current_red, current_green, current_blue, 0);
        }
        else if (fade_counter < off2) // 200ms 消灯
        {
            xtouch_neo_pixel_off(0);
        }
        else if (fade_counter < on3) // 100ms点灯
        {
            xtouch_neo_pixel_on(current_red, current_green, current_blue, 0);
        }
        else // 1200ms 消灯
        {
            xtouch_neo_pixel_off(0);
        }
        break;
    }
    
    // ステータス変更フラグをリセット
    print_status_changed = false;
    print_gcode_action_changed = false;
}

#endif