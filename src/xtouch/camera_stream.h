#ifndef _XPTOUCH_CAMERA_STREAM_H
#define _XPTOUCH_CAMERA_STREAM_H

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

#ifdef __XPTOUCH_PLATFORM_S3__

namespace xptouch_camera_stream
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
static std::vector<uint8_t> s_demo_still_buf;
static bool s_demo_still_loaded = false;
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
        strncpy(xPTouchConfig.xTouchCameraHost, host, sizeof(xPTouchConfig.xTouchCameraHost) - 1);
        xPTouchConfig.xTouchCameraHost[sizeof(xPTouchConfig.xTouchCameraHost) - 1] = '\0';
    }
    if (access_code && access_code[0])
    {
        strncpy(xPTouchConfig.xTouchCameraAccessCode, access_code, sizeof(xPTouchConfig.xTouchCameraAccessCode) - 1);
        xPTouchConfig.xTouchCameraAccessCode[sizeof(xPTouchConfig.xTouchCameraAccessCode) - 1] = '\0';
    }
}

inline bool update_from_wifi_json_if_exists()
{
    const bool has_wifi_json = xptouch_filesystem_exist(xptouch_sdcard_fs(), CAMERA_WIFI_JSON_PATH);
    const bool has_xptouch_json = xptouch_filesystem_exist(xptouch_sdcard_fs(), xptouch_paths_config);
    if (!has_wifi_json && !has_xptouch_json)
        return false;

    const char *cfg_path = has_wifi_json ? CAMERA_WIFI_JSON_PATH : xptouch_paths_config;
    DynamicJsonDocument cfg = xptouch_filesystem_readJson(xptouch_sdcard_fs(), cfg_path, false);
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
    return xPTouchConfig.xTouchCameraHost[0] ? xPTouchConfig.xTouchCameraHost : xPTouchConfig.xTouchHost;
}

inline const char *get_access_code()
{
    return xPTouchConfig.xTouchCameraAccessCode[0] ? xPTouchConfig.xTouchCameraAccessCode : xPTouchConfig.xTouchAccessCode;
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

inline bool is_valid_jpeg(const uint8_t *data, size_t len)
{
    return data && len >= 4 && data[0] == 0xFF && data[1] == 0xD8 &&
           data[len - 2] == 0xFF && data[len - 1] == 0xD9;
}

inline void reset_demo_still_cache()
{
    s_demo_still_loaded = false;
    s_demo_still_buf.clear();
}

/** デモモード: /demo/camera.jpg を読み込み（1回のみキャッシュ） */
inline bool ensure_demo_still_loaded()
{
    if (s_demo_still_loaded)
        return !s_demo_still_buf.empty();
    s_demo_still_loaded = true;
    s_demo_still_buf.clear();

    if (!xptouch_filesystem_exist(xptouch_sdcard_fs(), xptouch_paths_demo_camera))
    {
        ConsoleError.println(F("[xPTouch][E][CAM][DEMO] missing /demo/camera.jpg"));
        return false;
    }

    File f = xptouch_filesystem_open(xptouch_sdcard_fs(), xptouch_paths_demo_camera);
    if (!f)
    {
        ConsoleError.println(F("[xPTouch][E][CAM][DEMO] open failed"));
        return false;
    }

    const size_t sz = f.size();
    if (sz == 0 || sz > XPTOUCH_DEMO_CAMERA_JPEG_MAX)
    {
        f.close();
        ConsoleError.printf("[xPTouch][E][CAM][DEMO] bad size=%u\n", (unsigned)sz);
        return false;
    }

    s_demo_still_buf.resize(sz);
    const size_t n = f.read(s_demo_still_buf.data(), sz);
    f.close();
    if (n != sz || !is_valid_jpeg(s_demo_still_buf.data(), n))
    {
        s_demo_still_buf.clear();
        ConsoleError.println(F("[xPTouch][E][CAM][DEMO] invalid JPEG"));
        return false;
    }

    ConsoleInfo.printf("[xPTouch][I][CAM][DEMO] loaded still %u bytes\n", (unsigned)n);
    return true;
}

/** デモ用静止画。毎フレーム同じバッファを返す（consume では消費しない） */
inline bool consume_demo_still_frame(const uint8_t **data, size_t *len)
{
    if (!data || !len || !ensure_demo_still_loaded())
        return false;
    *data = s_demo_still_buf.data();
    *len = s_demo_still_buf.size();
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
    char host_buf[16] = {0};
    char access_buf[9] = {0};

    /* マルチプリンタ切替時は live の print.net を常に先に反映（旧挙動は wifi.json が先で return しており、カメラが前機のまま固定されていた） */
    if (print_obj.containsKey("net"))
    {
        JsonObject net = print_obj["net"].as<JsonObject>();
        if (!net.isNull())
        {
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
        }
    }

    if (host_buf[0] || access_buf[0])
        set_endpoint(host_buf, access_buf);

    /* MQTT にカメラ用 IP がまだ無いときだけ SD の wifi.json / xtouch.json で補完 */
    if (!xPTouchConfig.xTouchCameraHost[0])
        (void)update_from_wifi_json_if_exists();
}
} // namespace xptouch_camera_stream

#endif // __XPTOUCH_PLATFORM_S3__

#endif // _XPTOUCH_CAMERA_STREAM_H
