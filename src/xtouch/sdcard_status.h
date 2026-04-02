#ifndef _XLCD_SDCARD_STATUS
#define _XLCD_SDCARD_STATUS

#include <Arduino.h>
#include <SD_MMC.h>

/* SD カードの存在チェックは SD.cardType() が低レイヤ初期化を引き起こしやすいので、
 * 描画/タイマーのたびに呼ばないように「キャッシュして間引く」ためのユーティリティ。
 *
 * - 初回: xtouch_sdcard_setup() 側で mark_present(true) される
 * - SD 抜去後: 呼び出し元が間引き間隔に従って再チェックし、false に落とす
 */
static bool s_sd_present_cached = false;
static uint32_t s_sd_last_check_ms = 0;

static inline void xtouch_sdcard_mark_present(bool present)
{
    s_sd_present_cached = present;
    s_sd_last_check_ms = millis();
}

static inline bool xtouch_sdcard_is_present_cached(uint32_t interval_ms = 1000)
{
    uint32_t now = millis();
    if (now - s_sd_last_check_ms < interval_ms)
        return s_sd_present_cached;
    s_sd_last_check_ms = now;
    s_sd_present_cached = (SD_MMC.cardType() != CARD_NONE);
    return s_sd_present_cached;
}

#endif

