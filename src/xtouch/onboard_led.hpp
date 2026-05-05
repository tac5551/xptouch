#ifndef _XPTOUCH_ONBOARD_LED_HPP
#define _XPTOUCH_ONBOARD_LED_HPP

/**
 * @file onboard_led.hpp
 * @brief オンボード RGB（PWM）のシングルトン。static や C コールバックからは
 *        xptouch_onboard_led::OnboardRgbLed::instance() または xptouch_onboard_led_set 等で呼ぶ。
 */

#include "globals.h"
#include "debug.h"
#include "types.h"

#if defined(__XPTOUCH_SCREEN_28__)
#include "devices/2.8/screen.h"
#elif defined(__XPTOUCH_SCREEN_S3_028__)
#include "devices/s3_2.8/screen.h"
#elif defined(__XPTOUCH_SCREEN_S3_3248__)
#include "devices/s3_3248w535/screen.h"
#elif defined(__XPTOUCH_SCREEN_S3_050__)
#include "devices/5.0/screen.h"
#endif

namespace xptouch_onboard_led
{

class OnboardRgbLed
{
public:
    /** Meyers singleton — static メンバやコールバックから安全に参照できる */
    static OnboardRgbLed &instance()
    {
        static OnboardRgbLed s;
        return s;
    }

    OnboardRgbLed(const OnboardRgbLed &) = delete;
    OnboardRgbLed &operator=(const OnboardRgbLed &) = delete;

    bool ready() const { return initialized_; }
    int gpio_r() const { return gpio_r_; }
    int gpio_g() const { return gpio_g_; }
    int gpio_b() const { return gpio_b_; }

    void init()
    {
        if (initialized_)
            return;

#if defined(__XPTOUCH_SCREEN_28__)
        lgfx::boards::board_t board = tft.getBoard();
        if (board == lgfx::boards::board_t::board_Guition_ESP32_2432W328R ||
            board == lgfx::boards::board_t::board_Guition_ESP32_2432W328C)
        {
            ConsoleDebug.println("LED GPIO 4, 16, 17");
            gpio_r_ = 4;
            gpio_g_ = 16;
            gpio_b_ = 17;
        }
#endif
        ConsoleDebug.println("XPTOUCH_ONBOARD_LED R: " + String(gpio_r_) + " G: " + String(gpio_g_) + " B: " + String(gpio_b_));

        pinMode(gpio_r_, OUTPUT);
        pinMode(gpio_b_, OUTPUT);
        pinMode(gpio_g_, OUTPUT);

        ledcSetup(0, 5000, 8);
        ledcSetup(1, 5000, 8);
        ledcSetup(2, 5000, 8);

        ledcAttachPin(gpio_r_, 0);
        ledcAttachPin(gpio_g_, 1);
        ledcAttachPin(gpio_b_, 2);

        initialized_ = true;
        set(0, 0, 0);
    }

    /** アクティブロー想定で PWM を反転（255 - 値） */
    void set(int red, int green, int blue)
    {
        if (!initialized_)
            return;
        ledcWrite(0, 255 - red);
        ledcWrite(1, 255 - green);
        ledcWrite(2, 255 - blue);
    }

private:
    OnboardRgbLed() = default;

    int gpio_r_ = 0;
    int gpio_g_ = 0;
    int gpio_b_ = 0;
    bool initialized_ = false;
};

} // namespace xptouch_onboard_led

/** 既存呼び出し互換・static からのショートカット */
inline void xptouch_onboard_led_init(void)
{
    xptouch_onboard_led::OnboardRgbLed::instance().init();
}

inline void xptouch_onboard_led_set(int red, int green, int blue)
{
    xptouch_onboard_led::OnboardRgbLed::instance().set(red, green, blue);
}

inline bool xptouch_onboard_led_ready(void)
{
    return xptouch_onboard_led::OnboardRgbLed::instance().ready();
}

#endif // _XPTOUCH_ONBOARD_LED_HPP
