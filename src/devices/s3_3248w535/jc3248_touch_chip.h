#ifndef _JC3248_TOUCH_CHIP_H
#define _JC3248_TOUCH_CHIP_H

#include <Arduino.h>
#include <Wire.h>

#include "setting.h"

namespace {

static const uint8_t AXS_READ_TOUCHPAD[8] = {
    0xb5, 0xab, 0xa5, 0x5a, 0x0, 0x0, 0x0, 0x8,
};

} // namespace

struct JC3248TouchPoint {
    uint16_t x;
    uint16_t y;
    bool touched;
};

class JC3248Touch {
public:
    JC3248Touch() = default;

    bool begin()
    {
        Wire.begin(JC3248_TOUCH_SDA, JC3248_TOUCH_SCL);
        Wire.setClock(400000);
        pinMode(JC3248_TOUCH_INT, INPUT);
        Wire.beginTransmission(JC3248_TOUCH_ADDR);
        return Wire.endTransmission() == 0;
    }

    void setRotation(uint8_t rotation, int16_t logical_w, int16_t logical_h)
    {
        _rotation = rotation;
        _lw = logical_w;
        _lh = logical_h;
    }

    bool read(JC3248TouchPoint &point)
    {
        uint8_t data[8] = {0};

        Wire.beginTransmission(JC3248_TOUCH_ADDR);
        Wire.write(AXS_READ_TOUCHPAD, sizeof(AXS_READ_TOUCHPAD));
        if (Wire.endTransmission() != 0)
        {
            point.touched = false;
            return false;
        }

        delayMicroseconds(50);

        uint8_t n = Wire.requestFrom((int)JC3248_TOUCH_ADDR, (int)8);
        int i = 0;
        while (Wire.available() && i < 8)
        {
            data[i++] = Wire.read();
        }

        if (i < 8 || n < 8)
        {
            point.touched = false;
            return false;
        }

        if (data[0] != 0 || data[1] == 0)
        {
            point.touched = false;
            _last.touched = false;
            return false;
        }

        uint16_t raw_x = (uint16_t)(((data[2] & 0x0F) << 8) | data[3]);
        uint16_t raw_y = (uint16_t)(((data[4] & 0x0F) << 8) | data[5]);
        uint16_t x = raw_x;
        uint16_t y = raw_y;

        switch (_rotation)
        {
        case 1:
            point.x = y;
            point.y = (uint16_t)(JC3248_LCD_NATIVE_W - x - 1);
            break;
        case 2:
            point.x = (uint16_t)(JC3248_LCD_NATIVE_W - x - 1);
            point.y = (uint16_t)(JC3248_LCD_NATIVE_H - y - 1);
            break;
        case 3:
            point.x = (uint16_t)(JC3248_LCD_NATIVE_H - y - 1);
            point.y = x;
            break;
        default:
            point.x = x;
            point.y = y;
            break;
        }

        if (point.x >= (uint16_t)_lw)
        {
            point.x = (uint16_t)(_lw > 0 ? _lw - 1 : 0);
        }
        if (point.y >= (uint16_t)_lh)
        {
            point.y = (uint16_t)(_lh > 0 ? _lh - 1 : 0);
        }

        point.touched = true;
        _last = point;
        return true;
    }

    JC3248TouchPoint getLastTouch() { return _last; }

private:
    JC3248TouchPoint _last{};
    uint8_t _rotation = 0;
    int16_t _lw = JC3248_LCD_NATIVE_W;
    int16_t _lh = JC3248_LCD_NATIVE_H;
};

#endif
