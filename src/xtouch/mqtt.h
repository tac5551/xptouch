#ifndef _XLCD_MQTT
#define _XLCD_MQTT

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>
#include "ui/ui_msgs.h"
#include "types.h"
#include "autogrowstream.h"
#include "bbl-certs.h"
// #include "xtouch/ams-status.hpp"
#include "xtouch/cloud.hpp"

WiFiClientSecure xtouch_wiFiClientSecure;
PubSubClient xtouch_pubSubClient(xtouch_wiFiClientSecure);

String xtouch_mqtt_request_topic;
String xtouch_mqtt_report_topic;

#include "ams.h"
#include "device.h"
#include "trays.h"
#ifdef __XTOUCH_SCREEN_50__
#include "xtouch/thumbnail.h"
#endif
#define XTOUCH_MQTT_SERVER_TIMEOUT 20
#define XTOUCH_MQTT_SERVER_PUSH_STATUS_TIMEOUT 1800
#define XTOUCH_MQTT_SERVER_JSON_PARSE_SIZE 4192

/* ---------------------------------------------- */
bool xtouch_mqtt_firstConnectionDone = false;
int xtouch_mqtt_connection_timeout_count = 5;
int xtouch_mqtt_connection_fail_count = 5;
unsigned long long xtouch_mqtt_lastPushStatus = 0;

XtouchAutoGrowBufferStream stream;

void xtouch_mqtt_sendMsg(XTOUCH_MESSAGE message, unsigned long long data = 0)
{
    struct XTOUCH_MESSAGE_DATA eventData;
    eventData.data = data;
    lv_msg_send(message, &eventData);
}

void xtouch_mqtt_topic_setup()
{
    String xtouch_device_topic = String("device/") + xTouchConfig.xTouchSerialNumber;
    xtouch_mqtt_request_topic = xtouch_device_topic + String("/request");
    xtouch_mqtt_report_topic = xtouch_device_topic + String("/report");
}

#ifdef __XTOUCH_SCREEN_50__
/** printer.json から選択中以外の dev_id を最大 XTOUCH_OTHER_PRINTERS_MAX 件取得し、other_printer_* を埋める。クラウド MQTT セットアップ時のみ呼ぶ。 */
void xtouch_mqtt_load_other_printers()
{
    xtouch_other_printer_count = 0;
    for (int i = 0; i < XTOUCH_OTHER_PRINTERS_MAX; i++)
    {
        otherPrinters[i].valid = 0;
        xtouch_other_printer_dev_ids[i][0] = '\0';
    }
    DynamicJsonDocument printers = cloud.loadPrinters();
    JsonObject obj = printers.as<JsonObject>();
    int idx = 0;
    for (JsonPair p : obj)
    {
        if (idx >= XTOUCH_OTHER_PRINTERS_MAX)
            break;
        const char *dev_id = p.key().c_str();
        if (strcmp(dev_id, xTouchConfig.xTouchSerialNumber) == 0)
            continue;
        if (p.value().containsKey("dev_product_name"))
        {
            const char *product = p.value()["dev_product_name"].as<const char *>();
            if (product && (strcmp(product, "H2C") == 0 || strcmp(product, "H2D") == 0 || strcmp(product, "H2S") == 0))
                continue;
        }
        strncpy(xtouch_other_printer_dev_ids[idx], dev_id, 15);
        xtouch_other_printer_dev_ids[idx][15] = '\0';
        otherPrinters[idx].valid = 1;
        strncpy(otherPrinters[idx].dev_id, dev_id, 15);
        otherPrinters[idx].dev_id[15] = '\0';
        otherPrinters[idx].print_status = XTOUCH_PRINT_STATUS_IDLE;
        otherPrinters[idx].mc_print_percent = 0;
        otherPrinters[idx].mc_left_time = 0;
        otherPrinters[idx].subtask_name[0] = '\0';
        otherPrinters[idx].image_url[0] = '\0';
        otherPrinters[idx].current_layer = 0;
        otherPrinters[idx].total_layers = 0;
        if (p.value().containsKey("name"))
        {
            strncpy(otherPrinters[idx].name, p.value()["name"].as<const char *>(), 31);
            otherPrinters[idx].name[31] = '\0';
        }
        else
            otherPrinters[idx].name[0] = '\0';
        idx++;
    }
    xtouch_other_printer_count = idx;
}

/** Printers 画面用: 指定 dev_id に pushall 要求を送信（device/{dev_id}/request）。 */
static void xtouch_mqtt_pushall_for_dev(const char *dev_id)
{
    if (!dev_id || !dev_id[0])
        return;
    DynamicJsonDocument json(256);
    json["pushing"]["command"] = "pushall";
    json["pushing"]["version"] = 1;
    json["pushing"]["push_target"] = 1;
    json["pushing"]["sequence_id"] = xtouch_device_next_sequence();
    json["user_id"] = "123456789";

    String payload;
    serializeJson(json, payload);
    String topic = String("device/") + dev_id + "/request";
    ConsoleDebug.print(F("[xPTouch][MQTT] PUSHALL dev_id="));
    ConsoleDebug.print(dev_id);
    ConsoleDebug.print(F(" topic="));
    ConsoleDebug.print(topic);
    ConsoleDebug.print(F(" payload="));
    ConsoleDebug.println(payload);
    xtouch_pubSubClient.publish(topic.c_str(), payload.c_str());
}

/** Printers 画面用: メイン＋他プリンタへ pushall を送る（毎回呼び出し可）。 */
inline void xtouch_mqtt_pushall_all_printers_for_screen()
{
    xtouch_mqtt_pushall_for_dev(xTouchConfig.xTouchSerialNumber);
    for (int i = 0; i < xtouch_other_printer_count; i++)
    {
        if (!otherPrinters[i].valid)
            continue;
        xtouch_mqtt_pushall_for_dev(xtouch_other_printer_dev_ids[i]);
    }
}

/* C 側（ui_loaders.c など）から呼べるようにするラッパー。 */
#ifdef __cplusplus
extern "C" void xtouch_mqtt_pushall_all_printers_for_screen_c(void)
{
    xtouch_mqtt_pushall_all_printers_for_screen();
}
#endif

/** gcode_state 文字列を XTouchPrintStatus に変換（他プリンター用、device.h のマッピングと同一） */
static int xtouch_mqtt_gcode_state_to_status(const String &state)
{
    if (state == "IDLE")
        return XTOUCH_PRINT_STATUS_IDLE;
    if (state == "RUNNING")
        return XTOUCH_PRINT_STATUS_RUNNING;
    if (state == "PAUSE" || state == "Pause")
        return XTOUCH_PRINT_STATUS_PAUSED;
    if (state == "FINISH")
        return XTOUCH_PRINT_STATUS_FINISHED;
    if (state == "PREPARE")
        return XTOUCH_PRINT_STATUS_PREPARE;
    if (state == "FAILED")
        return XTOUCH_PRINT_STATUS_FAILED;
    return XTOUCH_PRINT_STATUS_IDLE;
}

void xtouch_mqtt_processPushStatusOther(int slot, JsonDocument &incomingJson)
{
    if (slot < 0 || slot >= xtouch_other_printer_count || !otherPrinters[slot].valid)
        return;
    if (!incomingJson.containsKey("print"))
        return;
    JsonObject print = incomingJson["print"].as<JsonObject>();
    if (print.containsKey("gcode_state"))
    {
        otherPrinters[slot].print_status = xtouch_mqtt_gcode_state_to_status(print["gcode_state"].as<String>());
    }
    if (print.containsKey("mc_percent"))
    {
        if (print["mc_percent"].is<String>())
            otherPrinters[slot].mc_print_percent = atoi(print["mc_percent"].as<String>().c_str());
        else
            otherPrinters[slot].mc_print_percent = print["mc_percent"].as<int>();
    }
    if (print.containsKey("mc_remaining_time"))
    {
        if (print["mc_remaining_time"].is<String>())
            otherPrinters[slot].mc_left_time = atoi(print["mc_remaining_time"].as<String>().c_str()) * 60;
        else
            otherPrinters[slot].mc_left_time = print["mc_remaining_time"].as<int>() * 60;
    }
    if (print.containsKey("subtask_name"))
    {
        strncpy(otherPrinters[slot].subtask_name, print["subtask_name"].as<const char *>(), 31);
        otherPrinters[slot].subtask_name[31] = '\0';
    }
    if (print.containsKey("task_id"))
    {
        const char *tid = print["task_id"].as<const char *>();
        strncpy(otherPrinters[slot].task_id, tid, sizeof(otherPrinters[slot].task_id) - 1);
        otherPrinters[slot].task_id[sizeof(otherPrinters[slot].task_id) - 1] = '\0';
#ifdef XTOUCH_DEBUG
        ConsoleDebug.print(F("[xPTouch][MQTT] other slot="));
        ConsoleDebug.print(slot);
        ConsoleDebug.print(F(" task_id="));
        ConsoleDebug.println(tid);
#endif
    }
    if (print.containsKey("url"))
    {
        const char *url_other = print["url"].as<const char *>();
        strncpy(otherPrinters[slot].image_url, url_other, sizeof(otherPrinters[slot].image_url) - 1);
        otherPrinters[slot].image_url[sizeof(otherPrinters[slot].image_url) - 1] = '\0';
#ifdef XTOUCH_DEBUG
        ConsoleDebug.print(F("[xPTouch][MQTT] URL other slot="));
        ConsoleDebug.print(slot);
        ConsoleDebug.print(F(" url="));
        ConsoleDebug.println(url_other);
#endif
    }
    if (print.containsKey("layer_num"))
        otherPrinters[slot].current_layer = print["layer_num"].as<int>();
    if (print.containsKey("total_layer_num"))
        otherPrinters[slot].total_layers = print["total_layer_num"].as<int>();
    /* 行インデックスで送る: 0=メイン, 1=他1台目, 2=他2台目。UI の row と一致させる */
    xtouch_mqtt_sendMsg(XTOUCH_ON_OTHER_PRINTER_UPDATE, (unsigned long long)(slot + 1));
}
#endif

void xtouch_mqtt_parse_tray(uint8_t ams_idx, uint8_t tray_idx, char *color, int loaded)
{

    uint64_t number = strtoll(color, NULL, 16);
    number <<= 8;
    number |= tray_idx << 4;
    number |= loaded;

    set_tray_status(ams_idx, tray_idx, number);
}

String xtouch_mqtt_parse_printer_type(String type_str)
{
    if (type_str == "3DPrinter-X1")
    {
        return "BL-P002";
    }
    else if (type_str == "3DPrinter-X1-Carbon")
    {
        return "BL-P001";
    }
    else if (type_str == "BL-P001")
    {
        return type_str;
    }
    else if (type_str == "BL-P003")
    {
        return type_str;
    }
    return "";
}

void xtouch_mqtt_update_slice_info(const char *project_id, const char *profile_id, const char *subtask_id, int plate_idx)
{
    strcpy(bambuStatus.project_id_, project_id);
    strcpy(bambuStatus.profile_id_, profile_id);
    strcpy(bambuStatus.subtask_id_, subtask_id);
}

void xtouch_mqtt_processPushStatus(JsonDocument &incomingJson)
{
    xtouch_mqtt_lastPushStatus = millis();
    //ConsoleDebug.println(F("[xPTouch][MQTT] ProcessPushStatus"));

    if (incomingJson != NULL && incomingJson.containsKey("print"))
    {

        // #pragma region printing
        if (incomingJson["print"].containsKey("print_gcode_action"))
        {
            bambuStatus.print_gcode_action = incomingJson["print"]["print_gcode_action"].as<int>();
        }

        if (incomingJson["print"].containsKey("print_real_action"))
        {
            bambuStatus.print_real_action = incomingJson["print"]["print_real_action"].as<int>();
        }

        if (incomingJson["print"].containsKey("print_type"))
        {
            strcpy(bambuStatus.print_type, incomingJson["print"]["print_type"]);
        }
        if (incomingJson["print"].containsKey("home_flag"))
        {
            bambuStatus.home_flag = incomingJson["print"]["home_flag"].as<int>();
        }

        if (incomingJson["print"].containsKey("hw_switch_state"))
        {
            bambuStatus.hw_switch_state = incomingJson["print"]["hw_switch_state"].as<int>();
        }

        if (incomingJson["print"].containsKey("mc_remaining_time"))
        {
            if (incomingJson["print"]["mc_remaining_time"].is<String>())
            {
                String timeStr = incomingJson["print"]["mc_remaining_time"].as<String>();

                bambuStatus.mc_left_time = atoi(timeStr.c_str()) * 60;
            }
            else if (incomingJson["print"]["mc_remaining_time"].is<int>())
            {
                bambuStatus.mc_left_time = incomingJson["print"]["mc_remaining_time"].as<int>() * 60;
            }
        }

        if (incomingJson["print"].containsKey("mc_percent"))
        {
            if (incomingJson["print"]["mc_percent"].is<String>())
                bambuStatus.mc_print_percent = atoi(incomingJson["print"]["mc_percent"].as<String>().c_str());
            else if (incomingJson["print"]["mc_percent"].is<int>())
                bambuStatus.mc_print_percent = incomingJson["print"]["mc_percent"].as<int>();
        }

        if (incomingJson["print"].containsKey("mc_print_sub_stage"))
        {
            bambuStatus.mc_print_sub_stage = incomingJson["print"]["mc_print_sub_stage"].as<int>();
        }

        if (incomingJson["print"].containsKey("mc_print_stage"))
        {
            if (incomingJson["print"]["mc_print_stage"].is<String>())
                bambuStatus.mc_print_stage = atoi(incomingJson["print"]["mc_print_stage"].as<String>().c_str());
            if (incomingJson["print"]["mc_print_stage"].is<int>())
                bambuStatus.mc_print_stage = incomingJson["print"]["mc_print_stage"].as<int>();
        }

        if (incomingJson["print"].containsKey("mc_print_error_code"))
        {
            if (incomingJson["print"]["mc_print_error_code"].is<int>())
                bambuStatus.mc_print_error_code = incomingJson["print"]["mc_print_error_code"].as<int>();
        }

        if (incomingJson["print"].containsKey("mc_print_line_number"))
        {
            if (incomingJson["print"]["mc_print_line_number"].is<String>())
            {
                String mc_print_line_number_str = incomingJson["print"]["mc_print_line_number"].as<String>();
                if (mc_print_line_number_str != "")
                {
                    bambuStatus.mc_print_line_number = atoi(mc_print_line_number_str.c_str());
                }
            }
        }

        if (incomingJson["print"].containsKey("print_error"))
        {
            if (incomingJson["print"]["print_error"].is<int>())
            {
                char prefix_str[9];
                bambuStatus.print_error = incomingJson["print"]["print_error"].as<int>();
                sprintf(prefix_str, "%08X", bambuStatus.print_error);

                if (xtouch_errors_isKeyPresent(prefix_str, device_error_keys, device_error_length))
                {
                    hms_enqueue(incomingJson["print"]["print_error"].as<unsigned long long>());
                    xtouch_mqtt_sendMsg(XTOUCH_ON_ERROR, 0);
                }
            }
        }

        // #pragma endregion

        // #pragma region online
        // #pragma endregion

        // #pragma region print_task
        if (incomingJson["print"].containsKey("printer_type"))
        {
            strcpy(bambuStatus.printer_type, xtouch_mqtt_parse_printer_type(incomingJson["print"]["printer_type"].as<String>()).c_str());
        }

        if (incomingJson["print"].containsKey("subtask_name"))
        {
            strcpy(bambuStatus.subtask_name, incomingJson["print"]["subtask_name"]);
        }

        if (incomingJson["print"].containsKey("layer_num"))
        {
            bambuStatus.current_layer = incomingJson["print"]["layer_num"].as<int>();
        }

        if (incomingJson["print"].containsKey("total_layer_num"))
        {
            bambuStatus.total_layers = incomingJson["print"]["total_layer_num"].as<int>();
        }

        if (incomingJson["print"].containsKey("gcode_state"))
        {
            xtouch_device_set_print_state(incomingJson["print"]["gcode_state"].as<String>());
            xtouch_mqtt_sendMsg(XTOUCH_ON_PRINT_STATUS); /* ポーズ等の状態変更をすぐUIへ */
        }

        if (incomingJson["print"].containsKey("queue_number"))
        {
            bambuStatus.queue_number = incomingJson["print"]["queue_number"].as<int>();
        }

        if (incomingJson["print"].containsKey("task_id"))
        {
            String new_tid_str = incomingJson["print"]["task_id"].as<String>();
            const char *new_tid = new_tid_str.c_str();
#ifdef __XTOUCH_SCREEN_50__
            if (new_tid[0] && strcmp(bambuStatus.task_id, new_tid) != 0)
            {
                /* TaskID が変わったら既存 URL を捨て、新しい Task のサムネ取得をキックする。 */
                bambuStatus.image_url[0] = '\0';
                if (cloud.loggedIn)
                {
                    char thumb_url[1024];
                    if (cloud.getTaskThumbnailUrl(new_tid, thumb_url, sizeof(thumb_url)))
                    {
                        strncpy(bambuStatus.image_url, thumb_url, sizeof(bambuStatus.image_url) - 1);
                        bambuStatus.image_url[sizeof(bambuStatus.image_url) - 1] = '\0';
                        /* Home/Printers 両方のスロットを一度取り直すようスケジュール */
                        xtouch_thumbnail_schedule_fetch_all();
                    }
                }
            }
#endif
            strncpy(bambuStatus.task_id, new_tid, sizeof(bambuStatus.task_id) - 1);
            bambuStatus.task_id[sizeof(bambuStatus.task_id) - 1] = '\0';
        }

        if (incomingJson["print"].containsKey("gcode_file"))
        {
            strcpy(bambuStatus.gcode_file, incomingJson["print"]["gcode_file"]);
            xtouch_mqtt_sendMsg(XTOUCH_ON_FILENAME_UPDATE, 0);
        }

        if (incomingJson["print"].containsKey("gcode_file_prepare_percent"))
        {
            String percent_str = incomingJson["print"]["gcode_file_prepare_percent"].as<String>();
            if (percent_str != "")
            {
                bambuStatus.gcode_file_prepare_percent = atoi(percent_str.c_str());
            }
        }

        if (incomingJson["print"].containsKey("project_id") && incomingJson["print"].containsKey("profile_id") && incomingJson["print"].containsKey("subtask_id"))
        {
            String obj_subtask_id_string = incomingJson["print"]["subtask_id"].as<String>();
            strcpy(bambuStatus.obj_subtask_id, obj_subtask_id_string.c_str());

            int plate_index = -1;
            /* parse local plate_index from task */
            if (obj_subtask_id_string == "0" && incomingJson["print"]["profile_id"].as<String>() != "0")
            {
                if (incomingJson["print"].containsKey("gcode_file"))
                {
                    String gcode_file_string = incomingJson["print"]["gcode_file"].as<String>();
                    strcpy(bambuStatus.gcode_file, incomingJson["print"]["gcode_file"]);

                    int idx_start = gcode_file_string.lastIndexOf("_") + 1;
                    int idx_end = gcode_file_string.lastIndexOf(".");
                    if (idx_start > 0 && idx_end > idx_start)
                    {
                        plate_index = atoi(gcode_file_string.substring(idx_start, idx_end - idx_start).c_str());
                        bambuStatus.plate_index = plate_index;
                    }
                }
            }
            xtouch_mqtt_update_slice_info(incomingJson["print"]["project_id"], incomingJson["print"]["profile_id"], incomingJson["print"]["subtask_id"], plate_index);

            {
                String new_tid_str = incomingJson["print"]["subtask_id"].as<String>();
                const char *new_tid = new_tid_str.c_str();
#ifdef __XTOUCH_SCREEN_50__
                if (new_tid[0] && strcmp(bambuStatus.task_id, new_tid) != 0)
                    bambuStatus.image_url[0] = '\0';
#endif
                strncpy(bambuStatus.task_id, new_tid, sizeof(bambuStatus.task_id) - 1);
                bambuStatus.task_id[sizeof(bambuStatus.task_id) - 1] = '\0';
            }
        }
#ifdef __XTOUCH_SCREEN_50__
        /* サムネイル取得用。LAN モードでは cloud 未ログインのため task_id だけでは取得不可。url があれば使用。task_id/subtask_id 処理の後に実行。 */
        if (incomingJson["print"].containsKey("url"))
        {
            const char *url = incomingJson["print"]["url"].as<const char *>();
            if (url && url[0])
            {
                strncpy(bambuStatus.image_url, url, sizeof(bambuStatus.image_url) - 1);
                bambuStatus.image_url[sizeof(bambuStatus.image_url) - 1] = '\0';
            }
        }
#endif
        // #pragma region print_task

        // #pragma region status

        if (incomingJson["print"].containsKey("bed_temper"))
        {
            bambuStatus.bed_temper = incomingJson["print"]["bed_temper"].as<double>();
            xtouch_mqtt_sendMsg(XTOUCH_ON_BED_TEMP, bambuStatus.bed_temper);
        }

        if (incomingJson["print"].containsKey("bed_target_temper"))
        {
            bambuStatus.bed_target_temper = incomingJson["print"]["bed_target_temper"].as<double>();
            xtouch_mqtt_sendMsg(XTOUCH_ON_BED_TARGET_TEMP, bambuStatus.bed_target_temper);
        }

        if (incomingJson["print"].containsKey("frame_temper"))
        {
            bambuStatus.frame_temp = incomingJson["print"]["frame_temper"].as<double>();
        }

        if (incomingJson["print"].containsKey("nozzle_temper"))
        {
            bambuStatus.nozzle_temper = incomingJson["print"]["nozzle_temper"].as<double>();
            xtouch_mqtt_sendMsg(XTOUCH_ON_NOZZLE_TEMP, bambuStatus.nozzle_temper);
        }

        if (incomingJson["print"].containsKey("nozzle_target_temper"))
        {
            bambuStatus.nozzle_target_temper = incomingJson["print"]["nozzle_target_temper"].as<double>();
            xtouch_mqtt_sendMsg(XTOUCH_ON_NOZZLE_TARGET_TEMP, bambuStatus.nozzle_target_temper);
        }

        if (incomingJson["print"].containsKey("chamber_temper") && !xTouchConfig.xTouchChamberSensorEnabled)
        {
            bambuStatus.chamber_temper = incomingJson["print"]["chamber_temper"].as<double>();
            xtouch_mqtt_sendMsg(XTOUCH_ON_CHAMBER_TEMP, bambuStatus.chamber_temper);
        }

        // link_th
        // link_ams
        if (incomingJson["print"].containsKey("wifi_signal"))
        {
            String wifi_signal = incomingJson["print"]["wifi_signal"].as<String>();
            wifi_signal.replace("dBm", "");
            bambuStatus.wifi_signal = abs(wifi_signal.toInt());
            xtouch_mqtt_sendMsg(XTOUCH_ON_WIFI_SIGNAL, bambuStatus.wifi_signal);
        }

        if (incomingJson["print"].containsKey("fan_gear"))
        {
            uint32_t fan_gear = incomingJson["print"]["fan_gear"].as<uint32_t>();
            bambuStatus.cooling_fan_speed = (int)((fan_gear & 0x000000FF) >> 0);
            bambuStatus.big_fan1_speed = (int)((fan_gear & 0x0000FF00) >> 8);
            bambuStatus.big_fan2_speed = (int)((fan_gear & 0x00FF0000) >> 16);
            xtouch_mqtt_sendMsg(XTOUCH_ON_PART_FAN_SPEED, bambuStatus.cooling_fan_speed);
            xtouch_mqtt_sendMsg(XTOUCH_ON_PART_AUX_SPEED, bambuStatus.big_fan1_speed);
            xtouch_mqtt_sendMsg(XTOUCH_ON_PART_CHAMBER_SPEED, bambuStatus.big_fan2_speed);
        }
        else
        {
            if (incomingJson["print"].containsKey("cooling_fan_speed"))
            {
                int speed = incomingJson["print"]["cooling_fan_speed"].as<int>();
                bambuStatus.cooling_fan_speed = round(floor(speed / float(1.5)) * float(25.5));
                xtouch_mqtt_sendMsg(XTOUCH_ON_PART_FAN_SPEED, bambuStatus.cooling_fan_speed);
            }

            if (incomingJson["print"].containsKey("big_fan1_speed"))
            {
                int speed = incomingJson["print"]["big_fan1_speed"].as<int>();
                bambuStatus.big_fan1_speed = round(floor(speed / float(1.5)) * float(25.5));
                xtouch_mqtt_sendMsg(XTOUCH_ON_PART_AUX_SPEED, bambuStatus.big_fan1_speed);
            }

            if (incomingJson["print"].containsKey("big_fan2_speed"))
            {
                int speed = incomingJson["print"]["big_fan2_speed"].as<int>();
                bambuStatus.big_fan2_speed = round(floor(speed / float(1.5)) * float(25.5));
                xtouch_mqtt_sendMsg(XTOUCH_ON_PART_CHAMBER_SPEED, bambuStatus.big_fan2_speed);
            }
        }

        // heatbreak_fan_speed

        if (incomingJson["print"].containsKey("spd_lvl"))
        {
            bambuStatus.printing_speed_lvl = incomingJson["print"]["spd_lvl"].as<int>();
        }

        if (incomingJson["print"].containsKey("spd_mag"))
        {
            bambuStatus.printing_speed_mag = incomingJson["print"]["spd_mag"].as<int>();
        }

        // stg
        // stg_cur
        // filam_bak
        // mess_production_state
        // lifecycle

        if (incomingJson["print"].containsKey("lights_report"))
        {

            if (incomingJson["print"]["lights_report"][0].containsKey("mode"))
            {
                struct XTOUCH_MESSAGE_DATA eventData;
                if (incomingJson["print"]["lights_report"][0]["mode"] == "on")
                {
                    bambuStatus.chamberLed = true;
                    eventData.data = 1;
                }
                else
                {
                    bambuStatus.chamberLed = false;
                    eventData.data = 0;
                }
                lv_msg_send(XTOUCH_ON_LIGHT_REPORT, &eventData);
            }
        }

        // sdcard

        // #pragma endregion

        if (incomingJson["print"].containsKey("nozzle_diameter"))
        {
            if (incomingJson["print"]["nozzle_diameter"].is<float>())
            {
                bambuStatus.nozzle_diameter = incomingJson["print"]["nozzle_diameter"].as<float>();
            }
            else if (incomingJson["print"]["nozzle_diameter"].is<String>())
            {
                bambuStatus.nozzle_diameter = incomingJson["print"]["nozzle_diameter"].as<String>().toFloat();
            }
        }

        if (incomingJson["print"].containsKey("nozzle_type"))
        {
            if (incomingJson["print"]["nozzle_type"].is<String>())
            {
                strcpy(bambuStatus.nozzle_type, incomingJson["print"]["nozzle_type"].as<String>().c_str());
            }
        }


        // #pragma region upgrade
        // #pragma endregion

        // #pragma region  camera
        if (incomingJson["print"].containsKey("ipcam"))
        {

            if (incomingJson["print"]["ipcam"].containsKey("ipcam_record"))
            {
                bambuStatus.camera_recording_when_printing = incomingJson["ipcam"]["ipcam_record"].as<String>() == "enable";
            }
            if (incomingJson["print"]["ipcam"].containsKey("timelapse"))
            {
                bambuStatus.camera_timelapse = incomingJson["print"]["ipcam"]["timelapse"].as<String>() == "enable";
            }
            if (incomingJson["print"]["ipcam"].containsKey("ipcam_dev"))
            {
                bambuStatus.has_ipcam = incomingJson["print"]["ipcam"]["ipcam_dev"].as<String>() == "1";
            }
            xtouch_mqtt_sendMsg(XTOUCH_ON_IPCAM);
        }

        // xcam

        // #pragma endregion

        // #pragma region hms
        if (incomingJson["print"].containsKey("hms"))
        {
            if (incomingJson["print"]["hms"].is<JsonArray>())
            {

                for (JsonVariant value : incomingJson["print"]["hms"].as<JsonArray>())
                {
                    JsonObject element = value.as<JsonObject>();
                    unsigned attr = element["attr"].as<unsigned>();
                    unsigned code = element["code"].as<unsigned>();
                    int module_id;
                    unsigned module_num;
                    unsigned part_id;
                    unsigned reserved;
                    int msg_level;
                    int msg_code;
                    unsigned int model_id_int = (attr >> 24) & 0xFF;
                    if (model_id_int < MODULE_MAX)
                        module_id = model_id_int;
                    else
                        module_id = MODULE_UKNOWN;
                    module_num = (attr >> 16) & 0xFF;
                    part_id = (attr >> 8) & 0xFF;
                    reserved = (attr >> 0) & 0xFF;
                    unsigned msg_level_int = code >> 16;
                    if (msg_level_int < HMS_MSG_LEVEL_MAX)
                        msg_level = msg_level_int;
                    else
                        msg_level = HMS_UNKNOWN;
                    msg_code = code & 0xFFFF;

                    char buffer[17];
                    sprintf(buffer, "%02X%02X%02X00000%01X%04X",
                            module_id,
                            module_num,
                            part_id,
                            msg_level,
                            msg_code);

                    char *endPtr;

                    unsigned long long intValue = strtoull(buffer, &endPtr, 16);

                    if (xtouch_errors_isKeyPresent(buffer, hms_error_values, hms_error_length))
                    {
                        hms_enqueue(intValue);
                        xtouch_mqtt_sendMsg(XTOUCH_ON_ERROR, 0);
                    }
                }
            }
        }

        // #pragma endregion
        // printf("process push_ams\n");
        // serializeJson(incomingJson["print"]["ams"], Serial);

        // #pragma region push_ams
        if (incomingJson["print"].containsKey("ams"))
        {
            // amsStatus.processAmsStatus(incomingJson["print"].as<JsonObject>());

            if (incomingJson["print"]["ams"].containsKey("ams_exist_bits"))
            {
                bambuStatus.ams_exist_bits = incomingJson["print"]["ams"]["ams_exist_bits"].as<String>().toInt();
            }

            if (incomingJson["print"].containsKey("ams_status"))
            {
                int ams_status = incomingJson["print"]["ams_status"].as<int>();
                xtouch_ams_parse_status(ams_status);
            }

            if (incomingJson["print"]["ams"].containsKey("tray_pre"))
            {
                bambuStatus.m_tray_pre = incomingJson["print"]["ams"]["tray_pre"].as<int>();
            }

            if (incomingJson["print"]["ams"].containsKey("tray_now"))
            {
                bambuStatus.m_tray_now = incomingJson["print"]["ams"]["tray_now"].as<int>();
                /* ロード完了: tray_now が 0-15 または 254 なら IDLE に戻す（プリンターが ams_status を送らない場合の保険） */
                if ((bambuStatus.m_tray_now >= 0 && bambuStatus.m_tray_now <= 15) || bambuStatus.m_tray_now == 254)
                {
                    if (bambuStatus.ams_status_main == AMS_STATUS_MAIN_FILAMENT_CHANGE)
                        bambuStatus.ams_status_main = AMS_STATUS_MAIN_IDLE;
                }
            }
            if (incomingJson["print"]["ams"].containsKey("ams"))
            {

                JsonArray ams_list = incomingJson["print"]["ams"]["ams"].as<JsonArray>();
                bambuStatus.ams = ams_list.size() > 0;
                xtouch_mqtt_sendMsg(XTOUCH_ON_AMS, ams_list.size() > 0 ? 1 : 0);

                char color[16];
                char traytype[16];

                for (uint8_t ams_idx = 0; ams_idx < ams_list.size(); ams_idx++)
                {

                    if (ams_list[ams_idx].containsKey("humidity"))
                    {
                        bambuStatus.ams_humidity[ams_idx] = 6 - ams_list[ams_idx]["humidity"].as<int>();
                        // printf("AMS humidity: %d\n", bambuStatus.ams_humidity);
                        xtouch_mqtt_sendMsg(XTOUCH_ON_AMS_HUMIDITY_UPDATE, 0);
                    }

                    if (ams_list[ams_idx].containsKey("temp"))
                    {
                        bambuStatus.ams_temperature[ams_idx] = ams_list[ams_idx]["temp"].as<float>();
                        // printf("AMS temp: %f\n", bambuStatus.ams_temperature);
                        xtouch_mqtt_sendMsg(XTOUCH_ON_AMS_TEMPERATURE_UPDATE, 0);
                    }

                    JsonArray trays = ams_list[ams_idx]["tray"].as<JsonArray>();
                    for (uint8_t tray_idx = 0; tray_idx < trays.size(); tray_idx++)
                    {
                        memset(color, 0, 16);
                        memset(traytype, 0, 16);
                        /* cols が存在し要素があればフィラメントあり。無い or 空なら空スロット（id のみの payload）。 */
                        int loaded = 0;
                        if (trays[tray_idx].containsKey("cols") && trays[tray_idx]["cols"].is<JsonArray>())
                        {
                            JsonArray cols = trays[tray_idx]["cols"].as<JsonArray>();
                            if (cols.size() > 0)
                                loaded = 1;
                        }
                        if (!loaded)
                        {
                            xtouch_mqtt_parse_tray(ams_idx, tray_idx, color, 0);
                            set_tray_type(ams_idx, tray_idx, traytype);
                            set_tray_color(ams_idx, tray_idx, color);
                            set_tray_setting_id(ams_idx, tray_idx, "");
                            continue;
                        }
                        if (trays[tray_idx].containsKey("tray_color"))
                            trays[tray_idx]["tray_color"].as<String>().toCharArray(color, 16);
                        if (trays[tray_idx].containsKey("tray_type"))
                            trays[tray_idx]["tray_type"].as<String>().toCharArray(traytype, 16);
                        color[6] = 0;
                        xtouch_mqtt_parse_tray(ams_idx, tray_idx, color, 1);
                        /* PushAll 等で type/color が含まれない payload のときは既存表示を維持（空で上書きしない） */
                        if (traytype[0] != '\0')
                            set_tray_type(ams_idx, tray_idx, traytype);
                        if (color[0] != '\0')
                            set_tray_color(ams_idx, tray_idx, color);
                        if (trays[tray_idx].containsKey("tray_info_idx"))
                        {
                            char sid[TRAY_SETTING_ID_LEN];
                            memset(sid, 0, sizeof(sid));
                            trays[tray_idx]["tray_info_idx"].as<String>().toCharArray(sid, TRAY_SETTING_ID_LEN);
                            set_tray_setting_id(ams_idx, tray_idx, sid);
                        }

                        int nt_min = 0, nt_max = 0;
                        if (trays[tray_idx].containsKey("nozzle_temp_min"))
                            nt_min = trays[tray_idx]["nozzle_temp_min"].as<int>();
                        if (trays[tray_idx].containsKey("nozzle_temp_max"))
                            nt_max = trays[tray_idx]["nozzle_temp_max"].as<int>();
                        if (nt_min == 0 && trays[tray_idx].containsKey("nozzle_temperature_range_low"))
                        {
                            JsonVariant v = trays[tray_idx]["nozzle_temperature_range_low"];
                            if (v.is<JsonArray>() && v.as<JsonArray>().size() > 0)
                            {
                                JsonVariant first = v.as<JsonArray>()[0];
                                nt_min = first.is<int>() ? first.as<int>() : first.as<String>().toInt();
                            }
                            else
                                nt_min = v.as<int>();
                        }
                        if (nt_max == 0 && trays[tray_idx].containsKey("nozzle_temperature_range_high"))
                        {
                            JsonVariant v = trays[tray_idx]["nozzle_temperature_range_high"];
                            if (v.is<JsonArray>() && v.as<JsonArray>().size() > 0)
                            {
                                JsonVariant first = v.as<JsonArray>()[0];
                                nt_max = first.is<int>() ? first.as<int>() : first.as<String>().toInt();
                            }
                            else
                                nt_max = v.as<int>();
                        }
                        // Serial.printf("[MQTT parse] ams=%u tray=%u nozzle_temp_min=%d nozzle_temp_max=%d (keys: min=%d max=%d range_low=%d range_high=%d)\n",
                        //     (unsigned)ams_idx, (unsigned)tray_idx,
                        //     nt_min, nt_max,
                        //     trays[tray_idx].containsKey("nozzle_temp_min") ? 1 : 0,
                        //     trays[tray_idx].containsKey("nozzle_temp_max") ? 1 : 0,
                        //     trays[tray_idx].containsKey("nozzle_temperature_range_low") ? 1 : 0,
                        //     trays[tray_idx].containsKey("nozzle_temperature_range_high") ? 1 : 0);
                        set_tray_temp(ams_idx, tray_idx, (nt_max + nt_min) / 2);
                        set_tray_temp_min_max(ams_idx, tray_idx, (uint16_t)nt_min, (uint16_t)nt_max);
                    }
                }

                if (incomingJson["print"]["ams"].containsKey("ams_exist_bits"))
                {
                    bambuStatus.ams_exist_bits = incomingJson["print"]["ams"]["ams_exist_bits"].as<String>().toInt();
                }
                if (incomingJson["print"]["ams"].containsKey("tray_exist_bits"))
                {
                    bambuStatus.tray_exist_bits = incomingJson["print"]["ams"]["tray_exist_bits"].as<String>().toInt();
                }
                if (incomingJson["print"]["ams"].containsKey("tray_read_done_bits"))
                {
                    bambuStatus.tray_read_done_bits = incomingJson["print"]["ams"]["tray_read_done_bits"].as<String>().toInt();
                }
                if (incomingJson["print"]["ams"].containsKey("tray_reading_bits"))
                {
                    bambuStatus.tray_reading_bits = incomingJson["print"]["ams"]["tray_reading_bits"].as<String>().toInt();
                    bambuStatus.ams_support_use_ams = true;
                }
                if (incomingJson["print"]["ams"].containsKey("tray_is_bbl_bits"))
                {
                    bambuStatus.tray_is_bbl_bits = incomingJson["print"]["ams"]["tray_is_bbl_bits"].as<String>().toInt();
                }
                if (incomingJson["print"]["ams"].containsKey("version"))
                {
                    if (incomingJson["print"]["ams"]["version"].is<int>())
                    {

                        bambuStatus.ams_version = incomingJson["print"]["ams"]["version"].as<int>();
                    }
                }

                if (incomingJson["print"]["ams"].containsKey("tray_tar"))
                {
                    bambuStatus.m_tray_tar = incomingJson["print"]["ams"]["tray_tar"].as<int>();
                }

                if (incomingJson["print"]["ams"].containsKey("ams_rfid_status"))
                {
                    bambuStatus.ams_rfid_status = incomingJson["print"]["ams"]["ams_rfid_status"].as<int>();
                }

                if (incomingJson["print"]["ams"].containsKey("humidity"))
                {
                    if (incomingJson["print"]["ams"]["humidity"].is<String>())
                    {
                        String humidity_str = incomingJson["print"]["ams"]["humidity"].as<String>();

                        bambuStatus.ams_humidity[0] = atoi(humidity_str.c_str());
                    }
                }
                if (incomingJson["print"]["ams"].containsKey("insert_flag") || incomingJson["print"]["ams"].containsKey("power_on_flag") || incomingJson["print"]["ams"].containsKey("calibrate_remain_flag"))
                {
                    if (bambuStatus.ams_user_setting_hold_count > 0)
                    {
                        bambuStatus.ams_user_setting_hold_count--;
                    }
                    else
                    {
                        if (incomingJson["print"]["ams"].containsKey("insert_flag"))
                        {
                            bambuStatus.ams_insert_flag = incomingJson["print"]["ams"]["insert_flag"].as<bool>();
                        }
                        if (incomingJson["print"]["ams"].containsKey("power_on_flag"))
                        {
                            bambuStatus.ams_power_on_flag = incomingJson["print"]["ams"]["power_on_flag"].as<bool>();
                        }
                        if (incomingJson["print"]["ams"].containsKey("calibrate_remain_flag"))
                        {
                            bambuStatus.ams_calibrate_remain_flag = incomingJson["print"]["ams"]["calibrate_remain_flag"].as<bool>();
                        }
                    }
                }
            }

            xtouch_mqtt_sendMsg(XTOUCH_ON_AMS_BITS, 0);
            xtouch_mqtt_sendMsg(XTOUCH_ON_AMS_STATE_UPDATE, 0);
            xtouch_mqtt_sendMsg(XTOUCH_ON_AMS_SLOT_UPDATE, 0);
            // printf("AMS status main %d\n", bambuStatus.ams_status_main);
            // printf("AMS status sub  %d\n", bambuStatus.ams_status_sub);
            // printf("AMS tray now  %d\n", bambuStatus.m_tray_now);
        }

        /* vt_tray: 254=External */
        if (incomingJson["print"].containsKey("vt_tray"))
        {
            bambuStatus.ams_support_virtual_tray = true;
            char color[16];
            char traytype[16];
            memset(color, 0, 16);
            memset(traytype, 0, 16);
            incomingJson["print"]["vt_tray"]["tray_color"].as<String>().toCharArray(color, 16);
            incomingJson["print"]["vt_tray"]["tray_type"].as<String>().toCharArray(traytype, 16);
            color[6] = 0;
            xtouch_mqtt_parse_tray(0, TRAY_ID_EXTERNAL, color, 1);
            set_tray_type(0, TRAY_ID_EXTERNAL, traytype);
            set_tray_color(0, TRAY_ID_EXTERNAL, color);
        }
        else
        {
            bambuStatus.ams_support_virtual_tray = false;
        }
        // #pragma endregion

        if (incomingJson["print"].containsKey("gcode_state") ||
            incomingJson["print"].containsKey("layer_num") ||
            incomingJson["print"].containsKey("total_layer_num") ||
            incomingJson["print"].containsKey("mc_remaining_time") ||
            incomingJson["print"].containsKey("mc_percent") ||
            incomingJson["print"].containsKey("spd_lvl") ||
            incomingJson["print"].containsKey("spd_mag"))
        {

            xtouch_mqtt_sendMsg(XTOUCH_ON_PRINT_STATUS);
        }
    }
}

void xtouch_mqtt_parseMessage(char *topic, byte *payload, unsigned int length, byte type = 0)
{

    // ConsoleDebug.println(F("[xPTouch][MQTT] ParseMessage"));
    DynamicJsonDocument incomingJson(XTOUCH_MQTT_SERVER_JSON_PARSE_SIZE);

    DynamicJsonDocument amsFilter(128);
    amsFilter["print"]["*"] = true;
    // amsFilter["camera"]["*"] = true;
    amsFilter["print"]["ams"] = true;

    auto deserializeError = deserializeJson(incomingJson, payload, length, DeserializationOption::Filter(amsFilter));

    // printf("xtouch_mqtt_parseMessage\n");
    // xtouch_debug_json(incomingJson);

    if (!deserializeError)
    {

        if ((millis() - xtouch_mqtt_lastPushStatus) > (XTOUCH_MQTT_SERVER_PUSH_STATUS_TIMEOUT * 1000))
        {
            Serial.println("[xPTouch][MQTT] Force Reconnect after no Push Status for " + String(XTOUCH_MQTT_SERVER_PUSH_STATUS_TIMEOUT) + "s");
            xtouch_pubSubClient.disconnect();
        }

#ifdef __XTOUCH_SCREEN_50__
        /* トピックから dev_id を取得: device/XXXX/report */
        char topic_dev_id[16] = {0};
        if (topic && strncmp(topic, "device/", 7) == 0)
        {
            const char *rest = topic + 7;
            const char *slash = strchr(rest, '/');
            if (slash && strcmp(slash, "/report") == 0)
            {
                size_t len = (size_t)(slash - rest);
                if (len >= sizeof(topic_dev_id))
                    len = sizeof(topic_dev_id) - 1;
                memcpy(topic_dev_id, rest, len);
                topic_dev_id[len] = '\0';
            }
        }
        if (topic_dev_id[0] != '\0' && strcmp(topic_dev_id, xTouchConfig.xTouchSerialNumber) != 0)
        {
            /* 他プリンターの report */
            int slot = -1;
            for (int i = 0; i < xtouch_other_printer_count; i++)
            {
                if (strcmp(xtouch_other_printer_dev_ids[i], topic_dev_id) == 0)
                {
                    slot = i;
                    break;
                }
            }
            if (slot >= 0 && incomingJson.containsKey("print") && incomingJson["print"].containsKey("command") &&
                incomingJson["print"]["command"].as<String>() == "push_status")
            {
                xtouch_mqtt_processPushStatusOther(slot, incomingJson);
            }
            return;
        }
#endif

        if (incomingJson.containsKey("print") && incomingJson["print"].containsKey("command"))
        {

            String command = incomingJson["print"]["command"].as<String>();
        }

        if (incomingJson.containsKey("print") && incomingJson["print"].containsKey("command"))
        {

            String command = incomingJson["print"]["command"].as<String>();

            if (command == "push_status")
            {
                xtouch_mqtt_processPushStatus(incomingJson);
#ifdef __XTOUCH_SCREEN_50__
                xtouch_thumbnail_update_path_for_slot(0);
                /* ホーム表示中に push_status で task_id 取得した場合、サムネイル取得をスケジュール（起動直後印刷中で未取得のとき） */
                if (xTouchConfig.currentScreenIndex == 0 && bambuStatus.task_id[0] && strcmp(bambuStatus.task_id, "0") != 0)
                    xtouch_thumbnail_schedule_fetch_all();
                xtouch_mqtt_sendMsg(XTOUCH_ON_OTHER_PRINTER_UPDATE, 0);
#endif
            }
            if (command == "ams_change_filament")
            {
                bambuStatus.m_tray_tar = incomingJson["target"].as<int>();
                xtouch_mqtt_sendMsg(XTOUCH_ON_AMS_SLOT_UPDATE, 0);
            }
            else if (command == "gcode_line")
            {
                ConsoleDebug.println(F("[xPTouch][MQTT] gcode_line ack"));
                ConsoleDebug.println(String((char *)payload));
            }

            // project_file
            // ams_filament_setting
            // xcam_control_set
            // print_option
            // extrusion_cali | flowrate_cali
            // extrusion_cali_set
            // extrusion_cali_sel
            // extrusion_cali_get
            // extrusion_cali_get_result
            // flowrate_get_result
        }

        // info

        if (incomingJson.containsKey("camera"))
        {
            if (incomingJson["camera"].containsKey("command"))
            {
                if (incomingJson["camera"]["command"].as<String>() == "ipcam_timelapse")
                {
                    bambuStatus.camera_timelapse = incomingJson["camera"]["control"].as<String>() == "enable";
                }
                else if (incomingJson["camera"]["command"].as<String>() == "ipcam_record_set")
                {
                    bambuStatus.camera_recording_when_printing = incomingJson["camera"]["control"].as<String>() == "enable";
                }
                xtouch_mqtt_sendMsg(XTOUCH_ON_IPCAM);
            }
        }

        // upgrade
        // event info
    }
    else
    {
        ConsoleError.println(F("[xPTouch][MQTT] ParseMessage deserializeJson failed"));
    }

    // if (firstParseMessage)
    // {
    //     firstParseMessage = false;
    //     xtouch_device_command_getPaCalibration();
    // }
}

void xtouch_pubSubClient_streamCallback(char *topic, byte *payload, unsigned int length)
{
    ConsoleDebug.print(F("[xPTouch][MQTT] RECV topic="));
    ConsoleDebug.print(topic);
    ConsoleDebug.print(F(" len="));
    ConsoleDebug.println(length);
    // xtouch_mqtt_parseMessage(topic, (byte *)stream.get_buffer(), stream.current_length(), 0);

    // if (stream.includes("\"ams\""))
    // {
    xtouch_mqtt_parseMessage(topic, (byte *)stream.get_buffer(), stream.current_length(), 1);
    // }

    stream.flush();
}

const char *xtouch_mqtt_generateRandomKey(int keyLength)
{
    char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static char key[17];

    for (int i = 0; i < keyLength; i++)
    {
        int randomIndex = random(sizeof(charset) - 1);
        key[i] = charset[randomIndex];
    }

    key[keyLength] = '\0';

    return key;
}

/** SSL(-76)/LOST_IP 後は見かけ上IPがあるが到達不能になるため、WiFi.reconnect() してから再試行する */
static void xtouch_mqtt_wifi_reconnect_and_wait(int timeout_ms)
{
    ConsoleInfo.println(F("[xPTouch][MQTT] WiFi reconnect before retry..."));
    WiFi.reconnect();
    for (int i = 0; i < (timeout_ms / 100); i++)
    {
        if (WiFi.status() == WL_CONNECTED)
            break;
        delay(100);
        lv_timer_handler();
        lv_task_handler();
        esp_task_wdt_reset();
    }
    delay(500);
}

/* 起動後はじめて MQTT 接続できたときだけホームへ。再接続では画面を変えない（不定期にロード画面に戻るのを防ぐ） */
static bool xtouch_mqtt_has_ever_connected = false;
void xtouch_mqtt_onMqttReady()
{
    if (!xtouch_mqtt_has_ever_connected)
    {
        xtouch_mqtt_has_ever_connected = true;
        loadScreen(0);
    }
    xtouch_mqtt_firstConnectionDone = true;
}

static void xtouch_mqtt_connect(const char *username, const char *password, const char *introCaption, bool clear_cloud_on_unauthorized)
{
    ConsoleInfo.println(F("[xPTouch][MQTT] Connecting"));

    if (!xtouch_mqtt_firstConnectionDone)
    {
        lv_label_set_text(introScreenCaption, introCaption);
        lv_timer_handler();
        lv_task_handler();
        delay(32);
    }

    xtouch_mqtt_firstConnectionDone = false;

    while (!xtouch_pubSubClient.connected())
    {
        String clientId = "XTouch-CLIENT-" + String(xtouch_mqtt_generateRandomKey(16));
        if (xtouch_pubSubClient.connect(clientId.c_str(), username, password))
        {
            ConsoleInfo.println(F("[xPTouch][MQTT] ---- CONNECTED ----"));

            /* メイン機の report は常に購読（1台のみのときもこれで push_status を受信） */
            ESP_LOGI("mqtt", "subscribe self report topic: %s", xtouch_mqtt_report_topic.c_str());
            xtouch_pubSubClient.subscribe(xtouch_mqtt_report_topic.c_str());
#ifdef __XTOUCH_SCREEN_50__
            for (int i = 0; i < xtouch_other_printer_count; i++)
            {
                String other_report = String("device/") + xtouch_other_printer_dev_ids[i] + "/report";
                ESP_LOGI("mqtt", "subscribe other report topic[%d]: %s", i, other_report.c_str());
                xtouch_pubSubClient.subscribe(other_report.c_str());
            }
#endif
            xtouch_device_pushall();
            xtouch_device_get_version();
            xtouch_mqtt_onMqttReady();
            xtouch_mqtt_lastPushStatus = millis();
            break;
        }
        else
        {
            ConsoleError.printf("[xPTouch][MQTT] ---- CONNECTION FAIL ----: %d\n", xtouch_pubSubClient.state());

            switch (xtouch_pubSubClient.state())
            {
            case -4: // MQTT_CONNECTION_TIMEOUT
                xtouch_mqtt_connection_timeout_count--;
                if (xtouch_mqtt_connection_timeout_count == 0)
                {
                    ESP.restart();
                }
                break;
            case -2: // MQTT_CONNECT_FAILED (Host unreachable 等)
            case -3: // MQTT_CONNECTION_LOST
            case -1: // MQTT_DISCONNECTED
                xtouch_mqtt_wifi_reconnect_and_wait(5000);
                if (xtouch_pubSubClient.state() == -2 && !xtouch_mqtt_firstConnectionDone)
                {
                    xtouch_mqtt_connection_fail_count--;
                    if (xtouch_mqtt_connection_fail_count == 0)
                    {
                        lv_label_set_text(introScreenCaption, LV_SYMBOL_WARNING " MQTT ERROR");
                        lv_timer_handler();
                        lv_task_handler();
                        delay(3000);
                        lv_label_set_text(introScreenCaption, LV_SYMBOL_REFRESH " REBOOTING");
                        lv_timer_handler();
                        lv_task_handler();
                        ESP.restart();
                    }
                }
                break;
            case 1: // MQTT BAD_PROTOCOL
            case 2: // MQTT BAD_CLIENT_ID
            case 3: // MQTT UNAVAILABLE
            case 4: // MQTT BAD_CREDENTIALS
            case 5: // MQTT UNAUTHORIZED
                if (!xtouch_mqtt_firstConnectionDone)
                {
                    lv_label_set_text(introScreenCaption, LV_SYMBOL_WARNING " MQTT ERROR");
                    lv_timer_handler();
                    lv_task_handler();
                    delay(3000);
                    lv_label_set_text(introScreenCaption, LV_SYMBOL_REFRESH " REBOOTING");
                    lv_timer_handler();
                    lv_task_handler();
                }
                if (clear_cloud_on_unauthorized)
                {
                    cloud.clearDeviceList();
                    cloud.clearPairList();
                    cloud.clearTokens();
                }
                ESP.restart();
                break;
            }
        }
        lv_timer_handler();
        lv_task_handler();
        delay(32);
        /* 接続待ちで長時間ブロックするため WDT をリセット（再起動してロード画面に戻るのを防ぐ） */
        esp_task_wdt_reset();
    }
}

void xtouch_cloud_mqtt_connect()
{
    xtouch_mqtt_connect(cloud.getUsername().c_str(), cloud.getAuthToken().c_str(),
                        LV_SYMBOL_CHARGE " Connecting to Cloud MQTT", true);
}

void xtouch_local_mqtt_connect()
{
    xtouch_mqtt_connect("bblp", xTouchConfig.xTouchAccessCode,
                        LV_SYMBOL_CHARGE " Connecting to Printer", false);
}

static void xtouch_mqtt_configure_client(const char *host)
{
    xtouch_wiFiClientSecure.flush();
    xtouch_wiFiClientSecure.stop();
    xtouch_wiFiClientSecure.setInsecure();

    xtouch_pubSubClient.setServer(host, 8883);
    xtouch_pubSubClient.setBufferSize(2048);
    xtouch_pubSubClient.setStream(stream);
    xtouch_pubSubClient.setCallback(xtouch_pubSubClient_streamCallback);
    xtouch_pubSubClient.setKeepAlive(10);
}

static void xtouch_mqtt_subscribe_commands(void)
{
    /* サイドバー Home / Light Reset / LCD Toggle もクラウド・ローカル共通で購読する */
    lv_msg_subscribe(XTOUCH_SIDEBAR_HOME, (lv_msg_subscribe_cb_t)xtouch_device_onSidebarHomeCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_LIGHT_RESET, (lv_msg_subscribe_cb_t)xtouch_device_onLightResetCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_LCD_TOGGLE, (lv_msg_subscribe_cb_t)xtouch_device_onLCDToggleCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_LIGHT_TOGGLE, (lv_msg_subscribe_cb_t)xtouch_device_onLightToggleCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_STOP, (lv_msg_subscribe_cb_t)xtouch_device_onStopCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_PAUSE, (lv_msg_subscribe_cb_t)xtouch_device_onPauseCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_RESUME, (lv_msg_subscribe_cb_t)xtouch_device_onResumeCommand, NULL);
#ifdef __XTOUCH_SCREEN_50__
    lv_msg_subscribe(XTOUCH_COMMAND_PAUSE_SLOT, (lv_msg_subscribe_cb_t)xtouch_device_onPauseSlotCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_STOP_SLOT, (lv_msg_subscribe_cb_t)xtouch_device_onStopSlotCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_RESUME_SLOT, (lv_msg_subscribe_cb_t)xtouch_device_onResumeSlotCommand, NULL);
#endif

    lv_msg_subscribe(XTOUCH_COMMAND_HOME, (lv_msg_subscribe_cb_t)xtouch_device_onHomeCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_LEFT, (lv_msg_subscribe_cb_t)xtouch_device_onLeftCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_RIGHT, (lv_msg_subscribe_cb_t)xtouch_device_onRightCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_UP, (lv_msg_subscribe_cb_t)xtouch_device_onUpCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_DOWN, (lv_msg_subscribe_cb_t)xtouch_device_onDownCommand, NULL);

    lv_msg_subscribe(XTOUCH_COMMAND_BED_UP, (lv_msg_subscribe_cb_t)xtouch_device_onBedUpCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_BED_DOWN, (lv_msg_subscribe_cb_t)xtouch_device_onBedDownCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_BED_TARGET_TEMP, (lv_msg_subscribe_cb_t)xtouch_device_onBedTargetTempCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_NOZZLE_TARGET_TEMP, (lv_msg_subscribe_cb_t)xtouch_device_onNozzleTargetCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_PART_FAN_SPEED, (lv_msg_subscribe_cb_t)xtouch_device_onPartSpeedCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_AUX_FAN_SPEED, (lv_msg_subscribe_cb_t)xtouch_device_onAuxSpeedCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_CHAMBER_FAN_SPEED, (lv_msg_subscribe_cb_t)xtouch_device_onChamberSpeedCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_PRINT_SPEED, (lv_msg_subscribe_cb_t)xtouch_device_onPrintSpeedCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_UNLOAD_FILAMENT, (lv_msg_subscribe_cb_t)xtouch_device_onUnloadFilament, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_LOAD_FILAMENT, (lv_msg_subscribe_cb_t)xtouch_device_onLoadFilament, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_AMS_CONTROL, (lv_msg_subscribe_cb_t)xtouch_device_command_ams_control, NULL);
    /* ローカル/クラウドどちらでも AMS ロード・アンロードは gcode_line で送るため常に購読 */
    lv_msg_subscribe(XTOUCH_COMMAND_AMS_LOAD_SLOT, (lv_msg_subscribe_cb_t)xtouch_device_command_ams_load, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_GCODE_M620_R, (lv_msg_subscribe_cb_t)xtouch_device_command_m620_r, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_AMS_UNLOAD_SLOT, (lv_msg_subscribe_cb_t)xtouch_device_command_ams_unload, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_AMS_REFRESH, (lv_msg_subscribe_cb_t)xtouch_device_command_ams_refresh, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_AMS_FILAMENT_SETTING, (lv_msg_subscribe_cb_t)xtouch_device_command_ams_filament_setting, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_AMS_FETCH_SLICER_TEMP, (lv_msg_subscribe_cb_t)xtouch_device_command_ams_fetch_slicer_temp, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_CLEAN_PRINT_ERROR, (lv_msg_subscribe_cb_t)xtouch_device_command_clean_print_error, NULL);

    lv_msg_subscribe(XTOUCH_COMMAND_EXTRUDE_UP, (lv_msg_subscribe_cb_t)xtouch_device_onNozzleUp, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_EXTRUDE_DOWN, (lv_msg_subscribe_cb_t)xtouch_device_onNozzleDown, NULL);
    /* UTIL → Nozzle change / Calibration はローカル操作でも使うので常に購読する */
    lv_msg_subscribe(XTOUCH_COMMAND_SET_UTIL_NOZZLE_CHANGE, (lv_msg_subscribe_cb_t)xtouch_device_onSetaccessoriesNozzleCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_SET_UTIL_CALIBRATION, (lv_msg_subscribe_cb_t)xtouch_device_onSetUtilCalibrationCommand, NULL);
}

void xtouch_cloud_mqtt_setup()
{
    lv_label_set_text(introScreenCaption, LV_SYMBOL_CHARGE " Connecting BBL Cloud");
    lv_timer_handler();
    lv_task_handler();
    delay(32);

    xtouch_mqtt_topic_setup();
#ifdef __XTOUCH_SCREEN_50__
    xtouch_mqtt_load_other_printers();
#endif
    xtouch_mqtt_configure_client(cloud.getMqttCloudHost());
    xtouch_mqtt_subscribe_commands();

    delay(2000);
}

void xtouch_local_mqtt_setup()
{
    lv_label_set_text(introScreenCaption, LV_SYMBOL_CHARGE " Connecting Printer");
    lv_timer_handler();
    lv_task_handler();
    delay(32);

    xtouch_mqtt_topic_setup();
    xtouch_mqtt_configure_client(xTouchConfig.xTouchHost);
    xtouch_mqtt_subscribe_commands();

    delay(2000);
}

// #define XTOUCH_MQTT_PUSHALL_INTERVAL_MS 5000
// static unsigned long lastPushAll = 0;
void xtouch_cloud_mqtt_loop()
{
    xtouch_pubSubClient.loop();
    if (!xtouch_pubSubClient.connected())
    {
        Serial.println("[xPTouch][MQTT] -----DISCONNECTED-----");
        xtouch_mqtt_wifi_reconnect_and_wait(5000);
        if(xTouchConfig.xTouchLanOnlyMode){
            xtouch_local_mqtt_connect();
        }else{
            xtouch_cloud_mqtt_connect();
        }

        return;
    }

    // unsigned long now = millis();
    // /* オーバーフロー対策: millis() は約49日で巻き戻る。そのときは基準をリセット */
    // if (now < lastPushAll)
    //     lastPushAll = now;
    // if (now - lastPushAll >= XTOUCH_MQTT_PUSHALL_INTERVAL_MS)
    // {
    //     Serial.println("lastPushAll: " + String(lastPushAll) + " now: " + String(now) + " interval: " + String(now - lastPushAll) + " interval: " + String(XTOUCH_MQTT_PUSHALL_INTERVAL_MS));
    //     lastPushAll = now;
    //     xtouch_device_pushall();
    // }
}

#endif