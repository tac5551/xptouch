#ifndef XTOUCH_M5STACK_H
#define XTOUCH_M5STACK_H

#if defined(__XTOUCH_SCREEN_28__)

#include <Arduino.h>
#include "devices/2.8/screen.h"
#include "ui/ui_msgs.h"
#include "ui/ui_events.h"
#include "xtouch/debug.h"

// M5Stack Basic 物理ボタン: 左=スリープ復帰, 中央=ライト, 右=PLA/OFF交互 (GPIO 39, 38, 37)
#define M5STACK_BTN_LEFT   39
#define M5STACK_BTN_CENTER 38
#define M5STACK_BTN_RIGHT  37
#define M5STACK_BTN_DEBOUNCE_MS 300
#define M5STACK_BTN_LEFT_COOLDOWN_MS 800  // スリープ/復帰の連打防止（GPIO39ノイズ対策）

#define M5STACK_BOARD_ID 2  // LovyanGFX AutoDetect board:2 = M5Stack

static volatile bool m5stack_btn_center = false;
static bool m5stack_buttons_enabled = false;
static uint32_t m5stack_btn_last_left = 0;
static uint32_t m5stack_btn_last_center = 0;
static uint32_t m5stack_btn_last_right = 0;
static bool m5stack_left_was_released = true;   // 左はポーリングで押下エッジのみ検出（ノイズ対策）
static bool m5stack_right_was_released = true;   // 右も同様
static bool m5stack_btn_c_pla_next = true;       // true=次押下でPLA, false=次押下でOFF

void IRAM_ATTR m5stack_btn_center_isr() { m5stack_btn_center = true; }

/** board_id は (int)tft.getBoard()。M5Stack のときだけ有効。 */
static void xtouch_m5stack_buttons_setup(int board_id)
{
    if (board_id != M5STACK_BOARD_ID)
        return;
    pinMode(M5STACK_BTN_LEFT, INPUT_PULLUP);
    pinMode(M5STACK_BTN_CENTER, INPUT_PULLUP);
    pinMode(M5STACK_BTN_RIGHT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(M5STACK_BTN_CENTER), m5stack_btn_center_isr, FALLING);
    m5stack_buttons_enabled = true;
    ConsoleDebug.println("M5Stack physical buttons enabled (L:wake/sleep C:light R:PLA/OFF)");
}

static void xtouch_m5stack_buttons_loop(void)
{
    if (!m5stack_buttons_enabled)
        return;
    uint32_t now = millis();
    // 左ボタン: ポーリングで「離す→押す」のエッジのみ反応（GPIO39ノイズ対策）
    bool left_now = (digitalRead(M5STACK_BTN_LEFT) == LOW);
    if (left_now && m5stack_left_was_released && (now - m5stack_btn_last_left >= M5STACK_BTN_LEFT_COOLDOWN_MS))
    {
        m5stack_btn_last_left = now;
        m5stack_left_was_released = false;
        if (xtouch_screen_touchFromPowerOff)
            xtouch_screen_wakeUp();
        else
            xtouch_screen_sleep();
    }
    if (!left_now)
        m5stack_left_was_released = true;
    if (m5stack_btn_center)
    {
        m5stack_btn_center = false;
        if (now - m5stack_btn_last_center >= M5STACK_BTN_DEBOUNCE_MS)
        {
            m5stack_btn_last_center = now;
            lv_msg_send(XTOUCH_COMMAND_LIGHT_TOGGLE, NULL);
        }
    }
    // 右ボタン: ポーリングでPLA/OFF交互（押下エッジのみ）
    bool right_now = (digitalRead(M5STACK_BTN_RIGHT) == LOW);
    if (right_now && m5stack_right_was_released && (now - m5stack_btn_last_right >= M5STACK_BTN_DEBOUNCE_MS))
    {
        m5stack_btn_last_right = now;
        m5stack_right_was_released = false;
        if (m5stack_btn_c_pla_next)
        {
            onPreHeatPLA(nullptr);
            m5stack_btn_c_pla_next = false;
        }
        else
        {
            onPreHeatOff(nullptr);
            m5stack_btn_c_pla_next = true;
        }
    }
    if (!right_now)
        m5stack_right_was_released = true;
}

#endif /* __XTOUCH_SCREEN_28__ */

#endif /* XTOUCH_M5STACK_H */
