#ifndef _XLCD_NET
#define _XLCD_NET

#include <WiFi.h>
#include "xtouch/debug.h"
#include <string.h>

void onWiFiEvent(arduino_event_id_t event, arduino_event_info_t info)
{
    if (event == ARDUINO_EVENT_WIFI_STA_LOST_IP)
    {
        ConsoleInfo.println(F("[xPTouch][WiFi] STA LOST IP, reconnecting..."));
        WiFi.reconnect();
    }
}
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <MD5Builder.h>
#include "bbl-certs.h"

int downloadFileToSDCard(const char *url, const char *fileName, void (*onProgress)(int) = NULL, void (*onMD5Check)(int) = NULL, const char *otaMD5 = NULL)
{

    WiFiClientSecure wifiClient;
    /* サムネイルなど S3 向けは証明書を検証せずに取得する。それ以外は従来通り CA を使う。 */
    if (fileName && strstr(fileName, "/tmp/") && strstr(fileName, ".png"))
    {
        wifiClient.setInsecure();
    }
    else
    {
        wifiClient.setCACert(xperiments_in);
    }
    // WiFiClientSecureのタイムアウトも延長
    wifiClient.setTimeout(60000); // 60秒

    HTTPClient http;

    String forceHttpsUrl = url;
    forceHttpsUrl.replace("http://", "https://");

    // Begin the HTTP request
    http.begin(wifiClient, forceHttpsUrl);
    
    // タイムアウトを延長（デフォルト5秒 → 60秒）
    http.setTimeout(60000); // 60秒

    int httpCode = http.GET();

    // Check for successful HTTP request
    bool success = false;
    MD5Builder md5Checker;

    if (httpCode == HTTP_CODE_OK)
    {
        File file = SD.open(fileName, FILE_WRITE);
        if (file)
        {
            Stream *response = &http.getStream();
            int responseTotalSize = http.getSize();
            int responseSize = 0;
            int lastProgress = 0;

            // Define a buffer size (e.g., 512 bytes)
            const int bufferSize = 512;
            uint8_t buffer[bufferSize];
            int bytesRead;

            while ((bytesRead = response->readBytes(buffer, bufferSize)) > 0)
            {
                int progress = (responseSize * 100) / responseTotalSize;
                if (onProgress && progress != lastProgress)
                {
                    onProgress(progress);
                }

                file.write(buffer, bytesRead);
                responseSize += bytesRead;
                lastProgress = progress;
            }

            file.close();
            success = true;
        }
        else
        {
            Serial.println("Failed to open file for writing");
        }

        if (otaMD5 != NULL && onMD5Check != NULL)
        {
            onMD5Check(-1);

            File fileMD5 = SD.open(fileName, FILE_READ);
            md5Checker.begin();
            md5Checker.addStream(fileMD5, fileMD5.size());
            md5Checker.calculate();
            fileMD5.close();

            success = strcmp(otaMD5, md5Checker.toString().c_str()) == 0;

            onMD5Check(success);
        }
        else
        {
            success = true;
        }
    }

    // End the HTTP request
    http.end();

    return success;
}

#ifdef __XTOUCH_SCREEN_50__

#include "esp_attr.h"
#include "xtouch/types.h"
#include "xtouch/globals.h"
#include "esp_log.h"
#include "xtouch/cloud.hpp"
#include <SD.h>

/** スロットの task_id をファイル名に使う。無効なら pthumb_N.png。buf に /tmp/XXX.png を書き、xtouch_thumbnail_slot_path[slot] も S: 付きで更新。 */
inline void getThumbPathForSlot(int slot, char *buf, size_t len)
{
    const char *tid = nullptr;
    if (slot == 0)
        tid = (bambuStatus.task_id[0] && strcmp(bambuStatus.task_id, "0") != 0) ? bambuStatus.task_id : nullptr;
    else if (slot >= 1 && slot - 1 < xtouch_other_printer_count && otherPrinters[slot - 1].valid)
        tid = (otherPrinters[slot - 1].task_id[0] && strcmp(otherPrinters[slot - 1].task_id, "0") != 0) ? otherPrinters[slot - 1].task_id : nullptr;
    if (tid && tid[0])
    {
        char safe[32];
        size_t j = 0;
        for (size_t i = 0; tid[i] != '\0' && j < sizeof(safe) - 1; i++)
        {
            char c = tid[i];
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')
                safe[j++] = c;
        }
        safe[j] = '\0';
        if (j > 0)
        {
            snprintf(buf, len, "/tmp/%.28s.png", safe);
            if (slot >= 0 && slot < XTOUCH_THUMB_SLOT_MAX)
                snprintf(xtouch_thumbnail_slot_path[slot], XTOUCH_THUMB_PATH_LEN, "S:%s", buf);
            return;
        }
    }
    snprintf(buf, len, "/tmp/pthumb_%d.png", slot);
    if (slot >= 0 && slot < XTOUCH_THUMB_SLOT_MAX)
        snprintf(xtouch_thumbnail_slot_path[slot], XTOUCH_THUMB_PATH_LEN, "S:%s", buf);
}

/** 指定スロット(0=メイン, 1..4=他機)のサムネイルを URL から取得し SD に保存。ファイル名は TaskID（既存なら DL しない）。
 *  @return ファイルが既にある/保存できた場合 true、未DL・失敗時は false */
inline bool downloadThumbnailForSlot(int slot)
{
    static const char *TAG = "thumbnail";

    const char *url = nullptr;
#if defined(__XTOUCH_SCREEN_50__) && defined(CONFIG_SPIRAM)
    static EXT_RAM_ATTR char resolved_url[1024]; /* S3 署名付き URL 用。PSRAM に配置。 */
#else
    static char resolved_url[1024];
#endif
    if (slot == 0)
    {
        url = bambuStatus.image_url;
        /* URL が MQTT から来ていない場合は Cloud task から取得を試みる */
        if ((!url || !url[0]) && cloud.loggedIn && bambuStatus.task_id[0] && strcmp(bambuStatus.task_id, "0") != 0)
        {
#ifdef XTOUCH_DEBUG
            ConsoleDebug.print(F("[xPTouch][THUMB] slot=0 resolve from task_id="));
            ConsoleDebug.println(bambuStatus.task_id);
#endif
            if (cloud.getTaskThumbnailUrl(bambuStatus.task_id, resolved_url, sizeof(resolved_url)))
            {
                strncpy(bambuStatus.image_url, resolved_url, sizeof(bambuStatus.image_url) - 1);
                bambuStatus.image_url[sizeof(bambuStatus.image_url) - 1] = '\0';
                url = bambuStatus.image_url;
            }
            else
            {
                /* 取得に失敗したらこのセッションでは再試行しない */
                bambuStatus.task_id[0] = '\0';
                bambuStatus.image_url[0] = '\0';
            }
        }
        if (!url || !url[0])
        {
#ifdef XTOUCH_DEBUG
            ConsoleDebug.print(F("[xPTouch][THUMB] slot="));
            ConsoleDebug.print(slot);
            ConsoleDebug.println(F(" main: no image_url"));
#endif
            return false;
        }
    }
    else
    {
        int idx = slot - 1;
        if (idx < 0 || idx >= xtouch_other_printer_count || !otherPrinters[idx].valid)
        {
#ifdef XTOUCH_DEBUG
            ConsoleDebug.print(F("[xPTouch][THUMB] slot="));
            ConsoleDebug.print(slot);
            ConsoleDebug.print(F(" other: invalid idx="));
            ConsoleDebug.print(idx);
            ConsoleDebug.print(F(" xtouch_other_printer_count="));
            ConsoleDebug.println(xtouch_other_printer_count);
#endif
            return false;
        }
        url = otherPrinters[idx].image_url;
        /* URL が MQTT から来ていない場合は Cloud task から取得を試みる */
        if ((!url || !url[0]) && cloud.loggedIn && otherPrinters[idx].task_id[0] && strcmp(otherPrinters[idx].task_id, "0") != 0)
        {
#ifdef XTOUCH_DEBUG
            ConsoleDebug.print(F("[xPTouch][THUMB] slot="));
            ConsoleDebug.print(slot);
            ConsoleDebug.print(F(" resolve from task_id="));
            ConsoleDebug.println(otherPrinters[idx].task_id);
#endif
            if (cloud.getTaskThumbnailUrl(otherPrinters[idx].task_id, resolved_url, sizeof(resolved_url)))
            {
                strncpy(otherPrinters[idx].image_url, resolved_url, sizeof(otherPrinters[idx].image_url) - 1);
                otherPrinters[idx].image_url[sizeof(otherPrinters[idx].image_url) - 1] = '\0';
                url = otherPrinters[idx].image_url;
            }
            else
            {
                /* 取得に失敗したらこのセッションでは再試行しない */
                otherPrinters[idx].task_id[0] = '\0';
                otherPrinters[idx].image_url[0] = '\0';
            }
        }
        if (!url || !url[0])
        {
#ifdef XTOUCH_DEBUG
            ConsoleDebug.print(F("[xPTouch][THUMB] slot="));
            ConsoleDebug.print(slot);
            ConsoleDebug.print(F(" other idx="));
            ConsoleDebug.print(idx);
            ConsoleDebug.println(F(" no image_url"));
#endif
            return false;
        }
    }

    char path[64];
    getThumbPathForSlot(slot, path, sizeof(path));
    if (SD.exists(path))
        return true; /* 既に同じ TaskID のファイルがあれば DL しない */
#ifdef XTOUCH_DEBUG
    ConsoleDebug.print(F("[xPTouch][THUMB] slot="));
    ConsoleDebug.print(slot);
    ConsoleDebug.print(F(" download start url="));
    ConsoleDebug.print(url);
    ConsoleDebug.print(F(" -> path="));
    ConsoleDebug.println(path);
    /* URL が長くて一行ログが途中で切れる場合に備え、改行しながらフル URL も出力する */
    ConsoleDebug.println(F("[xPTouch][THUMB] url full:"));
    if (url)
    {
        for (size_t i = 0; url[i] != '\0'; ++i)
        {
            ConsoleDebug.print(url[i]);
            if ((i + 1) % 80 == 0)
                ConsoleDebug.println();
        }
        ConsoleDebug.println();
    }
#endif

    int ok = downloadFileToSDCard(url, path);
#ifdef XTOUCH_DEBUG
    if (ok)
    {
        ConsoleDebug.print(F("[xPTouch][THUMB] slot="));
        ConsoleDebug.print(slot);
        ConsoleDebug.println(F(" download success"));
    }
    else
    {
        ConsoleDebug.print(F("[xPTouch][THUMB] slot="));
        ConsoleDebug.print(slot);
        ConsoleDebug.println(F(" download FAILED"));
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
    return true;
}

#endif

#endif