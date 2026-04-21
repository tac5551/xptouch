#ifndef _XTOUCH_P1S_VIDEO_H
#define _XTOUCH_P1S_VIDEO_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include "ui/ui_msgs.h"
#include "xtouch/sdcard.h"
#include "xtouch/types.h"
#include "xtouch/bblp.h"

#ifdef __XTOUCH_PLATFORM_S3__

static volatile bool xtouch_p1s_video_enabled = false;
static TaskHandle_t xtouch_p1s_video_task_handle = nullptr;
static uint8_t xtouch_p1s_video_frame_idx = 0;

static bool xtouch_p1s_video_read_exact(WiFiClientSecure &client, uint8_t *buf, size_t len)
{
    size_t off = 0;
    while (off < len)
    {
        int n = client.read(buf + off, len - off);
        if (n <= 0)
            return false;
        off += (size_t)n;
    }
    return true;
}

static void xtouch_p1s_video_build_auth(uint8_t *pkt, size_t len)
{
    if (!pkt || len < 80)
        return;
    memset(pkt, 0, len);
    uint32_t payload = 0x40; /* 64 bytes */
    uint32_t type = 0x3000;
    memcpy(pkt + 0, &payload, 4);
    memcpy(pkt + 4, &type, 4);
    strncpy((char *)(pkt + 16), "bblp", 31);
    strncpy((char *)(pkt + 48), xTouchConfig.xTouchAccessCode, 31);
}

static bool xtouch_p1s_video_fetch_one_frame(char *out_path, size_t out_path_len)
{
    if (!out_path || out_path_len == 0 || !xTouchConfig.xTouchHost[0] || !xTouchConfig.xTouchAccessCode[0])
        return false;

    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(5000);
    if (!client.connect(xTouchConfig.xTouchHost, 6000))
        return false;

    uint8_t auth[80];
    xtouch_p1s_video_build_auth(auth, sizeof(auth));
    if (client.write(auth, sizeof(auth)) != (int)sizeof(auth))
    {
        client.stop();
        return false;
    }

    uint8_t header[16];
    if (!xtouch_p1s_video_read_exact(client, header, sizeof(header)))
    {
        client.stop();
        return false;
    }

    uint32_t payload_size = 0;
    memcpy(&payload_size, header + 0, 4);
    if (payload_size < 64 || payload_size > (1024u * 512u))
    {
        client.stop();
        return false;
    }

    char path[32];
    snprintf(path, sizeof(path), "/tmp/p1v_%u.jpg", (unsigned)(xtouch_p1s_video_frame_idx & 1u));
    File f = xtouch_sdcard_open(path, FILE_WRITE);
    if (!f)
    {
        client.stop();
        return false;
    }

    uint8_t buf[1024];
    uint32_t left = payload_size;
    while (left > 0)
    {
        size_t chunk = left > sizeof(buf) ? sizeof(buf) : (size_t)left;
        if (!xtouch_p1s_video_read_exact(client, buf, chunk))
        {
            f.close();
            xtouch_sdcard_remove(path);
            client.stop();
            return false;
        }
        if (f.write(buf, chunk) != chunk)
        {
            f.close();
            xtouch_sdcard_remove(path);
            client.stop();
            return false;
        }
        left -= (uint32_t)chunk;
    }
    f.close();
    client.stop();

    strncpy(out_path, path, out_path_len - 1);
    out_path[out_path_len - 1] = '\0';
    return true;
}

static void xtouch_p1s_video_task(void *arg)
{
    (void)arg;
    while (true)
    {
        if (!xtouch_p1s_video_enabled)
        {
            vTaskDelay(pdMS_TO_TICKS(250));
            continue;
        }
        if (!xtouch_bblp_is_p1Series())
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        char path[32] = {0};
        if (xtouch_p1s_video_fetch_one_frame(path, sizeof(path)))
        {
            strncpy(xtouch_p1s_video_last_path, path, sizeof(xtouch_p1s_video_last_path) - 1);
            xtouch_p1s_video_last_path[sizeof(xtouch_p1s_video_last_path) - 1] = '\0';
            xtouch_p1s_video_frame_idx++;
            ui_msg_send(XTOUCH_ON_P1S_VIDEO_FRAME, xtouch_p1s_video_frame_idx, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(120));
    }
}

static void xtouch_p1s_video_on_start(void *s, lv_msg_t *m)
{
    (void)s;
    (void)m;
    xtouch_p1s_video_enabled = true;
}

static void xtouch_p1s_video_on_stop(void *s, lv_msg_t *m)
{
    (void)s;
    (void)m;
    xtouch_p1s_video_enabled = false;
}

inline void xtouch_p1s_video_subscribe_events()
{
    lv_msg_subscribe(XTOUCH_COMMAND_P1S_VIDEO_START, (lv_msg_subscribe_cb_t)xtouch_p1s_video_on_start, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_P1S_VIDEO_STOP, (lv_msg_subscribe_cb_t)xtouch_p1s_video_on_stop, NULL);
    if (!xtouch_p1s_video_task_handle)
    {
        xTaskCreatePinnedToCore(xtouch_p1s_video_task, "p1s_video", 8192, nullptr, 1, &xtouch_p1s_video_task_handle, 1);
    }
}

#endif

#endif
