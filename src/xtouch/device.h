#ifndef _XLCD_DEVICE
#define _XLCD_DEVICE

#include <Arduino.h>
#include <cstring>
#include "trays.h"
#include "cloud.hpp"
#include "ams_edit_temp.h"
#include "filesystem.h"
#include "paths.h"

#define XTOUCH_DEVICE_CONTROL_MOVE_SPEED_XY 3000
#define XTOUCH_DEVICE_CONTROL_MOVE_SPEED_Z 1500

char ams_gcode_buffer[2048];

const char *ams_load_gcode = "M620 S%d\n"           // トレイ番号（0始まり）。M621 S と一致必須
                             "M106 S255\n"             // フィラメントを排出
                             "M104 S250\n"             // ノズルを250度に加熱
                             "M17 S\n"                 // ステッパー有効（T実行前に必須・仕様）
                             "M17 X0.5 Y0.5\n"        // X/Y 微小移動
                             "G91\n"
                             "G1 Y-5 F1200\n"
                             "G1 Z3\n"
                             "G90\n"
                             "G28 X\n"
                             "M17 R\n"
                             "G1 X70 F21000\n"
                             "G1 Y245\n"
                             "G1 Y265 F3000\n"
                             "G4\n"
                             "M106 S0\n"
                             "M109 S250\n"             // 250度まで待機
                             "G1 X90\n"
                             "G1 Y255\n"
                             "G1 X120\n"
                             "G1 X20 Y50 F21000\n"
                             "G1 Y-3\n"
                             "T%d\n"                    // トレイ番号（M620 S と同じ）
                             "G1 X54 F12000\n"
                             "G1 Y265\n"
                             "G92 E0\n"
                             "G1 E40 F180\n"
                             "G4\n"
                             "M109 S%d\n"              // 材質温度まで待機（M104だと待たず進みフリーズの原因）
                             "G1 X70 F15000\n"
                             "G1 X76\n"
                             "G1 X65\n"
                             "G1 X76\n"
                             "G1 X65\n"
                             "G1 X90 F3000\n"
                             "G1 Y255\n"
                             "G1 X100\n"
                             "G1 Y265\n"
                             "G1 X70 F10000\n"
                             "G1 X100 F5000\n"
                             "G1 X70 F10000\n"
                             "G1 X100 F5000\n"
                             "G1 X165 F12000\n"
                             "G1 Y245\n"
                             "G1 X70\n"
                             "G1 Y265 F3000\n"
                             "G91\n"
                             "G1 Z-3 F1200\n"
                             "G90\n"
                             "M621 S%d\n"             // トレイ番号（M620 S と同じ）
                             "M104 S0\n";             // ロード完了後ヒーターオフ

const char *ams_unload_gcode = "M620 S255\n"
                               "M106 P1 S255\n"
                               "M104 S250\n"
                               "M17 S\n"
                               "M17 X0.5 Y0.5\n"
                               "G91\n"
                               "G1 Y-5 F3000\n"
                               "G1 Z3 F1200\n"
                               "G90\n"
                               "G28 X\n"
                               "M17 R\n"
                               "G1 X70 F21000\n"
                               "G1 Y245\n"
                               "G1 Y265 F3000\n"
                               "G4\n"
                               "M106 P1 S0\n"
                               "M109 S250\n"
                               "G1 X90 F3000\n"
                               "G1 Y255 F4000\n"
                               "G1 X100 F5000\n"
                               "G1 X120 F21000\n"
                               "G1 X20 Y50\n"
                               "G1 Y-3\n"
                               "T255\n"
                               "G4\n"
                               "M104 S0\n"
                               "G1 X70 F3000\n"
                               "G91\n"
                               "G1 Z-3 F1200\n"
                               "G90\n"
                               "M621 S255\n";

uint32_t xtouch_device_sequence_id = 0;

String xtouch_device_next_sequence()
{
    xtouch_device_sequence_id++;
    xtouch_device_sequence_id %= (UINT32_MAX - 1);
    return String(xtouch_device_sequence_id);
}

String xtouch_device_print_action(char const *action)
{
    DynamicJsonDocument json(256);
    json["print"]["command"] = action;
    json["print"]["param"] = "";
    json["print"]["sequence_id"] = xtouch_device_next_sequence();

    String result;
    serializeJson(json, result);
    return result;
}

String lastPrintState = "IDLE";
void xtouch_device_set_print_state(String state)
{
    if (state == "IDLE")
        bambuStatus.print_status = XTOUCH_PRINT_STATUS_IDLE;
    else if (state == "RUNNING")
        bambuStatus.print_status = XTOUCH_PRINT_STATUS_RUNNING;
    else if (state == "PAUSE" || state == "Pause")
        bambuStatus.print_status = XTOUCH_PRINT_STATUS_PAUSED;
    else if (state == "FINISH")
        bambuStatus.print_status = XTOUCH_PRINT_STATUS_FINISHED;
    else if (state == "PREPARE")
        bambuStatus.print_status = XTOUCH_PRINT_STATUS_PREPARE;
    else if (state == "FAILED")
        bambuStatus.print_status = XTOUCH_PRINT_STATUS_FAILED;

    if (lastPrintState != state && xTouchConfig.xTouchWakeOnPrint && state != "IDLE" && state != "FINISH" && state != "FAILED")
    {
        xtouch_screen_wakeUp();
    }

    lastPrintState = state;
}

void xtouch_device_publish(String request)
{
#ifdef XTOUCH_DEBUG_DETAIL
    ConsoleDetail.println("[GCODE] MQTT publish request");
    ConsoleDetail.print("[xPTouch][D][MQTT] PUB topic=");
    ConsoleDetail.print(xtouch_mqtt_request_topic);
    ConsoleDetail.print(" len=");
    ConsoleDetail.print(request.length());
    ConsoleDetail.print(" payload=");
    ConsoleDetail.println(request);
#endif

    xtouch_pubSubClient.publish(xtouch_mqtt_request_topic.c_str(), request.c_str());
    delay(10);
}

void xtouch_device_publish_to_dev(const char *dev_id, String request)
{
    if (!dev_id || !dev_id[0])
        return;
    String topic = String("device/") + dev_id + "/request";
    xtouch_pubSubClient.publish(topic.c_str(), request.c_str());
    delay(10);
}

void xtouch_device_get_version()
{
    DynamicJsonDocument json(256);
    json["info"]["command"] = "get_version";
    json["info"]["sequence_id"] = xtouch_device_next_sequence();
    String result;
    serializeJson(json, result);
    xtouch_device_publish(result);
}

void xtouch_device_pushall()
{
    DynamicJsonDocument json(256);
    json["pushing"]["command"] = "pushall";
    json["pushing"]["version"] = 1;
    json["pushing"]["push_target"] = 1;
    json["pushing"]["sequence_id"] = xtouch_device_next_sequence();
    json["user_id"] = "123456789";

    String result;
    serializeJson(json, result);
    xtouch_device_publish(result);
}

void xtouch_device_set_printing_speed(int lvl)
{
    DynamicJsonDocument json(256);
    json["print"]["command"] = "print_speed";
    json["print"]["sequence_id"] = xtouch_device_next_sequence();
    json["print"]["param"] = String(lvl);

    String result;
    serializeJson(json, result);
    xtouch_device_publish(result);
}

void xtouch_device_gcode_line(String line)
{
#ifdef XTOUCH_DEBUG_VERBOSE
    ConsoleVerbose.println("[GCODE send]");
    ConsoleVerbose.println(line);
    ConsoleVerbose.println("---");
#endif
    DynamicJsonDocument json(line.length() + 256);
    json["print"]["command"] = "gcode_line";
    json["print"]["sequence_id"] = xtouch_device_next_sequence();
    json["print"]["param"] = line.c_str();
    json["user_id"] = "123456789";

    String result;
    serializeJson(json, result);
    xtouch_device_publish(result);
}

void ui_event_comp_controlComponent_controlScreenHomeConfirm() { onControlHome(NULL); }

void xtouch_device_move_axis(String axis, double value, int speed)
{
    char cmd[256];
    if (!(bambuStatus.home_flag & 1 == 1) ||
        !(bambuStatus.home_flag >> 1 & 1 == 1) ||
        !(bambuStatus.home_flag >> 2 & 1 == 1))
    {
        ui_confirmPanel_show(LV_SYMBOL_WARNING " Start Homing Process?", ui_event_comp_controlComponent_controlScreenHomeConfirm);
        return;
    }
    sprintf(cmd, "M211 S \nM211 X1 Y1 Z1\nM1002 push_ref_mode\nG91 \nG1 %s%0.1f F%d\nM1002 pop_ref_mode\nM211 R\n", axis.c_str(), value, speed);
    xtouch_device_gcode_line(String(cmd));
}

void xtouch_device_onSidebarHomeCommand(lv_msg_t *m)
{
    xtouch_device_pushall();
}

void xtouch_device_onLCDToggleCommand(lv_msg_t *m)
{

    xtouch_screen_sleep();
}

void xtouch_device_onNeoPixelToggleCommand(lv_msg_t *m)
{
    printf("xtouch_device_onNeoPixelToggleCommand\n");
    xtouch_neopixel_enabled = !xtouch_neopixel_enabled;
    print_gcode_action_changed = true;
    struct XTOUCH_MESSAGE_DATA eventData;
    eventData.data = xtouch_neopixel_enabled;
    lv_msg_send(XTOUCH_ON_NEOPIXEL_REPORT, &eventData);
}

void xtouch_device_onLightResetCommand(lv_msg_t *m)
{
    xtouch_screen_startLEDOffTimer();
    printf("★ clear LEDOFF Timer\n");
}

void xtouch_device_onLightToggleCommand(lv_msg_t *m)
{

    DynamicJsonDocument json(256);
    json["system"]["sequence_id"] = xtouch_device_next_sequence();
    json["system"]["command"] = "ledctrl";
    json["system"]["led_node"] = "chamber_light";
    json["system"]["led_mode"] = bambuStatus.chamberLed ? "off" : "on";
    json["system"]["led_on_time"] = 500;
    json["system"]["led_off_time"] = 500;
    json["system"]["loop_times"] = 1;
    json["system"]["interval_time"] = 1000;

    String result;
    serializeJson(json, result);
    xtouch_device_publish(result);
    delay(10);

    if (json["system"]["led_mode"] == "on")
    {
        if (!xtouch_mqtt_light_on)
        {
            printf("★ Mqtt massage LED On Event reset LEDOFF Timer\n");
            xtouch_device_onLightResetCommand(m);
            xtouch_mqtt_light_on = true;
        }
    }
    else
    {
        xtouch_mqtt_light_on = false;
    }
}

void xtouch_device_onHomeCommand(lv_msg_t *m)
{
    xtouch_device_gcode_line("G28 \n");
}

void xtouch_device_onLeftCommand(lv_msg_t *m)
{
    xtouch_device_move_axis("X", -controlMode.inc, XTOUCH_DEVICE_CONTROL_MOVE_SPEED_XY);
}

void xtouch_device_onRightCommand(lv_msg_t *m)
{
    xtouch_device_move_axis("X", controlMode.inc, XTOUCH_DEVICE_CONTROL_MOVE_SPEED_XY);
}

void xtouch_device_onUpCommand(lv_msg_t *m)
{
    // Always move Y+ in control screen (AXIS toggle廃止)
    String axis = "Y";
    int multiplier = 1;
    xtouch_device_move_axis(axis, controlMode.inc * multiplier, XTOUCH_DEVICE_CONTROL_MOVE_SPEED_XY);
}

void xtouch_device_onDownCommand(lv_msg_t *m)
{
    // Always move Y- in control screen
    String axis = "Y";
    int multiplier = -1;
    xtouch_device_move_axis(axis, controlMode.inc * multiplier, XTOUCH_DEVICE_CONTROL_MOVE_SPEED_XY);
}

void xtouch_device_onMotorUnlockCommand(lv_msg_t *m)
{
    (void)m;
    xtouch_device_gcode_line("M18\n");
    delay(10);
    xtouch_device_pushall();
}

void xtouch_device_onBedUpCommand(lv_msg_t *m)
{
    // Bed Up/Down は常に Z 軸専用（Z+ がノズル上方向の機種では Up=-1, Down=+1）
    String axis = "Z";
    int multiplier = -1;
    xtouch_device_move_axis(axis, controlMode.inc * multiplier, XTOUCH_DEVICE_CONTROL_MOVE_SPEED_Z);
}

void xtouch_device_onBedDownCommand(lv_msg_t *m)
{
    String axis = "Z";
    int multiplier = 1;
    xtouch_device_move_axis(axis, controlMode.inc * multiplier, XTOUCH_DEVICE_CONTROL_MOVE_SPEED_Z);
}

void xtouch_device_onBedTargetTempCommand(lv_msg_t *m)
{
    bambuStatus.bed_target_temper = controlMode.target_bed_temper;
    xtouch_device_gcode_line("M140 S" + String(controlMode.target_bed_temper) + "\n");
    xtouch_device_pushall();
}

void xtouch_device_onNozzleTargetCommand(lv_msg_t *m)
{
    bambuStatus.nozzle_target_temper = controlMode.target_nozzle_temper;
    xtouch_device_gcode_line("M104 S" + String(controlMode.target_nozzle_temper) + "\n");
    xtouch_device_pushall();
}

void xtouch_device_onPartSpeedCommand(lv_msg_t *m)
{
    xtouch_device_gcode_line("M106 P1 S" + String(bambuStatus.cooling_fan_speed) + " \n");
    delay(10);
    xtouch_device_pushall();
}

void xtouch_device_onAuxSpeedCommand(lv_msg_t *m)
{
    xtouch_device_gcode_line("M106 P2 S" + String(bambuStatus.big_fan1_speed) + "\n");
    delay(10);
    xtouch_device_pushall();
}

void xtouch_device_onChamberSpeedCommand(lv_msg_t *m)
{
    xtouch_device_gcode_line("M106 P3 S" + String(bambuStatus.big_fan2_speed) + "\n");
    delay(10);
    xtouch_device_pushall();
}

void xtouch_device_onPrintSpeedCommand(lv_msg_t *m)
{
    xtouch_device_set_printing_speed(bambuStatus.printing_speed_lvl);
    delay(10);
    xtouch_device_pushall();
}

void xtouch_device_onIncSwitchCommand(lv_msg_t *m)
{
    if (controlMode.inc == 1)
        controlMode.inc = 10;
    else if (controlMode.inc == 10)
        controlMode.inc = 100;
    else if (controlMode.inc == 100)
        controlMode.inc = 1;
}

void xtouch_device_onStopCommand(lv_msg_t *m)
{
    xtouch_device_publish(xtouch_device_print_action("stop"));
    delay(10);
    xtouch_device_pushall();
}

void xtouch_device_onPauseCommand(lv_msg_t *m)
{
    xtouch_device_publish(xtouch_device_print_action("pause"));
    delay(10);
    xtouch_device_pushall();
}

void xtouch_device_onResumeCommand(lv_msg_t *m)
{
    xtouch_device_publish(xtouch_device_print_action("resume"));
    delay(10);
    xtouch_device_pushall();
}

#ifdef __XTOUCH_PLATFORM_S3__
extern "C" void xtouch_mqtt_pushall_all_printers_for_screen_c(void);

static void xtouch_device_print_action_to_slot(int slot, const char *action)
{
    const char *dev_id = (slot == 0) ? xTouchConfig.xTouchSerialNumber
        : (slot - 1 < xtouch_other_printer_count && otherPrinters[slot - 1].valid)
            ? otherPrinters[slot - 1].dev_id : nullptr;
    if (!dev_id)
        return;
    xtouch_device_publish_to_dev(dev_id, xtouch_device_print_action(action));
    xtouch_mqtt_pushall_all_printers_for_screen_c();
}

void xtouch_device_onPauseSlotCommand(void *s, lv_msg_t *m)
{
    (void)s;
    const void *p = m ? lv_msg_get_payload(m) : nullptr;
    if (!p)
        return;
    int slot = (int)(intptr_t)p - 1; /* UI は slot+1 を送る（0 を NULL で送れないため） */
    xtouch_device_print_action_to_slot(slot, "pause");
}

void xtouch_device_onStopSlotCommand(void *s, lv_msg_t *m)
{
    (void)s;
    const void *p = m ? lv_msg_get_payload(m) : nullptr;
    if (!p)
        return;
    int slot = (int)(intptr_t)p - 1;
    xtouch_device_print_action_to_slot(slot, "stop");
}

void xtouch_device_onResumeSlotCommand(void *s, lv_msg_t *m)
{
    (void)s;
    const void *p = m ? lv_msg_get_payload(m) : nullptr;
    if (!p)
        return;
    int slot = (int)(intptr_t)p - 1;
    xtouch_device_print_action_to_slot(slot, "resume");
}
#endif

void xtouch_device_onNozzleUp(lv_msg_t *m)
{
    (void)m;
    /* AXIS の 1/10/100 と同じ controlMode.inc で送り量を変える */
    xtouch_device_gcode_line(String("M83 \nG0 E-") + String((int)controlMode.inc) + ".0 F900\n");
    delay(10);
    xtouch_device_pushall();
}

void xtouch_device_onNozzleDown(lv_msg_t *m)
{
    (void)m;
    xtouch_device_gcode_line(String("M83 \nG0 E") + String((int)controlMode.inc) + ".0 F900\n");
    delay(10);
    xtouch_device_pushall();
}

void xtouch_device_onLoadFilament(lv_msg_t *m)
{
    (void)m;
    if (!xtouch_can_load_filament())
    {
        printf("can't load filament right now\n");
        return;
    }
    if (bambuStatus.m_tray_now == TRAY_ID_EXTERNAL)
    {
        printf("EXT load skip: external filament already active (unload first)\n");
        return;
    }
    /* EXT(254) ロード: 設定済み温度を使用し MQTT ams_change_filament で送る */
    uint16_t tar_temp = get_tray_temp(0, TRAY_ID_EXTERNAL);
    const char *tray_type = get_tray_type(0, TRAY_ID_EXTERNAL);
#ifdef XTOUCH_DEBUG_VERBOSE
    ConsoleVerbose.printf("[EXT load] tray_type=%s tar_temp=%u\n", tray_type ? tray_type : "(null)", (unsigned)tar_temp);
#endif
    if (tar_temp < 100 || !tray_type || !tray_type[0] || strcmp(tray_type, "null") == 0)
    {
        ui_confirmPanel_show(LV_SYMBOL_WARNING " Set filament type and temperature\nin AMS View / Edit first.", ui_confirmPanel_hide);
        return;
    }
    DynamicJsonDocument json(256);
    json["print"]["command"] = "ams_change_filament";
    json["print"]["sequence_id"] = xtouch_device_next_sequence();
    json["print"]["target"] = 254;
    json["print"]["ams_id"] = 254;
    json["print"]["slot_id"] = 0;
    json["print"]["curr_temp"] = (int)bambuStatus.nozzle_temper;
    json["print"]["tar_temp"] = (int)tar_temp;
    String result;
    serializeJson(json, result);
#ifdef XTOUCH_DEBUG_VERBOSE
    ConsoleVerbose.println("[EXT load] MQTT ams_change_filament request");
    ConsoleVerbose.print("[xPTouch][V][MQTT] PUB EXT len=");
    ConsoleVerbose.print(result.length());
    ConsoleVerbose.print(" payload=");
    ConsoleVerbose.println(result);
#endif
    xtouch_device_publish(result);
}

void xtouch_device_onUnloadFilament(lv_msg_t *m)
{
    printf("xtouch_device_onUnloadFilament\n");
    // if (xtouch_bblp_is_x1Series() && !bambuStatus.ams_support_virtual_tray)
    // {
    //     printf("xtouch_bblp_is_x1Series() && !bambuStatus.ams_support_virtual_tray\n");
    //     DynamicJsonDocument json(256);
    //     json["print"]["command"] = "gcode_file";
    //     json["print"]["param"] = "/usr/etc/print/filament_unload.gcode";
    //     json["print"]["sequence_id"] = xtouch_device_next_sequence();
    //     String result;
    //     serializeJson(json, result);
    //     xtouch_device_publish(result);
    // }
    // else if (xtouch_bblp_is_p1Series() || (xtouch_bblp_is_x1Series() && bambuStatus.ams_support_virtual_tray))
    // {
    //     printf("xtouch_bblp_is_p1Series() || (xtouch_bblp_is_x1Series() && bambuStatus.ams_support_virtual_tray)\n");
    //     xtouch_device_gcode_line("M620 S255\nM106 P1 S255\nM104 S250\nM17 S\nM17 X0.5 Y0.5\nG91\nG1 Y-5 F3000\nG1 Z3 F1200\nG90\nG28 X\nM17 R\nG1 X70 F21000\nG1 Y245\nG1 Y265 F3000\nG4\nM106 P1 S0\nM109 S250\nG1 X90 F3000\nG1 Y255 F4000\nG1 X100 F5000\nG1 X120 F21000\nG1 X20 Y50\nG1 Y-3\nT255\nG4\nM104 S0\nG1 X70 F3000\n\nG91\nG1 Z-3 F1200\nG90\nM621 S255\n\n");
    // }
    // else
    {
        printf("xtouch_device_onUnloadFilament\n");
        DynamicJsonDocument json(256);
        json["print"]["command"] = "unload_filament";
        json["print"]["sequence_id"] = xtouch_device_next_sequence();
        String result;
        serializeJson(json, result);
        xtouch_device_publish(result);
    }
}

void xtouch_device_command_ams_control(void *s, lv_msg_t *m)
{
    const char *action = (const char *)m->payload;

    if (
        strcmp(action, "resume") == 0 ||
        strcmp(action, "reset") == 0 ||
        strcmp(action, "pause") == 0 ||
        strcmp(action, "done") == 0)
    {
        DynamicJsonDocument json(256);
        json["print"]["command"] = "ams_control";
        json["print"]["sequence_id"] = xtouch_device_next_sequence();
        json["print"]["param"] = action;
        String result;
        serializeJson(json, result);
        xtouch_device_publish(result);
    }
}

void xtouch_device_command_ams_load(void *s, lv_msg_t *m)
{
    (void)s;
    const void *payload = m->payload;
    uint16_t slot = payload ? (uint16_t)(unsigned long)payload - 1 : 0;
    uint8_t tmp_ams_id = slot / 100;
    uint8_t tmp_slot_id = slot % 100;
    uint16_t tray_index = (uint16_t)tmp_ams_id * 4 + tmp_slot_id;

    Serial.printf("[AMS load] command_ams_load slot=%u tray_index=%u m_tray_now=%u\n", (unsigned)slot, (unsigned)tray_index, (unsigned)bambuStatus.m_tray_now);

    if (bambuStatus.m_tray_now == TRAY_ID_EXTERNAL)
    {
        Serial.println(F("[AMS load] skip: external filament active (unload first)"));
        return;
    }

    if (bambuStatus.m_tray_now == tray_index)
    {
        Serial.println(F("[AMS load] skip: already on tray"));
        return;
    }

    const char *tray_type = get_tray_type(tmp_ams_id, tmp_slot_id);
    if (!tray_type || !tray_type[0] || strcmp(tray_type, "null") == 0)
    {
        ui_confirmPanel_show(LV_SYMBOL_WARNING " Set filament type and temperature\nin AMS View / Edit first.", ui_confirmPanel_hide);
        return;
    }

    /* スロット毎に設定した材質温度（ABS/PLA等）を使用。get_tray_temp は AMS Edit / MQTT で設定された値 */
    uint16_t tar_temp = get_tray_temp(tmp_ams_id, tmp_slot_id);
    if (tar_temp < 100)
        tar_temp = 0;

    bambuStatus.ams_status_main = AMS_STATUS_MAIN_FILAMENT_CHANGE;
    struct XTOUCH_MESSAGE_DATA eventData;
    eventData.data = 0;
    lv_msg_send(XTOUCH_ON_AMS_SLOT_UPDATE, &eventData);
    lv_msg_send(XTOUCH_ON_AMS_STATE_UPDATE, &eventData);

    DynamicJsonDocument json(256);
    json["print"]["command"] = "ams_change_filament";
    json["print"]["sequence_id"] = xtouch_device_next_sequence();
    json["print"]["ams_id"] = tmp_ams_id;
    json["print"]["slot_id"] = tray_index;
    json["print"]["curr_temp"] = (int)tar_temp; /* スロット設定温度（材質に応じた温度） */
    json["print"]["tar_temp"] = (int)tar_temp;
    json["print"]["target"] = tray_index;
    String result;
    serializeJson(json, result);
    xtouch_device_publish(result);
}

void xtouch_device_command_ams_unload(void *s, lv_msg_t *m)
{
    int tn = bambuStatus.m_tray_now;
    /* 0–15=AMS スロット、254=EXT、255=一部 FW の外部相当。>15 でこれら以外は gcode 対象外 */
    if (tn < 0 || (tn > 15 && tn != TRAY_ID_EXTERNAL && tn != 255))
        return;
    printf("AMS unload\n");

    bambuStatus.ams_status_main = AMS_STATUS_MAIN_FILAMENT_CHANGE;
    struct XTOUCH_MESSAGE_DATA eventData;
    eventData.data = 0;
    lv_msg_send(XTOUCH_ON_AMS_SLOT_UPDATE, &eventData);
    lv_msg_send(XTOUCH_ON_AMS_STATE_UPDATE, &eventData);

    xtouch_device_gcode_line(ams_unload_gcode);
}

/** payload = (void*)(uintptr_t)tray_index。AMS1=0-3, AMS2=4-7, AMS3=8-11, AMS4=12-15。M620 R<tray_index> を gcode_line で送る（EXT時はUIから送らない） */
void xtouch_device_command_m620_r(void *s, lv_msg_t *m)
{
    (void)s;
    uint16_t tray_index = m->payload ? (uint16_t)(uintptr_t)m->payload : 0;
    if (tray_index > 15)
        return;
    char buf[32];
    snprintf(buf, sizeof(buf), "M620 R%u \n", (unsigned)tray_index);
    xtouch_device_gcode_line(String(buf));
}

static char s_ams_fetch_pending_id[16];

/* 不正 JSON 検出時に UI へ送るメッセージ用バッファ（ダイアログ表示）。 */
static char s_ams_json_error_buf[96];

/* Extention が生成したローカル JSON (/xtouch/filament/json/<setting_id>.json) から温度範囲・filament_id・tray_type を読む。
 * フォーマット: { "filament_id": "GFA00", "nozzle_min": 200, "nozzle_max": 260, "tray_type": "PLA Matte" }
 * 成功時 true。min/max のどちらか一方だけでも >0 なら有効とみなす。不正な JSON の場合はダイアログ用メッセージを送って false。 */
static bool xtouch_load_local_slicer_temps(const char *setting_id, int *out_min, int *out_max, char *out_filament_id, size_t out_filament_id_size, char *out_tray_type, size_t out_tray_type_size)
{
    if (!setting_id || !*setting_id || !out_min || !out_max)
        return false;
    char path[96];
    snprintf(path, sizeof(path), "%s/%s.json", xtouch_paths_filament_json_dir, setting_id);

    if (!xtouch_sdcard_exists(path))
    {
        snprintf(s_ams_json_error_buf, sizeof(s_ams_json_error_buf), "No JSON file\n%s", path);
        lv_msg_send(XTOUCH_AMS_EDIT_JSON_ERROR, s_ams_json_error_buf);
        return false;
    }
    File configFile = xtouch_sdcard_open(path);
    if (!configFile || !configFile.available())
    {
        snprintf(s_ams_json_error_buf, sizeof(s_ams_json_error_buf), "No JSON file\n%s", path);
        lv_msg_send(XTOUCH_AMS_EDIT_JSON_ERROR, s_ams_json_error_buf);
        return false;
    }
    DynamicJsonDocument doc(320);
    DeserializationError err = deserializeJson(doc, configFile);
    configFile.close();
    if (err)
    {
        snprintf(s_ams_json_error_buf, sizeof(s_ams_json_error_buf), "Invalid JSON\n%s", path);
        lv_msg_send(XTOUCH_AMS_EDIT_JSON_ERROR, s_ams_json_error_buf);
        return false;
    }
    if (!doc.containsKey("nozzle_min") && !doc.containsKey("nozzle_max"))
        return false;

    int min_val = doc["nozzle_min"] | 0;
    int max_val = doc["nozzle_max"] | 0;
    if (min_val <= 0 && max_val <= 0)
        return false;

    *out_min = min_val;
    *out_max = max_val;
    if (out_filament_id && out_filament_id_size > 0)
    {
        const char *fid = doc["filament_id"] | "";
        if (fid && fid[0])
        {
            strncpy(out_filament_id, fid, out_filament_id_size - 1);
            out_filament_id[out_filament_id_size - 1] = '\0';
        }
        else
        {
            out_filament_id[0] = '\0';
        }
    }
    if (out_tray_type && out_tray_type_size > 0)
    {
        const char *tt = doc["tray_type"] | "";
        if (tt && tt[0])
        {
            strncpy(out_tray_type, tt, out_tray_type_size - 1);
            out_tray_type[out_tray_type_size - 1] = '\0';
        }
        else
        {
            out_tray_type[0] = '\0';
        }
    }
    return true;
}

/** フィラメント選択時に UI から送られる。payload = (const char*) tray_info_idx。lv_timer で非同期に API 取得し ams_edit_* グローバルに保存。 */
static void xtouch_ams_fetch_slicer_timer_cb(lv_timer_t *t)
{
    int min_val = 0, max_val = 0;
    char filament_buf[16] = {0};
    char tray_type_buf[16] = {0};
    bool ok = false;

    /* 2.8 / 5.0 とも Cloud API ではなく、Extention が生成したローカル JSON から温度・filament_id・tray_type を読む。 */
    if (s_ams_fetch_pending_id[0] != '\0')
        ok = xtouch_load_local_slicer_temps(s_ams_fetch_pending_id, &min_val, &max_val, filament_buf, sizeof(filament_buf), tray_type_buf, sizeof(tray_type_buf));
    if (ok)
        ams_edit_set_fetched_temps(s_ams_fetch_pending_id, min_val, max_val, filament_buf[0] ? filament_buf : nullptr, tray_type_buf[0] ? tray_type_buf : nullptr);
    else
        ams_edit_set_fetched_temps(s_ams_fetch_pending_id, 0, 0, nullptr, nullptr);
    lv_msg_send(XTOUCH_AMS_EDIT_FETCHED_TEMP, NULL);
    lv_timer_del(t);
}

/** EXT Reset 直後は即時 pushall が適用前の vt_tray のことがある。AMS View 復帰時に中途半端な表示にならないよう遅延でもう一度要求する */
static void xtouch_ams_ext_reset_pushall_delayed_cb(lv_timer_t *t)
{
    xtouch_device_pushall();
    lv_timer_del(t);
}

void xtouch_device_command_ams_fetch_slicer_temp(void *s, lv_msg_t *m)
{
    if (m->payload == NULL)
        return;
    const char *id = (const char *)m->payload;
    strncpy(s_ams_fetch_pending_id, id, 15);
    s_ams_fetch_pending_id[15] = '\0';
    /* ローカル JSON 読み込みのみなのでタイマーは短めで十分。 */
    lv_timer_create(xtouch_ams_fetch_slicer_timer_cb, 50, NULL);
}

/** Save / Reset 時に UI から送られる ams_filament_setting。payload = (const XTOUCH_AMS_FILAMENT_SETTING_PAYLOAD*)。
 * AMS では extrusion_cali_sel を続けて送らないと設定がキャンセルされがち。EXT の Reset（vt_tray クリア）は extrusion_cali_sel を送らない。 */
void xtouch_device_command_ams_filament_setting(void *s, lv_msg_t *m)
{
    if (m->payload == NULL)
        return;
    const struct XTOUCH_AMS_FILAMENT_SETTING_PAYLOAD *p = (const struct XTOUCH_AMS_FILAMENT_SETTING_PAYLOAD *)m->payload;
    /* EXT の Reset（vt_tray クリア）: 空の extrusion_cali_sel が効かない／以前の filament_id で確定してしまうため送らない */
    const bool ext_vt_reset = (p->ams_id == 255 && p->nozzle_temp_min == 0 && p->nozzle_temp_max == 0 && p->tray_info_idx[0] == '\0' &&
                               p->tray_type[0] == '\0');

    DynamicJsonDocument json(512);
    json["print"]["sequence_id"] = xtouch_device_next_sequence();
    json["print"]["command"] = "ams_filament_setting";
    json["print"]["ams_id"] = p->ams_id;
    json["print"]["tray_id"] = p->tray_id;
    /* ams_filament_setting: 純正の成功 report は EXT でも slot_id=254。extrusion_cali_sel は EXT で slot_id なし */
    if (p->ams_id == 255)
        json["print"]["slot_id"] = TRAY_ID_EXTERNAL;
    else
        json["print"]["slot_id"] = p->ams_id * 4 + p->tray_id;
    const char *tray_info_val = (p->filament_id[0] != '\0') ? p->filament_id : p->tray_info_idx;
    json["print"]["tray_info_idx"] = tray_info_val;
    if (p->ams_id != 255)
        json["print"]["setting_id"] = p->tray_info_idx;
    json["print"]["tray_color"] = p->tray_color;
    json["print"]["nozzle_temp_min"] = p->nozzle_temp_min;
    json["print"]["nozzle_temp_max"] = p->nozzle_temp_max;
    json["print"]["tray_type"] = p->tray_type;
    String result;
    serializeJson(json, result);
    xtouch_device_publish(result);

    if (!ext_vt_reset)
    {
        /* 設定を確定するため extrusion_cali_sel（EXT Reset 除く）。EXT Save 時は slot_id なし */
        char nozzle_d_buf[8];
        if (bambuStatus.nozzle_diameter > 0.1f && bambuStatus.nozzle_diameter < 1.0f)
            snprintf(nozzle_d_buf, sizeof(nozzle_d_buf), "%.1f", (double)bambuStatus.nozzle_diameter);
        else
            snprintf(nozzle_d_buf, sizeof(nozzle_d_buf), "0.4");
        DynamicJsonDocument json2(384);
        json2["print"]["sequence_id"] = xtouch_device_next_sequence();
        json2["print"]["command"] = "extrusion_cali_sel";
        json2["print"]["ams_id"] = p->ams_id;
        json2["print"]["tray_id"] = p->tray_id;
        if (p->ams_id != 255)
            json2["print"]["slot_id"] = p->ams_id * 4 + p->tray_id;
        json2["print"]["filament_id"] = (p->filament_id[0] != '\0') ? p->filament_id : p->tray_info_idx;
        json2["print"]["nozzle_diameter"] = nozzle_d_buf;
        if (p->ams_id == 255)
            json2["print"]["nozzle_volume_type"] = "normal";
        json2["print"]["cali_idx"] = -1;
        String result2;
        serializeJson(json2, result2);
        xtouch_device_publish(result2);
    }

    /* EXT Save は純正に合わせ再送しない。AMS は最小 ams_filament_setting で確定 */
    if (p->ams_id != 255)
    {
        DynamicJsonDocument json3(192);
        json3["print"]["sequence_id"] = xtouch_device_next_sequence();
        json3["print"]["command"] = "ams_filament_setting";
        json3["print"]["ams_id"] = p->ams_id;
        json3["print"]["tray_id"] = p->tray_id;
        String result3;
        serializeJson(json3, result3);
        xtouch_device_publish(result3);
    }

    xtouch_device_pushall();
    if (ext_vt_reset)
        lv_timer_create(xtouch_ams_ext_reset_pushall_delayed_cb, 450, NULL);
}

/** M620 R# で AMS をリフレッシュ（トレイ情報をプリンターから再取得）。payload = (void*)(uintptr_t)tray_index (0〜15) */
void xtouch_device_command_ams_refresh(void *s, lv_msg_t *m)
{
    if (m->payload == NULL)
        return;
    uint16_t tray_index = (uint16_t)(uintptr_t)m->payload;
    if (tray_index > 15)
        return;
    char buf[32];
    snprintf(buf, sizeof(buf), "M620 R%d\n", tray_index);
    xtouch_device_gcode_line(String(buf));
}

void xtouch_device_command_clean_print_error(void *s, lv_msg_t *m)
{

    if (m->payload != NULL)
    {
        const ClearErrorMessage *message = static_cast<const ClearErrorMessage *>(m->payload);

        DynamicJsonDocument json(256);
        json["print"]["command"] = "clean_print_error";
        json["print"]["sequence_id"] = xtouch_device_next_sequence();
        json["print"]["subtask_id"] = message->subtask_id;
        json["print"]["print_error"] = message->print_error;
        String result;
        serializeJson(json, result);
        xtouch_device_publish(result);
    }
}


void xtouch_device_onSetaccessoriesNozzleCommand(lv_msg_t *m)
{
    DynamicJsonDocument json(256);
    json["system"]["sequence_id"] = xtouch_device_next_sequence();
    json["system"]["accessory_type"] = "nozzle";
    json["system"]["command"] = "set_accessories";
    json["system"]["nozzle_diameter"] = bambuStatus.nozzle_diameter;
    json["system"]["nozzle_type"] = bambuStatus.nozzle_type;
    String result;
    serializeJson(json, result);
    xtouch_device_publish(result);

    delay(10);
    xtouch_device_pushall();
}

void xtouch_device_onSetUtilCalibrationCommand(lv_msg_t *m)
{
    uint8_t bitmask = xTouchConfig.xTouchUtilCalibrationBitmask;
    printf("xtouch_device_onSetUtilCalibrationCommand: bitmask=%d\n", bitmask);

    DynamicJsonDocument json(256);
    json["print"]["sequence_id"] = xtouch_device_next_sequence();
    json["print"]["command"] = "calibration";
    json["print"]["option"] = bitmask;
    String result;
    serializeJson(json, result);
    xtouch_device_publish(result);

    delay(10);
    xtouch_device_pushall();
}


void xtouch_device_onPreHeatPLACommand(lv_msg_t *m)
{
    printf("xtouch_device_onPreHeatPLACommand\n");
}

void xtouch_device_onPreHeatABSCommand(lv_msg_t *m)
{
    printf("xtouch_device_onPreHeatABSCommand\n");
}

void xtouch_device_onPreHeatOffCommand(lv_msg_t *m)
{
    printf("xtouch_device_onPreHeatOffCommand\n");
}

// void xtouch_device_command_getPaCalibration()
// {

//     Serial1.println("getPaCalibration");
//     DynamicJsonDocument doc(1024);
//     doc["print"]["command"] = "extrusion_cali_get";
//     doc["print"]["sequence_id"] = "456";
//     doc["print"]["filament_id"] = "";
//     doc["print"]["nozzle_diameter"] = "0.4";

//     // {"print":{"command":"extrusion_cali_get","filament_id":"","nozzle_diameter":"0.4","sequence_id":"20313","filaments":[],"reason":"success","result":"success"}}
//     String result;
//     serializeJson(doc, result);
//     xtouch_device_publish(result);
// }

#ifdef __XTOUCH_PLATFORM_S3__
/** push_status 受信後など、task_id に応じて xtouch_thumbnail_slot_path[slot] を更新する。thumbnail.h で実装。 */
void xtouch_thumbnail_update_path_for_slot(int slot);
/** task / 接続先変化時にサムネ LGFX キャッシュを捨てる。thumbnail.h で実装。 */
void xtouch_thumbnail_invalidate_slot(int slot);
void xtouch_thumbnail_invalidate_all_slots(void);
void xtouch_thumbnail_update_path_all_slots(void);
#endif

#endif