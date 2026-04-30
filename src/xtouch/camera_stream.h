#ifndef _XTOUCH_CAMERA_STREAM_H
#define _XTOUCH_CAMERA_STREAM_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <vector>
#include "xtouch/types.h"
#include "xtouch/debug.h"
#include "xtouch/filesystem.h"
#include "xtouch/sdcard.h"
#include "xtouch/paths.h"

#ifdef __XTOUCH_PLATFORM_S3__

namespace xtouch_camera_stream
{
static const uint16_t CAMERA_PORT = 6000;
static const char *CAMERA_USER = "bblp";
static WiFiClientSecure s_client;
static bool s_enabled = false;
static bool s_connected = false;
static uint32_t s_last_connect_attempt_ms = 0;
static uint32_t s_connect_backoff_ms = 1000;
static uint32_t s_ok_frames = 0;
static std::vector<uint8_t> s_frame_buf;
static bool s_frame_ready = false;
static const char *CAMERA_WIFI_JSON_PATH = "/wifi.json";

inline void set_endpoint(const char *host, const char *access_code)
{
    if (host && host[0])
    {
        strncpy(xTouchConfig.xTouchCameraHost, host, sizeof(xTouchConfig.xTouchCameraHost) - 1);
        xTouchConfig.xTouchCameraHost[sizeof(xTouchConfig.xTouchCameraHost) - 1] = '\0';
    }
    if (access_code && access_code[0])
    {
        strncpy(xTouchConfig.xTouchCameraAccessCode, access_code, sizeof(xTouchConfig.xTouchCameraAccessCode) - 1);
        xTouchConfig.xTouchCameraAccessCode[sizeof(xTouchConfig.xTouchCameraAccessCode) - 1] = '\0';
    }
}

inline bool update_from_wifi_json_if_exists()
{
    const bool has_wifi_json = xtouch_filesystem_exist(xtouch_sdcard_fs(), CAMERA_WIFI_JSON_PATH);
    const bool has_xtouch_json = xtouch_filesystem_exist(xtouch_sdcard_fs(), xtouch_paths_config);
    if (!has_wifi_json && !has_xtouch_json)
        return false;

    const char *cfg_path = has_wifi_json ? CAMERA_WIFI_JSON_PATH : xtouch_paths_config;
    DynamicJsonDocument cfg = xtouch_filesystem_readJson(xtouch_sdcard_fs(), cfg_path, false);
    if (cfg.isNull())
        return false;

    char host_buf[16] = {0};
    char access_buf[9] = {0};

    if (cfg.containsKey("mqtt") && cfg["mqtt"].is<JsonObject>())
    {
        JsonObject mqtt = cfg["mqtt"].as<JsonObject>();
        if (mqtt.containsKey("host"))
        {
            String host = mqtt["host"].as<String>();
            host.toCharArray(host_buf, sizeof(host_buf));
        }
        if (mqtt.containsKey("accessCode"))
        {
            String ac = mqtt["accessCode"].as<String>();
            ac.toCharArray(access_buf, sizeof(access_buf));
        }
        else if (mqtt.containsKey("access_code"))
        {
            String ac = mqtt["access_code"].as<String>();
            ac.toCharArray(access_buf, sizeof(access_buf));
        }
    }

    if (!host_buf[0] && cfg.containsKey("host"))
    {
        String host = cfg["host"].as<String>();
        host.toCharArray(host_buf, sizeof(host_buf));
    }
    if (!access_buf[0] && cfg.containsKey("accessCode"))
    {
        String ac = cfg["accessCode"].as<String>();
        ac.toCharArray(access_buf, sizeof(access_buf));
    }
    if (!access_buf[0] && cfg.containsKey("access_code"))
    {
        String ac = cfg["access_code"].as<String>();
        ac.toCharArray(access_buf, sizeof(access_buf));
    }

    if (host_buf[0] || access_buf[0])
    {
        set_endpoint(host_buf, access_buf);
        return true;
    }
    return false;
}

inline const char *get_host()
{
    return xTouchConfig.xTouchCameraHost[0] ? xTouchConfig.xTouchCameraHost : xTouchConfig.xTouchHost;
}

inline const char *get_access_code()
{
    return xTouchConfig.xTouchCameraAccessCode[0] ? xTouchConfig.xTouchCameraAccessCode : xTouchConfig.xTouchAccessCode;
}

inline bool read_exact(uint8_t *dst, size_t len, uint32_t timeout_ms)
{
    size_t off = 0;
    uint32_t start = millis();
    while (off < len)
    {
        if (!s_client.connected())
            return false;
        int n = s_client.read(dst + off, len - off);
        if (n > 0)
        {
            off += (size_t)n;
            start = millis();
        }
        else
        {
            if (millis() - start > timeout_ms)
                return false;
            delay(1);
        }
    }
    return true;
}

inline bool open_stream()
{
    const char *host = get_host();
    const char *code = get_access_code();
    if (!host || !host[0] || !code || !code[0])
        return false;

    s_client.stop();
    s_client.setInsecure();
    s_client.setTimeout(12000);
    if (!s_client.connect(host, CAMERA_PORT))
        return false;

    uint8_t auth[80] = {0};
    auth[0] = 0x40;
    auth[4] = 0x00;
    auth[5] = 0x30;
    memcpy(auth + 16, CAMERA_USER, strlen(CAMERA_USER));
    memcpy(auth + 48, code, strlen(code));

    size_t written = s_client.write(auth, sizeof(auth));
    if (written != sizeof(auth))
    {
        s_client.stop();
        return false;
    }
    s_connected = true;
    s_connect_backoff_ms = 1000;
    ConsoleInfo.printf("[xPTouch][I][CAM] stream connected host=%s:%u\n", host, CAMERA_PORT);
    return true;
}

inline void close_stream()
{
    if (s_client.connected())
        s_client.stop();
    s_connected = false;
}

inline bool drain_one_frame()
{
    uint8_t header[16];
    if (!read_exact(header, sizeof(header), 3000))
        return false;

    uint32_t payload = (uint32_t)header[0] | ((uint32_t)header[1] << 8) | ((uint32_t)header[2] << 16) | ((uint32_t)header[3] << 24);
    if (payload == 0 || payload > (512u * 1024u))
        return false;

    s_frame_buf.resize((size_t)payload);
    if (!read_exact(s_frame_buf.data(), (size_t)payload, 5000))
        return false;

    if (payload >= 4 &&
        s_frame_buf[0] == 0xFF && s_frame_buf[1] == 0xD8 &&
        s_frame_buf[payload - 2] == 0xFF && s_frame_buf[payload - 1] == 0xD9)
    {
        s_frame_ready = true;
    }

    s_ok_frames++;
    if ((s_ok_frames % 60u) == 1u)
        ConsoleVerbose.printf("[xPTouch][V][CAM] frame=%lu bytes=%lu\n", (unsigned long)s_ok_frames, (unsigned long)payload);
    return true;
}

inline bool consume_latest_frame(const uint8_t **data, size_t *len)
{
    if (!data || !len || !s_frame_ready || s_frame_buf.empty())
        return false;
    *data = s_frame_buf.data();
    *len = s_frame_buf.size();
    s_frame_ready = false;
    return true;
}

inline void start_if_needed()
{
    if (s_enabled)
        return;
    s_enabled = true;
    s_last_connect_attempt_ms = 0;
    s_connect_backoff_ms = 1000;
}

inline void stop_if_needed()
{
    if (!s_enabled)
        return;
    s_enabled = false;
    close_stream();
    s_frame_ready = false;
    s_frame_buf.clear();
    ConsoleInfo.println("[xPTouch][I][CAM] stream stopped");
}

inline void loop_once()
{
    if (!s_enabled)
        return;
    if (WiFi.status() != WL_CONNECTED)
    {
        close_stream();
        return;
    }
    if (!s_client.connected())
    {
        uint32_t now = millis();
        if (now - s_last_connect_attempt_ms < s_connect_backoff_ms)
            return;
        s_last_connect_attempt_ms = now;
        if (!open_stream())
        {
            s_connect_backoff_ms = (s_connect_backoff_ms < 8000) ? (s_connect_backoff_ms * 2) : 8000;
        }
        return;
    }
    if (!drain_one_frame())
    {
        close_stream();
    }
}

inline void update_from_push_status_net(JsonObject print_obj)
{
    /* wifi.json / xtouch.json がある場合はそちらを優先して接続先を固定 */
    if (update_from_wifi_json_if_exists())
        return;

    if (!print_obj.containsKey("net"))
        return;
    JsonObject net = print_obj["net"].as<JsonObject>();
    if (net.isNull())
        return;

    char host_buf[16] = {0};
    char access_buf[9] = {0};

    if (net.containsKey("ip"))
    {
        String ip = net["ip"].as<String>();
        ip.toCharArray(host_buf, sizeof(host_buf));
    }
    if (net.containsKey("access_code"))
    {
        String ac = net["access_code"].as<String>();
        ac.toCharArray(access_buf, sizeof(access_buf));
    }
    else if (net.containsKey("accessCode"))
    {
        String ac = net["accessCode"].as<String>();
        ac.toCharArray(access_buf, sizeof(access_buf));
    }

    if (net.containsKey("info") && net["info"].is<JsonArray>())
    {
        JsonArray info = net["info"].as<JsonArray>();
        for (JsonVariant v : info)
        {
            if (!v.is<JsonObject>())
                continue;
            JsonObject o = v.as<JsonObject>();
            if (!host_buf[0] && o.containsKey("ip"))
            {
                uint32_t ip_le = o["ip"].as<uint32_t>();
                if (ip_le != 0)
                {
                    snprintf(host_buf, sizeof(host_buf), "%u.%u.%u.%u",
                             (unsigned)(ip_le & 0xFFu),
                             (unsigned)((ip_le >> 8) & 0xFFu),
                             (unsigned)((ip_le >> 16) & 0xFFu),
                             (unsigned)((ip_le >> 24) & 0xFFu));
                }
            }
            if (!access_buf[0] && o.containsKey("access_code"))
            {
                String ac = o["access_code"].as<String>();
                ac.toCharArray(access_buf, sizeof(access_buf));
            }
            if (!access_buf[0] && o.containsKey("accessCode"))
            {
                String ac = o["accessCode"].as<String>();
                ac.toCharArray(access_buf, sizeof(access_buf));
            }
        }
    }

    if (host_buf[0] || access_buf[0])
        set_endpoint(host_buf, access_buf);
}
} // namespace xtouch_camera_stream

#endif // __XTOUCH_PLATFORM_S3__

#endif // _XTOUCH_CAMERA_STREAM_H
