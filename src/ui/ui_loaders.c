
#include "ui.h"
#include "ui_msgs.h"

void sendMqttMsg(int message, uint32_t data)
{
    struct XTOUCH_MESSAGE_DATA eventData;
    eventData.data = data;
    lv_msg_send(message, &eventData);
}

void fillScreenData(int screen)
{
    switch (screen)
    {
    case 0:
        sendMqttMsg(XTOUCH_ON_BED_TEMP, bambuStatus.bed_temper);
        sendMqttMsg(XTOUCH_ON_BED_TARGET_TEMP, bambuStatus.bed_target_temper);
        sendMqttMsg(XTOUCH_ON_NOZZLE_TEMP, bambuStatus.nozzle_temper);
        sendMqttMsg(XTOUCH_ON_NOZZLE_TARGET_TEMP, bambuStatus.nozzle_target_temper);
        sendMqttMsg(XTOUCH_ON_LIGHT_REPORT, bambuStatus.chamberLed);
        sendMqttMsg(XTOUCH_ON_AMS, bambuStatus.ams);
        sendMqttMsg(XTOUCH_ON_PRINT_STATUS, 0);
        sendMqttMsg(XTOUCH_ON_CHAMBER_TEMP, bambuStatus.chamber_temper);
        break;
    case 1:
        sendMqttMsg(XTOUCH_ON_BED_TEMP, bambuStatus.bed_temper);
        sendMqttMsg(XTOUCH_ON_BED_TARGET_TEMP, bambuStatus.bed_target_temper);
        sendMqttMsg(XTOUCH_ON_NOZZLE_TEMP, bambuStatus.nozzle_temper);
        sendMqttMsg(XTOUCH_ON_NOZZLE_TARGET_TEMP, bambuStatus.nozzle_target_temper);
        sendMqttMsg(XTOUCH_ON_PART_FAN_SPEED, bambuStatus.cooling_fan_speed);
        sendMqttMsg(XTOUCH_ON_PART_AUX_SPEED, bambuStatus.big_fan1_speed);
        sendMqttMsg(XTOUCH_ON_PART_CHAMBER_SPEED, bambuStatus.big_fan2_speed);
        break;
    case 2:
        sendMqttMsg(XTOUCH_CONTROL_INC_SWITCH, controlMode.inc);
        break;
    case 3:
        sendMqttMsg(XTOUCH_ON_NOZZLE_TEMP, bambuStatus.nozzle_temper);
                break;
    case 7:
    case 13:
        sendMqttMsg(XTOUCH_ON_AMS, bambuStatus.ams);
        sendMqttMsg(XTOUCH_ON_AMS_BITS, 0);
        sendMqttMsg(XTOUCH_ON_AMS_SLOT_UPDATE, 0);
        sendMqttMsg(XTOUCH_ON_AMS_HUMIDITY_UPDATE, 0);
        break;
    }
}

void loadScreen(int screen)
{
#ifdef __XTOUCH_SCREEN_50__
    ui_printersListContainer = NULL;
    if (screen != 6)
    {
        struct XTOUCH_MESSAGE_DATA eventData;
        eventData.data = 0;
        eventData.data2 = 0;
        lv_msg_send(XTOUCH_PRINTERS_THUMB_TIMER_STOP, &eventData);
    }
#endif
    xTouchConfig.currentScreenIndex = screen;
    lv_obj_t *current = lv_scr_act();
    if (current != NULL)
    {
        lv_obj_clean(current);
        lv_obj_del(current);
    }
    switch (screen)
    {
    case -1:
        ui_introScreen_screen_init();
        lv_disp_load_scr(introScreen);
        break;
    case 0:
        ui_homeScreen_screen_init();
        lv_disp_load_scr(ui_homeScreen);
        break;
    case 1:
       ui_temperatureScreen_screen_init();
       lv_disp_load_scr(ui_temperatureScreen);
        break;
    case 2:
        ui_controlScreen_screen_init();
        lv_disp_load_scr(ui_controlScreen);
        break;
    case 3:
        ui_filamentScreen_screen_init();
        lv_disp_load_scr(ui_filamentScreen);
        break;
    case 4:
        ui_settingsScreen_screen_init();
        lv_disp_load_scr(ui_settingsScreen);
        break;
    case 5:
        ui_printerPairScreen_screen_init();
        lv_disp_load_scr(ui_printerPairScreen);
        break;
#ifdef __XTOUCH_SCREEN_50__
    case 6:
        if (xTouchConfig.xTouchLanOnlyMode) {
            screen = 0;
            xTouchConfig.currentScreenIndex = 0;
            ui_homeScreen_screen_init();
            lv_disp_load_scr(ui_homeScreen);
        } else {
            ui_printersScreen_screen_init();
            lv_disp_load_scr(ui_printersScreen);
            {
                struct XTOUCH_MESSAGE_DATA eventData;
                eventData.data = 0;
                eventData.data2 = 0;
                lv_msg_send(XTOUCH_PRINTERS_THUMB_TIMER_START, &eventData);
            }
            /* 全スロット取得スケジュールは Printers コンポーネント側でイベント送信 → xtouch が購読 */
            /* 画面遷移時に一度だけ全プリンタへ pushall を投げる（ループ側のポーリングはしない） */
            extern void xtouch_mqtt_pushall_all_printers_for_screen_c(void);
            xtouch_mqtt_pushall_all_printers_for_screen_c();
        }
        break;
#endif
    case 7:
        ui_amsViewScreen_screen_init();
        lv_disp_load_scr(ui_amsViewScreen);
        break;
    case 9:
        ui_characterScreen_screen_init();
        lv_disp_load_scr(ui_characterScreen);
        break;
    case 10:
        ui_utilNozzleChangeScreen_screen_init();
        lv_disp_load_scr(ui_utilNozzleChangeScreen);
        break;
    case 11:
        ui_utilScreen_screen_init();
        lv_disp_load_scr(ui_utilScreen);
        break;
    case 12:
        ui_utilCalibrationScreen_screen_init();
        lv_disp_load_scr(ui_utilCalibrationScreen);
        break;
    case 13:
        ui_amsEditScreen_screen_init();
        ui_amsEditComponent_refresh_filament_options();
        lv_disp_load_scr(ui_amsEditScreen);
        break;
    case 14:
        ui_amsEditColorScreen_screen_init();
        lv_disp_load_scr(ui_amsEditColorScreen);
        break;
    }
    fillScreenData(screen);

    // サイドバーのハイライト (5inch: 0=Home,1=Printers,2=Temp,3=AMS,4=Settings / 2.8: 0=Home,1=Temp,2=AMS,3=Settings)
    int sidebar_index = -1;
    switch (screen)
    {
    case 0: sidebar_index = 0; break;
#ifdef __XTOUCH_SCREEN_50__
    case 6: sidebar_index = 1; break;
#endif
    case 1: case 2: case 3: case 11: sidebar_index = 1; break; /* Temp/Control/Nozzle/Util タブ */
    case 7: case 10: case 12: case 13: case 14: sidebar_index = 2; break; /* AMS 系 */
    case 4: sidebar_index = 3; break; /* Settings */
    default: break;
    }
#ifdef __XTOUCH_SCREEN_50__
    if (sidebar_index >= 1 && screen != 6)
        sidebar_index++;
#endif
    if (sidebar_index >= 0)
    {
        ui_sidebarComponent_set_active(sidebar_index);
    }
}

void initTopLayer()
{
    ui_confirmComponent = ui_confirmPanel_create(lv_layer_top());
    lv_obj_add_flag(ui_confirmComponent, LV_OBJ_FLAG_HIDDEN);
    ui_hmsComponent = ui_hmsPanel_create(lv_layer_top());
    lv_obj_add_flag(ui_hmsComponent, LV_OBJ_FLAG_HIDDEN);
}