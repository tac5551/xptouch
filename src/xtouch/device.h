#ifndef _XLCD_DEVICE
#define _XLCD_DEVICE

#include <Arduino.h>
#include "trays.h"

#define XTOUCH_DEVICE_CONTROL_MOVE_SPEED_XY 3000
#define XTOUCH_DEVICE_CONTROL_MOVE_SPEED_Z 1500

char ams_gcode_buffer[2048];

const char *ams_load_gcode = "M620 S%d\n"           //AMSを有効にする
                             "M106 S255\n"             //フィラメントを排出
                             "M104 S250\n"             //ノズルを250度に加熱　slicerではここから始まる
                             "M17 S\n"                 //S軸を有効にする　　　　もともとはない
                             "M17 X0.5 Y0.5\n"        //X軸, Y軸を0.5mmに移動　もともとはない
                             "G91\n"                //相対位置
                             "G1 Y-5 F1200\n"       //すこし逃がす
                             "G1 Z3\n"               //Z軸を3mm上げる
                             "G90\n"                 //絶対位置
                             "G28 X\n"               //X軸を原点に戻す
                             "M17 R\n"                //R軸をリセット
                             "G1 X70 F21000\n"         //X軸を70mmに移動（X軸移動速度1200mm/min）
                             "G1 Y245\n"                //Y軸を245mmに移動    
                             "G1 Y265 F3000\n"          //Y軸を265mmに移動 パージエリア
                             "G4\n"                     //ウェイト
                             "M106 S0\n"                //フィラメントを排出    
                             "M109 S250\n"             //ノズルを250度に加熱    
                             "G1 X90\n"                 //X軸を90mmに移動
                             "G1 Y255\n"                //Y軸を255mmに移動
                             "G1 X120\n"                 //X軸を120mmに移動
                             "G1 X20 Y50 F21000\n"      //X軸を20mm, Y軸を50mmに移動（X軸移動速度1200mm/min）
                             "G1 Y-3\n"                 //Y軸を-3mmに移動
                             "T%d\n"                    //ツールの選択（ツール番号）
                             "G1 X54 F12000\n"                 //X軸を54mmに移動
                             "G1 Y265\n"                //Y軸を265mmに移動
                            // M400         //一時停止
                             "G92 E0\n"                 //E軸を0にリセット
                             "G1 E40 F180\n"            //E軸を40mmに移動（E軸移動速度180mm/min）
                             "G4\n"
                             "M104 S%d\n"
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
                             "M621 S%d\n";

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
    else if (state == "PAUSE")
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
    // Serial.println(request);
    xtouch_pubSubClient.publish(xtouch_mqtt_request_topic.c_str(), request.c_str());
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
    neopixel_enabled = !neopixel_enabled;
    struct XTOUCH_MESSAGE_DATA eventData;
    eventData.data = neopixel_enabled;
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
    String axis = "Y";
    int multiplier = axis == "Y" ? 1 : -1;
    xtouch_device_move_axis(axis, controlMode.inc * multiplier, axis == "Y" ? XTOUCH_DEVICE_CONTROL_MOVE_SPEED_XY : XTOUCH_DEVICE_CONTROL_MOVE_SPEED_Z);
}

void xtouch_device_onDownCommand(lv_msg_t *m)
{
    String axis = "Y";
    int multiplier = axis == "Y" ? -1 : 1;
    xtouch_device_move_axis(axis, controlMode.inc * multiplier, axis == "Y" ? XTOUCH_DEVICE_CONTROL_MOVE_SPEED_XY : XTOUCH_DEVICE_CONTROL_MOVE_SPEED_Z);
}

void xtouch_device_onBedUpCommand(lv_msg_t *m)
{
    String axis = "Z";
    int multiplier = axis == "Y" ? 1 : -1;
    xtouch_device_move_axis(axis, controlMode.inc * multiplier, axis == "Y" ? XTOUCH_DEVICE_CONTROL_MOVE_SPEED_XY : XTOUCH_DEVICE_CONTROL_MOVE_SPEED_Z);
}

void xtouch_device_onBedDownCommand(lv_msg_t *m)
{
    String axis = "Z";
    int multiplier = axis == "Y" ? -1 : 1;
    xtouch_device_move_axis(axis, controlMode.inc * multiplier, axis == "Y" ? XTOUCH_DEVICE_CONTROL_MOVE_SPEED_XY : XTOUCH_DEVICE_CONTROL_MOVE_SPEED_Z);
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

void xtouch_device_onNozzleUp(lv_msg_t *m)
{
    xtouch_device_gcode_line("M83 \nG0 E-10.0 F900\n");
    delay(10);
    xtouch_device_pushall();
}

void xtouch_device_onNozzleDown(lv_msg_t *m)
{
    xtouch_device_gcode_line("M83 \nG0 E10.0 F900\n");
    delay(10);
    xtouch_device_pushall();
}

void xtouch_device_onLoadFilament(lv_msg_t *m)
{
    if (xtouch_can_load_filament())
    {
        xtouch_device_gcode_line("M620 S254\nM106 S255\nM104 S250\nM17 S\nM17 X0.5 Y0.5\nG91\nG1 Y-5 F1200\nG1 Z3\nG90\nG28 X\nM17 R\nG1 X70 F21000\nG1 Y245\nG1 Y265 F3000\nG4\nM106 S0\nM109 S250\nG1 X90\nG1 Y255\nG1 X120\nG1 X20 Y50 F21000\nG1 Y-3\nT254\nG1 X54\nG1 Y265\nG92 E0\nG1 E40 F180\nG4\nM104 S0\nG1 X70 F15000\nG1 X76\nG1 X65\nG1 X76\nG1 X65\nG1 X90 F3000\nG1 Y255\nG1 X100\nG1 Y265\nG1 X70 F10000\nG1 X100 F5000\nG1 X70 F10000\nG1 X100 F5000\nG1 X165 F12000\nG1 Y245\nG1 X70\nG1 Y265 F3000\nG91\nG1 Z-3 F1200\nG90\nM621 S254\n\n");
    }
    else
    {
        printf("can't load filament right now\n");
        printf("bambuStatus.ams_status_main %d\n", bambuStatus.ams_status_main);
        printf("bambuStatus.hw_switch_state %d\n", bambuStatus.hw_switch_state);
        printf("bambuStatus.m_tray_now %d\n", bambuStatus.m_tray_now);
    }
}

void xtouch_device_onUnloadFilament(lv_msg_t *m)
{
    if (xtouch_bblp_is_x1Series() && !bambuStatus.ams_support_virtual_tray)
    {

        DynamicJsonDocument json(256);
        json["print"]["command"] = "gcode_file";
        json["print"]["param"] = "/usr/etc/print/filament_unload.gcode";
        json["print"]["sequence_id"] = xtouch_device_next_sequence();
        String result;
        serializeJson(json, result);
        xtouch_device_publish(result);
    }
    else if (xtouch_bblp_is_p1Series() || (xtouch_bblp_is_x1Series() && bambuStatus.ams_support_virtual_tray))
    {
        xtouch_device_gcode_line("M620 S255\nM106 P1 S255\nM104 S250\nM17 S\nM17 X0.5 Y0.5\nG91\nG1 Y-5 F3000\nG1 Z3 F1200\nG90\nG28 X\nM17 R\nG1 X70 F21000\nG1 Y245\nG1 Y265 F3000\nG4\nM106 P1 S0\nM109 S250\nG1 X90 F3000\nG1 Y255 F4000\nG1 X100 F5000\nG1 X120 F21000\nG1 X20 Y50\nG1 Y-3\nT255\nG4\nM104 S0\nG1 X70 F3000\n\nG91\nG1 Z-3 F1200\nG90\nM621 S255\n\n");
    }
    else
    {
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
    uint16_t slot = *((uint16_t *)&m->payload) - 1;

    if (bambuStatus.m_tray_now == slot)
    {
        return;
    }

    uint8_t tmp_ams_id = slot / 100;
    uint8_t tmp_slot_id = slot % 100;

    uint16_t slot_id = tmp_slot_id + 1;
    if (strcmp(get_tray_type(tmp_ams_id, slot_id), "null") == 0)
    {
        xtouch_device_pushall();
        return;
    }

    bambuStatus.ams_status_main = AMS_STATUS_MAIN_FILAMENT_CHANGE;
    struct XTOUCH_MESSAGE_DATA eventData;
    eventData.data = 0;
    lv_msg_send(XTOUCH_ON_AMS_SLOT_UPDATE, &eventData);
    lv_msg_send(XTOUCH_ON_AMS_STATE_UPDATE, &eventData);

    if (bambuStatus.m_tray_now != 254)
    {
        xtouch_device_gcode_line(ams_unload_gcode);
    }
    memset(ams_gcode_buffer, 0, 700);
    sprintf(ams_gcode_buffer, ams_load_gcode, tmp_ams_id, tmp_slot_id, get_tray_temp(tmp_ams_id, tmp_slot_id), tmp_slot_id);
    xtouch_device_gcode_line(ams_gcode_buffer);
}

void xtouch_device_command_ams_unload(void *s, lv_msg_t *m)
{
    if (bambuStatus.m_tray_now > 15)
    {
        return;
    }
    printf("AMS unload\n");

    bambuStatus.ams_status_main = AMS_STATUS_MAIN_FILAMENT_CHANGE;
    struct XTOUCH_MESSAGE_DATA eventData;
    eventData.data = 0;
    lv_msg_send(XTOUCH_ON_AMS_SLOT_UPDATE, &eventData);
    lv_msg_send(XTOUCH_ON_AMS_STATE_UPDATE, &eventData);

    xtouch_device_gcode_line(ams_unload_gcode);
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

#endif