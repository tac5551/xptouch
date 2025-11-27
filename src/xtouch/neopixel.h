#ifndef _XLCD_NEOPIXEL_CONTROL
#define _XLCD_NEOPIXEL_CONTROL

#include "globals.h"
#include "debug.h"
#include "types.h"

#define PIXEL_COUNT 50
int NeoPixelCount = PIXEL_COUNT;

#include <Adafruit_NeoPixel.h>
#if defined(__XTOUCH_SCREEN_28__)
#include "devices/2.8/screen.h"
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, 27, NEO_GRB + NEO_KHZ800);
#elif defined(__XTOUCH_SCREEN_50__)
#include "devices/5.0/screen.h"
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, 17, NEO_GRB + NEO_KHZ800);
#endif

void xtouch_neo_pixel_timer_init(int pin);
void xtouch_neo_pixel_init(int pin);
void xtouch_neo_pixel_on(int iRed, int iGreen, int iBlue);
void xtouch_neo_pixel_off();
void xtouch_neo_pixel_control_timer_create();
void xtouch_neo_pixel_control_timer_start(int pin);
void xtouch_neo_pixel_control_timer_stop();
void xtouch_neo_pixel_set_led_color(int red, int green, int blue);
void xtouch_neo_pixel_set_pettern(int pattern);
void xtouch_neo_pixel_set_status_timeout(int timeout);
void xtouch_neo_pixel_control_timer_handler(lv_timer_t *timer);
void xtouch_neo_pixel_set_brightness(int brightness);
void xtouch_neo_pixel_set_num(int num);
void xtouch_neo_pixel_reset_all();
void xtouch_neo_pixel_set_idle_led_enabled(bool enabled);

lv_timer_t *xtouch_neo_pixel_control_timer;
bool xtouch_neo_pixel_control_started = false;
int last_print_status = 0;
bool print_status_changed = false;
int last_print_gcode_action = 0;
bool print_gcode_action_changed = false;

int status_timeout = -1; // -1:無期限 x:x秒でステータスにかかわらずIDLEに戻す
int alarm_timeout = -1; // Alarm Timeout : from setting value
bool idle_led_enabled = true; // Idle LED Enabled : from setting value

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

void xtouch_neo_pixel_control_timer_start(int pin)
{
    if (!xtouch_neo_pixel_control_started)
    {
        xtouch_neo_pixel_init(pin);
        xtouch_neo_pixel_control_started = true;
    }
    xtouch_neo_pixel_control_timer_create();
}

void xtouch_neo_pixel_control_timer_stop()
{
    lv_timer_pause(xtouch_neo_pixel_control_timer);
}

void xtouch_neo_pixel_timer_init(int pin)
{
    if (xTouchConfig.xTouchNeoPixelNumValue > 0)
    {
        xtouch_neo_pixel_control_timer_start(pin);
    }
    else
    {
        if (xtouch_neo_pixel_control_started)
        {
            xtouch_neo_pixel_control_timer_stop();
        }
        // LED個数が0の場合は消灯
        xtouch_neo_pixel_reset_all();
    }
}

void xtouch_neo_pixel_init(int pin)
{
    strip.setPin(pin);
    strip.begin();

}

void xtouch_neo_pixel_off()
{
    xtouch_neo_pixel_on(0, 0, 0);
}

void xtouch_neo_pixel_on(int iRed, int iGreen, int iBlue)
{
    // ひとつづつつけていく
    for (int j = 0; j < NeoPixelCount; j++)
    {
        strip.setPixelColor(j, strip.Color(iRed, iGreen, iBlue));
        strip.show();
    }
}


void xtouch_neo_pixel_set_led_color(int red, int green, int blue)
{
    led_color[0] = red;
    led_color[1] = green;
    led_color[2] = blue;
}

void xtouch_neo_pixel_set_num(int num)
{
    int backup = NeoPixelCount;
    NeoPixelCount = num;
    
    // LED個数が0に設定された場合は消灯
    if (num == 0)
    {
        xtouch_neo_pixel_reset_all();
        if (xtouch_neo_pixel_control_started)
        {
            xtouch_neo_pixel_control_timer_stop();
        }
    }
    
    // LED個数が変更されたとき、flowing_positionをリセット
    if (backup != num)
    {
        // flowing_positionを0にリセット（速度を一定に保つため）
        flowing_position = 0;
        // アニメーション状態もリセット
        animation_frame = 0;
        animation_direction = 1;
        blink_state = false;
        blink_counter = 0;
        fade_counter = 0;
        fade_direction = 1;
    }
}

void xtouch_neo_pixel_reset_all()
{
    for (int i = 0; i < PIXEL_COUNT; i++)
    {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.show();
}
void xtouch_neo_pixel_set_brightness(int brightness)
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

void xtouch_neo_pixel_set_alarm_timeout(int timeout)
{
    alarm_timeout = timeout;
}

void xtouch_neo_pixel_set_idle_led_enabled(bool enabled)
{
    idle_led_enabled = enabled;
}

void xtouch_neo_pixel_control_timer_handler(lv_timer_t *timer)
{
    // status_timeoutのカウントダウン処理
    if (status_timeout > 0)
    {
        status_timeout -= timer_tick * 10; // main delay 10ms
        if (status_timeout <= 0)
        {
            ConsoleDebug.println("[xPTouch][LED] print_status : " + String(bambuStatus.print_status) + " , print_gcode_action : " + String(bambuStatus.print_gcode_action) + "percent : " + String(bambuStatus.mc_print_percent));
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
            xtouch_neo_pixel_set_led_color(128, 40, 0); // オレンジ色
            xtouch_neo_pixel_set_pettern(4);
        }
        else if (bambuStatus.print_status == XTOUCH_PRINT_STATUS_RUNNING)
        {
            if (bambuStatus.print_gcode_action == 0) // 0	turn off light and wait extrude temperature
            {
                xtouch_neo_pixel_set_led_color(0, 0, 128); // 青色
                xtouch_neo_pixel_set_pettern(4);           // 進捗表示パターン
            }
            else if (bambuStatus.print_gcode_action == 1) // 1	bed leveling
            {
                xtouch_neo_pixel_set_led_color(128, 55, 0); // オレンジ色
                xtouch_neo_pixel_set_pettern(4);
            }
            else if (bambuStatus.print_gcode_action == 2) // 2	heatbet preheat
            {
                xtouch_neo_pixel_set_led_color(128, 30, 0); // オレンジ色
                xtouch_neo_pixel_set_pettern(3);
            }
            else if (bambuStatus.print_gcode_action == 8) // 8	draw extrinsic para cali pain
            {
                xtouch_neo_pixel_set_led_color(128, 65, 0); // オレンジ色
                xtouch_neo_pixel_set_pettern(4);
            }
            else if (bambuStatus.print_gcode_action == 13) // 13	home after wipe mouth
            {
                xtouch_neo_pixel_set_led_color(128, 70, 0); // オレンジ色
                xtouch_neo_pixel_set_pettern(4);
            }
            else if (bambuStatus.print_gcode_action == 14) // 14	nozzle wipe
            {
                xtouch_neo_pixel_set_led_color(128, 75, 0); // オレンジ色
                xtouch_neo_pixel_set_pettern(4);
            }
            else if (bambuStatus.print_gcode_action == 255) // 255	unknown
            {
                xtouch_neo_pixel_set_led_color(0, 80, 128); // シアン（水色）
                xtouch_neo_pixel_set_pettern(3);
            }
            else
            {
                xtouch_neo_pixel_set_led_color(0, 0, 0); // オフ
            }
        }
        else if (bambuStatus.print_status == XTOUCH_PRINT_STATUS_PAUSED)
        {
            xtouch_neo_pixel_set_led_color(64, 64, 128); // 白色
            xtouch_neo_pixel_set_pettern(1);
        }
        else if (bambuStatus.print_status == XTOUCH_PRINT_STATUS_FINISHED)
        {
            xtouch_neo_pixel_set_led_color(0, 128, 0);
            xtouch_neo_pixel_set_pettern(1);
            if(alarm_timeout > 0){
                xtouch_neo_pixel_set_status_timeout(alarm_timeout * 30 * 1000);
            }
            else{
                xtouch_neo_pixel_set_status_timeout(-1);
            }
        }
        else if (bambuStatus.print_status == XTOUCH_PRINT_STATUS_FAILED)
        {
            xtouch_neo_pixel_set_led_color(64, 0, 0); // 赤色の点滅
            xtouch_neo_pixel_set_pettern(6);
            if(alarm_timeout > 0){
                xtouch_neo_pixel_set_status_timeout(alarm_timeout * 30 * 1000);
            }
            else{
                xtouch_neo_pixel_set_status_timeout(-1);
            }
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
        ConsoleDebug.println("[xPTouch][LED] print_status : " + String(bambuStatus.print_status) + " , print_gcode_action : " + String(bambuStatus.print_gcode_action) + " , percent : " + String(bambuStatus.mc_print_percent));
        ConsoleDebug.println("[xPTouch][LED] pettern : " + String(pettern) + " , led_color : " + String(led_color[0]) + " , " + String(led_color[1]) + " , " + String(led_color[2]) + " status_timeout : " + String(status_timeout));
    }

    // タイムアウト時は無条件でIDLE状態に設定
    if (status_timeout == 0)
    {
        ConsoleDebug.println("[xPTouch][LED] status_timeout : " + String(status_timeout));
        xtouch_neo_pixel_set_led_color(32, 0, 0);
        xtouch_neo_pixel_set_pettern(5);
        print_status_changed = true; // ステータス変更状態とする
        status_timeout = -1;         // タイムアウトを無制限に。
    }

    // Idle LEDが無効の場合はIDLE状態でもLEDを消灯
    if (!idle_led_enabled && pettern == 5)
    {
        xtouch_neo_pixel_set_led_color(0, 0, 0);
        xtouch_neo_pixel_set_pettern(0);
        print_status_changed = true;
    // Idle LEDが有効の場合はIDLE状態に戻す
    }else if (idle_led_enabled && pettern == 0 && led_color[0] == 0 && led_color[1] == 0 && led_color[2] == 0){
        xtouch_neo_pixel_set_led_color(32, 0, 0);
        xtouch_neo_pixel_set_pettern(5);
        print_status_changed = true;
    }

    if (!neopixel_enabled)
    {
        xtouch_neo_pixel_set_led_color(0, 0, 0);
        xtouch_neo_pixel_set_pettern(0);
        print_status_changed = true;
    }

    // ステータスが変わっていない場合のみアニメーション処理を実行
    // パターンに応じたアニメーション処理
    int current_red = led_color[0];
    int current_green = led_color[1];
    int current_blue = led_color[2];
    int fade_progress = 0;       // 変数宣言をswitch文の外に移動
    int brightness_range = 0;    // 変数宣言をswitch文の外に移動
    int current_brightness = 0;  // 変数宣言をswitch文の外に移動
    int progress_leds = 0;       // 進捗表示用変数をswitch文の外に移動
    float smooth_position = 0.0; // 流れる光用変数をswitch文の外に移動
    int progress_percent = 0;    // 進捗率用変数をswitch文の外に移動
    int fade_interval = 1000;

    // Setting の明るさが0の場合は無条件でOFF
    if (xTouchConfig.xTouchNeoPixelBlightnessValue <= 5)
    {
        xtouch_neo_pixel_off();
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
            xtouch_neo_pixel_on(current_red, current_green, current_blue);
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
        xtouch_neo_pixel_on(current_red, current_green, current_blue);
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
        xtouch_neo_pixel_on(current_red, current_green, current_blue);
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
            if (brightness < blightness_min)
                brightness = blightness_min;
            if (brightness > blightness_max)
                brightness = blightness_max;

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
        if (progress_percent > 0 && progress_percent < 100)
        {
            progress_percent = ((progress_percent + 9) / 10) * 10; // 10%ごとに切り上げ
        }
        if (progress_percent > 0 && progress_percent < 10)
        {
            progress_percent = 10; // 最低10%として1個点灯
        }

        // 進捗率に基づいてLEDの個数を計算
        progress_leds = (progress_percent * NeoPixelCount) / 100;
        if (progress_leds > NeoPixelCount)
            progress_leds = NeoPixelCount;
        if (progress_leds < 1 && progress_percent > 0)
            progress_leds = 1; // 最低1個は点灯

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
            if (brightness < blightness_min)
                brightness = blightness_min;
            if (brightness > blightness_max)
                brightness = blightness_max;

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
            if (brightness < blightness_min)
                brightness = blightness_min;
            if (brightness > blightness_max)
                brightness = blightness_max;

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
            xtouch_neo_pixel_on(current_red, current_green, current_blue);
        }
        else if (fade_counter < off1) // 200ms 消灯
        {
            xtouch_neo_pixel_off();
        }
        else if (fade_counter < on2) // 100ms点灯
        {
            xtouch_neo_pixel_on(current_red, current_green, current_blue);
        }
        else if (fade_counter < off2) // 200ms 消灯
        {
            xtouch_neo_pixel_off();
        }
        else if (fade_counter < on3) // 100ms点灯
        {
            xtouch_neo_pixel_on(current_red, current_green, current_blue);
        }
        else // 1200ms 消灯
        {
            xtouch_neo_pixel_off();
        }
        break;
    }

    // ステータス変更フラグをリセット
    print_status_changed = false;
    print_gcode_action_changed = false;
}

#endif