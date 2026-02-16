#ifndef _XLCD_MQTT
#define _XLCD_MQTT

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
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
#define XTOUCH_MQTT_SERVER_TIMEOUT 20
#define XTOUCH_MQTT_SERVER_PUSH_STATUS_TIMEOUT 1800
#define XTOUCH_MQTT_SERVER_JSON_PARSE_SIZE 8192

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
        if (incomingJson["print"].containsKey("url"))
        {
            strcpy(bambuStatus.image_url, incomingJson["print"]["url"]);
        }

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
        }

        if (incomingJson["print"].containsKey("queue_number"))
        {
            bambuStatus.queue_number = incomingJson["print"]["queue_number"].as<int>();
        }

        if (incomingJson["print"].containsKey("task_id"))
        {
            strcpy(bambuStatus.task_id, incomingJson["print"]["task_id"]);
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

            strcpy(bambuStatus.task_id, incomingJson["print"]["subtask_id"]);
        }
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
                        trays[tray_idx]["tray_color"].as<String>().toCharArray(color, 16);
                        trays[tray_idx]["tray_type"].as<String>().toCharArray(traytype, 16);

                        color[6] = 0;

                        xtouch_mqtt_parse_tray(ams_idx, tray_idx + 1, color, trays[tray_idx]["n"].as<int>());

                        set_tray_type(ams_idx, tray_idx + 1, traytype);

                        set_tray_temp(ams_idx, tray_idx + 1, (trays[tray_idx]["nozzle_temp_max"].as<int>() + trays[tray_idx]["nozzle_temp_min"].as<int>()) / 2);
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

        // vt_tray
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
            xtouch_mqtt_parse_tray(0, (uint8_t)incomingJson["print"]["vt_tray"]["n"].as<int>(), color, 0);

            set_tray_type(0, 0, traytype);
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

void xtouch_mqtt_onMqttReady()
{
    if (!xtouch_mqtt_firstConnectionDone)
    {
        loadScreen(0);
    }
    xtouch_mqtt_firstConnectionDone = true;
}

void xtouch_cloud_mqtt_connect()
{

    ConsoleInfo.println(F("[xPTouch][MQTT] Connecting"));

    if (!xtouch_mqtt_firstConnectionDone)
    {
        lv_label_set_text(introScreenCaption, LV_SYMBOL_CHARGE " Connecting to Cloud MQTT");
        lv_timer_handler();
        lv_task_handler();
        delay(32);
    }

    xtouch_mqtt_firstConnectionDone = false;

    while (!xtouch_pubSubClient.connected())
    {
        String clientId = "XTouch-CLIENT-" + String(xtouch_mqtt_generateRandomKey(16));
        if (xtouch_pubSubClient.connect(clientId.c_str(), cloud.getUsername().c_str(), cloud.getAuthToken().c_str()))
        {
            ConsoleInfo.println(F("[xPTouch][MQTT] ---- CONNECTED ----"));

            xtouch_pubSubClient.subscribe(xtouch_mqtt_report_topic.c_str());
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
            case -2: // MQTT_CONNECT_FAILED

                if (!xtouch_mqtt_firstConnectionDone)
                {
                    xtouch_mqtt_connection_fail_count--;
                    if (xtouch_mqtt_connection_fail_count == 0)
                    {
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
                        ESP.restart();
                    }
                }
                break;
            case -3: // MQTT_CONNECTION_LOST
            case -1: // MQTT_DISCONNECTED

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
                cloud.clearDeviceList();
                cloud.clearPairList();
                cloud.clearTokens();
                ESP.restart();

                break;
            };
        }
        lv_timer_handler();
        lv_task_handler();
        delay(32);
    }
}

void xtouch_local_mqtt_connect()
{

    ConsoleInfo.println(F("[XTouch][MQTT] Connecting"));

    if (!xtouch_mqtt_firstConnectionDone)
    {
        lv_label_set_text(introScreenCaption, LV_SYMBOL_CHARGE " Connecting to Cloud MQTT");
        lv_timer_handler();
        lv_task_handler();
        delay(32);
    }

    xtouch_mqtt_firstConnectionDone = false;

    while (!xtouch_pubSubClient.connected())
    {
        String clientId = "XTOUCH-CLIENT-" + String(xtouch_mqtt_generateRandomKey(16));
        if (xtouch_pubSubClient.connect("clientId.c_str()", "bblp", xTouchConfig.xTouchAccessCode))
        {
            ConsoleInfo.println(F("[XTouch][MQTT] ---- CONNECTED ----"));

            xtouch_pubSubClient.subscribe(xtouch_mqtt_report_topic.c_str());
            xtouch_device_pushall();
            xtouch_device_get_version();
            xtouch_mqtt_onMqttReady();
            xtouch_mqtt_lastPushStatus = millis();
            break;
        }
        else
        {
            ConsoleError.printf("[XTouch][MQTT] ---- CONNECTION FAIL ----: %d\n", xtouch_pubSubClient.state());

            switch (xtouch_pubSubClient.state())
            {

            case -4: // MQTT_CONNECTION_TIMEOUT
                xtouch_mqtt_connection_timeout_count--;
                if (xtouch_mqtt_connection_timeout_count == 0)
                {
                    ESP.restart();
                }
                break;
            case -2: // MQTT_CONNECT_FAILED

                if (!xtouch_mqtt_firstConnectionDone)
                {
                    xtouch_mqtt_connection_fail_count--;
                    if (xtouch_mqtt_connection_fail_count == 0)
                    {
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
                        ESP.restart();
                    }
                }
                break;
            case -3: // MQTT_CONNECTION_LOST
            case -1: // MQTT_DISCONNECTED

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
                // cloud.clearDeviceList();
                // cloud.clearPairList();
                // cloud.clearTokens();
                ESP.restart();

                break;
            };
        }
        lv_timer_handler();
        lv_task_handler();
        delay(32);
    }
}

void xtouch_cloud_mqtt_setup()
{
    lv_label_set_text(introScreenCaption, LV_SYMBOL_CHARGE " Connecting BBL Cloud");
    lv_timer_handler();
    lv_task_handler();
    delay(32);

    xtouch_mqtt_topic_setup();

    xtouch_wiFiClientSecure.flush();
    xtouch_wiFiClientSecure.stop();

    // xtouch_wiFiClientSecure.setCACert(cloud.getRegion() == "China" ? cn_mqtt_bambulab_com : us_mqtt_bambulab_com);
    xtouch_wiFiClientSecure.setInsecure();

    xtouch_pubSubClient.setServer(cloud.getMqttCloudHost(), 8883);
    xtouch_pubSubClient.setBufferSize(2048); // 2KB for mqtt message JWT output
    xtouch_pubSubClient.setStream(stream);
    xtouch_pubSubClient.setCallback(xtouch_pubSubClient_streamCallback);
    xtouch_pubSubClient.setKeepAlive(10);

    /* home */
    lv_msg_subscribe(XTOUCH_SIDEBAR_HOME, (lv_msg_subscribe_cb_t)xtouch_device_onSidebarHomeCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_LIGHT_RESET, (lv_msg_subscribe_cb_t)xtouch_device_onLightResetCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_LIGHT_TOGGLE, (lv_msg_subscribe_cb_t)xtouch_device_onLightToggleCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_LCD_TOGGLE, (lv_msg_subscribe_cb_t)xtouch_device_onLCDToggleCommand, NULL);

    lv_msg_subscribe(XTOUCH_COMMAND_STOP, (lv_msg_subscribe_cb_t)xtouch_device_onStopCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_PAUSE, (lv_msg_subscribe_cb_t)xtouch_device_onPauseCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_RESUME, (lv_msg_subscribe_cb_t)xtouch_device_onResumeCommand, NULL);

    /* control */
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
    lv_msg_subscribe(XTOUCH_COMMAND_AMS_LOAD_SLOT, (lv_msg_subscribe_cb_t)xtouch_device_command_ams_load, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_AMS_UNLOAD_SLOT, (lv_msg_subscribe_cb_t)xtouch_device_command_ams_unload, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_CLEAN_PRINT_ERROR, (lv_msg_subscribe_cb_t)xtouch_device_command_clean_print_error, NULL);

    /* filament */
    lv_msg_subscribe(XTOUCH_COMMAND_EXTRUDE_UP, (lv_msg_subscribe_cb_t)xtouch_device_onNozzleUp, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_EXTRUDE_DOWN, (lv_msg_subscribe_cb_t)xtouch_device_onNozzleDown, NULL);

    lv_msg_subscribe(XTOUCH_COMMAND_SET_UTIL_NOZZLE_CHANGE, (lv_msg_subscribe_cb_t)xtouch_device_onSetaccessoriesNozzleCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_SET_UTIL_CALIBRATION, (lv_msg_subscribe_cb_t)xtouch_device_onSetUtilCalibrationCommand, NULL);


    delay(2000);
}
void xtouch_local_mqtt_setup()
{
    lv_label_set_text(introScreenCaption, LV_SYMBOL_CHARGE " Connecting Printer");
    lv_timer_handler();
    lv_task_handler();
    delay(32);

    xtouch_mqtt_topic_setup();

    xtouch_wiFiClientSecure.flush();
    xtouch_wiFiClientSecure.stop();

    xtouch_wiFiClientSecure.setInsecure();

    xtouch_pubSubClient.setServer(xTouchConfig.xTouchHost, 8883);
    xtouch_pubSubClient.setBufferSize(2048); // 2KB for mqtt message JWT output
    xtouch_pubSubClient.setStream(stream);
    xtouch_pubSubClient.setCallback(xtouch_pubSubClient_streamCallback);
    xtouch_pubSubClient.setKeepAlive(10);

    /* home */
    lv_msg_subscribe(XTOUCH_COMMAND_LIGHT_TOGGLE, (lv_msg_subscribe_cb_t)xtouch_device_onLightToggleCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_STOP, (lv_msg_subscribe_cb_t)xtouch_device_onStopCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_PAUSE, (lv_msg_subscribe_cb_t)xtouch_device_onPauseCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_RESUME, (lv_msg_subscribe_cb_t)xtouch_device_onResumeCommand, NULL);

    /* control */
    lv_msg_subscribe(XTOUCH_COMMAND_HOME, (lv_msg_subscribe_cb_t)xtouch_device_onHomeCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_LEFT, (lv_msg_subscribe_cb_t)xtouch_device_onLeftCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_RIGHT, (lv_msg_subscribe_cb_t)xtouch_device_onRightCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_UP, (lv_msg_subscribe_cb_t)xtouch_device_onUpCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_DOWN, (lv_msg_subscribe_cb_t)xtouch_device_onDownCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_BED_TARGET_TEMP, (lv_msg_subscribe_cb_t)xtouch_device_onBedTargetTempCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_NOZZLE_TARGET_TEMP, (lv_msg_subscribe_cb_t)xtouch_device_onNozzleTargetCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_PART_FAN_SPEED, (lv_msg_subscribe_cb_t)xtouch_device_onPartSpeedCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_AUX_FAN_SPEED, (lv_msg_subscribe_cb_t)xtouch_device_onAuxSpeedCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_CHAMBER_FAN_SPEED, (lv_msg_subscribe_cb_t)xtouch_device_onChamberSpeedCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_PRINT_SPEED, (lv_msg_subscribe_cb_t)xtouch_device_onPrintSpeedCommand, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_UNLOAD_FILAMENT, (lv_msg_subscribe_cb_t)xtouch_device_onUnloadFilament, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_LOAD_FILAMENT, (lv_msg_subscribe_cb_t)xtouch_device_onLoadFilament, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_AMS_CONTROL, (lv_msg_subscribe_cb_t)xtouch_device_command_ams_control, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_CLEAN_PRINT_ERROR, (lv_msg_subscribe_cb_t)xtouch_device_command_clean_print_error, NULL);

    /* filament */
    lv_msg_subscribe(XTOUCH_COMMAND_EXTRUDE_UP, (lv_msg_subscribe_cb_t)xtouch_device_onNozzleUp, NULL);
    lv_msg_subscribe(XTOUCH_COMMAND_EXTRUDE_DOWN, (lv_msg_subscribe_cb_t)xtouch_device_onNozzleDown, NULL);

    delay(2000);
}

#define XTOUCH_MQTT_PUSHALL_INTERVAL_MS 5000

void xtouch_cloud_mqtt_loop()
{
    xtouch_pubSubClient.loop();
    if (!xtouch_pubSubClient.connected())
    {
        Serial.println("[xPTouch][MQTT] -----DISCONNECTED-----");
        if(xTouchConfig.xTouchLanOnlyMode){
            xtouch_local_mqtt_connect();
        }else{
            xtouch_cloud_mqtt_connect();
        }

        return;
    }

    static unsigned long lastPushAll = 0;
    unsigned long now = millis();
    if (now - lastPushAll >= XTOUCH_MQTT_PUSHALL_INTERVAL_MS)
    {
        lastPushAll = now;
        xtouch_device_pushall();
    }
}

#endif