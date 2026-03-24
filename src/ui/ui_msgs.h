#ifndef _XLCD_MESSAGING
#define _XLCD_MESSAGING

#ifdef __cplusplus
extern "C"
{
#endif

    enum XTOUCH_MESSAGE
    {
        XTOUCH_ON_MQTT,
        XTOUCH_ON_LIGHT_REPORT,
        XTOUCH_ON_NEOPIXEL_REPORT,  
        XTOUCH_ON_BED_TEMP,
        XTOUCH_ON_BED_TARGET_TEMP,
        XTOUCH_ON_NOZZLE_TEMP,
        XTOUCH_ON_NOZZLE_TARGET_TEMP,
        XTOUCH_ON_CHAMBER_TEMP,
        XTOUCH_ON_AMS,
        XTOUCH_ON_WIFI_SIGNAL,
        XTOUCH_ON_PART_FAN_SPEED,
        XTOUCH_ON_PART_AUX_SPEED,
        XTOUCH_ON_PART_CHAMBER_SPEED,
        XTOUCH_ON_IPCAM,
        XTOUCH_ON_IPCAM_TIMELAPSE,
        XTOUCH_ON_PRINT_STATUS,
        XTOUCH_ON_AMS_BITS,
        XTOUCH_ON_ERROR,
        XTOUCH_ON_SSDP,
        XTOUCH_ON_AMS_SLOT_UPDATE,
        XTOUCH_ON_AMS_STATE_UPDATE,
        XTOUCH_ON_AMS_HUMIDITY_UPDATE,
        XTOUCH_ON_AMS_TEMPERATURE_UPDATE,
        XTOUCH_AMS_EDIT_FETCHED_TEMP,
        XTOUCH_AMS_EDIT_JSON_ERROR,
        XTOUCH_ON_FILENAME_UPDATE,
        XTOUCH_ON_CLOUD_SELECT,
        XTOUCH_ON_CODE_ENTERED,
        XTOUCH_SIDEBAR_HOME,
        XTOUCH_COMMAND_STOP,
        XTOUCH_COMMAND_PAUSE,
        XTOUCH_COMMAND_PAUSE_SLOT,
        XTOUCH_COMMAND_STOP_SLOT,
        XTOUCH_COMMAND_RESUME_SLOT,
        XTOUCH_COMMAND_RESUME,
        XTOUCH_COMMAND_LIGHT_TOGGLE,
        XTOUCH_COMMAND_LIGHT_RESET,
        XTOUCH_COMMAND_LCD_TOGGLE,
        XTOUCH_COMMAND_NEOPIXEL_TOGGLE,
        XTOUCH_COMMAND_HOME,
        XTOUCH_COMMAND_RIGHT,
        XTOUCH_COMMAND_LEFT,
        XTOUCH_COMMAND_UP,
        XTOUCH_COMMAND_DOWN,
        XTOUCH_COMMAND_BED_UP,
        XTOUCH_COMMAND_BED_DOWN,
        XTOUCH_COMMAND_BED_TARGET_TEMP,
        XTOUCH_COMMAND_NOZZLE_TARGET_TEMP,
        XTOUCH_COMMAND_PART_FAN_SPEED,
        XTOUCH_COMMAND_AUX_FAN_SPEED,
        XTOUCH_COMMAND_CHAMBER_FAN_SPEED,
        XTOUCH_COMMAND_PRINT_SPEED,
        XTOUCH_COMMAND_EXTRUDE_UP,
        XTOUCH_COMMAND_EXTRUDE_DOWN,
        XTOUCH_COMMAND_UNLOAD_FILAMENT,
        XTOUCH_COMMAND_LOAD_FILAMENT,
        XTOUCH_COMMAND_CLEAN_PRINT_ERROR,
        XTOUCH_COMMAND_AMS_CONTROL,
        XTOUCH_COMMAND_AMS_LOAD_SLOT,
        XTOUCH_COMMAND_GCODE_M620_R, /* スロット■クリック: payload=(void*)(uintptr_t)tray_index (0-15, AMS1=0-3, AMS2=4-7, …) */
        XTOUCH_COMMAND_AMS_UNLOAD_SLOT,
        XTOUCH_COMMAND_AMS_REFRESH,
        XTOUCH_COMMAND_AMS_FILAMENT_SETTING,
        XTOUCH_COMMAND_AMS_FETCH_SLICER_TEMP,
        XTOUCH_COMMAND_SET_UTIL_NOZZLE_CHANGE,
        XTOUCH_COMMAND_SET_UTIL_CALIBRATION,

        XTOUCH_CONTROL_INC_SWITCH,
        XTOUCH_SETTINGS_RESET_DEVICE,
        XTOUCH_SETTINGS_OTA_UPDATE_NOW,
        XTOUCH_SETTINGS_UNPAIR,
        XTOUCH_SETTINGS_BACKLIGHT,
        XTOUCH_SETTINGS_BACKLIGHT_SET,
        XTOUCH_SETTINGS_TFTOFF_SET,
        XTOUCH_SETTINGS_LEDOFF_SET,
        // XTOUCH_SETTINGS_NEOPIXEL_NUM_SET,
        // XTOUCH_SETTINGS_NEOPIXEL_SET,
        XTOUCH_SETTINGS_TFT_INVERT,
        XTOUCH_SETTINGS_TFT_FLIP,
        // XTOUCH_SETTINGS_CHAMBER_TEMP,
        XTOUCH_SETTINGS_SAVE,

        XTOUCH_OPTIONAL_NEOPIXEL_NUM_SET,
        XTOUCH_OPTIONAL_NEOPIXEL_SET,
        XTOUCH_OPTIONAL_CHAMBER_TEMP,
        XTOUCH_OPTIONAL_ALARM_TIMEOUT_SET,
        XTOUCH_OPTIONAL_IDLE_LED_SET,

        XTOUCH_FIRMWARE_UPDATE,
        XTOUCH_FIRMWARE_UPDATE_PROGRESS,

        XTOUCH_ON_CHARACTER_FACEPOTITION_UPDATE,
        XTOUCH_ON_CHARACTER_LEFT_EYE_UPDATE,
        XTOUCH_ON_CHARACTER_RIGHT_EYE_UPDATE,
        XTOUCH_ON_CHARACTER_LEFT_EYE_POSITION_X_UPDATE,
        XTOUCH_ON_CHARACTER_RIGHT_EYE_POSITION_X_UPDATE,
        XTOUCH_ON_CHARACTER_MOUTH_UPDATE,

        XTOUCH_PREHEAT_BUTTON1,
        XTOUCH_PREHEAT_BUTTON2,
        XTOUCH_PREHEAT_BUTTON3,

        XTOUCH_ON_OTHER_PRINTER_UPDATE,
        /** Printers 画面でサムネイル全スロット取得をスケジュールせよ（コンポーネント表示時。購読は xtouch） */
        XTOUCH_PRINTERS_SCHEDULE_THUMB_FETCH,
        /** サムネイルタイマー開始（Printers 画面表示時。購読は xtouch） */
        XTOUCH_PRINTERS_THUMB_TIMER_START,
        /** サムネイルタイマー停止（Printers 画面離脱時。購読は xtouch） */
        XTOUCH_PRINTERS_THUMB_TIMER_STOP,
        /** Printers 一覧の表示を更新せよ（初期表示・再描画。購読は画面側） */
        XTOUCH_PRINTERS_LIST_REFRESH,
        /** History 画面: Cloud から履歴取得を依頼（購読は xtouch） */
        XTOUCH_HISTORY_FETCH,
        /** History 一覧の表示を更新せよ（購読は画面側） */
        XTOUCH_HISTORY_LIST_REFRESH,
        /** History 再印刷: payload data = 履歴行インデックス（0-based） */
        XTOUCH_HISTORY_REPRINT,
        /** History 再印刷 API 成功後に送信。購読側で Home へ遷移する */
        XTOUCH_HISTORY_REPRINT_DONE,
        /** History リプリント設定画面からの確定: data=履歴行インデックス, data2=プリンタスロット(0=自機,1以降other) */
        XTOUCH_HISTORY_REPRINT_WITH_OPTIONS,
        /** History リプリント設定画面: 選択タスクの amsDetailMapping 詳細取得を依頼（購読は xtouch） */
        XTOUCH_HISTORY_REPRINT_DETAIL_FETCH,
        /** History リプリント設定画面: amsDetailMapping 詳細取得完了（購読は画面側。再描画用） */
        XTOUCH_HISTORY_REPRINT_DETAIL_READY,
        /** History リプリント設定: mapping行に対する AMS スロット選択。data=map_index, data2=(ams_id&0xFF)|((tray_id&0xFF)<<8) */
        XTOUCH_HISTORY_REPRINT_SLOT_PICKED,
        /** サムネイル全画面非表示オプション変更（settings 保存後に送る。xtouch が購読） */
        XTOUCH_THUMBNAILS_HIDE_MODE_CHANGED,
    };

    struct XTOUCH_MESSAGE_DATA
    {
        unsigned long long data;
        unsigned long long data2;
    };

    /** Save 時に送る ams_filament_setting 用。tray_info_idx は setting_id（フル）。filament_id は Cloud の filament_id（例: GFB00）。 */
    struct XTOUCH_AMS_FILAMENT_SETTING_PAYLOAD
    {
        int ams_id;
        int tray_id;
        char tray_info_idx[16];
        char filament_id[16];
        char tray_color[12];
        int nozzle_temp_min;
        int nozzle_temp_max;
        char tray_type[16];
    };

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif