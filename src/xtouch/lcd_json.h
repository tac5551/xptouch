#ifndef XTOUCH_LCD_JSON_H
#define XTOUCH_LCD_JSON_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <string.h>

#include "xtouch/paths.h"
#include "xtouch/filesystem.h"
#include "xtouch/eeprom.h"

#if defined(__XTOUCH_SCREEN_50__)

#define XTOUCH_LCD_JSON_DOC_CAP 768

/** 書き込み直後に EEPROM を再読みし、written と一致するか。不一致時は差分を 1 バイトずつ出力 */
static bool lcd_json_eeprom_verify_reread(const uint8_t *written, uint8_t *reread_out)
{
    xtouch_eeprom_read_all(reread_out);
    if (memcmp(written, reread_out, XTOUCH_EEPROM_SIZE) == 0)
    {
        Serial.println("[lcd.json] verify: reread == written (all bytes OK)");
        return true;
    }
    Serial.println("[lcd.json] verify: FAIL reread != written");
    for (size_t i = 0; i < XTOUCH_EEPROM_SIZE; i++)
    {
        if (written[i] != reread_out[i])
            Serial.printf("[lcd.json] verify:   offset %u  written=0x%02X  reread=0x%02X\n", (unsigned)i,
                          (unsigned)written[i], (unsigned)reread_out[i]);
    }
    return false;
}

static uint32_t lcd_json_eff_hz_from_raw(uint32_t raw)
{
    if (raw == 0u || raw == 0xFFFFFFFFu)
        return XTOUCH_RGB_PCLK_HZ_DEFAULT;
    if (raw < XTOUCH_RGB_PCLK_HZ_MIN || raw > XTOUCH_RGB_PCLK_HZ_MAX)
        return XTOUCH_RGB_PCLK_HZ_DEFAULT;
    return raw;
}

/** EEPROM 内容を JSON キー名に対応させて 1 項目ずつ Serial 出力 */
static void lcd_json_serial_dump_eeprom_fields(const uint8_t *v)
{
    Serial.println("[lcd.json] verify: --- EEPROM dump (per field) ---");
    Serial.printf("[lcd.json] verify: tft_flip (byte 0) = %u\n", (unsigned)v[XTOUCH_EEPROM_POS_TFTFLIP]);
    Serial.printf("[lcd.json] verify: reserved byte[1] = 0x%02X\n", (unsigned)v[1]);
    Serial.printf("[lcd.json] verify: reserved byte[2] = 0x%02X\n", (unsigned)v[2]);
    Serial.printf("[lcd.json] verify: reserved byte[3] = 0x%02X\n", (unsigned)v[3]);

    uint32_t raw = xtouch_u32_from_le(v + XTOUCH_EEPROM_POS_RGB_PCLK_HZ);
    uint32_t eff = lcd_json_eff_hz_from_raw(raw);
    Serial.printf("[lcd.json] verify: freq_write raw (bytes 4..7 LE) = 0x%08lX\n", (unsigned long)raw);
    Serial.printf("[lcd.json] verify: freq_write effective Hz (boot interpretation) = %lu\n", (unsigned long)eff);

    uint8_t magic = v[XTOUCH_EEPROM_POS_LCD_TIMING_MAGIC];
    bool timing_on = (magic == XTOUCH_EEPROM_LCD_TIMING_MAGIC);
    Serial.printf("[lcd.json] verify: lcd_timing_magic (byte 8) = 0x%02X (%s)\n", (unsigned)magic,
                  timing_on ? "timing block active" : "timing inactive, defaults at boot");

    const uint8_t *t = v + XTOUCH_EEPROM_POS_LCD_TIMING_BASE;
    Serial.printf("[lcd.json] verify: hsync_polarity = %u\n", (unsigned)t[0]);
    Serial.printf("[lcd.json] verify: hsync_front_porch = %u\n", (unsigned)t[1]);
    Serial.printf("[lcd.json] verify: hsync_pulse_width = %u\n", (unsigned)t[2]);
    Serial.printf("[lcd.json] verify: hsync_back_porch = %u\n", (unsigned)t[3]);
    Serial.printf("[lcd.json] verify: vsync_polarity = %u\n", (unsigned)t[4]);
    Serial.printf("[lcd.json] verify: vsync_front_porch = %u\n", (unsigned)t[5]);
    Serial.printf("[lcd.json] verify: vsync_pulse_width = %u\n", (unsigned)t[6]);
    Serial.printf("[lcd.json] verify: vsync_back_porch = %u\n", (unsigned)t[7]);
    Serial.printf("[lcd.json] verify: pclk_active_neg = %u\n", (unsigned)t[8]);
    Serial.printf("[lcd.json] verify: de_idle_high = %u\n", (unsigned)t[9]);
    Serial.printf("[lcd.json] verify: pclk_idle_high = %u\n", (unsigned)t[10]);

    if (XTOUCH_EEPROM_SIZE > 20)
    {
        Serial.print("[lcd.json] verify: tail bytes[20..");
        Serial.print(XTOUCH_EEPROM_SIZE - 1);
        Serial.print("] =");
        for (size_t i = 20; i < XTOUCH_EEPROM_SIZE; i++)
            Serial.printf(" %02X", (unsigned)v[i]);
        Serial.println();
    }
    Serial.println("[lcd.json] verify: --- end dump ---");
}

static bool lcd_json_apply_bool(JsonObject o, const char *key, uint8_t *t, unsigned idx)
{
    if (!o.containsKey(key))
        return true;
    JsonVariant v = o[key];
    if (v.is<bool>())
        t[idx] = v.as<bool>() ? 1u : 0u;
    else
    {
        int x = v.as<int>();
        if (x != 0 && x != 1)
            return false;
        t[idx] = (uint8_t)x;
    }
    return true;
}

static bool lcd_json_apply_u8(JsonObject o, const char *key, uint8_t *t, unsigned idx, int lo, int hi)
{
    if (!o.containsKey(key))
        return true;
    int x = o[key].as<int>();
    if (x < lo || x > hi)
        return false;
    t[idx] = (uint8_t)x;
    return true;
}

/**
 * SD ルート /lcd.json … Bus_RGB::config と同名キー（freq_write ほか）。
 * 互換: rgb_pclk_hz / pclk_hz → freq_write
 * disable: 1（または true）… LCD 拡張領域のみクリア（offset 4..19: PCLK+タイミング）。TFT 反転(0)は維持（他キーより優先）。
 * タイミングを 1 つでも指定したら EEPROM 拡張ブロック有効（magic）。freq のみなら周波数だけ更新。
 */
void xtouch_lcd_json_apply_from_sd_and_reboot(void)
{
    if (!xtouch_filesystem_exist(SD, xtouch_paths_lcd_json))
        return;

    DynamicJsonDocument doc = xtouch_filesystem_readJson(SD, xtouch_paths_lcd_json, false, XTOUCH_LCD_JSON_DOC_CAP);
    JsonObject o = doc.as<JsonObject>();
    if (o.isNull())
    {
        Serial.println("[lcd.json] root must be object — kept");
        return;
    }

    if (o.containsKey("disable"))
    {
        JsonVariant dv = o["disable"];
        bool wipe = dv.is<bool>() ? dv.as<bool>() : (dv.as<int>() != 0);
        if (wipe)
        {
            uint8_t buf[XTOUCH_EEPROM_SIZE];
            xtouch_eeprom_read_all(buf);
            xtouch_eeprom_lcd_extension_clear_keep_flip(buf);
            xtouch_eeprom_write_all(buf);
            uint8_t verify_dis[XTOUCH_EEPROM_SIZE];
            if (!lcd_json_eeprom_verify_reread(buf, verify_dis))
            {
                Serial.println("[lcd.json] disable: verify failed — kept lcd.json, no reboot");
                return;
            }
            lcd_json_serial_dump_eeprom_fields(verify_dis);
            if (!xtouch_filesystem_deleteFile(SD, xtouch_paths_lcd_json))
            {
                Serial.println("[lcd.json] LCD EEPROM area cleared but delete failed — not rebooting");
                return;
            }
            Serial.println("[lcd.json] disable: LCD ext EEPROM (4..19) cleared, TFT flip kept, rebooting...");
            delay(800);
            ESP.restart();
            return;
        }
    }

    const bool has_freq = o.containsKey("freq_write") || o.containsKey("rgb_pclk_hz") || o.containsKey("pclk_hz");
    const bool has_timing =
        o.containsKey("hsync_polarity") || o.containsKey("hsync_front_porch") || o.containsKey("hsync_pulse_width") ||
        o.containsKey("hsync_back_porch") || o.containsKey("vsync_polarity") || o.containsKey("vsync_front_porch") ||
        o.containsKey("vsync_pulse_width") || o.containsKey("vsync_back_porch") || o.containsKey("pclk_active_neg") ||
        o.containsKey("de_idle_high") || o.containsKey("pclk_idle_high");

    if (!has_freq && !has_timing)
    {
        Serial.println("[lcd.json] no known keys — use freq_write / hsync_* / vsync_* / pclk_* / de_idle_high");
        return;
    }

    uint8_t buf[XTOUCH_EEPROM_SIZE];
    xtouch_eeprom_read_all(buf);

    if (has_freq)
    {
        int64_t v = -1;
        if (o.containsKey("freq_write"))
            v = o["freq_write"].as<int64_t>();
        else if (o.containsKey("rgb_pclk_hz"))
            v = o["rgb_pclk_hz"].as<int64_t>();
        else
            v = o["pclk_hz"].as<int64_t>();

        uint32_t hz = (uint32_t)v;
        if (hz != 0u && hz != 0xFFFFFFFFu)
        {
            if (hz < XTOUCH_RGB_PCLK_HZ_MIN || hz > XTOUCH_RGB_PCLK_HZ_MAX)
            {
                Serial.printf("[lcd.json] invalid freq_write %lu — kept file\n", (unsigned long)hz);
                return;
            }
        }
        xtouch_u32_to_le(buf + XTOUCH_EEPROM_POS_RGB_PCLK_HZ, hz);
    }

    if (has_timing)
    {
        uint8_t t[11];
        if (xtouch_eeprom_lcd_ext_timing_valid(buf))
            memcpy(t, buf + XTOUCH_EEPROM_POS_LCD_TIMING_BASE, 11);
        else
            xtouch_eeprom_lcd_timing_defaults_u8(t);

        if (!lcd_json_apply_bool(o, "hsync_polarity", t, 0) ||
            !lcd_json_apply_u8(o, "hsync_front_porch", t, 1, 0, 127) ||
            !lcd_json_apply_u8(o, "hsync_pulse_width", t, 2, 0, 127) ||
            !lcd_json_apply_u8(o, "hsync_back_porch", t, 3, 0, 127) ||
            !lcd_json_apply_bool(o, "vsync_polarity", t, 4) ||
            !lcd_json_apply_u8(o, "vsync_front_porch", t, 5, 0, 127) ||
            !lcd_json_apply_u8(o, "vsync_pulse_width", t, 6, 0, 127) ||
            !lcd_json_apply_u8(o, "vsync_back_porch", t, 7, 0, 127) ||
            !lcd_json_apply_bool(o, "pclk_active_neg", t, 8) ||
            !lcd_json_apply_bool(o, "de_idle_high", t, 9) ||
            !lcd_json_apply_bool(o, "pclk_idle_high", t, 10))
        {
            Serial.println("[lcd.json] timing value out of range (bool 0/1, porches 0–127) — kept file");
            return;
        }

        buf[XTOUCH_EEPROM_POS_LCD_TIMING_MAGIC] = XTOUCH_EEPROM_LCD_TIMING_MAGIC;
        memcpy(buf + XTOUCH_EEPROM_POS_LCD_TIMING_BASE, t, 11);
    }

    xtouch_eeprom_write_all(buf);

    uint8_t verify[XTOUCH_EEPROM_SIZE];
    if (!lcd_json_eeprom_verify_reread(buf, verify))
    {
        Serial.println("[lcd.json] verify failed — kept lcd.json, no reboot");
        return;
    }
    lcd_json_serial_dump_eeprom_fields(verify);

    if (!xtouch_filesystem_deleteFile(SD, xtouch_paths_lcd_json))
    {
        Serial.println("[lcd.json] EEPROM ok but delete failed — not rebooting");
        return;
    }

    Serial.println("[lcd.json] EEPROM updated, rebooting...");
    delay(800);
    ESP.restart();
}

#else

static inline void xtouch_lcd_json_apply_from_sd_and_reboot(void) {}

#endif

#endif
