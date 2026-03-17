#include <Arduino.h>
#include <stdio.h>
#include "../ui.h"
#include "../ui_msgs.h"
#include "../../xtouch/trays.h"
#include "xtouch/globals.h"
#include <time.h>

#define SLOT_COUNT 5
#define AMS_BORDER 1

extern int time12hFormat;
char last_gcode_file[128];
char bed_target[10];
char nozzle_target[10];
const char *xtouch_device_get_print_state()
{
    switch (bambuStatus.print_status)
    {
    case XTOUCH_PRINT_STATUS_IDLE:
        return LV_SYMBOL_OK " Ready";
    case XTOUCH_PRINT_STATUS_FINISHED:
        return LV_SYMBOL_OK "Finished";
    case XTOUCH_PRINT_STATUS_FAILED:
        return LV_SYMBOL_WARNING "Failed";
    default:
        return "N/A";
    }
}

#ifdef __XTOUCH_SCREEN_50__
/* 5インチ: スロット0サムネ更新でホームのサムネ img を更新（Printers と同じ配列で描画） */
static void ui_event_home_thumb_update(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_MSG_RECEIVED)
        return;
    lv_msg_t *m = lv_event_get_msg(e);
    if (!m || lv_msg_get_id(m) != XTOUCH_ON_OTHER_PRINTER_UPDATE)
        return;
    const void *p = lv_msg_get_payload(m);
    if ((intptr_t)p != 1)
        return; /* スロット0 のみ (送信側は +1 で 1) */
    ui_thumb_set_img_src_from_slot(lv_event_get_target(e), 0);
}
#endif

void ui_event_comp_homeComponent_mainScreenPlayPauseButton(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_homeComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onHomeControllerPlayPause(e);
    }
}
void ui_event_comp_homeComponent_mainScreenStopButton(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_homeComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onHomeControllerStop(e);
    }
}
void ui_event_comp_homeComponent_mainScreenSpeedDropDown(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_homeComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        onHomeSpeedSelection(e);
    }
}
void ui_event_comp_homeComponent_mainScreenLightButton(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_homeComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onHomeLight(e);
        _ui_state_modify(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENRIGHT_MAINSCREENLIGHTBUTTON], LV_STATE_CHECKED, _UI_MODIFY_STATE_TOGGLE);
    }
}
void ui_event_comp_homeComponent_mainScreenLCDButton(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_homeComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onHomeLCD(e);
    }
}
void ui_event_comp_homeComponent_mainScreenNeoPixelButton(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_homeComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onHomeNeoPixel(e);
    }
}

void ui_event_comp_homeComponent_mainScreenBedTemp(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_homeComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onHomeBedTemp(e);
    }
}
void ui_event_comp_homeComponent_mainScreenNozzleTemp(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_homeComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onHomeNozzleTemp(e, 0);
    }
}

void ui_event_comp_homeComponent_mainScreenSpeedChange(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    int last_speed = bambuStatus.printing_speed_lvl;
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        char buf[32];
        uint16_t index = lv_dropdown_get_selected(target);
        switch (index)
        {
        case 0:
            bambuStatus.printing_speed_lvl = XTOUCH_SPEED_LEVEL_SILENCE;
            break;
        case 1:
            bambuStatus.printing_speed_lvl = XTOUCH_SPEED_LEVEL_NORMAL;
            break;
        case 2:
            bambuStatus.printing_speed_lvl = XTOUCH_SPEED_LEVEL_RAPID;
            break;
        case 3:
            bambuStatus.printing_speed_lvl = XTOUCH_SPEED_LEVEL_RAMPAGE;
            break;
        }
        lv_msg_send(XTOUCH_COMMAND_PRINT_SPEED, NULL);
    }
}

void onXTouchLightMessage(lv_event_t *e)
{

    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);

    struct XTOUCH_MESSAGE_DATA *message = (struct XTOUCH_MESSAGE_DATA *)m->payload;

    if (message->data == 1)
    {
        lv_obj_add_state(target, LV_STATE_CHECKED);
        if (!xtouch_mqtt_light_on)
        {
            printf("★ Mqtt massage LED On Event reset LEDOFF Timer\n");
            lv_msg_send(XTOUCH_COMMAND_LIGHT_RESET, NULL);
            xtouch_mqtt_light_on = true;
        }
    }
    else
    {
        lv_obj_clear_state(target, LV_STATE_CHECKED);
        xtouch_mqtt_light_on = false;
    }
}

void onXTouchNeoPixelMessage(lv_event_t *e)
{

    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);

    struct XTOUCH_MESSAGE_DATA *message = (struct XTOUCH_MESSAGE_DATA *)m->payload;

    if (message->data == 1)
    {
        lv_obj_add_state(target, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_clear_state(target, LV_STATE_CHECKED);
    }
}


void onXTouchAMSUpdate(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);
    uint16_t user_data = (uint16_t)(uintptr_t)lv_event_get_user_data(e);

    (void)m;
    /* Home: slot 0=External(254), slot 1-4=AMS1 の tray 0-3。ams_id=0 固定 */
    uint8_t tmp_ams_id = 0;
    uint8_t tmp_tray_id = (user_data == 0) ? TRAY_ID_EXTERNAL : (uint8_t)(user_data - 1);

    uint64_t tray_status = get_tray_status(tmp_ams_id, tmp_tray_id);
    uint16_t tray_id = (uint16_t)((tray_status >> 4) & 0x0F);
    uint16_t loaded = (uint16_t)(tray_status & 0x01);
    char *tray_type = get_tray_type(tmp_ams_id, tmp_tray_id);

    lv_label_set_text(target, "");
    int match = (tmp_tray_id == TRAY_ID_EXTERNAL) ? 1 : (tmp_tray_id == tray_id);
    if (match)
    {
        uint32_t color_code = (uint32_t)((tray_status >> 8) & 0xFFFFFF);
        lv_color_t color = (color_code != 0) ? lv_color_hex(color_code) : lv_color_hex(0x444444);
        lv_color_t color_inv = (color_code != 0) ? lv_color_hex((0xFFFFFF - color_code) & 0xFFFFFF) : lv_color_hex(0xFFFFFF);

        lv_obj_set_style_bg_color(target, color, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(target, color_inv, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(target, color, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(target, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);
        if (!loaded)
            lv_label_set_text(target, "X");
        else if (tray_type && tray_type[0] != '\0' && strcmp(tray_type, "null") != 0)
            lv_label_set_text(target, tray_type);
        else
            lv_label_set_text(target, "-");

        int is_current = (tmp_tray_id == TRAY_ID_EXTERNAL && bambuStatus.m_tray_now == TRAY_ID_EXTERNAL) ||
                        (tmp_tray_id <= 3 && bambuStatus.m_tray_now >= 0 && bambuStatus.m_tray_now <= 15 &&
                         (uint8_t)(bambuStatus.m_tray_now & 3) == tmp_tray_id);
        if (is_current)
        {
            lv_obj_set_style_border_color(target, color_inv, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(target, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
}

void onXTouchBedTemp(lv_event_t *e)
{

    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);

    struct XTOUCH_MESSAGE_DATA *message = (struct XTOUCH_MESSAGE_DATA *)m->payload;

    char value[10];

    itoa(message->data, value, 10);
    lv_label_set_text(target, value);
}

void onXTouchChamberTemp(lv_event_t *e)
{

    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);

    struct XTOUCH_MESSAGE_DATA *message = (struct XTOUCH_MESSAGE_DATA *)m->payload;
    char value[10];

    itoa(message->data, value, 10);
    lv_label_set_text(target, value);
}

void onXTouchBedTempTarget(lv_event_t *e)
{

    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);

    struct XTOUCH_MESSAGE_DATA *message = (struct XTOUCH_MESSAGE_DATA *)m->payload;
    lv_obj_set_style_text_color(target, message->data > 0 ? lv_color_hex(0xff682a) : lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
}

void onXTouchAMS(lv_event_t *e)
{

    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);

    struct XTOUCH_MESSAGE_DATA *message = (struct XTOUCH_MESSAGE_DATA *)m->payload;
    if (message->data == 1)
    {
        lv_obj_clear_flag(target, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(target, LV_OBJ_FLAG_HIDDEN);
    }
}

void onXTouchWifiSignal(lv_event_t *e)
{

    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);

    struct XTOUCH_MESSAGE_DATA *message = (struct XTOUCH_MESSAGE_DATA *)m->payload;
    lv_obj_set_style_text_color(target, message->data < 50 ? lv_color_hex(0x2aff00) : lv_color_hex(0xff682a), LV_PART_MAIN | LV_STATE_DEFAULT);
}

void onXTouchIPCam(lv_event_t *e)
{

    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);
    if (bambuStatus.has_ipcam)
    {
        lv_obj_clear_flag(target, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_text_color(target, bambuStatus.camera_recording_when_printing || bambuStatus.camera_timelapse ? lv_color_hex(0x2aff00) : lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(target, bambuStatus.camera_timelapse ? "y 2" : "y");
    }
    else
    {
        lv_obj_add_flag(target, LV_OBJ_FLAG_HIDDEN);
    }
}

int printingLevelToIndex(int lvl)
{

    switch (lvl)
    {
    case XTOUCH_SPEED_LEVEL_INVALID:
        return 1;
    case XTOUCH_SPEED_LEVEL_SILENCE:
        return 0;
    case XTOUCH_SPEED_LEVEL_NORMAL:
        return 1;
    case XTOUCH_SPEED_LEVEL_RAPID:
        return 2;
    case XTOUCH_SPEED_LEVEL_RAMPAGE:
        return 3;
    }
}

void onXTouchPrintStatus(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_homeComponent = lv_event_get_user_data(e);

    ui_confirmPanel_hide(); // hide confirm panel if new data comes
    //printf("[xPTouch][LED] print_status : %d print_gcode_action : %d print_real_action : %d percent : %d layer : %d/%d mc_print_sub_stage : %d mc_print_stage : %d\n", bambuStatus.print_status, bambuStatus.print_gcode_action, bambuStatus.print_real_action, bambuStatus.mc_print_percent, bambuStatus.current_layer, bambuStatus.total_layers, bambuStatus.mc_print_sub_stage, bambuStatus.mc_print_stage);

    /* RUNNING/PAUSED のときは常にレイヤー数を表示（Heating から切り替わる） */
    if (bambuStatus.print_status == XTOUCH_PRINT_STATUS_RUNNING || bambuStatus.print_status == XTOUCH_PRINT_STATUS_PAUSED)
    {
        char layerText[32];
        sprintf(layerText, "%d/%d", bambuStatus.current_layer, bambuStatus.total_layers);
        lv_label_set_text(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER2_MAINSCREENLAYER], layerText);
    }
    else
    switch (bambuStatus.print_gcode_action)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 8:
    case 13:
    case 14:
    case 18:
    case 25:
    case 48:
    {
        const char *msg = "Unknown";
        switch (bambuStatus.print_gcode_action)
        {
        case 0:  msg = "Wait heating"; break;
        case 1:  msg = "Bed leveling"; break;
        case 2:  msg = "Heatbed preheat"; break;
        case 3:  msg = "Vibration compensation"; break;
        case 8:  msg = "Calibration"; break;
        case 13: msg = "Head Homing"; break;
        case 14: msg = "Nozzle wipe"; break;
        case 18: msg = "Micro Rider Calibration"; break;
        case 25: msg = "Motor noise cancelling"; break;
        case 48: msg = "Motor noise cancelling"; break;
        default: break;
        }
        lv_label_set_text(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER2_MAINSCREENLAYER], msg);
        break;
    }
    case 255:
        /* keep previous label text, do not update */
        break;
    default:
    {
        /* other: show layer count */
        char layerText[32];
        sprintf(layerText, "%d/%d", bambuStatus.current_layer, bambuStatus.total_layers);
        lv_label_set_text(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER2_MAINSCREENLAYER], layerText);
        break;
    }
    }
    lv_slider_set_value(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENPROGRESSBAR], bambuStatus.mc_print_percent, LV_ANIM_ON);

#ifdef __XTOUCH_SCREEN_50__
    {
        lv_obj_t *subtaskLabel = comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER_MAINSCREENSUBTASKLABEL];
        if (subtaskLabel)
        {
            static char subtaskBuf[24];
            const char *src = bambuStatus.subtask_name[0] ? bambuStatus.subtask_name : "";
            int n = 0;
            while (n < 20 && src[n] != '\0')
            {
                subtaskBuf[n] = src[n];
                n++;
            }
            subtaskBuf[n] = '\0';
            if (src[n] != '\0')
            {
                if (n > 3)
                    n = 17;
                subtaskBuf[n++] = '.';
                subtaskBuf[n++] = '.';
                subtaskBuf[n++] = '.';
                subtaskBuf[n] = '\0';
            }
            lv_label_set_text(subtaskLabel, subtaskBuf);
        }
    }
#endif

    char remainingTimeText[48];
    _ui_seconds_to_timeleft(bambuStatus.mc_left_time, remainingTimeText);

    char percentAndRemaining[100]; // Adjust the size accordingly
    sprintf(percentAndRemaining, "%d%% -- %s", bambuStatus.mc_print_percent, remainingTimeText);

    lv_label_set_text(target, percentAndRemaining);

    lv_obj_t *playPauseButton = comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER_MAINSCREENPLAYPAUSEBUTTON];
    lv_obj_t *stopButton = comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER_MAINSCREENSTOPBUTTON];
    lv_obj_t *nozzleIcon = comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER_MAINSCREENNOZZLEICON];
    lv_obj_t *preheatBox = comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPREHEATBOX];
    lv_obj_t *dropDown = comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENCENTRAL_MAINSCREENSPEEDDROPDOWN];
    lv_obj_t *statusCaption = comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENSTATUS_MAINSCREENSTATUSCAPTION];

    /* メイン左カラムのボックス構成（上から）:
     * 1.AMS 2.Player 3.ロゴ+メッセージ 4.Preheat 5.進捗・レイヤー 6.速度
     * 状態ごとに表示/非表示と flex_grow を切り替え */
    switch (bambuStatus.print_status)
    {
    case XTOUCH_PRINT_STATUS_PAUSED:
        lv_label_set_text(playPauseButton, "z");
        if (playPauseButton)
            lv_obj_clear_flag(playPauseButton, LV_OBJ_FLAG_HIDDEN);
        if (stopButton)
            lv_obj_clear_flag(stopButton, LV_OBJ_FLAG_HIDDEN);
        if (statusCaption)
        {
            lv_label_set_text(statusCaption, LV_SYMBOL_PAUSE " Paused");
            lv_obj_clear_flag(statusCaption, LV_OBJ_FLAG_HIDDEN);
        }
        /* 1.AMS 2.Player(サムネ高さを約2/3に) 3.ロゴ(非表示) 4.Preheat(非表示) 5.進捗/レイヤー 6.速度 比率 2:2:1 */
        lv_obj_clear_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENSTATUSBAR], LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_flex_grow(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER], 2, 0);
        lv_obj_set_style_flex_grow(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPROGRESSBOX], 2, 0);
        lv_obj_set_style_flex_grow(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENCENTRAL], 1, 0);
        lv_obj_set_style_flex_grow(ui_mainStatusComponent, 0, 0);
        if (preheatBox)
        {
            lv_obj_set_style_flex_grow(preheatBox, 0, 0);
            lv_obj_add_flag(preheatBox, LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_clear_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER], LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPROGRESSBOX], LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENCENTRAL], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENIDLE], LV_OBJ_FLAG_HIDDEN);
        if (nozzleIcon)
            lv_obj_clear_flag(nozzleIcon, LV_OBJ_FLAG_HIDDEN);


        lv_obj_clear_state(dropDown, LV_STATE_DISABLED);
        break;
    case XTOUCH_PRINT_STATUS_RUNNING:
        lv_label_set_text(playPauseButton, "0");
        if (playPauseButton)
            lv_obj_clear_flag(playPauseButton, LV_OBJ_FLAG_HIDDEN);
        if (stopButton)
            lv_obj_clear_flag(stopButton, LV_OBJ_FLAG_HIDDEN);
        if (statusCaption)
            lv_obj_add_flag(statusCaption, LV_OBJ_FLAG_HIDDEN);
        /* 1.AMS 2.Player(サムネ高さを約2/3に) 3.ロゴ(非表示) 4.Preheat(非表示) 5.進捗/レイヤー 6.速度 比率 2:2:1 */
        lv_obj_clear_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENSTATUSBAR], LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_flex_grow(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER], 2, 0);
        lv_obj_set_style_flex_grow(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPROGRESSBOX], 2, 0);
        lv_obj_set_style_flex_grow(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENCENTRAL], 1, 0);
        lv_obj_set_style_flex_grow(ui_mainStatusComponent, 0, 0);
        if (preheatBox)
        {
            lv_obj_set_style_flex_grow(preheatBox, 0, 0);
            lv_obj_add_flag(preheatBox, LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_clear_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER], LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPROGRESSBOX], LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENCENTRAL], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENIDLE], LV_OBJ_FLAG_HIDDEN);
        if (nozzleIcon)
            lv_obj_clear_flag(nozzleIcon, LV_OBJ_FLAG_HIDDEN);


        lv_obj_clear_state(dropDown, LV_STATE_DISABLED);
        break;
    case XTOUCH_PRINT_STATUS_PREPARE:
        if (playPauseButton)
            lv_obj_clear_flag(playPauseButton, LV_OBJ_FLAG_HIDDEN);
        if (stopButton)
            lv_obj_clear_flag(stopButton, LV_OBJ_FLAG_HIDDEN);
        if (statusCaption)
            lv_obj_add_flag(statusCaption, LV_OBJ_FLAG_HIDDEN);
        /* 1.AMS 2.Player(サムネ高さを約2/3に) 3.ロゴ(非表示) 4.Preheat(非表示) 5.進捗/レイヤー 6.速度 比率 2:2:1 */
        lv_obj_clear_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENSTATUSBAR], LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_flex_grow(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER], 2, 0);
        lv_obj_set_style_flex_grow(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPROGRESSBOX], 2, 0);
        lv_obj_set_style_flex_grow(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENCENTRAL], 1, 0);
        lv_obj_set_style_flex_grow(ui_mainStatusComponent, 0, 0);
        if (preheatBox)
        {
            lv_obj_set_style_flex_grow(preheatBox, 0, 0);
            lv_obj_add_flag(preheatBox, LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_clear_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER], LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPROGRESSBOX], LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENCENTRAL], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENIDLE], LV_OBJ_FLAG_HIDDEN);
        if (nozzleIcon)
            lv_obj_clear_flag(nozzleIcon, LV_OBJ_FLAG_HIDDEN);

        lv_obj_add_state(dropDown, LV_STATE_DISABLED);
        break;

    case XTOUCH_PRINT_STATUS_IDLE:
    case XTOUCH_PRINT_STATUS_FINISHED:
    case XTOUCH_PRINT_STATUS_FAILED:
        if (playPauseButton)
            lv_obj_add_flag(playPauseButton, LV_OBJ_FLAG_HIDDEN);
        if (stopButton)
            lv_obj_add_flag(stopButton, LV_OBJ_FLAG_HIDDEN);
        /* ロゴ下のスペースで Ready/Finished/Failed を 2.8・5インチ共通表示 */
        if (statusCaption)
        {
            lv_label_set_text(statusCaption, xtouch_device_get_print_state());
            lv_obj_clear_flag(statusCaption, LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_clear_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENSTATUSBAR], LV_OBJ_FLAG_HIDDEN);
        /* 1.AMS 2.Player 3.ロゴ+メッセージ / 4.Preheat（設定で表示可否、機種ごとに比率変更） 5,6は基本非表示 */
#ifdef __XTOUCH_SCREEN_50__
        /* 5インチ: Player + ロゴを残し、Preheat ボタンはコンパクトに表示 */
        lv_obj_set_style_flex_grow(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER], 1, 0);
        lv_obj_set_style_flex_grow(ui_mainStatusComponent, 1, 0);
        if (preheatBox)
        {
            if (xTouchConfig.xTouchPreheatEnabled)
            {
                lv_obj_set_style_flex_grow(preheatBox, 1, 0);
                lv_obj_clear_flag(preheatBox, LV_OBJ_FLAG_HIDDEN);
            }
            else
            {
                lv_obj_set_style_flex_grow(preheatBox, 0, 0);
                lv_obj_add_flag(preheatBox, LV_OBJ_FLAG_HIDDEN);
            }
        }
        lv_obj_clear_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER], LV_OBJ_FLAG_HIDDEN);
#else
        /* 2.8インチ: ロゴ+メッセージ と Preheat ボタンをメインに使う */
        lv_obj_set_style_flex_grow(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER], 1, 0);
        lv_obj_set_style_flex_grow(ui_mainStatusComponent, 1, 0);
        if (preheatBox)
        {
            if (xTouchConfig.xTouchPreheatEnabled)
            {
                lv_obj_set_style_flex_grow(preheatBox, 1, 0);
                lv_obj_clear_flag(preheatBox, LV_OBJ_FLAG_HIDDEN);
            }
            else
            {
                lv_obj_set_style_flex_grow(preheatBox, 0, 0);
                lv_obj_add_flag(preheatBox, LV_OBJ_FLAG_HIDDEN);
            }
        }
        lv_obj_add_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER], LV_OBJ_FLAG_HIDDEN);
#endif
        lv_obj_set_style_flex_grow(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPROGRESSBOX], 0, 0);
        lv_obj_set_style_flex_grow(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENCENTRAL], 0, 0);

        lv_obj_add_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPROGRESSBOX], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENCENTRAL], LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(comp_homeComponent[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENIDLE], LV_OBJ_FLAG_HIDDEN);
        if (nozzleIcon)
            lv_obj_clear_flag(nozzleIcon, LV_OBJ_FLAG_HIDDEN);

        lv_obj_add_state(dropDown, LV_STATE_DISABLED);
        break;
    
    }

    lv_obj_clear_state(playPauseButton, LV_STATE_DISABLED);

    lv_dropdown_set_selected(dropDown, printingLevelToIndex(bambuStatus.printing_speed_lvl));
}

// COMPONENT homeComponent

lv_obj_t *ui_homeComponent_create(lv_obj_t *comp_parent)
{

    lv_obj_t *cui_homeComponent;
    cui_homeComponent = lv_obj_create(comp_parent);
    //    lv_obj_set_width(cui_homeComponent, lv_pct(100));
    lv_obj_set_height(cui_homeComponent, lv_pct(100));
    lv_obj_set_flex_grow(cui_homeComponent, 1);
    lv_obj_set_x(cui_homeComponent, 386);
    lv_obj_set_y(cui_homeComponent, 178);
    lv_obj_set_flex_flow(cui_homeComponent, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_homeComponent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_homeComponent, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_homeComponent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_homeComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_homeComponent, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_homeComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_homeComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_homeComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_homeComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_homeComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_homeComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_homeComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_homeComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_homeComponent, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_homeComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_mainScreenLeft;
    cui_mainScreenLeft = lv_obj_create(cui_homeComponent);
    lv_obj_set_width(cui_mainScreenLeft, lv_pct(75));
    lv_obj_set_height(cui_mainScreenLeft, lv_pct(100));
    lv_obj_set_x(cui_mainScreenLeft, 386);
    lv_obj_set_y(cui_mainScreenLeft, 178);
    lv_obj_set_flex_flow(cui_mainScreenLeft, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_mainScreenLeft, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_mainScreenLeft, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenLeft, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_mainScreenLeft, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_mainScreenLeft, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenLeft, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenLeft, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenLeft, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenLeft, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenLeft, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenLeft, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenLeft, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenLeft, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenLeft, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenLeft, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_mainScreenStatusBar;
    cui_mainScreenStatusBar = lv_obj_create(cui_mainScreenLeft);
    lv_obj_set_height(cui_mainScreenStatusBar, lv_pct(12));
    lv_obj_set_width(cui_mainScreenStatusBar, lv_pct(100));
    lv_obj_set_x(cui_mainScreenStatusBar, 386);
    lv_obj_set_y(cui_mainScreenStatusBar, 178);
    lv_obj_set_flex_flow(cui_mainScreenStatusBar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenStatusBar, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_mainScreenStatusBar, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenStatusBar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_mainScreenStatusBar, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenStatusBar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenStatusBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenStatusBar, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenStatusBar, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenStatusBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenStatusBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenStatusBar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenStatusBar, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenStatusBar, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenStatusBar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

//     lv_obj_t *cui_mainScreenWifiIcon;
//     cui_mainScreenWifiIcon = lv_label_create(cui_mainScreenStatusBar);
//     lv_obj_set_width(cui_mainScreenWifiIcon, LV_SIZE_CONTENT);  /// 50
//     lv_obj_set_height(cui_mainScreenWifiIcon, LV_SIZE_CONTENT); /// 16
//     lv_label_set_text(cui_mainScreenWifiIcon, "x");
//     lv_obj_clear_flag(cui_mainScreenWifiIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
//     lv_obj_set_scrollbar_mode(cui_mainScreenWifiIcon, LV_SCROLLBAR_MODE_OFF);
// #ifdef __XTOUCH_SCREEN_50__
//     lv_obj_set_style_text_font(cui_mainScreenWifiIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
// #else
//     lv_obj_set_style_text_font(cui_mainScreenWifiIcon, &ui_font_xlcdmin, LV_PART_MAIN | LV_STATE_DEFAULT);
// #endif
//     lv_obj_t *cui_mainScreenCameraIcon;
//     cui_mainScreenCameraIcon = lv_label_create(cui_mainScreenStatusBar);
//     lv_obj_set_height(cui_mainScreenCameraIcon, LV_SIZE_CONTENT); /// 16
//     lv_obj_set_flex_grow(cui_mainScreenCameraIcon, 1);
//     lv_label_set_text(cui_mainScreenCameraIcon, "y");
//     lv_obj_clear_flag(cui_mainScreenCameraIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
//     lv_obj_set_scrollbar_mode(cui_mainScreenCameraIcon, LV_SCROLLBAR_MODE_OFF);
//     lv_obj_set_style_pad_left(cui_mainScreenCameraIcon, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//     lv_obj_set_style_pad_right(cui_mainScreenCameraIcon, 32, LV_PART_MAIN | LV_STATE_DEFAULT);
//     lv_obj_set_style_pad_top(cui_mainScreenCameraIcon, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//     lv_obj_set_style_pad_bottom(cui_mainScreenCameraIcon, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

// #ifdef __XTOUCH_SCREEN_50__
//     lv_obj_set_style_text_font(cui_mainScreenCameraIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
// #else
//     lv_obj_set_style_text_font(cui_mainScreenCameraIcon, &ui_font_xlcdmin, LV_PART_MAIN | LV_STATE_DEFAULT);
// #endif

    //1 AMSbox
    lv_obj_t *cui_mainScreenAMS;
    cui_mainScreenAMS = lv_obj_create(cui_mainScreenStatusBar);
    lv_obj_set_height(cui_mainScreenAMS, lv_pct(70));
    lv_obj_set_flex_grow(cui_mainScreenAMS, 1);
    lv_obj_set_x(cui_mainScreenAMS, 386);
    lv_obj_set_y(cui_mainScreenAMS, 178);
    lv_obj_set_flex_flow(cui_mainScreenAMS, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenAMS, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_mainScreenAMS, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenAMS, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_mainScreenAMS, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_mainScreenAMS, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenAMS, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenAMS, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenAMS, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenAMS, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenAMS, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenAMS, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenAMS, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenAMS, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenAMS, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenAMS, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(cui_mainScreenAMS, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *cui_mainScreenAMSColor0;
    cui_mainScreenAMSColor0 = lv_label_create(cui_mainScreenAMS);
    lv_obj_set_height(cui_mainScreenAMSColor0, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenAMSColor0, 1);
    lv_obj_set_x(cui_mainScreenAMSColor0, 386);
    lv_obj_set_y(cui_mainScreenAMSColor0, 178);
    lv_obj_set_flex_flow(cui_mainScreenAMSColor0, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenAMSColor0, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_mainScreenAMSColor0, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenAMSColor0, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_mainScreenAMSColor0, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_mainScreenAMSColor0, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenAMSColor0, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenAMSColor0, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenAMSColor0, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenAMSColor0, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenAMSColor0, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenAMSColor0, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenAMSColor0, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenAMSColor0, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenAMSColor0, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenAMSColor0, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(cui_mainScreenAMSColor0, LV_ALIGN_CENTER);
    lv_obj_set_style_text_align(cui_mainScreenAMSColor0, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_mainScreenAMSColor0, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_mainScreenAMSColor1;
    cui_mainScreenAMSColor1 = lv_label_create(cui_mainScreenAMS);
    lv_obj_set_height(cui_mainScreenAMSColor1, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenAMSColor1, 1);
    lv_obj_set_x(cui_mainScreenAMSColor1, 386);
    lv_obj_set_y(cui_mainScreenAMSColor1, 178);
    lv_obj_set_flex_flow(cui_mainScreenAMSColor1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenAMSColor1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_mainScreenAMSColor1, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenAMSColor1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_mainScreenAMSColor1, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_mainScreenAMSColor1, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenAMSColor1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenAMSColor1, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenAMSColor1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenAMSColor1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenAMSColor1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenAMSColor1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenAMSColor1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenAMSColor1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenAMSColor1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenAMSColor1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(cui_mainScreenAMSColor1, LV_ALIGN_CENTER);
    lv_obj_set_style_text_align(cui_mainScreenAMSColor1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_mainScreenAMSColor1, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_mainScreenAMSColor1, "x");

    lv_obj_t *cui_mainScreenAMSColor2;
    cui_mainScreenAMSColor2 = lv_label_create(cui_mainScreenAMS);
    lv_obj_set_height(cui_mainScreenAMSColor2, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenAMSColor2, 1);
    lv_obj_set_x(cui_mainScreenAMSColor2, 386);
    lv_obj_set_y(cui_mainScreenAMSColor2, 178);
    lv_obj_set_flex_flow(cui_mainScreenAMSColor2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenAMSColor2, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_mainScreenAMSColor2, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenAMSColor2, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_mainScreenAMSColor2, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_mainScreenAMSColor2, lv_color_hex(0x00FF00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenAMSColor2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenAMSColor2, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenAMSColor2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenAMSColor2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenAMSColor2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenAMSColor2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenAMSColor2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenAMSColor2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenAMSColor2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenAMSColor2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(cui_mainScreenAMSColor2, LV_ALIGN_CENTER);
    lv_obj_set_style_text_align(cui_mainScreenAMSColor2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_mainScreenAMSColor2, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_mainScreenAMSColor2, "x");

    lv_obj_t *cui_mainScreenAMSColor3;
    cui_mainScreenAMSColor3 = lv_label_create(cui_mainScreenAMS);
    lv_obj_set_height(cui_mainScreenAMSColor3, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenAMSColor3, 1);
    lv_obj_set_x(cui_mainScreenAMSColor3, 386);
    lv_obj_set_y(cui_mainScreenAMSColor3, 178);
    lv_obj_set_flex_flow(cui_mainScreenAMSColor3, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenAMSColor3, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_mainScreenAMSColor3, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenAMSColor3, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_mainScreenAMSColor3, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_mainScreenAMSColor3, lv_color_hex(0x0000FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenAMSColor3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenAMSColor3, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenAMSColor3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenAMSColor3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenAMSColor3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenAMSColor3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenAMSColor3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenAMSColor3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenAMSColor3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenAMSColor3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(cui_mainScreenAMSColor3, LV_ALIGN_CENTER);
    lv_obj_set_style_text_align(cui_mainScreenAMSColor3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_mainScreenAMSColor3, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_mainScreenAMSColor3, "x");

    lv_obj_t *cui_mainScreenAMSColor4;
    cui_mainScreenAMSColor4 = lv_label_create(cui_mainScreenAMS);
    lv_obj_set_height(cui_mainScreenAMSColor4, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenAMSColor4, 1);
    lv_obj_set_x(cui_mainScreenAMSColor4, 386);
    lv_obj_set_y(cui_mainScreenAMSColor4, 178);
    lv_obj_set_flex_flow(cui_mainScreenAMSColor4, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenAMSColor4, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_mainScreenAMSColor4, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenAMSColor4, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_mainScreenAMSColor4, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_mainScreenAMSColor4, lv_color_hex(0x00FFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenAMSColor4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenAMSColor4, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenAMSColor4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenAMSColor4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenAMSColor4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenAMSColor4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenAMSColor4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenAMSColor4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenAMSColor4, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenAMSColor4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(cui_mainScreenAMSColor4, LV_ALIGN_CENTER);
    lv_obj_set_style_text_align(cui_mainScreenAMSColor4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_mainScreenAMSColor4, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_mainScreenAMSColor4, "x");

    /* 2番ボックス: サムネ/ボタンの Player */
    lv_obj_t *cui_mainScreenPlayer;
    cui_mainScreenPlayer = lv_obj_create(cui_mainScreenLeft);
    lv_obj_set_width(cui_mainScreenPlayer, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenPlayer, 3);
    lv_obj_set_x(cui_mainScreenPlayer, 386);
    lv_obj_set_y(cui_mainScreenPlayer, 178);
    lv_obj_set_flex_flow(cui_mainScreenPlayer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_mainScreenPlayer, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_mainScreenPlayer, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenPlayer, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_mainScreenPlayer, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenPlayer, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenPlayer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenPlayer, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenPlayer, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenPlayer, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenPlayer, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenPlayer, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenPlayer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenPlayer, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenPlayer, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 3番ボックス: ロゴ＋メッセージ（HOME専用の mainStatus ボックス） */
    ui_mainStatusComponent = lv_obj_create(cui_mainScreenLeft);
    lv_obj_set_width(ui_mainStatusComponent, lv_pct(100));
    lv_obj_set_flex_grow(ui_mainStatusComponent, 1);
    lv_obj_set_flex_flow(ui_mainStatusComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_mainStatusComponent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(ui_mainStatusComponent,
                      LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE |
                          LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE |
                          LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(ui_mainStatusComponent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(ui_mainStatusComponent, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_mainStatusComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_mainStatusComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_mainStatusComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_mainStatusComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_mainStatusComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_mainStatusComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_mainStatusComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_mainStatusComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_mainStatusComponent, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_mainStatusComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* ロゴ */
    lv_obj_t *cui_mainScreenStatusLogo;
    cui_mainScreenStatusLogo = lv_img_create(ui_mainStatusComponent);
    lv_obj_set_width(cui_mainScreenStatusLogo, 150);
    lv_obj_set_height(cui_mainScreenStatusLogo, LV_SIZE_CONTENT);
    lv_obj_clear_flag(cui_mainScreenStatusLogo,
                      LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
                          LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC |
                          LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_size(cui_mainScreenStatusLogo, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_scrollbar_mode(cui_mainScreenStatusLogo, LV_SCROLLBAR_MODE_OFF);
    lv_img_set_src(cui_mainScreenStatusLogo, &img_logo2);

    /* ロゴ下メッセージ (HOME専用) */
    lv_obj_t *cui_mainScreenStatusCaption;
    cui_mainScreenStatusCaption = lv_label_create(ui_mainStatusComponent);
    lv_obj_set_width(cui_mainScreenStatusCaption, LV_SIZE_CONTENT);
    lv_obj_set_height(cui_mainScreenStatusCaption, LV_SIZE_CONTENT);
    lv_label_set_text(cui_mainScreenStatusCaption, "N/A");
    lv_obj_set_style_text_font(cui_mainScreenStatusCaption, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
#ifdef __XTOUCH_SCREEN_50__
    /* 5インチではロゴ下メッセージは使わないので非表示にする（HOMEも同様のポリシー） */
    lv_obj_add_flag(cui_mainScreenStatusCaption, LV_OBJ_FLAG_HIDDEN);
#endif

    /* 4番ボックス: Preheat ボタン（HOME専用の Preheat ボックス） */
    lv_obj_t *ui_mainPreheatBox;
    ui_mainPreheatBox = lv_obj_create(cui_mainScreenLeft);
    lv_obj_set_width(ui_mainPreheatBox, lv_pct(100));
    lv_obj_set_flex_grow(ui_mainPreheatBox, 1);
    lv_obj_set_flex_flow(ui_mainPreheatBox, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_mainPreheatBox, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(ui_mainPreheatBox, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE |
                                         LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE |
                                         LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(ui_mainPreheatBox, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(ui_mainPreheatBox, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_mainPreheatBox, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_mainPreheatBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_mainPreheatBox, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_mainPreheatBox, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_mainPreheatBox, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_mainPreheatBox, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_mainPreheatBox, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_mainPreheatBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 4番ボックス内に Preheat ボタン行を素直に定義 */
    lv_obj_t *preHeatBox1;
    preHeatBox1 = lv_obj_create(ui_mainPreheatBox);
    lv_obj_set_width(preHeatBox1, lv_pct(100));
    lv_obj_set_height(preHeatBox1, lv_pct(50));
    lv_obj_set_x(preHeatBox1, 386);
    lv_obj_set_y(preHeatBox1, 178);
    lv_obj_set_flex_flow(preHeatBox1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(preHeatBox1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(preHeatBox1, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(preHeatBox1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(preHeatBox1, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(preHeatBox1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(preHeatBox1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(preHeatBox1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(preHeatBox1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(preHeatBox1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(preHeatBox1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(preHeatBox1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(preHeatBox1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(preHeatBox1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(preHeatBox1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *preHeatBox2;
    preHeatBox2 = lv_obj_create(ui_mainPreheatBox);
    lv_obj_set_width(preHeatBox2, lv_pct(100));
    lv_obj_set_height(preHeatBox2, lv_pct(50));
    lv_obj_set_x(preHeatBox2, 386);
    lv_obj_set_y(preHeatBox2, 178);
    lv_obj_set_flex_flow(preHeatBox2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(preHeatBox2, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(preHeatBox2, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(preHeatBox2, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(preHeatBox2, lv_color_hex(0x550000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(preHeatBox2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(preHeatBox2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(preHeatBox2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(preHeatBox2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(preHeatBox2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(preHeatBox2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(preHeatBox2, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(preHeatBox2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(preHeatBox2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(preHeatBox2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* Preheat ボタン（PLA / ABS / Off）: 5インチと同様に「コンテナ＋ラベル」の一まとまりで扱う */
    lv_obj_t *homePreHeatBtn1Cont = lv_obj_create(preHeatBox1);
    lv_obj_set_width(homePreHeatBtn1Cont, lv_pct(37));
    lv_obj_set_height(homePreHeatBtn1Cont, lv_pct(100));
    lv_obj_set_flex_grow(homePreHeatBtn1Cont, 2);
    lv_obj_set_align(homePreHeatBtn1Cont, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(homePreHeatBtn1Cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(homePreHeatBtn1Cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(homePreHeatBtn1Cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(homePreHeatBtn1Cont, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(homePreHeatBtn1Cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(homePreHeatBtn1Cont, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(homePreHeatBtn1Cont, lv_color_hex(0x007700), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(homePreHeatBtn1Cont, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(homePreHeatBtn1Cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(homePreHeatBtn1Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(homePreHeatBtn1Cont, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(homePreHeatBtn1Cont, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(homePreHeatBtn1Cont, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *homePreHeatButton1 = lv_label_create(homePreHeatBtn1Cont);
    lv_obj_set_width(homePreHeatButton1, LV_SIZE_CONTENT);
    lv_obj_set_height(homePreHeatButton1, LV_SIZE_CONTENT);
    lv_label_set_text(homePreHeatButton1, "PLA");
    lv_obj_set_style_text_align(homePreHeatButton1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(homePreHeatButton1, lv_font_middle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(homePreHeatButton1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DISABLED);

    lv_obj_t *homePreHeatBtn2Cont = lv_obj_create(preHeatBox1);
    lv_obj_set_width(homePreHeatBtn2Cont, lv_pct(37));
    lv_obj_set_height(homePreHeatBtn2Cont, lv_pct(100));
    lv_obj_set_flex_grow(homePreHeatBtn2Cont, 2);
    lv_obj_set_align(homePreHeatBtn2Cont, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(homePreHeatBtn2Cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(homePreHeatBtn2Cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(homePreHeatBtn2Cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(homePreHeatBtn2Cont, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(homePreHeatBtn2Cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(homePreHeatBtn2Cont, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(homePreHeatBtn2Cont, lv_color_hex(0x000077), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(homePreHeatBtn2Cont, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(homePreHeatBtn2Cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(homePreHeatBtn2Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(homePreHeatBtn2Cont, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(homePreHeatBtn2Cont, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(homePreHeatBtn2Cont, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *homePreHeatButton2 = lv_label_create(homePreHeatBtn2Cont);
    lv_obj_set_width(homePreHeatButton2, LV_SIZE_CONTENT);
    lv_obj_set_height(homePreHeatButton2, LV_SIZE_CONTENT);
    lv_label_set_text(homePreHeatButton2, "ABS");
    lv_obj_set_style_text_align(homePreHeatButton2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(homePreHeatButton2, lv_font_middle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(homePreHeatButton2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DISABLED);

    lv_obj_t *homePreHeatBtn3Cont = lv_obj_create(preHeatBox2);
    lv_obj_set_width(homePreHeatBtn3Cont, lv_pct(26));
    lv_obj_set_height(homePreHeatBtn3Cont, lv_pct(100));
    lv_obj_set_flex_grow(homePreHeatBtn3Cont, 2);
    lv_obj_set_align(homePreHeatBtn3Cont, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(homePreHeatBtn3Cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(homePreHeatBtn3Cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(homePreHeatBtn3Cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(homePreHeatBtn3Cont, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(homePreHeatBtn3Cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(homePreHeatBtn3Cont, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(homePreHeatBtn3Cont, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(homePreHeatBtn3Cont, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(homePreHeatBtn3Cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(homePreHeatBtn3Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(homePreHeatBtn3Cont, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(homePreHeatBtn3Cont, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(homePreHeatBtn3Cont, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *homePreHeatButton3 = lv_label_create(homePreHeatBtn3Cont);
    lv_obj_set_width(homePreHeatButton3, LV_SIZE_CONTENT);
    lv_obj_set_height(homePreHeatButton3, LV_SIZE_CONTENT);
    lv_label_set_text(homePreHeatButton3, "Off");
    lv_obj_set_style_text_align(homePreHeatButton3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(homePreHeatButton3, lv_font_middle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(homePreHeatButton3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DISABLED);

    /* HOME 側 Preheat ボタンは直接 onPreHeatXXX にバインドする */
    lv_obj_add_event_cb(homePreHeatBtn1Cont, onPreHeatPLA, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(homePreHeatBtn2Cont, onPreHeatABS, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(homePreHeatBtn3Cont, onPreHeatOff, LV_EVENT_CLICKED, NULL);

    /* 5番ボックス: レイヤー/進捗用の専用コンテナ */
    lv_obj_t *cui_mainScreenProgressBox;
    cui_mainScreenProgressBox = lv_obj_create(cui_mainScreenLeft);
    lv_obj_set_width(cui_mainScreenProgressBox, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenProgressBox, 1);
    lv_obj_set_flex_flow(cui_mainScreenProgressBox, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_mainScreenProgressBox, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_mainScreenProgressBox, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenProgressBox, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_mainScreenProgressBox, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenProgressBox, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenProgressBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenProgressBox, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenProgressBox, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenProgressBox, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenProgressBox, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenProgressBox, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenProgressBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_mainScreenController;
    cui_mainScreenController = lv_obj_create(cui_mainScreenPlayer);
    lv_obj_set_width(cui_mainScreenController, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenController, 4);
    lv_obj_set_x(cui_mainScreenController, 364);
    lv_obj_set_y(cui_mainScreenController, 183);
    lv_obj_set_flex_flow(cui_mainScreenController, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenController, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_mainScreenController, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenController, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_mainScreenController, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_mainScreenController, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenController, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenController, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenController, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenController, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenController, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenController, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenController, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenController, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenController, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_mainScreenNozzleIcon;
#if defined(__XTOUCH_SCREEN_50__)
    /* 5インチ: 非LAN はサムネイル表示、LAN モードは 2.8 同様ラベルのみ */
    if (!xTouchConfig.xTouchLanOnlyMode)
    {
        cui_mainScreenNozzleIcon = lv_img_create(cui_mainScreenController);
        lv_obj_set_width(cui_mainScreenNozzleIcon, 150);
        lv_obj_set_height(cui_mainScreenNozzleIcon, 150);
        lv_img_set_size_mode(cui_mainScreenNozzleIcon, LV_IMG_SIZE_MODE_REAL);
        lv_obj_set_style_bg_color(cui_mainScreenNozzleIcon, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(cui_mainScreenNozzleIcon, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(cui_mainScreenNozzleIcon, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
        ui_thumb_set_img_src_from_slot(cui_mainScreenNozzleIcon, 0);
        ui_homeThumbImg = cui_mainScreenNozzleIcon;
    }
    else
    {
        ui_homeThumbImg = NULL;
        cui_mainScreenNozzleIcon = lv_label_create(cui_mainScreenController);
        lv_obj_set_width(cui_mainScreenNozzleIcon, LV_SIZE_CONTENT);
        lv_obj_set_height(cui_mainScreenNozzleIcon, LV_SIZE_CONTENT);
        lv_label_set_text(cui_mainScreenNozzleIcon, "p");
        lv_obj_clear_flag(cui_mainScreenNozzleIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
        lv_obj_set_scrollbar_mode(cui_mainScreenNozzleIcon, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_style_text_font(cui_mainScreenNozzleIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
#else
    /* 2.8: サムネイル非表示 */
    cui_mainScreenNozzleIcon = lv_label_create(cui_mainScreenController);
    lv_obj_set_width(cui_mainScreenNozzleIcon, LV_SIZE_CONTENT);  /// 50
    lv_obj_set_height(cui_mainScreenNozzleIcon, LV_SIZE_CONTENT); /// 24
    lv_label_set_text(cui_mainScreenNozzleIcon, "p");
    lv_obj_clear_flag(cui_mainScreenNozzleIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenNozzleIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_mainScreenNozzleIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif

#if defined(__XTOUCH_SCREEN_50__)
    /* 5インチ: サムネ右のボタン（横並び）＋subtask ラベル。ラッパーは幅広め。 */
    lv_obj_t *cui_mainScreenControllerRight = lv_obj_create(cui_mainScreenController);
    lv_obj_set_width(cui_mainScreenControllerRight, 340);
    lv_obj_set_height(cui_mainScreenControllerRight, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cui_mainScreenControllerRight, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_mainScreenControllerRight, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_mainScreenControllerRight, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_style_bg_opa(cui_mainScreenControllerRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenControllerRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenControllerRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenControllerRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenControllerRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    /* Pause と Stop を横並びにする行 */
    lv_obj_t *cui_mainScreenControllerRightBtnRow = lv_obj_create(cui_mainScreenControllerRight);
    lv_obj_set_width(cui_mainScreenControllerRightBtnRow, 340);
    lv_obj_set_height(cui_mainScreenControllerRightBtnRow, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cui_mainScreenControllerRightBtnRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenControllerRightBtnRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_mainScreenControllerRightBtnRow, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_style_bg_opa(cui_mainScreenControllerRightBtnRow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenControllerRightBtnRow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenControllerRightBtnRow, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenControllerRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenControllerRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif

    lv_obj_t *cui_mainScreenPlayPauseButton;
#if defined(__XTOUCH_SCREEN_50__)
    cui_mainScreenPlayPauseButton = lv_label_create(cui_mainScreenControllerRightBtnRow);
#else
    cui_mainScreenPlayPauseButton = lv_label_create(cui_mainScreenController);
#endif
    lv_obj_set_height(cui_mainScreenPlayPauseButton, LV_SIZE_CONTENT); /// 48
#ifdef __XTOUCH_SCREEN_50__
    lv_obj_set_width(cui_mainScreenPlayPauseButton, 120);
#else
    lv_obj_set_flex_grow(cui_mainScreenPlayPauseButton, 1);
#endif
    lv_obj_set_align(cui_mainScreenPlayPauseButton, LV_ALIGN_CENTER);
    lv_label_set_text(cui_mainScreenPlayPauseButton, "0");
    lv_obj_add_flag(cui_mainScreenPlayPauseButton, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_mainScreenPlayPauseButton, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenPlayPauseButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_mainScreenPlayPauseButton, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_mainScreenPlayPauseButton, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_mainScreenPlayPauseButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(cui_mainScreenPlayPauseButton, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(cui_mainScreenPlayPauseButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenPlayPauseButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenPlayPauseButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenPlayPauseButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenPlayPauseButton, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenPlayPauseButton, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenPlayPauseButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenPlayPauseButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_mainScreenPlayPauseButton, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_mainScreenPlayPauseButton, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_mainScreenStopButton;
#if defined(__XTOUCH_SCREEN_50__)
    cui_mainScreenStopButton = lv_label_create(cui_mainScreenControllerRightBtnRow);
#else
    cui_mainScreenStopButton = lv_label_create(cui_mainScreenController);
#endif
    lv_obj_set_height(cui_mainScreenStopButton, LV_SIZE_CONTENT); /// 48
#ifdef __XTOUCH_SCREEN_50__
    lv_obj_set_width(cui_mainScreenStopButton, 120);
#else
    lv_obj_set_flex_grow(cui_mainScreenStopButton, 1);
#endif
    lv_obj_set_align(cui_mainScreenStopButton, LV_ALIGN_CENTER);
    lv_label_set_text(cui_mainScreenStopButton, "1");
    lv_obj_add_flag(cui_mainScreenStopButton, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_mainScreenStopButton, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenStopButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_mainScreenStopButton, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_mainScreenStopButton, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_mainScreenStopButton, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(cui_mainScreenStopButton, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(cui_mainScreenStopButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenStopButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenStopButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenStopButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenStopButton, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenStopButton, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenStopButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenStopButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_mainScreenStopButton, lv_color_hex(0xff682a), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_mainScreenStopButton, 255, LV_PART_MAIN | LV_STATE_PRESSED);

#if defined(__XTOUCH_SCREEN_50__)
    /* Failed/Finished 時にボタン位置に表示するラベル */
    lv_obj_t *cui_mainScreenFinishedFailedLabel = lv_label_create(cui_mainScreenControllerRightBtnRow);
    lv_obj_set_width(cui_mainScreenFinishedFailedLabel, 248);
    lv_obj_set_height(cui_mainScreenFinishedFailedLabel, LV_SIZE_CONTENT);
    lv_label_set_text(cui_mainScreenFinishedFailedLabel, "Finished");
    lv_obj_set_style_text_font(cui_mainScreenFinishedFailedLabel, lv_font_middle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenFinishedFailedLabel, lv_color_hex(0xaaaaaa), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(cui_mainScreenFinishedFailedLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(cui_mainScreenFinishedFailedLabel, LV_OBJ_FLAG_HIDDEN);

    /* ボタン下に subtask_name の先頭を表示（最大約20文字想定で省略） */
    lv_obj_t *cui_mainScreenSubtaskLabel = lv_label_create(cui_mainScreenControllerRight);
    lv_obj_set_width(cui_mainScreenSubtaskLabel, 340);
    lv_obj_set_height(cui_mainScreenSubtaskLabel, LV_SIZE_CONTENT);
    lv_obj_set_style_min_height(cui_mainScreenSubtaskLabel, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_mainScreenSubtaskLabel, " ");
    /* Small と同じサイズ: 2.8=14px, 5=28px */
    lv_obj_set_style_text_font(cui_mainScreenSubtaskLabel,
#if defined(__XTOUCH_SCREEN_28__)
        &lv_font_notosans_14,
#else
        &lv_font_notosans_28,
#endif
        LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenSubtaskLabel, lv_color_hex(0xaaaaaa), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_long_mode(cui_mainScreenSubtaskLabel, LV_LABEL_LONG_CLIP);
#endif

    lv_obj_t *cui_mainScreenProgressBar;
    cui_mainScreenProgressBar = lv_slider_create(cui_mainScreenProgressBox);
    lv_slider_set_value(cui_mainScreenProgressBar, 60, LV_ANIM_OFF);
    if (lv_slider_get_mode(cui_mainScreenProgressBar) == LV_SLIDER_MODE_RANGE)
        lv_slider_set_left_value(cui_mainScreenProgressBar, 0, LV_ANIM_OFF);
    lv_obj_set_width(cui_mainScreenProgressBar, lv_pct(100));
    lv_obj_set_height(cui_mainScreenProgressBar, 14);
    lv_obj_set_align(cui_mainScreenProgressBar, LV_ALIGN_CENTER);
    lv_obj_set_flex_flow(cui_mainScreenProgressBar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenProgressBar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_mainScreenProgressBar, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE); /// Flags
    lv_obj_set_style_bg_color(cui_mainScreenProgressBar, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenProgressBar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(cui_mainScreenProgressBar, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenProgressBar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(cui_mainScreenProgressBar, lv_color_hex(0xCCCCCC), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenProgressBar, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenProgressBar, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenProgressBar, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenProgressBar, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenProgressBar, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    lv_obj_t *cui_mainScreenController1;
    cui_mainScreenController1 = lv_obj_create(cui_mainScreenProgressBox);
    lv_obj_set_width(cui_mainScreenController1, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenController1, 1);
    lv_obj_set_x(cui_mainScreenController1, 364);
    lv_obj_set_y(cui_mainScreenController1, 183);
    lv_obj_set_flex_flow(cui_mainScreenController1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenController1, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_mainScreenController1, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenController1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_mainScreenController1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_mainScreenController1, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenController1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenController1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenController1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenController1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenController1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenController1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenController1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenController1, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenController1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenController1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_mainScreenTimeLeftIcon;
    cui_mainScreenTimeLeftIcon = lv_label_create(cui_mainScreenController1);
    lv_obj_set_width(cui_mainScreenTimeLeftIcon, LV_SIZE_CONTENT);  /// 50
    lv_obj_set_height(cui_mainScreenTimeLeftIcon, LV_SIZE_CONTENT); /// 16
    lv_label_set_text(cui_mainScreenTimeLeftIcon, "2");
    lv_obj_clear_flag(cui_mainScreenTimeLeftIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenTimeLeftIcon, LV_SCROLLBAR_MODE_OFF);
#ifdef __XTOUCH_SCREEN_50__
    lv_obj_set_style_text_font(cui_mainScreenTimeLeftIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
#else
    lv_obj_set_style_text_font(cui_mainScreenTimeLeftIcon, &ui_font_xlcdmin, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif
    lv_obj_t *cui_mainScreenTimeLeft;
    cui_mainScreenTimeLeft = lv_label_create(cui_mainScreenController1);
    lv_obj_set_width(cui_mainScreenTimeLeft, LV_SIZE_CONTENT);  /// 3
    lv_obj_set_height(cui_mainScreenTimeLeft, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(cui_mainScreenTimeLeft, LV_ALIGN_CENTER);
    lv_label_set_long_mode(cui_mainScreenTimeLeft, LV_LABEL_LONG_CLIP);
    lv_label_set_text(cui_mainScreenTimeLeft, "N/A");
    lv_obj_clear_flag(cui_mainScreenTimeLeft, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenTimeLeft, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_mainScreenTimeLeft, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_mainScreenController2;
    cui_mainScreenController2 = lv_obj_create(cui_mainScreenProgressBox);
    lv_obj_set_width(cui_mainScreenController2, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenController2, 1);
    lv_obj_set_x(cui_mainScreenController2, 364);
    lv_obj_set_y(cui_mainScreenController2, 183);
    lv_obj_set_flex_flow(cui_mainScreenController2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenController2, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_mainScreenController2, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenController2, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_mainScreenController2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_mainScreenController2, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenController2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenController2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenController2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenController2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenController2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenController2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenController2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenController2, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenController2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenController2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_mainScreenLayerIcon;
    cui_mainScreenLayerIcon = lv_label_create(cui_mainScreenController2);
    lv_obj_set_width(cui_mainScreenLayerIcon, LV_SIZE_CONTENT);  /// 50
    lv_obj_set_height(cui_mainScreenLayerIcon, LV_SIZE_CONTENT); /// 16
    lv_label_set_text(cui_mainScreenLayerIcon, "3");
    lv_obj_clear_flag(cui_mainScreenLayerIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenLayerIcon, LV_SCROLLBAR_MODE_OFF);
#ifdef __XTOUCH_SCREEN_50__
    lv_obj_set_style_text_font(cui_mainScreenLayerIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
#else
    lv_obj_set_style_text_font(cui_mainScreenLayerIcon, &ui_font_xlcdmin, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif
    lv_obj_t *cui_mainScreenLayer;
    cui_mainScreenLayer = lv_label_create(cui_mainScreenController2);
    lv_obj_set_width(cui_mainScreenLayer, LV_SIZE_CONTENT);  /// 3
    lv_obj_set_height(cui_mainScreenLayer, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(cui_mainScreenLayer, LV_ALIGN_CENTER);
    lv_label_set_long_mode(cui_mainScreenLayer, LV_LABEL_LONG_CLIP);
    lv_label_set_text(cui_mainScreenLayer, "0/0");
    lv_obj_clear_flag(cui_mainScreenLayer, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenLayer, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_mainScreenLayer, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_mainScreenCentral;
    cui_mainScreenCentral = lv_obj_create(cui_mainScreenLeft);
    lv_obj_set_width(cui_mainScreenCentral, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenCentral, 1);
    lv_obj_set_x(cui_mainScreenCentral, 386);
    lv_obj_set_y(cui_mainScreenCentral, 178);
    lv_obj_set_flex_flow(cui_mainScreenCentral, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenCentral, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_mainScreenCentral, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenCentral, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_mainScreenCentral, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenCentral, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenCentral, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenCentral, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenCentral, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenCentral, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenCentral, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenCentral, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenCentral, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenCentral, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenCentral, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_mainScreenSpeedIcon;
    cui_mainScreenSpeedIcon = lv_label_create(cui_mainScreenCentral);
    lv_obj_set_width(cui_mainScreenSpeedIcon, LV_SIZE_CONTENT);  /// 50
    lv_obj_set_height(cui_mainScreenSpeedIcon, LV_SIZE_CONTENT); /// 16
    lv_label_set_text(cui_mainScreenSpeedIcon, "h");
    lv_obj_clear_flag(cui_mainScreenSpeedIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenSpeedIcon, LV_SCROLLBAR_MODE_OFF);
#ifdef __XTOUCH_SCREEN_50__
    lv_obj_set_style_text_font(cui_mainScreenSpeedIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
#else
    lv_obj_set_style_text_font(cui_mainScreenSpeedIcon, &ui_font_xlcdmin, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif
    lv_obj_t *cui_mainScreenSpeedDropDown;
    cui_mainScreenSpeedDropDown = lv_dropdown_create(cui_mainScreenCentral);
    lv_dropdown_set_options(cui_mainScreenSpeedDropDown, "Silent\nStandard\nSport\nLudicrous");
    lv_obj_set_width(cui_mainScreenSpeedDropDown, lv_pct(80));
    lv_obj_set_height(cui_mainScreenSpeedDropDown, LV_SIZE_CONTENT);           /// 1
    lv_obj_add_state(cui_mainScreenSpeedDropDown, LV_STATE_PRESSED);           /// States
    lv_obj_add_flag(cui_mainScreenSpeedDropDown, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_set_style_bg_color(cui_mainScreenSpeedDropDown, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenSpeedDropDown, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenSpeedDropDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_mainScreenSpeedDropDown, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_dropdown_set_selected(cui_mainScreenSpeedDropDown, 1);

    lv_obj_set_style_text_color(cui_mainScreenSpeedDropDown, lv_color_hex(0x2aff00), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenSpeedDropDown, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_text_letter_space(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), 32, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), 0, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), 32, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), 255, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), lv_color_hex(0x2aff00), LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), 255, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_PRESSED);
    lv_obj_set_style_text_opa(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), 255, LV_PART_SELECTED | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), lv_color_hex(0x00AA00), LV_PART_SELECTED | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(lv_dropdown_get_list(cui_mainScreenSpeedDropDown), 255, LV_PART_SELECTED | LV_STATE_PRESSED);

    lv_obj_t *cui_mainScreenRight;
    cui_mainScreenRight = lv_obj_create(cui_homeComponent);
    lv_obj_set_width(cui_mainScreenRight, lv_pct(25));
    lv_obj_set_height(cui_mainScreenRight, lv_pct(100));
    lv_obj_set_x(cui_mainScreenRight, 386);
    lv_obj_set_y(cui_mainScreenRight, 178);
    lv_obj_set_flex_flow(cui_mainScreenRight, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_mainScreenRight, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_mainScreenRight, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenRight, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_mainScreenRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_mainScreenRight, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenRight, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenRight, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenRight, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenRight, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenRight, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_mainScreenLightButton;
    cui_mainScreenLightButton = lv_obj_create(cui_mainScreenRight);
    lv_obj_set_height(cui_mainScreenLightButton, lv_pct(20));
    lv_obj_set_width(cui_mainScreenLightButton, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenLightButton, 1);
    lv_obj_set_x(cui_mainScreenLightButton, 386);
    lv_obj_set_y(cui_mainScreenLightButton, 178);
    lv_obj_set_flex_flow(cui_mainScreenLightButton, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenLightButton, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // lv_obj_add_state(cui_mainScreenLightButton, LV_STATE_CHECKED);                                                                                                                                                                                                          /// States
    lv_obj_clear_flag(cui_mainScreenLightButton, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenLightButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_mainScreenLightButton, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenLightButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenLightButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenLightButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenLightButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenLightButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenLightButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenLightButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenLightButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenLightButton, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenLightButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(cui_mainScreenLightButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_border_opa(cui_mainScreenLightButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_border_width(cui_mainScreenLightButton, 3, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(cui_mainScreenLightButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cui_mainScreenLightButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);

    lv_obj_t *cui_mainScreenLightButtonIcon;
    cui_mainScreenLightButtonIcon = lv_label_create(cui_mainScreenLightButton);
    lv_obj_set_width(cui_mainScreenLightButtonIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_mainScreenLightButtonIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(cui_mainScreenLightButtonIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_mainScreenLightButtonIcon, "w");
    lv_obj_clear_flag(cui_mainScreenLightButtonIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenLightButtonIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_mainScreenLightButtonIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_mainScreenLCDButton;
    cui_mainScreenLCDButton = lv_obj_create(cui_mainScreenRight);
    lv_obj_set_height(cui_mainScreenLCDButton, lv_pct(15));
    lv_obj_set_width(cui_mainScreenLCDButton, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenLCDButton, 1);
    lv_obj_set_x(cui_mainScreenLCDButton, 386);
    lv_obj_set_y(cui_mainScreenLCDButton, 178);
    lv_obj_set_flex_flow(cui_mainScreenLCDButton, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenLCDButton, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // lv_obj_add_state(cui_mainScreenLCDButton, LV_STATE_CHECKED);                                                                                                                                                                                                          /// States
    lv_obj_clear_flag(cui_mainScreenLCDButton, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenLCDButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_mainScreenLCDButton, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenLCDButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenLCDButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenLCDButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenLCDButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenLCDButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenLCDButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenLCDButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenLCDButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenLCDButton, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenLCDButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(cui_mainScreenLCDButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_border_opa(cui_mainScreenLCDButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_border_width(cui_mainScreenLCDButton, 3, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(cui_mainScreenLCDButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cui_mainScreenLCDButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);

    lv_obj_t *cui_mainScreenLCDButtonIcon;
    cui_mainScreenLCDButtonIcon = lv_label_create(cui_mainScreenLCDButton);
    lv_obj_set_width(cui_mainScreenLCDButtonIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_mainScreenLCDButtonIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(cui_mainScreenLCDButtonIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_mainScreenLCDButtonIcon, LV_SYMBOL_IMAGE " LCD");
    lv_obj_clear_flag(cui_mainScreenLCDButtonIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenLCDButtonIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_mainScreenLCDButtonIcon, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);



    lv_obj_t *cui_mainScreenNeoPixelButton;
    cui_mainScreenNeoPixelButton = lv_obj_create(cui_mainScreenRight);
    lv_obj_set_height(cui_mainScreenNeoPixelButton, lv_pct(15));
    lv_obj_set_width(cui_mainScreenNeoPixelButton, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenNeoPixelButton, 1);
    lv_obj_set_x(cui_mainScreenNeoPixelButton, 386);
    lv_obj_set_y(cui_mainScreenNeoPixelButton, 178);
    lv_obj_set_flex_flow(cui_mainScreenNeoPixelButton, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenNeoPixelButton, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    //lv_obj_add_state(cui_mainScreenNeoPixelButton, LV_STATE_CHECKED);                                                                                                                                                                                                          /// States
    lv_obj_clear_flag(cui_mainScreenNeoPixelButton, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenNeoPixelButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_mainScreenNeoPixelButton, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenNeoPixelButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenNeoPixelButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenNeoPixelButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenNeoPixelButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenNeoPixelButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenNeoPixelButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenNeoPixelButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenNeoPixelButton, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenNeoPixelButton, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenNeoPixelButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(cui_mainScreenNeoPixelButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_border_opa(cui_mainScreenNeoPixelButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_border_width(cui_mainScreenNeoPixelButton, 3, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(cui_mainScreenNeoPixelButton, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(cui_mainScreenNeoPixelButton, 255, LV_PART_MAIN | LV_STATE_CHECKED);

    lv_obj_t *cui_mainScreenNeoPixelButtonIcon;
    cui_mainScreenNeoPixelButtonIcon = lv_label_create(cui_mainScreenNeoPixelButton);
    lv_obj_set_width(cui_mainScreenNeoPixelButtonIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_mainScreenNeoPixelButtonIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(cui_mainScreenNeoPixelButtonIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_mainScreenNeoPixelButtonIcon, " LEDBAR");
    lv_obj_clear_flag(cui_mainScreenNeoPixelButtonIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenNeoPixelButtonIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_mainScreenNeoPixelButtonIcon, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    if (xTouchConfig.xTouchNeoPixelNumValue == 0)
    {
        lv_obj_add_flag(cui_mainScreenNeoPixelButton, LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_t *cui_mainScreenTemps;
    cui_mainScreenTemps = lv_obj_create(cui_mainScreenRight);
    if (xTouchConfig.xTouchNeoPixelNumValue == 0)
    {
        lv_obj_set_height(cui_mainScreenTemps, lv_pct(65));
    }else{
        lv_obj_set_height(cui_mainScreenTemps, lv_pct(50));
    }
    lv_obj_set_width(cui_mainScreenTemps, lv_pct(100));
    lv_obj_set_x(cui_mainScreenTemps, 386);
    lv_obj_set_y(cui_mainScreenTemps, 178);
    lv_obj_set_flex_flow(cui_mainScreenTemps, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_mainScreenTemps, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_mainScreenTemps, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenTemps, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_mainScreenTemps, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenTemps, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenTemps, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenTemps, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenTemps, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenTemps, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenTemps, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenTemps, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenTemps, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenTemps, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenTemps, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_mainScreenNozzleTemp;
    cui_mainScreenNozzleTemp = lv_obj_create(cui_mainScreenTemps);
    lv_obj_set_width(cui_mainScreenNozzleTemp, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenNozzleTemp, 1);
    lv_obj_set_x(cui_mainScreenNozzleTemp, 386);
    lv_obj_set_y(cui_mainScreenNozzleTemp, 178);
    lv_obj_set_flex_flow(cui_mainScreenNozzleTemp, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenNozzleTemp, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_mainScreenNozzleTemp, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenNozzleTemp, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_mainScreenNozzleTemp, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenNozzleTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenNozzleTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenNozzleTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenNozzleTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenNozzleTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenNozzleTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenNozzleTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenNozzleTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenNozzleTemp, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenNozzleTemp, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_mainScreenNozzleTemp, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_mainScreenNozzleTemp, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_mainScreenNozzleTempIcon;
    cui_mainScreenNozzleTempIcon = lv_label_create(cui_mainScreenNozzleTemp);
    lv_obj_set_width(cui_mainScreenNozzleTempIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_mainScreenNozzleTempIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(cui_mainScreenNozzleTempIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_mainScreenNozzleTempIcon, "f");
    lv_obj_set_style_text_color(cui_mainScreenNozzleTempIcon, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenNozzleTempIcon, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
#ifdef __XTOUCH_SCREEN_50__
    lv_obj_set_style_text_font(cui_mainScreenNozzleTempIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
#else
    lv_obj_set_style_text_font(cui_mainScreenNozzleTempIcon, &ui_font_xlcdmin, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif
    lv_obj_t *cui_mainScreenNozzleTempValue;
    cui_mainScreenNozzleTempValue = lv_label_create(cui_mainScreenNozzleTemp);
    lv_obj_set_width(cui_mainScreenNozzleTempValue, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_mainScreenNozzleTempValue, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(cui_mainScreenNozzleTempValue, LV_ALIGN_CENTER);
    lv_label_set_text(cui_mainScreenNozzleTempValue, "--");
    lv_obj_set_style_text_font(cui_mainScreenNozzleTempValue, lv_font_middle, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_mainScreenBedTemp;
    cui_mainScreenBedTemp = lv_obj_create(cui_mainScreenTemps);
    lv_obj_set_width(cui_mainScreenBedTemp, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenBedTemp, 1);
    lv_obj_set_x(cui_mainScreenBedTemp, 386);
    lv_obj_set_y(cui_mainScreenBedTemp, 178);
    lv_obj_set_flex_flow(cui_mainScreenBedTemp, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenBedTemp, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_mainScreenBedTemp, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenBedTemp, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_mainScreenBedTemp, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenBedTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenBedTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenBedTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenBedTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenBedTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenBedTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenBedTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenBedTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenBedTemp, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenBedTemp, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_mainScreenBedTemp, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_mainScreenBedTemp, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_mainScreenBedTempIcon;
    cui_mainScreenBedTempIcon = lv_label_create(cui_mainScreenBedTemp);
    lv_obj_set_width(cui_mainScreenBedTempIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_mainScreenBedTempIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(cui_mainScreenBedTempIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_mainScreenBedTempIcon, "e");
    lv_obj_set_style_text_color(cui_mainScreenBedTempIcon, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenBedTempIcon, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
#ifdef __XTOUCH_SCREEN_50__
    lv_obj_set_style_text_font(cui_mainScreenBedTempIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
#else
    lv_obj_set_style_text_font(cui_mainScreenBedTempIcon, &ui_font_xlcdmin, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif
    lv_obj_t *cui_mainScreenBedTempValue;
    cui_mainScreenBedTempValue = lv_label_create(cui_mainScreenBedTemp);
    lv_obj_set_width(cui_mainScreenBedTempValue, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_mainScreenBedTempValue, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(cui_mainScreenBedTempValue, LV_ALIGN_CENTER);
    lv_label_set_text(cui_mainScreenBedTempValue, "--");
    lv_obj_set_style_text_font(cui_mainScreenBedTempValue, lv_font_middle, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_mainScreenChamberTemp;
    cui_mainScreenChamberTemp = lv_obj_create(cui_mainScreenTemps);
    lv_obj_set_width(cui_mainScreenChamberTemp, lv_pct(100));
    lv_obj_set_flex_grow(cui_mainScreenChamberTemp, 1);
    lv_obj_set_x(cui_mainScreenChamberTemp, 386);
    lv_obj_set_y(cui_mainScreenChamberTemp, 178);
    lv_obj_set_flex_flow(cui_mainScreenChamberTemp, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_mainScreenChamberTemp, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_mainScreenChamberTemp, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_mainScreenChamberTemp, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_mainScreenChamberTemp, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_mainScreenChamberTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_mainScreenChamberTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_mainScreenChamberTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_mainScreenChamberTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_mainScreenChamberTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_mainScreenChamberTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_mainScreenChamberTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_mainScreenChamberTemp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_mainScreenChamberTemp, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenChamberTemp, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    if (xtouch_bblp_is_p1Series() && !xTouchConfig.xTouchChamberSensorEnabled)
    {
        lv_obj_add_flag(cui_mainScreenChamberTemp, LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_t *cui_mainScreenChamberTempIcon;
    cui_mainScreenChamberTempIcon = lv_label_create(cui_mainScreenChamberTemp);
    lv_obj_set_width(cui_mainScreenChamberTempIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_mainScreenChamberTempIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(cui_mainScreenChamberTempIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_mainScreenChamberTempIcon, "g");
    lv_obj_set_style_text_color(cui_mainScreenChamberTempIcon, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_mainScreenChamberTempIcon, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
#ifdef __XTOUCH_SCREEN_50__
    lv_obj_set_style_text_font(cui_mainScreenChamberTempIcon, &ui_font_xlcd, LV_PART_MAIN | LV_STATE_DEFAULT);
#else
    lv_obj_set_style_text_font(cui_mainScreenChamberTempIcon, &ui_font_xlcdmin, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif
    lv_obj_t *cui_mainScreenChamberTempValue;
    cui_mainScreenChamberTempValue = lv_label_create(cui_mainScreenChamberTemp);
    lv_obj_set_width(cui_mainScreenChamberTempValue, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_mainScreenChamberTempValue, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(cui_mainScreenChamberTempValue, LV_ALIGN_CENTER);
    lv_label_set_text(cui_mainScreenChamberTempValue, "--");
    lv_obj_set_style_text_font(cui_mainScreenChamberTempValue, lv_font_middle, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t **children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_HOMECOMPONENT_NUM);
    children[UI_COMP_HOMECOMPONENT_HOMECOMPONENT] = cui_homeComponent;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT] = cui_mainScreenLeft;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENSTATUSBAR] = cui_mainScreenStatusBar;
    // children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENSTATUSBAR_MAINSCREENWIFIICON] = cui_mainScreenWifiIcon;
    // children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENSTATUSBAR_MAINSCREENCAMERAICON] = cui_mainScreenCameraIcon;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENSTATUSBAR_MAINSCREENAMS] = cui_mainScreenAMS;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENSTATUSBAR_MAINSCREENAMS_MAINSCREENAMSCOLOR0] = cui_mainScreenAMSColor0;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENSTATUSBAR_MAINSCREENAMS_MAINSCREENAMSCOLOR1] = cui_mainScreenAMSColor1;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENSTATUSBAR_MAINSCREENAMS_MAINSCREENAMSCOLOR2] = cui_mainScreenAMSColor2;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENSTATUSBAR_MAINSCREENAMS_MAINSCREENAMSCOLOR3] = cui_mainScreenAMSColor3;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENSTATUSBAR_MAINSCREENAMS_MAINSCREENAMSCOLOR4] = cui_mainScreenAMSColor4;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENIDLE] = ui_mainStatusComponent;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER] = cui_mainScreenPlayer;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER] = cui_mainScreenController;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER_MAINSCREENNOZZLEICON] = cui_mainScreenNozzleIcon;
#if defined(__XTOUCH_SCREEN_50__)
    if (!xTouchConfig.xTouchLanOnlyMode)
    {
        lv_msg_subsribe_obj(XTOUCH_ON_OTHER_PRINTER_UPDATE, cui_mainScreenNozzleIcon, NULL);
        lv_obj_add_event_cb(cui_mainScreenNozzleIcon, ui_event_home_thumb_update, LV_EVENT_MSG_RECEIVED, NULL);
    }
#endif
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER_MAINSCREENPLAYPAUSEBUTTON] = cui_mainScreenPlayPauseButton;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER_MAINSCREENSTOPBUTTON] = cui_mainScreenStopButton;
#if defined(__XTOUCH_SCREEN_50__)
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER_MAINSCREENSUBTASKLABEL] = cui_mainScreenSubtaskLabel;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER_MAINSCREENFINISHEDFAILEDLABEL] = cui_mainScreenFinishedFailedLabel;
#else
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER_MAINSCREENSUBTASKLABEL] = NULL;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER_MAINSCREENFINISHEDFAILEDLABEL] = NULL;
#endif
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPROGRESSBOX] = cui_mainScreenProgressBox;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENPROGRESSBAR] = cui_mainScreenProgressBar;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER1] = cui_mainScreenController1;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER1_MAINSCREENTIMELEFTICON] = cui_mainScreenTimeLeftIcon;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER1_MAINSCREENTIMELEFT] = cui_mainScreenTimeLeft;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER2] = cui_mainScreenController2;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER2_MAINSCREENLAYERICON] = cui_mainScreenLayerIcon;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPLAYER_MAINSCREENCONTROLLER2_MAINSCREENLAYER] = cui_mainScreenLayer;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENCENTRAL] = cui_mainScreenCentral;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENCENTRAL_MAINSCREENSPEEDICON] = cui_mainScreenSpeedIcon;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENCENTRAL_MAINSCREENSPEEDDROPDOWN] = cui_mainScreenSpeedDropDown;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENRIGHT] = cui_mainScreenRight;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENRIGHT_MAINSCREENLIGHTBUTTON] = cui_mainScreenLightButton;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENRIGHT_MAINSCREENLIGHTBUTTON_MAINSCREENLIGHTBUTTONICON] = cui_mainScreenLightButtonIcon;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENRIGHT_MAINSCREENNEOPIXELBUTTON] = cui_mainScreenNeoPixelButton;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENRIGHT_MAINSCREENNEOPIXELBUTTON_MAINSCREENNEOPIXELBUTTONICON] = cui_mainScreenNeoPixelButtonIcon;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENRIGHT_MAINSCREENTEMPS] = cui_mainScreenTemps;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENRIGHT_MAINSCREENTEMPS_MAINSCREENNOZZLETEMP] = cui_mainScreenNozzleTemp;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENRIGHT_MAINSCREENTEMPS_MAINSCREENNOZZLETEMP_MAINSCREENNOZZLETEMPICON] = cui_mainScreenNozzleTempIcon;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENRIGHT_MAINSCREENTEMPS_MAINSCREENNOZZLETEMP_MAINSCREENNOZZLETEMPVALUE] = cui_mainScreenNozzleTempValue;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENRIGHT_MAINSCREENTEMPS_MAINSCREENBEDTEMP] = cui_mainScreenBedTemp;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENRIGHT_MAINSCREENTEMPS_MAINSCREENBEDTEMP_MAINSCREENBEDTEMPICON] = cui_mainScreenBedTempIcon;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENRIGHT_MAINSCREENTEMPS_MAINSCREENBEDTEMP_MAINSCREENBEDTEMPVALUE] = cui_mainScreenBedTempValue;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENRIGHT_MAINSCREENTEMPS_MAINSCREENCHAMBERTEMP] = cui_mainScreenChamberTemp;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENRIGHT_MAINSCREENTEMPS_MAINSCREENCHAMBERTEMP_MAINSCREENCHAMBERTEMPICON] = cui_mainScreenChamberTempIcon;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENRIGHT_MAINSCREENTEMPS_MAINSCREENCHAMBERTEMP_MAINSCREENCHAMBERTEMPVALUE] = cui_mainScreenChamberTempValue;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENPREHEATBOX] = ui_mainPreheatBox;
    children[UI_COMP_HOMECOMPONENT_MAINSCREENLEFT_MAINSCREENSTATUS_MAINSCREENSTATUSCAPTION] = cui_mainScreenStatusCaption;

    lv_obj_add_event_cb(cui_homeComponent, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(cui_homeComponent, del_component_child_event_cb, LV_EVENT_DELETE, children);
    lv_obj_add_event_cb(cui_mainScreenPlayPauseButton, ui_event_comp_homeComponent_mainScreenPlayPauseButton, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_mainScreenStopButton, ui_event_comp_homeComponent_mainScreenStopButton, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_mainScreenSpeedDropDown, ui_event_comp_homeComponent_mainScreenSpeedDropDown, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_mainScreenLightButton, ui_event_comp_homeComponent_mainScreenLightButton, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_mainScreenLCDButton, ui_event_comp_homeComponent_mainScreenLCDButton, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_mainScreenNeoPixelButton, ui_event_comp_homeComponent_mainScreenNeoPixelButton, LV_EVENT_ALL, children);

    lv_obj_add_event_cb(cui_mainScreenBedTemp, ui_event_comp_homeComponent_mainScreenBedTemp, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_mainScreenNozzleTemp, ui_event_comp_homeComponent_mainScreenNozzleTemp, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_mainScreenSpeedDropDown, ui_event_comp_homeComponent_mainScreenSpeedChange, LV_EVENT_ALL, children);

    lv_obj_add_event_cb(cui_mainScreenAMSColor0, onXTouchAMSUpdate, LV_EVENT_MSG_RECEIVED, (void *)0);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_mainScreenAMSColor0, (void *)0);

    lv_obj_add_event_cb(cui_mainScreenAMSColor1, onXTouchAMSUpdate, LV_EVENT_MSG_RECEIVED, (void *)1);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_mainScreenAMSColor1, (void *)1);

    lv_obj_add_event_cb(cui_mainScreenAMSColor2, onXTouchAMSUpdate, LV_EVENT_MSG_RECEIVED, (void *)2);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_mainScreenAMSColor2, (void *)2);

    lv_obj_add_event_cb(cui_mainScreenAMSColor3, onXTouchAMSUpdate, LV_EVENT_MSG_RECEIVED, (void *)3);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_mainScreenAMSColor3, (void *)3);

    lv_obj_add_event_cb(cui_mainScreenAMSColor4, onXTouchAMSUpdate, LV_EVENT_MSG_RECEIVED, (void *)4);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_mainScreenAMSColor4, (void *)4);

    lv_obj_add_event_cb(cui_mainScreenLightButton, onXTouchLightMessage, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_LIGHT_REPORT, cui_mainScreenLightButton, NULL);

    lv_obj_add_event_cb(cui_mainScreenNeoPixelButton, onXTouchNeoPixelMessage, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_NEOPIXEL_REPORT, cui_mainScreenNeoPixelButton, NULL);

    lv_obj_add_event_cb(cui_mainScreenBedTempValue, onXTouchBedTemp, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_BED_TEMP, cui_mainScreenBedTempValue, NULL);

    lv_obj_add_event_cb(cui_mainScreenBedTempIcon, onXTouchBedTempTarget, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_BED_TARGET_TEMP, cui_mainScreenBedTempIcon, NULL);

    lv_obj_add_event_cb(cui_mainScreenNozzleTempValue, onXTouchBedTemp, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_NOZZLE_TEMP, cui_mainScreenNozzleTempValue, NULL);

    lv_obj_add_event_cb(cui_mainScreenChamberTempValue, onXTouchChamberTemp, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_CHAMBER_TEMP, cui_mainScreenChamberTempValue, NULL);

    lv_obj_add_event_cb(cui_mainScreenNozzleTempIcon, onXTouchBedTempTarget, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_NOZZLE_TARGET_TEMP, cui_mainScreenNozzleTempIcon, NULL);

    lv_obj_add_event_cb(cui_mainScreenAMS, onXTouchAMS, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS, cui_mainScreenAMS, NULL);

    // lv_obj_add_event_cb(cui_mainScreenWifiIcon, onXTouchWifiSignal, LV_EVENT_MSG_RECEIVED, NULL);
    // lv_msg_subsribe_obj(XTOUCH_ON_WIFI_SIGNAL, cui_mainScreenWifiIcon, NULL);

    // lv_obj_add_event_cb(cui_mainScreenCameraIcon, onXTouchIPCam, LV_EVENT_MSG_RECEIVED, NULL);
    // lv_msg_subsribe_obj(XTOUCH_ON_IPCAM, cui_mainScreenCameraIcon, NULL);

    lv_obj_add_event_cb(cui_mainScreenTimeLeft, onXTouchPrintStatus, LV_EVENT_MSG_RECEIVED, children);
    lv_msg_subsribe_obj(XTOUCH_ON_PRINT_STATUS, cui_mainScreenTimeLeft, NULL);

    /* 設定で PreHeat ボタンの表示を切り替え（loadScreen で毎回 create するためここで十分） */
    if (!xTouchConfig.xTouchPreheatEnabled)
        lv_obj_add_flag(ui_mainPreheatBox, LV_OBJ_FLAG_HIDDEN);

    ui_comp_homeComponent_create_hook(cui_homeComponent);

    struct XTOUCH_MESSAGE_DATA eventData;
    eventData.data = 0;
    lv_msg_send(XTOUCH_ON_AMS_SLOT_UPDATE, &eventData);
    lv_msg_send(XTOUCH_ON_WIFI_SIGNAL, &eventData);

    eventData.data = xtouch_neopixel_enabled;
    lv_msg_send(XTOUCH_ON_NEOPIXEL_REPORT, &eventData);

    return cui_homeComponent;
}
