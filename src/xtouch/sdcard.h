#ifndef _XLCD_SDCARD
#define _XLCD_SDCARD

#include "FS.h"
#include <ArduinoJson.h>
#include <Arduino.h>
#include <string.h>
#include "xtouch/sdcard_status.h"
#include "xtouch/filesystem.h"
#include "xtouch/paths.h"
#include "xtouch/debug.h"

#if defined(__XTOUCH_SCREEN_28__)
#include <SPI.h>
#include <SD.h>
#include "../devices/2.8/sd_spi_pins.h"
#elif defined(__XTOUCH_SCREEN_S3_050__)
#include <SPI.h>
#include <SD.h>
#include "../devices/5.0/sd_spi_pins.h"
#else
#include "SD_MMC.h"
#if defined(__XTOUCH_SCREEN_S3_028__)
#include "../devices/s3_2.8/sd_mmc_pins.h"
#elif defined(__XTOUCH_SCREEN_S3_3248__)
#include "../devices/s3_3248w535/sd_mmc_pins.h"
#else
#ifndef XTOUCH_SD_SCK
#define XTOUCH_SD_SCK 38
#endif
#ifndef XTOUCH_SD_CMD
#define XTOUCH_SD_CMD 40
#endif
#ifndef XTOUCH_SD_D0
#define XTOUCH_SD_D0 39
#endif
#ifndef XTOUCH_SD_D1
#define XTOUCH_SD_D1 (-1)
#endif
#ifndef XTOUCH_SD_D2
#define XTOUCH_SD_D2 (-1)
#endif
#ifndef XTOUCH_SD_D3
#define XTOUCH_SD_D3 (-1)
#endif
#endif
#endif

#if defined(__XTOUCH_SCREEN_28__) || defined(__XTOUCH_SCREEN_S3_050__)

static inline fs::FS &xtouch_sdcard_fs()
{
    return SD;
}

static inline bool xtouch_sdcard_exists(const char *path)
{
    return SD.exists(path);
}

static inline File xtouch_sdcard_open(const char *path, const char *mode = "r")
{
    return SD.open(path, mode);
}

static inline bool xtouch_sdcard_remove(const char *path)
{
    return SD.remove(path);
}

static inline uint8_t xtouch_sdcard_card_type()
{
    return SD.cardType();
}

#else

static inline fs::FS &xtouch_sdcard_fs()
{
    return SD_MMC;
}

static inline bool xtouch_sdcard_exists(const char *path)
{
    return SD_MMC.exists(path);
}

static inline File xtouch_sdcard_open(const char *path, const char *mode = "r")
{
    return SD_MMC.open(path, mode);
}

static inline bool xtouch_sdcard_remove(const char *path)
{
    return SD_MMC.remove(path);
}

static inline uint8_t xtouch_sdcard_card_type()
{
    return SD_MMC.cardType();
}

#endif

/** 保存先パスの親ディレクトリを作成。無いと FILE_WRITE で「no permits for creation」になる。 */
static inline bool xtouch_sdcard_ensure_parent_dir(const char *filePath)
{
    if (!filePath)
        return false;
    const char *slash = strrchr(filePath, '/');
    if (!slash || slash <= filePath)
        return true;
    char dirbuf[96];
    size_t n = (size_t)(slash - filePath);
    if (n >= sizeof(dirbuf))
        return false;
    memcpy(dirbuf, filePath, n);
    dirbuf[n] = '\0';
    if (xtouch_filesystem_mkdir(xtouch_sdcard_fs(), dirbuf))
        return true;
    ConsoleVerbose.printf("[xPTouch][V][SD] ensure_parent_dir mkdir failed: %s\n", dirbuf);
    return false;
}

#if defined(__XTOUCH_SCREEN_28__) || defined(__XTOUCH_SCREEN_S3_050__)

/** ESP32 2.8" / S3 5" は SPI。SCK/MISO/MOSI は各 sd_spi_pins、CS は sd_cs_pin（mode1bit は未使用）。 */
bool xtouch_sdcard_setup(int8_t sd_cs_pin, bool mode1bit = true)
{
    (void)mode1bit;
    const uint32_t sd_freq_hz = 10000000;
    int8_t cs = sd_cs_pin;
    if (cs < 0) {
#if defined(__XTOUCH_SCREEN_28__)
        cs = 5;
#else
        cs = 10; /* S3 5" / JC3248W535 暫定 */
#endif
    }
    SPI.begin(XTOUCH_SD_SPI_SCK, XTOUCH_SD_SPI_MISO, XTOUCH_SD_SPI_MOSI, cs);
    bool ok = SD.begin(cs, SPI, sd_freq_hz);
    if (!ok)
    {
        lv_label_set_text(introScreenCaption, LV_SYMBOL_SD_CARD " INSERT SD CARD");
        lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_timer_handler();

        ConsoleError.println("[xPTouch][E][SD] Card Mount Failed");
        xtouch_sdcard_mark_present(false);
        return false;
    }

    lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);

    uint8_t cardType = xtouch_sdcard_card_type();

    if (cardType == CARD_NONE)
    {
        ConsoleError.println("[xPTouch][E][SD] No SD card attached");
        xtouch_sdcard_mark_present(false);
        return false;
    }

    ConsoleInfo.printf("[xPTouch][I][SD] Card Type: %d\n", cardType);

    if (cardType == CARD_MMC)
    {
        ConsoleInfo.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        ConsoleInfo.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        ConsoleInfo.println("SDHC");
    }
    else
    {
        ConsoleInfo.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    ConsoleInfo.printf("[xPTouch][I][SD] Card Size: %lluMB\n", cardSize);
    xtouch_filesystem_mkdir(xtouch_sdcard_fs(), xtouch_paths_root);
    xtouch_filesystem_mkdir(xtouch_sdcard_fs(), "/tmp");

    xtouch_sdcard_mark_present(true);
    return true;
}

#else

/** SD_MMC。SCK/CMD/D0/D1/D2 は sd_mmc_pins。D3 は 4bit かつ sd_cs_pin>=0 なら sd_cs_pin、それ以外は sd_mmc_pins。SD_MMC.begin(..., mode1bit, ...) は main の sd_mode_1bit。 */
bool xtouch_sdcard_setup(int8_t sd_cs_pin, bool mode1bit = true)
{
    const uint32_t sd_freq_hz = 10000000; /* 10MHz */
    int pin_d3 = XTOUCH_SD_D3;
    if (!mode1bit && sd_cs_pin >= 0)
        pin_d3 = (int)sd_cs_pin;
    SD_MMC.setPins(XTOUCH_SD_SCK, XTOUCH_SD_CMD, XTOUCH_SD_D0, XTOUCH_SD_D1, XTOUCH_SD_D2, pin_d3);
    bool ok = SD_MMC.begin("/sdcard", mode1bit, true, sd_freq_hz);
    if (!ok)
    {
        lv_label_set_text(introScreenCaption, LV_SYMBOL_SD_CARD " INSERT SD CARD");
        lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_timer_handler();

        ConsoleError.println("[xPTouch][E][SD] Card Mount Failed");
        xtouch_sdcard_mark_present(false);
        return false;
    }

    lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);

    uint8_t cardType = xtouch_sdcard_card_type();

    if (cardType == CARD_NONE)
    {
        ConsoleError.println("[xPTouch][E][SD] No SD card attached");
        xtouch_sdcard_mark_present(false);
        return false;
    }

    ConsoleInfo.printf("[xPTouch][I][SD] Card Type: %d\n", cardType);

    if (cardType == CARD_MMC)
    {
        ConsoleInfo.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        ConsoleInfo.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        ConsoleInfo.println("SDHC");
    }
    else
    {
        ConsoleInfo.println("UNKNOWN");
    }

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    ConsoleInfo.printf("[xPTouch][I][SD] Card Size: %lluMB\n", cardSize);
    xtouch_filesystem_mkdir(xtouch_sdcard_fs(), xtouch_paths_root);
    xtouch_filesystem_mkdir(xtouch_sdcard_fs(), "/tmp");

    xtouch_sdcard_mark_present(true);
    return true;
}

#endif

#endif
