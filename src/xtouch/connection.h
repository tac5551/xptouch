#ifndef _XLCD_CONNECTION
#define _XLCD_CONNECTION

#include <WiFi.h>
#include "mbedtls/base64.h"
#include <ArduinoJson.h>
#include "filesystem.h"
#include "sdcard.h"
#include "paths.h"

bool xtouch_wifi_setup()
{
    DynamicJsonDocument wifiConfig(1024);
    bool cloud_mode = false;
    if(xtouch_filesystem_exist(xtouch_sdcard_fs(), xtouch_paths_provisioning)){
        cloud_mode = true;
        lv_label_set_text(introScreenCaption, wifiConfig.isNull() ? LV_SYMBOL_SD_CARD "provisioning mode" : LV_SYMBOL_WARNING " Inaccurate provisioning.json and xtouch.json");
        lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_timer_handler();
        lv_task_handler();
        wifiConfig = xtouch_filesystem_readJson(xtouch_sdcard_fs(), xtouch_paths_provisioning);
    }else if(xtouch_filesystem_exist(xtouch_sdcard_fs(), xtouch_paths_config)){
        lv_label_set_text(introScreenCaption, wifiConfig.isNull() ? LV_SYMBOL_SD_CARD "Lan only mode" : LV_SYMBOL_WARNING " Inaccurate xtouch.json");
        lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_timer_handler();
        lv_task_handler();
        wifiConfig = xtouch_filesystem_readJson(xtouch_sdcard_fs(), xtouch_paths_config);
        strcpy(xTouchConfig.xTouchAccessCode, wifiConfig["mqtt"]["accessCode"].as<const char *>());
        strcpy(xTouchConfig.xTouchSerialNumber, wifiConfig["mqtt"]["serialNumber"].as<const char *>());
        strcpy(xTouchConfig.xTouchHost, wifiConfig["mqtt"]["host"].as<const char *>());
        strcpy(xTouchConfig.xTouchPrinterModel, wifiConfig["mqtt"]["printerModel"].as<const char *>());
    }else{
        lv_label_set_text(introScreenCaption, wifiConfig.isNull() ? LV_SYMBOL_SD_CARD " Missing provisioning.json and xtouch.json" : LV_SYMBOL_WARNING " Inaccurate provisioning.json and xtouch.json");
        lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_timer_handler();
        lv_task_handler();
        return false;
    }

    if (wifiConfig.isNull() || !wifiConfig.containsKey("ssid") || !wifiConfig.containsKey("pwd"))
    {
        lv_label_set_text(introScreenCaption, wifiConfig.isNull() ? LV_SYMBOL_SD_CARD " Missing provisioning.json and xtouch.json" : LV_SYMBOL_WARNING " Inaccurate provisioning.json and xtouch.json");
        lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_timer_handler();
        lv_task_handler();
        return false;
    }
    String ssidB64String = wifiConfig["ssid"].as<const char *>();
    String ssidPWDString = wifiConfig["pwd"].as<const char *>();
    delay(1000);

    int timeout = wifiConfig.containsKey("timeout") ? wifiConfig["timeout"].as<int>() : 3000;

    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.setAutoReconnect(false); /* 自動再接続が暴れるとログが洪水になるため、自前で制御する */
    ConsoleInfo.println(F("[xPTouch][I][CONNECTION] Connecting to WiFi .."));
    WiFi.onEvent(onWiFiEvent);

    lv_label_set_text(introScreenCaption, LV_SYMBOL_WIFI " Connecting");
    lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_timer_handler();
    lv_task_handler();

    const int attempts_per_round = 2; /* 要望: 2回で区切る */
    int round = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        round++;
        lv_label_set_text(introScreenCaption, LV_SYMBOL_WIFI " Connecting");
        lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_timer_handler();
        lv_task_handler();

        for (int attempt = 0; attempt < attempts_per_round; attempt++)
        {
            WiFi.disconnect(true, true);
            delay(200);
            WiFi.begin(ssidB64String.c_str(), ssidPWDString.c_str());

            unsigned long start = millis();
            while (millis() - start < (unsigned long)timeout)
            {
                wl_status_t st = WiFi.status();
                if (st == WL_CONNECTED)
                    break;
                delay(50);
            }
            if (WiFi.status() == WL_CONNECTED)
                break;
        }

        if (WiFi.status() == WL_CONNECTED)
            break;

        /* 2回失敗したら少し待って次ラウンド（ログ洪水/電波状況変化待ち） */
        lv_label_set_text(introScreenCaption, LV_SYMBOL_WARNING " WiFi retry...");
        lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0xffaa00), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_timer_handler();
        lv_task_handler();
        delay(2000);
    }

    WiFi.setTxPower(WIFI_POWER_19_5dBm); // https://github.com/G6EJD/ESP32-8266-Adjust-WiFi-RF-Power-Output/blob/main/README.md

    /* Cloudモードのときだけ DNS を 1.1.1.1 に固定（us.mqtt.bambulab.com 解決用） */
    if (cloud_mode)
    {
        WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), IPAddress(1, 1, 1, 1));
    }

    delay(1000);
    lv_label_set_text(introScreenCaption, LV_SYMBOL_WIFI " Connected");
    lv_timer_handler();
    lv_task_handler();
    delay(1000);
    ConsoleInfo.print(F("[xPTouch][I][CONNECTION] Connected to the WiFi network with IP: "));
    ConsoleInfo.println(WiFi.localIP());
    ConsoleInfo.print(F("[xPTouch][I][CONNECTION] DNS: "));
    ConsoleInfo.println(WiFi.dnsIP());

    return true;
}

#endif