#ifndef _XLCD_FIRMWARE
#define _XLCD_FIRMWARE

#include <Arduino.h>
#include <Update.h>
#include "xtouch/sdcard.h"
#include <stdio.h>
// #include <semver.h>

struct SemVer
{
    int major;
    int minor;
    int patch;
};

bool parseSemVer(const char *version, SemVer &semver)
{
    int parsed = sscanf(version, "%d.%d.%d", &semver.major, &semver.minor, &semver.patch);
    return parsed == 3;
}

int compareSemVer(const SemVer &v1, const SemVer &v2)
{
    if (v1.major != v2.major)
    {
        return v1.major - v2.major;
    }
    if (v1.minor != v2.minor)
    {
        return v1.minor - v2.minor;
    }
    if (v1.patch != v2.patch)
    {
        return v1.patch - v2.patch;
    }
    return 0; // Both versions are equal
}

bool isSemVerGreaterThan(const SemVer &v1, const SemVer &v2)
{
    return compareSemVer(v1, v2) > 0;
}

bool isSemVerLessThan(const SemVer &v1, const SemVer &v2)
{
    return compareSemVer(v1, v2) < 0;
}

bool isSemVerEqualTo(const SemVer &v1, const SemVer &v2)
{
    return compareSemVer(v1, v2) == 0;
}

#define XPTOUCH_FIRMWARE_DOWNLOAD_RETRIES 5
byte xptouch_firmware_updateDownloadRetries = 0;

bool xptouch_firmware_semverNeedsUpdate(const char *compare)
{
    SemVer version1, version2;

    if (parseSemVer(XPTOUCH_FIRMWARE_VERSION, version1) && parseSemVer(compare, version2))
    {
        if (isSemVerGreaterThan(version2, version1))
        {
            return true;
        }
        return false;
    }

    return false;
}

void xptouch_firmware_onProgress(size_t currSize, size_t totalSize)
{
    int16_t progress = (currSize * 100) / totalSize;
    lv_label_set_text_fmt(introScreenCaption, LV_SYMBOL_CHARGE " Updating %d%%", progress);
    lv_timer_handler();
    lv_task_handler();
}

/* force: 手動「Update Now」用。true のとき OTA 設定に関わらずチェックする。起動時は false。 */
void xptouch_firmware_checkOnlineFirmwareUpdate(bool force = false)
{
    if (!force && !xPTouchConfig.xTouchOTAEnabled)
    {
        return;
    }
    printf(" Checking for OTA Update\n");
    lv_label_set_text(introScreenCaption, LV_SYMBOL_CHARGE " Checking for OTA Update");
    lv_timer_handler();
    lv_task_handler();

    bool hasOTAConfigFile = downloadFileToSDCard(xptouch_paths_firmware_ota_file, xptouch_paths_firmware_ota_json);

    if (hasOTAConfigFile)
    {

        DynamicJsonDocument doc = xptouch_filesystem_readJson(xptouch_sdcard_fs(), xptouch_paths_firmware_ota_json);
        if (xptouch_firmware_semverNeedsUpdate(doc["version"]))
        {
            printf(" Downloading Update%d%%\n", 0);
            lv_label_set_text_fmt(introScreenCaption, LV_SYMBOL_CHARGE " Downloading Update %d%%", 0);
            lv_timer_handler();
            lv_task_handler();

            bool xptouch_firmware_hasFirmwareUpdate = downloadFileToSDCard(
                doc["url"],
                xptouch_paths_firmware_ota_fw,
                [](int progress)
                {
                    printf(" Downloading Update%d%%\n", progress);
                    lv_label_set_text_fmt(introScreenCaption, LV_SYMBOL_CHARGE " Downloading Update %d%%", progress);
                    lv_timer_handler();
                    lv_task_handler();
                },
                [](int state)
                {
                    switch (state)
                    {
                    case -1:
                        printf(" Verifying Update\n");
                        lv_label_set_text(introScreenCaption, LV_SYMBOL_CHARGE " Verifying Update");
                        break;
                    case 0:
                        printf(" Invalid Update MD5\n");
                        lv_label_set_text(introScreenCaption, LV_SYMBOL_CHARGE " Invalid Update MD5");
                        break;
                    case 1:
                        printf(" Update Verified\n");
                        lv_label_set_text(introScreenCaption, LV_SYMBOL_CHARGE " Update Verified");
                        break;
                    }
                    lv_timer_handler();
                    lv_task_handler();
                },
                doc["md5"]);

            if (xptouch_firmware_hasFirmwareUpdate)
            {
                printf(" Update downloaded\n");
                lv_label_set_text(introScreenCaption, LV_SYMBOL_OK " Update downloaded");
                lv_timer_handler();
                lv_task_handler();
                delay(3000);
                ESP.restart();
            }
            else
            {
                printf(" Failed to download update. Retry (%d/%d)\n", xptouch_firmware_updateDownloadRetries + 1, XPTOUCH_FIRMWARE_DOWNLOAD_RETRIES);
                lv_label_set_text_fmt(introScreenCaption, LV_SYMBOL_WARNING " Failed to download update. Retry (%d/%d)", xptouch_firmware_updateDownloadRetries + 1, XPTOUCH_FIRMWARE_DOWNLOAD_RETRIES);
                lv_timer_handler();
                lv_task_handler();
                delay(32);
                lv_timer_handler();
                lv_task_handler();
                delay(3000);
                xptouch_firmware_updateDownloadRetries++;
                if (xptouch_firmware_updateDownloadRetries == XPTOUCH_FIRMWARE_DOWNLOAD_RETRIES)
                {
                    // we are unable to download correctly the file
                    // disable OTA to be able to boot
                    xPTouchConfig.xTouchOTAEnabled = false;
                    xptouch_settings_save();
                    xptouch_filesystem_deleteFile(xptouch_sdcard_fs(), xptouch_paths_firmware_ota_fw);
                    xptouch_filesystem_deleteFile(xptouch_sdcard_fs(), xptouch_paths_firmware_ota_json);
                    ESP.restart();
                }
                else
                {
                    xptouch_firmware_checkOnlineFirmwareUpdate(force);
                }
            }
        }
        else
        {
            xptouch_filesystem_deleteFile(xptouch_sdcard_fs(), xptouch_paths_firmware_ota_json);
        }
    }
    else
    {
        printf(" Failed to download update\n");
        lv_label_set_text(introScreenCaption, LV_SYMBOL_WARNING " Failed to download update");
        lv_timer_handler();
        lv_task_handler();
        delay(3000);
    }
    loadScreen(0);
    lv_timer_handler();
    lv_task_handler();
}

void xptouch_firmware_checkFirmwareUpdate(void)
{

    if (xptouch_filesystem_exist(xptouch_sdcard_fs(), xptouch_paths_firmware_ota_fw))
    {
        DynamicJsonDocument doc = xptouch_filesystem_readJson(xptouch_sdcard_fs(), xptouch_paths_firmware_ota_json);
        File firmware = xptouch_filesystem_open(xptouch_sdcard_fs(), xptouch_paths_firmware_ota_fw);
        Update.onProgress(xptouch_firmware_onProgress);
        Update.begin(firmware.size(), U_FLASH);
        if (doc.containsKey("md5"))
        {
            Update.setMD5(doc["md5"]);
        }
        Update.writeStream(firmware);
        bool updateSucceeded = Update.end();
        firmware.close();

        if (updateSucceeded)
        {
            printf(" Update finished\n");
            lv_label_set_text(introScreenCaption, LV_SYMBOL_OK " Update finished");
            lv_timer_handler();
            lv_task_handler();
        }
        else
        {
            printf(" Update error\n");
            lv_label_set_text(introScreenCaption, LV_SYMBOL_WARNING " Update error");
            lv_timer_handler();
            lv_task_handler();
            delay(3000);
            printf(" Deleting firmware file\n");
            lv_label_set_text(introScreenCaption, LV_SYMBOL_TRASH " Deleting firmware file");
            lv_timer_handler();
            lv_task_handler();
        }

        xptouch_filesystem_deleteFile(xptouch_sdcard_fs(), xptouch_paths_firmware_ota_json);
        xptouch_filesystem_deleteFile(xptouch_sdcard_fs(), xptouch_paths_firmware_ota_fw);

        delay(2000);
        ESP.restart();
    }
}
#endif