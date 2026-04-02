#ifndef _XLCD_SDCARD
#define _XLCD_SDCARD

#include "FS.h"
#include "SD_MMC.h"
#include <SPI.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <string.h>
#include "xtouch/sdcard_status.h"
#include "xtouch/filesystem.h"
#include "xtouch/paths.h"
#include "xtouch/debug.h"

#define SD_SCK  38
#define SD_CMD 40
#define SD_D0 39
#define SD_D1 41
#define SD_D2 48
#define SD_D3 47

/* SD backend abstraction: migrate call sites to these wrappers */
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

bool xtouch_sdcard_setup(int8_t sd_cs_pin, bool mode1bit = true)
{
    /* SD_MMC 周波数（配線・カード次第で 257 等の読み取りエラーが出る場合はさらに下げる） */
    const uint32_t sd_freq_hz = 10000000; /* 10MHz */
    //bool ok = (sd_cs_pin < 0) ? SD.begin() : SD.begin(sd_cs_pin, SPI, sd_freq_hz);
    SD_MMC.setPins(SD_SCK, SD_CMD, SD_D0, SD_D1, SD_D2, SD_D3);
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