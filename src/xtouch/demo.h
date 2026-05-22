#ifndef _XPTOUCH_DEMO
#define _XPTOUCH_DEMO

#include <Arduino.h>
#include <ArduinoJson.h>
#include "types.h"
#include "paths.h"
#include "filesystem.h"
#include "sdcard.h"
#include "debug.h"
#include "cloud.hpp"
#include "ui/ui.h"

void xptouch_mqtt_processPushStatus(JsonDocument &incomingJson);
#if defined(__XPTOUCH_PLATFORM_S3__)
void xptouch_mqtt_processPushStatusOther(int slot, JsonDocument &incomingJson);
void xptouch_mqtt_load_other_printers(void);
#endif
void xptouch_mqtt_topic_setup(void);
#include "ui/ui_loaders.h"
#include "ui/ui_msgs.h"

#if defined(__XPTOUCH_PLATFORM_S3__)
#include "thumbnail.h"
#include "globals.h"
#endif

/** provisioning.json の demo フラグを xPTouchConfig に反映（WiFi 前に呼ぶ） */
inline void xptouch_demo_detect_flag(void)
{
    xPTouchConfig.xTouchDemoMode = false;
    if (!xptouch_filesystem_exist(xptouch_sdcard_fs(), xptouch_paths_provisioning))
        return;
    DynamicJsonDocument doc = xptouch_filesystem_readJson(xptouch_sdcard_fs(), xptouch_paths_provisioning, false, 2048);
    if (!doc.isNull() && doc.containsKey("demo") && doc["demo"].as<bool>())
        xPTouchConfig.xTouchDemoMode = true;
}

/** Settings: demo フラグを書き換えて再起動（次回起動から反映） */
inline void xptouch_demo_set_flag_and_restart(bool enabled)
{
    DynamicJsonDocument doc(2048);
    if (xptouch_filesystem_exist(xptouch_sdcard_fs(), xptouch_paths_provisioning))
    {
        doc = xptouch_filesystem_readJson(xptouch_sdcard_fs(), xptouch_paths_provisioning, false, 2048);
        if (doc.isNull())
            doc.to<JsonObject>();
    }
    else
    {
        doc.to<JsonObject>();
    }

    doc["demo"] = enabled;
    xptouch_filesystem_writeJson(xptouch_sdcard_fs(), xptouch_paths_provisioning, doc, false, 2048);
    ConsoleInfo.printf("[xPTouch][I][DEMO] provisioning demo=%d -> restart\n", enabled ? 1 : 0);
    delay(200);
    ESP.restart();
}

inline void xptouch_demo_toggle_and_restart(void)
{
    xptouch_demo_set_flag_and_restart(!xPTouchConfig.xTouchDemoMode);
}

static bool xptouch_demo_pushall_path_for_slot(int slot, char *path, size_t path_len)
{
    if (!path || path_len < 16 || slot < 0 || slot >= XPTOUCH_DEMO_PUSHALL_SLOT_MAX)
        return false;
    snprintf(path, path_len, "%s/pushall%d.json", xptouch_paths_demo_dir, slot);
    return true;
}

/** /demo/pushall{N}.json を読み bambuStatus / otherPrinters に適用。成功で true */
static bool xptouch_demo_apply_pushall_slot(int slot)
{
    char path[48];
    if (!xptouch_demo_pushall_path_for_slot(slot, path, sizeof(path)))
        return false;
    if (!xptouch_filesystem_exist(xptouch_sdcard_fs(), path))
        return false;

    DynamicJsonDocument doc = xptouch_filesystem_readJson(
        xptouch_sdcard_fs(), path, false, XPTOUCH_DEMO_PUSHALL_JSON_CAP);
    if (doc.isNull() || !doc.containsKey("print"))
    {
        ConsoleError.printf("[xPTouch][E][DEMO] invalid %s\n", path);
        return false;
    }

    if (slot == 0)
    {
        xptouch_mqtt_processPushStatus(doc);
    }
#if defined(__XPTOUCH_PLATFORM_S3__)
    else
    {
        xptouch_mqtt_processPushStatusOther(slot - 1, doc);
    }
#else
    else
    {
        return false;
    }
#endif

    ConsoleInfo.printf("[xPTouch][I][DEMO] applied %s\n", path);
    return true;
}

/** ペア無し時: printer.json の先頭 dev_id をメインにする */
static void xptouch_demo_ensure_main_printer_identity(void)
{
    if (cloud.isPaired())
    {
        cloud.loadPair();
        return;
    }

    DynamicJsonDocument printers = cloud.loadPrinters();
    JsonObject root = printers.as<JsonObject>();
    if (!root.isNull())
    {
        for (JsonPair p : root)
        {
            const char *dev_id = p.key().c_str();
            if (!dev_id || !dev_id[0])
                continue;
            cloud.setCurrentDevice(String(dev_id));
            if (p.value().containsKey("name"))
                cloud.setPrinterName(p.value()["name"].as<String>());
            if (p.value().containsKey("dev_model_name"))
                cloud.setCurrentModel(p.value()["dev_model_name"].as<String>());
            strncpy(xPTouchConfig.xTouchPairedSerialNumber, xPTouchConfig.xTouchSerialNumber,
                    sizeof(xPTouchConfig.xTouchPairedSerialNumber) - 1);
            xPTouchConfig.xTouchPairedSerialNumber[sizeof(xPTouchConfig.xTouchPairedSerialNumber) - 1] = '\0';
            ConsoleInfo.printf("[xPTouch][I][DEMO] main dev_id=%s\n", xPTouchConfig.xTouchSerialNumber);
            return;
        }
    }

    cloud.setCurrentDevice(String("DEMO0000000001"));
    cloud.setPrinterName(String("Demo Printer"));
    cloud.setCurrentModel(String("BL-P001"));
    strncpy(xPTouchConfig.xTouchPairedSerialNumber, xPTouchConfig.xTouchSerialNumber,
            sizeof(xPTouchConfig.xTouchPairedSerialNumber) - 1);
    xPTouchConfig.xTouchPairedSerialNumber[sizeof(xPTouchConfig.xTouchPairedSerialNumber) - 1] = '\0';
}

/** pushall 要求の代わりにデモ JSON を再読込（Home / Printers） */
inline void xptouch_demo_reload_all_pushall(void)
{
    for (int slot = 0; slot < XPTOUCH_DEMO_PUSHALL_SLOT_MAX; slot++)
        xptouch_demo_apply_pushall_slot(slot);

    ui_msg_send(XPTOUCH_ON_PRINT_STATUS, 0, 0);
    ui_msg_send(XPTOUCH_ON_FILENAME_UPDATE, 0, 0);
    ui_msg_send(XPTOUCH_PRINTERS_LIST_REFRESH, 0, 0);
#if defined(__XPTOUCH_PLATFORM_S3__)
    xptouch_thumbnail_update_path_all_slots();
    xptouch_thumbnail_schedule_fetch_all();
    for (int s = 0; s < XPTOUCH_THUMB_SLOT_MAX; s++)
        ui_msg_send(XPTOUCH_ON_OTHER_PRINTER_UPDATE, (unsigned long long)(intptr_t)(s + 1), 0);
#endif
}

extern "C" void xptouch_demo_reload_pushall_slot_c(int slot)
{
    if (!xptouch_demo_apply_pushall_slot(slot))
        return;
    ui_msg_send(XPTOUCH_ON_PRINT_STATUS, 0, 0);
    ui_msg_send(XPTOUCH_ON_FILENAME_UPDATE, 0, 0);
    ui_msg_send(XPTOUCH_PRINTERS_LIST_REFRESH, 0, 0);
#if defined(__XPTOUCH_PLATFORM_S3__)
    xptouch_thumbnail_update_path_for_slot(slot);
    xptouch_thumbnail_schedule_fetch_all();
    ui_msg_send(XPTOUCH_ON_OTHER_PRINTER_UPDATE, (unsigned long long)(intptr_t)(slot + 1), 0);
#endif
}

extern "C" void xptouch_demo_reload_all_pushall_c(void)
{
    xptouch_demo_reload_all_pushall();
}

#if defined(__XPTOUCH_PLATFORM_S3__)
static void xptouch_demo_copy_json_str(JsonVariant v, const char *key, char *dst, size_t dst_len)
{
    if (!dst || dst_len == 0)
        return;
    dst[0] = '\0';
    if (!v.containsKey(key))
        return;
    const char *s = v[key].as<const char *>();
    if (s && s[0])
    {
        strncpy(dst, s, dst_len - 1);
        dst[dst_len - 1] = '\0';
    }
}

/** /demo/history.json → xptouch_history_tasks[]（Cloud getMyTasks の代替） */
inline bool xptouch_demo_load_history_from_sd(void)
{
    if (!xptouch_filesystem_exist(xptouch_sdcard_fs(), xptouch_paths_demo_history))
    {
        ConsoleError.println(F("[xPTouch][E][DEMO] missing /demo/history.json"));
        return false;
    }

    DynamicJsonDocument doc = xptouch_filesystem_readJson(
        xptouch_sdcard_fs(), xptouch_paths_demo_history, false, XPTOUCH_DEMO_HISTORY_JSON_CAP);
    JsonArray tasks = doc["tasks"].as<JsonArray>();
    if (doc.isNull() || tasks.isNull())
    {
        ConsoleError.println(F("[xPTouch][E][DEMO] invalid /demo/history.json"));
        return false;
    }

    memset(xptouch_history_tasks, 0, sizeof(xptouch_history_tasks));
    xptouch_history_count = 0;

    int n = 0;
    for (JsonVariant v : tasks)
    {
        if (n >= XPTOUCH_HISTORY_TASKS_MAX)
            break;
        if (!v.containsKey("task_id"))
            continue;
        const char *tid = v["task_id"].as<const char *>();
        if (!tid || !tid[0] || strcmp(tid, "0") == 0)
            continue;

        xptouch_history_task_t *t = &xptouch_history_tasks[n];
        memset(t, 0, sizeof(*t));
        xptouch_demo_copy_json_str(v, "task_id", t->task_id, sizeof(t->task_id));
        xptouch_demo_copy_json_str(v, "title", t->title, sizeof(t->title));
        xptouch_demo_copy_json_str(v, "cover_url", t->cover_url, sizeof(t->cover_url));
        xptouch_demo_copy_json_str(v, "device_name", t->device_name, sizeof(t->device_name));
        xptouch_demo_copy_json_str(v, "device_model", t->device_model, sizeof(t->device_model));
        xptouch_demo_copy_json_str(v, "start_time", t->start_time, sizeof(t->start_time));
        xptouch_demo_copy_json_str(v, "end_time", t->end_time, sizeof(t->end_time));
        xptouch_demo_copy_json_str(v, "model_id", t->model_id, sizeof(t->model_id));
        if (v.containsKey("profile_id"))
            t->profile_id = v["profile_id"].as<int>();
        if (v.containsKey("plate_index"))
            t->plate_index = v["plate_index"].as<int>();
        if (v.containsKey("status"))
            t->status = v["status"].as<int>();
        if (v.containsKey("is_printable"))
            t->is_printable = v["is_printable"].as<bool>() ? 1 : 0;
        if (v.containsKey("has_ams_mapping"))
            t->has_ams_mapping = v["has_ams_mapping"].as<bool>() ? 1 : 0;
        t->valid = 1;
        n++;
    }

    xptouch_history_count = n;
    ConsoleInfo.printf("[xPTouch][I][DEMO] history loaded n=%d\n", n);
    return n > 0;
}
#endif

/** WiFi/MQTT なしでデモ状態を構築し Home へ */
inline void xptouch_demo_setup(void)
{
    ConsoleInfo.println(F("[xPTouch][I][DEMO] setup"));

    lv_label_set_text(introScreenCaption, LV_SYMBOL_IMAGE " Demo mode");
    lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0x00AAFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_timer_handler();
    lv_task_handler();

    xptouch_demo_ensure_main_printer_identity();
    cloud.applyStoredPrinterJsonSettingsToConfig();
    xPTouchConfig.xTouchHistoryEnabled = true;
    xPTouchConfig.xTouchMultiPrinterMonitorEnabled = true;
    xPTouchConfig.xTouchP1sCameraStreamEnabled = true;

#ifdef __XPTOUCH_PLATFORM_S3__
    xptouch_mqtt_load_other_printers();
#endif

    for (int slot = 0; slot < XPTOUCH_DEMO_PUSHALL_SLOT_MAX; slot++)
        xptouch_demo_apply_pushall_slot(slot);

    xptouch_mqtt_topic_setup();
    xptouch_mqtt_subscribe_commands();

#if defined(__XPTOUCH_PLATFORM_S3__)
    xptouch_thumbnail_update_path_all_slots();
    xptouch_thumbnail_schedule_fetch_all();
#endif

    ui_msg_send(XPTOUCH_ON_PRINT_STATUS, 0, 0);
    ui_msg_send(XPTOUCH_ON_FILENAME_UPDATE, 0, 0);
    ui_msg_send(XPTOUCH_PRINTERS_LIST_REFRESH, 0, 0);

    loadScreen(0);
    lv_timer_handler();
    lv_task_handler();
}

#endif
