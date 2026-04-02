
#include "ui.h"

#ifdef __XTOUCH_PLATFORM_S3__
static bool s_home_thumb_global_subscribed = false;
/** IMAGE 受信（サムネ DL 完了）時に Home 表示中なら slot 0 を必ず再描画する。オブジェクト購読が届かない場合の保険。 */
static void on_home_thumb_global(lv_msg_t *m, void *user_data)
{
    (void)user_data;
    if (!m || lv_msg_get_id(m) != XTOUCH_ON_OTHER_PRINTER_UPDATE)
        return;
    if (!ui_msg_payload_is_main_thumb_refresh(lv_msg_get_payload(m)))
        return;
    if (xTouchConfig.currentScreenIndex != 0 || !ui_homeThumbImg)
        return;
    ui_thumb_set_img_src_from_slot(ui_homeThumbImg, 0);
}
#endif


void fillScreenData(int screen)
{
    switch (screen)
    {
    case 0:
        ui_msg_send(XTOUCH_ON_BED_TEMP, bambuStatus.bed_temper, 0);
        ui_msg_send(XTOUCH_ON_BED_TARGET_TEMP, bambuStatus.bed_target_temper, 0);
        ui_msg_send(XTOUCH_ON_NOZZLE_TEMP, bambuStatus.nozzle_temper, 0);
        ui_msg_send(XTOUCH_ON_NOZZLE_TARGET_TEMP, bambuStatus.nozzle_target_temper, 0);
        ui_msg_send(XTOUCH_ON_LIGHT_REPORT, bambuStatus.chamberLed, 0);
        ui_msg_send(XTOUCH_ON_AMS, bambuStatus.ams, 0);
        ui_msg_send(XTOUCH_ON_PRINT_STATUS, 0, 0);
        ui_msg_send(XTOUCH_ON_CHAMBER_TEMP, bambuStatus.chamber_temper, 0);
        break;
    case 1:
        ui_msg_send(XTOUCH_ON_BED_TEMP, bambuStatus.bed_temper, 0);
        ui_msg_send(XTOUCH_ON_BED_TARGET_TEMP, bambuStatus.bed_target_temper, 0);
        ui_msg_send(XTOUCH_ON_NOZZLE_TEMP, bambuStatus.nozzle_temper, 0);
        ui_msg_send(XTOUCH_ON_NOZZLE_TARGET_TEMP, bambuStatus.nozzle_target_temper, 0);
        ui_msg_send(XTOUCH_ON_PART_FAN_SPEED, bambuStatus.cooling_fan_speed, 0);
        ui_msg_send(XTOUCH_ON_PART_AUX_SPEED, bambuStatus.big_fan1_speed, 0);
        ui_msg_send(XTOUCH_ON_PART_CHAMBER_SPEED, bambuStatus.big_fan2_speed, 0);
        break;
    case 2:
        ui_msg_send(XTOUCH_CONTROL_INC_SWITCH, controlMode.inc, 0);
        break;
    case 3:
        ui_msg_send(XTOUCH_ON_NOZZLE_TEMP, bambuStatus.nozzle_temper, 0);
                break;
    case 7:
    case 13:
        ui_msg_send(XTOUCH_ON_AMS, bambuStatus.ams, 0);
        ui_msg_send(XTOUCH_ON_AMS_BITS, 0, 0);
        ui_msg_send(XTOUCH_ON_AMS_SLOT_UPDATE, 0, 0);
        ui_msg_send(XTOUCH_ON_AMS_HUMIDITY_UPDATE, 0, 0);
        break;
    }
}

void loadScreen(int screen)
{
#ifdef __XTOUCH_PLATFORM_S3__
    const int prev_screen = xTouchConfig.currentScreenIndex;

    ui_printersListContainer = NULL;
    if (screen != 15)
        ui_historyListContainer = NULL;

    if (screen != 6 && screen != 0)
        ui_msg_send(XTOUCH_PRINTERS_THUMB_TIMER_STOP, 0, 0);

    /* History カバー DL 待ちを捨てる。History 未使用時はキュー空で実質ノーオペ。遷移先が 15 のときは FETCH 後に再キューされる */
    if (prev_screen != screen)
        ui_msg_send(XTOUCH_HISTORY_COVER_DL_CANCEL, 0, 0);
#endif

    xTouchConfig.currentScreenIndex = screen;
    lv_obj_t *current = lv_scr_act();
#ifdef __XTOUCH_PLATFORM_S3__
    if (current == ui_homeScreen)
        ui_homeThumbImg = NULL;
#endif
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
#ifdef __XTOUCH_PLATFORM_S3__
        if (!s_home_thumb_global_subscribed)
        {
            lv_msg_subscribe(XTOUCH_ON_OTHER_PRINTER_UPDATE, (lv_msg_subscribe_cb_t)on_home_thumb_global, NULL);
            s_home_thumb_global_subscribed = true;
        }
        ui_msg_send(XTOUCH_PRINTERS_THUMB_TIMER_START, 0, 0);
        ui_msg_send(XTOUCH_PRINTERS_SCHEDULE_THUMB_FETCH, 0, 0);
        /* cleanup は送らない（Printers 入室時のみ）。pushall でメインプリンタの image_url/task_id を取得 */
        extern void xtouch_mqtt_pushall_all_printers_for_screen_c(void);
        xtouch_mqtt_pushall_all_printers_for_screen_c();
#endif
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
#ifdef __XTOUCH_PLATFORM_S3__
    case 6:
        if (xTouchConfig.xTouchLanOnlyMode) {
            screen = 0;
            xTouchConfig.currentScreenIndex = 0;
            ui_homeScreen_screen_init();
            lv_disp_load_scr(ui_homeScreen);
        } else {
            /* LIST_REFRESH より先に slot を現在の task_id に再バインド（cache_refresh_done で旧 dsc が残るのを防ぐ） */
            ui_msg_send(XTOUCH_PRINTERS_THUMB_REBIND, 0, 0);
            ui_printersScreen_screen_init();
            lv_disp_load_scr(ui_printersScreen);
            ui_msg_send(XTOUCH_PRINTERS_THUMB_TIMER_START, 0, 0);
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
#ifdef __XTOUCH_PLATFORM_S3__
    case 15:
        ui_historyScreen_screen_init();
        lv_disp_load_scr(ui_historyScreen);
        break;
    case 16:
        ui_historyReprintScreen_screen_init();
        lv_disp_load_scr(ui_historyReprintScreen);
        break;
#endif
    }
    fillScreenData(screen);

    // サイドバーのハイライト
    // 5inch: 0=Home,1=Printers,2=Temp,3=AMS,4=Settings (History はオプション機能)
    // 2.8inch: 0=Home,1=Temp,2=AMS,3=Settings
    int sidebar_index = -1;
#ifdef __XTOUCH_PLATFORM_S3__
    switch (screen)
    {
    case 0:
        sidebar_index = 0; // Home
        break;
    case 6:
        sidebar_index = 1; // Printers
        break;
    case 1: case 2: case 3: case 10: case 11: case 12:
        sidebar_index = 2; // Temps / Control 系
        break;
    case 7: case 13: case 14:
        sidebar_index = 3; // AMS 系
        break;
    case 4:
        sidebar_index = 4; // Settings
        break;
    case 15:
        // History は Printers/Temps の間に位置づける（アイコン自体の表示/非表示は別途制御）
        sidebar_index = 2;
        break;
    case 16:
        // History リプリント設定画面も History と同じグループ扱い
        sidebar_index = 2;
        break;
    default:
        break;
    }
    if (sidebar_index >= 1 && screen != 6 && screen != 15)
        sidebar_index++;
#else
    switch (screen)
    {
    case 0:
        sidebar_index = 0; // Home
        break;
    case 1: case 2: case 3: case 10: case 11: case 12:
        sidebar_index = 1; // Temps / Control 系
        break;
    case 7: case 13: case 14:
        sidebar_index = 2; // AMS 系
        break;
    case 4:
        sidebar_index = 3; // Settings
        break;
    default:
        break;
    }
#endif
    if (sidebar_index >= 0)
    {
        ui_sidebarComponent_set_active(sidebar_index);
    }
}

#ifdef __XTOUCH_PLATFORM_S3__
static void on_settings_save_sidebar(const void *payload, void *user_data)
{
    (void)payload;
    (void)user_data;
    ui_sidebarComponent_updatePrintersVisibility();
    ui_sidebarComponent_updateHistoryVisibility();
}

static void on_reprint_done_goto_home(lv_msg_t *m, void *user_data)
{
    (void)m;
    (void)user_data;
    loadScreen(0);
}
#endif

void initTopLayer()
{
    ui_confirmComponent = ui_confirmPanel_create(lv_layer_top());
    lv_obj_add_flag(ui_confirmComponent, LV_OBJ_FLAG_HIDDEN);
    ui_hmsComponent = ui_hmsPanel_create(lv_layer_top());
    lv_obj_add_flag(ui_hmsComponent, LV_OBJ_FLAG_HIDDEN);
#ifdef __XTOUCH_PLATFORM_S3__
    lv_msg_subscribe(XTOUCH_SETTINGS_SAVE, (lv_msg_subscribe_cb_t)on_settings_save_sidebar, NULL);
    lv_msg_subscribe(XTOUCH_HISTORY_REPRINT_DONE, (lv_msg_subscribe_cb_t)on_reprint_done_goto_home, NULL);
#endif
}