#ifndef _XLCD_NET
#define _XLCD_NET

#include <WiFi.h>
#include "xtouch/debug.h"
#include <string.h>

void onWiFiEvent(arduino_event_id_t event, arduino_event_info_t info)
{
    if (event == ARDUINO_EVENT_WIFI_STA_LOST_IP)
    {
        ConsoleInfo.println("[xPTouch][I][NET] STA LOST IP, reconnecting...");
        WiFi.reconnect();
    }
}
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <MD5Builder.h>
#include "bbl-certs.h"
#include "xtouch/paths.h"
#include "xtouch/sdcard_status.h"
#include "xtouch/sdcard.h"
#ifdef __XTOUCH_PLATFORM_S3__
#include "xtouch/cloud.hpp"
#endif

int downloadFileToSDCard(const char *url, const char *fileName, void (*onProgress)(int) = NULL, void (*onMD5Check)(int) = NULL, const char *otaMD5 = NULL)
{
#ifdef __XTOUCH_PLATFORM_S3__
    /* Cloud API（HttpLockGuard）と同時に別 WiFiClientSecure を張ると mbedTLS の内部ヒープが枯渇し (-32512)、接続失敗する */
    cloud.httpLockExternal();
    struct NetDownloadHttpUnlock
    {
        ~NetDownloadHttpUnlock() { cloud.httpUnlockExternal(); }
    } net_download_http_unlock;
#endif

    WiFiClientSecure wifiClient;
    /* サムネイルなど S3 向け、および OTA ホスト(xtouch_paths_firmware_ota_host)向けは証明書を検証せずに取得する。
     * それ以外は従来通り CA を使う。 */
    bool isThumbnail = (fileName && strstr(fileName, "/tmp/") && strstr(fileName, ".png"));
    bool isOtaHost = (url && strstr(url, xtouch_paths_firmware_ota_host));
    if (isThumbnail || isOtaHost)
    {
        wifiClient.setInsecure();
    }
    else
    {
        wifiClient.setCACert(xperiments_in);
    }
    // WiFiClientSecure のタイムアウトも延長（最大に近い 65 秒）
    // setTimeout は uint16_t なので 65535ms が上限付近
    wifiClient.setTimeout(65000); // 約 65 秒

    HTTPClient http;

    String forceHttpsUrl = url;
    forceHttpsUrl.replace("http://", "https://");

    // Begin the HTTP request
    http.begin(wifiClient, forceHttpsUrl);
    
    // タイムアウトを延長（デフォルト5秒 → 約65秒）
    http.setTimeout(65000); // 約 65 秒（HTTPClient::setTimeout は uint16_t 指定）

    int httpCode = http.GET();

    // Check for successful HTTP request
    bool success = false;
    MD5Builder md5Checker;

    if (httpCode == HTTP_CODE_OK)
    {
        // SD カード未挿入時は書き込みを試みずに終了（Printers 画面アイドル時の書き込みエラー対策）
        if (!xtouch_sdcard_is_present_cached())
        {
            ConsoleError.println("[xPTouch][E][NET] SD card not present, skip downloadFileToSDCard");
            http.end();
            return 0;
        }

        (void)xtouch_sdcard_ensure_parent_dir(fileName);

        bool wrote_ok = false;
        File file = xtouch_sdcard_open(fileName, FILE_WRITE);
        if (file)
        {
            Stream *response = &http.getStream();
            int responseTotalSize = http.getSize();
            int responseSize = 0;
            int lastProgress = 0;

            const int bufferSize = 512;
            uint8_t buffer[bufferSize];
            int bytesRead;
            bool stream_ok = true;

            while ((bytesRead = response->readBytes(buffer, bufferSize)) > 0)
            {
                int progress = (responseTotalSize > 0) ? ((responseSize * 100) / responseTotalSize) : 0;
                if (onProgress && progress != lastProgress)
                {
                    onProgress(progress);
                }

                size_t wr = file.write(buffer, (size_t)bytesRead);
                if (wr != (size_t)bytesRead)
                {
                    ConsoleVerbose.println("[xPTouch][V][NET] SD write short, abort stream");
                    stream_ok = false;
                    break;
                }
                responseSize += bytesRead;
                lastProgress = progress;
            }

            file.close();
            wrote_ok = stream_ok;

            if (!stream_ok)
                xtouch_sdcard_remove(fileName);

            if (wrote_ok && (otaMD5 == NULL || onMD5Check == NULL))
            {
                success = true;
                if (responseTotalSize > 0 && responseSize != responseTotalSize)
                {
                    ConsoleVerbose.printf("[xPTouch][V][NET] download incomplete %d/%d\n", responseSize, responseTotalSize);
                    success = false;
                    xtouch_sdcard_remove(fileName);
                }
                if (success && isThumbnail)
                {
                    File vf = xtouch_sdcard_open(fileName, FILE_READ);
                    if (!vf || vf.size() < 64)
                    {
                        if (vf)
                            vf.close();
                        ConsoleVerbose.println("[xPTouch][V][NET] thumbnail verify failed after write");
                        success = false;
                        xtouch_sdcard_remove(fileName);
                    }
                    else
                    {
                        vf.close();
                    }
                }
            }
        }
        else
        {
            Serial.println("Failed to open file for writing");
        }

        if (otaMD5 != NULL && onMD5Check != NULL)
        {
            onMD5Check(-1);

            File fileMD5 = xtouch_sdcard_open(fileName, FILE_READ);
            if (!fileMD5)
            {
                success = false;
            }
            else
            {
                md5Checker.begin();
                md5Checker.addStream(fileMD5, fileMD5.size());
                md5Checker.calculate();
                fileMD5.close();

                success = strcmp(otaMD5, md5Checker.toString().c_str()) == 0;
            }

            onMD5Check(success);
        }
        else if (!wrote_ok)
        {
            success = false;
        }
    }

    // End the HTTP request
    http.end();

    return success;
}

#ifdef __XTOUCH_PLATFORM_S3__

#include "esp_attr.h"
#include "xtouch/types.h"
#include "xtouch/globals.h"
#include "esp_log.h"

/** 任意の task_id から History / Home 共通の SD パス `/tmp/{sanitized}.png` を生成する（1 箇所に集約）。
 *  旧: History は safe が 23 文字上限、Home は `%.28s` で切り詰めており、長い id で別ファイル扱いになり Home が再 DL していた。 */
inline bool getThumbPathForTaskId(const char *tid, char *buf, size_t len)
{
    if (!buf || len == 0)
        return false;
    buf[0] = '\0';
    if (!tid || !tid[0] || strcmp(tid, "0") == 0)
        return false;
    char safe[32];
    size_t j = 0;
    for (size_t i = 0; tid[i] != '\0' && j < sizeof(safe) - 1; i++)
    {
        char c = tid[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')
            safe[j++] = c;
    }
    safe[j] = '\0';
    if (j == 0)
        return false;
    snprintf(buf, len, "/tmp/%s.png", safe);
    return buf[0] != '\0';
}

/** スロットの task_id をファイル名に使う。task_id が無効な場合は buf に空文字を書き込む（pthumb_N は廃止）。 */
inline void getThumbPathForSlot(int slot, char *buf, size_t len)
{
    if (!buf || len == 0)
        return;
    buf[0] = '\0';
    const char *tid = nullptr;
    if (slot == 0)
        tid = (bambuStatus.task_id[0] && strcmp(bambuStatus.task_id, "0") != 0) ? bambuStatus.task_id : nullptr;
    else if (slot >= 1 && slot - 1 < xtouch_other_printer_count && otherPrinters[slot - 1].valid)
        tid = (otherPrinters[slot - 1].task_id[0] && strcmp(otherPrinters[slot - 1].task_id, "0") != 0) ? otherPrinters[slot - 1].task_id : nullptr;
    if (!tid || !tid[0])
        return;
    (void)getThumbPathForTaskId(tid, buf, len);
}

/** 指定スロットのサムネイル URL と保存 path を取得（メインスレッドで呼ぶ。Cloud 解決で bambuStatus/otherPrinters を更新する）。
 *  @return URL が取得できた場合 true。ワーカーに渡す url_out/path_out を埋める。 */
inline bool getThumbnailUrlAndPathForSlot(int slot, char *url_out, size_t url_size, char *path_out, size_t path_size)
{
    if (!url_out || url_size == 0 || !path_out || path_size == 0)
        return false;
    url_out[0] = path_out[0] = '\0';

    const char *url = nullptr;
#if defined(__XTOUCH_PLATFORM_S3__) && defined(CONFIG_SPIRAM)
    static EXT_RAM_ATTR char resolved_url[1024];
#else
    static char resolved_url[1024];
#endif
    /* Home(0) では同期 Cloud HTTP 解決を行わない（UI 操作優先）。
     * 詳細解決は Printers(6) でのみ許可する。 */
    const bool allow_cloud_resolve = (xTouchConfig.currentScreenIndex == 6);

    if (slot == 0)
    {
        url = bambuStatus.image_url;
        if ((!url || !url[0]) && allow_cloud_resolve && cloud.loggedIn && bambuStatus.task_id[0] && strcmp(bambuStatus.task_id, "0") != 0)
        {
            /* 起動直後などで task_id が古い場合は /my/tasks?limit=1 と突き合わせ、違っていれば諦めてロゴにフォールバックする。 */
            if (!cloud.isCurrentTaskForDevice(bambuStatus.task_id))
            {
                bambuStatus.task_id[0] = '\0';
                bambuStatus.image_url[0] = '\0';
            }
            else if (cloud.getTaskThumbnailUrl(bambuStatus.task_id, resolved_url, sizeof(resolved_url)))
            {
                strncpy(bambuStatus.image_url, resolved_url, sizeof(bambuStatus.image_url) - 1);
                bambuStatus.image_url[sizeof(bambuStatus.image_url) - 1] = '\0';
                url = bambuStatus.image_url;
            }
            else
            {
                bambuStatus.task_id[0] = '\0';
                bambuStatus.image_url[0] = '\0';
            }
        }
        if (!url || !url[0])
            return false;
    }
    else
    {
        int idx = slot - 1;
        if (idx < 0 || idx >= xtouch_other_printer_count || !otherPrinters[idx].valid)
            return false;
        url = otherPrinters[idx].image_url;
        if ((!url || !url[0]) && allow_cloud_resolve && cloud.loggedIn && otherPrinters[idx].task_id[0] && strcmp(otherPrinters[idx].task_id, "0") != 0)
        {
            if (!cloud.isCurrentTaskForDevice(otherPrinters[idx].task_id))
            {
                otherPrinters[idx].task_id[0] = '\0';
                otherPrinters[idx].image_url[0] = '\0';
            }
            else if (cloud.getTaskThumbnailUrl(otherPrinters[idx].task_id, resolved_url, sizeof(resolved_url)))
            {
                strncpy(otherPrinters[idx].image_url, resolved_url, sizeof(otherPrinters[idx].image_url) - 1);
                otherPrinters[idx].image_url[sizeof(otherPrinters[idx].image_url) - 1] = '\0';
                url = otherPrinters[idx].image_url;
            }
            else
            {
                otherPrinters[idx].task_id[0] = '\0';
                otherPrinters[idx].image_url[0] = '\0';
            }
        }
        if (!url || !url[0])
            return false;
    }
    getThumbPathForSlot(slot, path_out, path_size);
    if (!path_out[0])
        return false;
    strncpy(url_out, url, url_size - 1);
    url_out[url_size - 1] = '\0';
    return true;
}

/** 指定スロット(0=メイン, 1..4=他機)のサムネイルを URL から取得し SD に保存。ファイル名は TaskID（既存なら DL しない）。
 *  @return ファイルが既にある/保存できた場合 true、未DL・失敗時は false */
inline bool downloadThumbnailForSlot(int slot)
{
    static const char *TAG = "thumbnail";
    char url_buf[1024];
    char path[64];
    if (!getThumbnailUrlAndPathForSlot(slot, url_buf, sizeof(url_buf), path, sizeof(path)))
        return false;
    if (xtouch_sdcard_exists(path))
    {
        /* 既に同じ TaskID のファイルがあれば DL しない。UI 用 path を設定 */
        if (slot >= 0 && slot < XTOUCH_THUMB_SLOT_MAX)
            snprintf(xtouch_thumbnail_slot_path[slot], XTOUCH_THUMB_PATH_LEN, "S:%s", path);
        return true;
    }

    int ok = downloadFileToSDCard(url_buf, path);
#ifdef XTOUCH_DEBUG_DETAIL
    if (ok)
    {
        ConsoleVerbose.printf("[xPTouch][V][NET] slot=%d download success\n", slot);
    }
    else
    {
        ConsoleError.printf("[xPTouch][E][NET] slot=%d download FAILED\n", slot);
    }
#endif
    if (!ok)
    {
        /* ダウンロード失敗時も再試行しないよう、このスロットの URL / task_id をクリアする */
        if (slot == 0)
        {
            bambuStatus.image_url[0] = '\0';
            bambuStatus.task_id[0] = '\0';
        }
        else
        {
            int idx = slot - 1;
            if (idx >= 0 && idx < xtouch_other_printer_count)
            {
                otherPrinters[idx].image_url[0] = '\0';
                otherPrinters[idx].task_id[0] = '\0';
            }
        }
        return false;
    }
    /* 保存成功。UI 用 path を設定（存在しないファイルを LVGL に渡さない） */
    if (slot >= 0 && slot < XTOUCH_THUMB_SLOT_MAX)
        snprintf(xtouch_thumbnail_slot_path[slot], XTOUCH_THUMB_PATH_LEN, "S:%s", path);
    return true;
}

#endif

#endif