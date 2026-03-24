#ifndef _XLCD_EEPROM
#define _XLCD_EEPROM

#include <string.h>
#include <stdint.h>

#include "xtouch/paths.h"
#include "SPIFFS.h"

/* 5インチ: TFT flip + freq_write + 任意 Bus_RGB タイミング（lcd.json と EEPROM） */
#if defined(__XTOUCH_SCREEN_50__)
#define XTOUCH_SETTING_BODY_DISABLED
#include "devices/5.0/setting.h"
#undef XTOUCH_SETTING_BODY_DISABLED

#define XTOUCH_EEPROM_SIZE 32
#define XTOUCH_EEPROM_POS_TFTFLIP 0
/** little-endian uint32_t。0 または 0xFFFFFFFF は freq 未設定（JC8048_BUS_DEFAULT_FREQ_WRITE 相当） */
#define XTOUCH_EEPROM_POS_RGB_PCLK_HZ 4
/** 0xA5 のとき 9..19 の 11 バイトを Bus_RGB タイミングとして有効 */
#define XTOUCH_EEPROM_POS_LCD_TIMING_MAGIC 8
#define XTOUCH_EEPROM_LCD_TIMING_MAGIC 0xA5u
#define XTOUCH_EEPROM_POS_LCD_TIMING_BASE 9
#define XTOUCH_RGB_PCLK_HZ_DEFAULT JC8048_BUS_DEFAULT_FREQ_WRITE
#define XTOUCH_RGB_PCLK_HZ_MIN 8000000u
#define XTOUCH_RGB_PCLK_HZ_MAX 20000000u
#else
#define XTOUCH_EEPROM_SIZE 12
#define XTOUCH_EEPROM_POS_TFTFLIP 0
#endif

static inline void xtouch_eeprom_read_all(uint8_t *buf)
{
    if (!SPIFFS.exists(xtouch_paths_eeprom))
    {
        memset(buf, 0, XTOUCH_EEPROM_SIZE);
        return;
    }
    File f = SPIFFS.open(xtouch_paths_eeprom, FILE_READ);
    if (!f)
    {
        memset(buf, 0, XTOUCH_EEPROM_SIZE);
        return;
    }
    size_t n = f.read(buf, XTOUCH_EEPROM_SIZE);
    if (n < (size_t)XTOUCH_EEPROM_SIZE)
        memset(buf + n, 0, XTOUCH_EEPROM_SIZE - (int)n);
    f.close();
}

static inline void xtouch_eeprom_write_all(const uint8_t *buf)
{
    File f = SPIFFS.open(xtouch_paths_eeprom, FILE_WRITE);
    if (!f)
        return;
    f.write(buf, XTOUCH_EEPROM_SIZE);
    f.close();
}

void xtouch_eeprom_setup()
{
    SPIFFS.begin(true);
    delay(200);

    if (!SPIFFS.exists(xtouch_paths_eeprom))
    {
        uint8_t buf[XTOUCH_EEPROM_SIZE];
        memset(buf, 0, XTOUCH_EEPROM_SIZE);
        xtouch_eeprom_write_all(buf);
        return;
    }

#if defined(__XTOUCH_SCREEN_50__)
    File f = SPIFFS.open(xtouch_paths_eeprom, FILE_READ);
    if (!f)
        return;
    size_t sz = f.size();
    f.close();
    if (sz < (size_t)XTOUCH_EEPROM_SIZE)
    {
        uint8_t buf[XTOUCH_EEPROM_SIZE];
        memset(buf, 0, XTOUCH_EEPROM_SIZE);
        File fr = SPIFFS.open(xtouch_paths_eeprom, FILE_READ);
        if (fr)
        {
            fr.read(buf, sz);
            fr.close();
        }
        /* 旧レイアウトでは offset 4 の uint32 は未定義のことがある */
        memset(buf + XTOUCH_EEPROM_POS_RGB_PCLK_HZ, 0, 4);
        memset(buf + XTOUCH_EEPROM_POS_LCD_TIMING_MAGIC, 0, (size_t)XTOUCH_EEPROM_SIZE - XTOUCH_EEPROM_POS_LCD_TIMING_MAGIC);
        xtouch_eeprom_write_all(buf);
    }
#endif
}

#if defined(__XTOUCH_SCREEN_50__)
/** 工場デフォルトの 11 バイト（EEPROM 9..19 と同順）を out に書く */
static inline void xtouch_eeprom_lcd_timing_defaults_u8(uint8_t *out11)
{
    out11[0] = (uint8_t)(JC8048_BUS_DEFAULT_HSYNC_POLARITY ? 1u : 0u);
    out11[1] = (uint8_t)JC8048_BUS_DEFAULT_HSYNC_FRONT_PORCH;
    out11[2] = (uint8_t)JC8048_BUS_DEFAULT_HSYNC_PULSE_WIDTH;
    out11[3] = (uint8_t)JC8048_BUS_DEFAULT_HSYNC_BACK_PORCH;
    out11[4] = (uint8_t)(JC8048_BUS_DEFAULT_VSYNC_POLARITY ? 1u : 0u);
    out11[5] = (uint8_t)JC8048_BUS_DEFAULT_VSYNC_FRONT_PORCH;
    out11[6] = (uint8_t)JC8048_BUS_DEFAULT_VSYNC_PULSE_WIDTH;
    out11[7] = (uint8_t)JC8048_BUS_DEFAULT_VSYNC_BACK_PORCH;
    out11[8] = (uint8_t)(JC8048_BUS_DEFAULT_PCLK_ACTIVE_NEG ? 1u : 0u);
    out11[9] = (uint8_t)(JC8048_BUS_DEFAULT_DE_IDLE_HIGH ? 1u : 0u);
    out11[10] = (uint8_t)(JC8048_BUS_DEFAULT_PCLK_IDLE_HIGH ? 1u : 0u);
}

static inline bool xtouch_eeprom_lcd_ext_timing_valid(const uint8_t *buf)
{
    return buf[XTOUCH_EEPROM_POS_LCD_TIMING_MAGIC] == XTOUCH_EEPROM_LCD_TIMING_MAGIC;
}

/** PCLK + タイミング拡張（offset 4..19）のみ 0。TFT 反転(0)と予約(1..3)は維持 */
static inline void xtouch_eeprom_lcd_extension_clear_keep_flip(uint8_t *buf)
{
    memset(buf + XTOUCH_EEPROM_POS_RGB_PCLK_HZ, 0,
           (size_t)((XTOUCH_EEPROM_POS_LCD_TIMING_BASE + 11u) - (unsigned)XTOUCH_EEPROM_POS_RGB_PCLK_HZ));
}
#endif

void xtouch_eeprom_write(int address, byte value)
{
    if (address < 0 || address >= XTOUCH_EEPROM_SIZE)
        return;
    uint8_t buf[XTOUCH_EEPROM_SIZE];
    xtouch_eeprom_read_all(buf);
    buf[address] = (uint8_t)value;
    xtouch_eeprom_write_all(buf);
}

uint8_t xtouch_eeprom_read(int address)
{
    if (address < 0 || address >= XTOUCH_EEPROM_SIZE)
        return 0;
    uint8_t buf[XTOUCH_EEPROM_SIZE];
    xtouch_eeprom_read_all(buf);
    return buf[address];
}

#if defined(__XTOUCH_SCREEN_50__)

static inline uint32_t xtouch_u32_from_le(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static inline void xtouch_u32_to_le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
    p[2] = (uint8_t)((v >> 16) & 0xFF);
    p[3] = (uint8_t)((v >> 24) & 0xFF);
}

/** offset 4 の 4 バイトを little-endian uint32 としてそのまま返す（0/範囲の解釈なし） */
uint32_t xtouch_eeprom_rgb_pclk_raw_u32(void)
{
    uint8_t buf[XTOUCH_EEPROM_SIZE];
    xtouch_eeprom_read_all(buf);
    return xtouch_u32_from_le(buf + XTOUCH_EEPROM_POS_RGB_PCLK_HZ);
}

uint32_t xtouch_eeprom_rgb_pclk_hz_read(void)
{
    uint8_t buf[XTOUCH_EEPROM_SIZE];
    xtouch_eeprom_read_all(buf);
    uint32_t v = xtouch_u32_from_le(buf + XTOUCH_EEPROM_POS_RGB_PCLK_HZ);
    /* 未設定: 0 / 消去値。範囲外・ゴミはクランプせずデフォルト（クランプ MAX だと常に 20MHz に見える） */
    if (v == 0u || v == 0xFFFFFFFFu)
        return XTOUCH_RGB_PCLK_HZ_DEFAULT;
    if (v < XTOUCH_RGB_PCLK_HZ_MIN || v > XTOUCH_RGB_PCLK_HZ_MAX)
        return XTOUCH_RGB_PCLK_HZ_DEFAULT;
    return v;
}

/** true: 保存した（次回起動から有効。即時反映には再初期化が必要） */
bool xtouch_eeprom_rgb_pclk_hz_store(uint32_t hz)
{
    if (hz != 0u && hz != 0xFFFFFFFFu)
    {
        if (hz < XTOUCH_RGB_PCLK_HZ_MIN || hz > XTOUCH_RGB_PCLK_HZ_MAX)
            return false;
    }
    uint8_t buf[XTOUCH_EEPROM_SIZE];
    xtouch_eeprom_read_all(buf);
    xtouch_u32_to_le(buf + XTOUCH_EEPROM_POS_RGB_PCLK_HZ, hz);
    xtouch_eeprom_write_all(buf);
    return true;
}

/**
 * raw が有効な Hz（8M–20M）でも 0 / 0xFFFFFFFF でもないとき、ゴミとみなして 0 クリアする。
 * setup 早期（screen 初期化の前）に 1 回呼ぶ。
 */
void xtouch_eeprom_rgb_pclk_heal_invalid_storage(void)
{
    uint32_t r = xtouch_eeprom_rgb_pclk_raw_u32();
    if (r == 0u || r == 0xFFFFFFFFu)
        return;
    if (r >= XTOUCH_RGB_PCLK_HZ_MIN && r <= XTOUCH_RGB_PCLK_HZ_MAX)
        return;
    xtouch_eeprom_rgb_pclk_hz_store(0u);
    Serial.printf("[EEPROM] RGB PCLK slot invalid raw=0x%08lX — cleared (use default 14 MHz)\n",
                  (unsigned long)r);
}

#endif /* __XTOUCH_SCREEN_50__ */

#endif
