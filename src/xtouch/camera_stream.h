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
static uint8_t s_header_buf[16];
static size_t s_header_off = 0;
static uint32_t s_payload_len = 0;
static size_t s_payload_off = 0;
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

inline void reset_frame_parser()
{
    s_header_off = 0;
    s_payload_len = 0;
    s_payload_off = 0;
    s_frame_buf.clear();
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
    reset_frame_parser();
    ConsoleInfo.printf("[xPTouch][I][CAM] stream connected host=%s:%u\n", host, CAMERA_PORT);
    return true;
}

inline void close_stream()
{
    if (s_client.connected())
        s_client.stop();
    s_connected = false;
    reset_frame_parser();
}

inline bool pump_stream_nonblocking()
{
    if (!s_client.connected())
        return false;

    /* loop() 1回あたりの受信処理を制限して UI 応答性を優先 */
    int steps = 0;
    while (s_client.available() > 0 && steps < 8)
    {
        if (s_header_off < sizeof(s_header_buf))
        {
            int n = s_client.read(s_header_buf + s_header_off, sizeof(s_header_buf) - s_header_off);
            if (n <= 0)
                break;
            s_header_off += (size_t)n;
            steps++;
            if (s_header_off < sizeof(s_header_buf))
                continue;

            s_payload_len = (uint32_t)s_header_buf[0] | ((uint32_t)s_header_buf[1] << 8) |
                            ((uint32_t)s_header_buf[2] << 16) | ((uint32_t)s_header_buf[3] << 24);
            if (s_payload_len == 0 || s_payload_len > (512u * 1024u))
                return false;

            s_frame_buf.resize((size_t)s_payload_len);
            s_payload_off = 0;
        }

        if (s_payload_len > 0 && s_payload_off < (size_t)s_payload_len)
        {
            size_t remaining = (size_t)s_payload_len - s_payload_off;
            size_t chunk = remaining > 4096 ? 4096 : remaining;
            int n = s_client.read(s_frame_buf.data() + s_payload_off, chunk);
            if (n <= 0)
                break;
            s_payload_off += (size_t)n;
            steps++;
        }

        if (s_payload_len > 0 && s_payload_off >= (size_t)s_payload_len)
        {
            if (s_payload_len >= 4 &&
                s_frame_buf[0] == 0xFF && s_frame_buf[1] == 0xD8 &&
                s_frame_buf[s_payload_len - 2] == 0xFF && s_frame_buf[s_payload_len - 1] == 0xD9)
            {
                s_frame_ready = true;
            }

            s_ok_frames++;
            if ((s_ok_frames % 60u) == 1u)
                ConsoleVerbose.printf("[xPTouch][V][CAM] frame=%lu bytes=%lu\n", (unsigned long)s_ok_frames, (unsigned long)s_payload_len);

            s_header_off = 0;
            s_payload_len = 0;
            s_payload_off = 0;
        }
    }

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
    if (!pump_stream_nonblocking())
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
