#ifndef _XLCD_MESSAGING
#define _XLCD_MESSAGING

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    enum XPTOUCH_MESSAGE
    {
        XPTOUCH_ON_MQTT,
        XPTOUCH_ON_LIGHT_REPORT,
        XPTOUCH_ON_NEOPIXEL_REPORT,  
        XPTOUCH_ON_BED_TEMP,
        XPTOUCH_ON_BED_TARGET_TEMP,
        XPTOUCH_ON_NOZZLE_TEMP,
        XPTOUCH_ON_NOZZLE_TARGET_TEMP,
        XPTOUCH_ON_CHAMBER_TEMP,
        XPTOUCH_ON_AMS,
        XPTOUCH_ON_WIFI_SIGNAL,
        XPTOUCH_ON_PART_FAN_SPEED,
        XPTOUCH_ON_PART_AUX_SPEED,
        XPTOUCH_ON_PART_CHAMBER_SPEED,
        XPTOUCH_ON_IPCAM,
        XPTOUCH_ON_IPCAM_TIMELAPSE,
        XPTOUCH_ON_PRINT_STATUS,
        XPTOUCH_ON_AMS_BITS,
        XPTOUCH_ON_ERROR,
        XPTOUCH_ON_SSDP,
        XPTOUCH_ON_AMS_SLOT_UPDATE,
        XPTOUCH_ON_AMS_STATE_UPDATE,
        XPTOUCH_ON_AMS_HUMIDITY_UPDATE,
        XPTOUCH_ON_AMS_TEMPERATURE_UPDATE,
        XPTOUCH_AMS_EDIT_FETCHED_TEMP,
        XPTOUCH_AMS_EDIT_JSON_ERROR,
        XPTOUCH_ON_FILENAME_UPDATE,
        XPTOUCH_ON_CLOUD_SELECT,
        XPTOUCH_ON_CODE_ENTERED,
        XPTOUCH_SIDEBAR_HOME,
        XPTOUCH_COMMAND_STOP,
        XPTOUCH_COMMAND_PAUSE,
        XPTOUCH_COMMAND_PAUSE_SLOT,
        XPTOUCH_COMMAND_STOP_SLOT,
        XPTOUCH_COMMAND_RESUME_SLOT,
        XPTOUCH_COMMAND_RESUME,
        XPTOUCH_COMMAND_LIGHT_TOGGLE,
        XPTOUCH_COMMAND_LIGHT_RESET,
        XPTOUCH_COMMAND_LCD_TOGGLE,
        XPTOUCH_COMMAND_NEOPIXEL_TOGGLE,
        XPTOUCH_COMMAND_HOME,
        XPTOUCH_COMMAND_RIGHT,
        XPTOUCH_COMMAND_LEFT,
        XPTOUCH_COMMAND_UP,
        XPTOUCH_COMMAND_DOWN,
        /** AXIS: モーター解除（M18。Bambu 系でも一般的。購読は mqtt.h / device.h） */
        XPTOUCH_COMMAND_MOTOR_UNLOCK,
        XPTOUCH_COMMAND_BED_UP,
        XPTOUCH_COMMAND_BED_DOWN,
        XPTOUCH_COMMAND_BED_TARGET_TEMP,
        XPTOUCH_COMMAND_NOZZLE_TARGET_TEMP,
        XPTOUCH_COMMAND_PART_FAN_SPEED,
        XPTOUCH_COMMAND_AUX_FAN_SPEED,
        XPTOUCH_COMMAND_CHAMBER_FAN_SPEED,
        XPTOUCH_COMMAND_PRINT_SPEED,
        XPTOUCH_COMMAND_EXTRUDE_UP,
        XPTOUCH_COMMAND_EXTRUDE_DOWN,
        XPTOUCH_COMMAND_UNLOAD_FILAMENT,
        XPTOUCH_COMMAND_LOAD_FILAMENT,
        XPTOUCH_COMMAND_CLEAN_PRINT_ERROR,
        XPTOUCH_COMMAND_AMS_CONTROL,
        XPTOUCH_COMMAND_AMS_LOAD_SLOT,
        XPTOUCH_COMMAND_GCODE_M620_R, /* スロット■クリック: payload=(void*)(uintptr_t)tray_index (0-15, AMS1=0-3, AMS2=4-7, …) */
        XPTOUCH_COMMAND_AMS_UNLOAD_SLOT,
        XPTOUCH_COMMAND_AMS_REFRESH,
        XPTOUCH_COMMAND_AMS_FILAMENT_SETTING,
        XPTOUCH_COMMAND_AMS_FETCH_SLICER_TEMP,
        XPTOUCH_COMMAND_SET_UTIL_NOZZLE_CHANGE,
        XPTOUCH_COMMAND_SET_UTIL_CALIBRATION,

        XPTOUCH_CONTROL_INC_SWITCH,
        XPTOUCH_SETTINGS_RESET_DEVICE,
        XPTOUCH_SETTINGS_OTA_UPDATE_NOW,
        XPTOUCH_SETTINGS_UNPAIR,
        /** SD /tmp のサムネ・履歴カバー PNG を削除し LGFX キャッシュを無効化（購読は events.h / xtouch） */
        XPTOUCH_SETTINGS_CLEAR_CACHE,
        /** provisioning.json の demo をトグルして再起動 */
        XPTOUCH_SETTINGS_DEMO_MODE_TOGGLE,
        XPTOUCH_SETTINGS_BACKLIGHT,
        XPTOUCH_SETTINGS_BACKLIGHT_SET,
        XPTOUCH_SETTINGS_TFTOFF_SET,
        XPTOUCH_SETTINGS_LEDOFF_SET,
        // XPTOUCH_SETTINGS_NEOPIXEL_NUM_SET,
        // XPTOUCH_SETTINGS_NEOPIXEL_SET,
        XPTOUCH_SETTINGS_TFT_INVERT,
        XPTOUCH_SETTINGS_TFT_FLIP,
        // XPTOUCH_SETTINGS_CHAMBER_TEMP,
        XPTOUCH_SETTINGS_SAVE,

        XPTOUCH_OPTIONAL_NEOPIXEL_NUM_SET,
        XPTOUCH_OPTIONAL_NEOPIXEL_SET,
        XPTOUCH_OPTIONAL_CHAMBER_TEMP,
        XPTOUCH_OPTIONAL_ALARM_TIMEOUT_SET,
        XPTOUCH_OPTIONAL_IDLE_LED_SET,

        XPTOUCH_FIRMWARE_UPDATE,
        XPTOUCH_FIRMWARE_UPDATE_PROGRESS,

        XPTOUCH_ON_CHARACTER_FACEPOTITION_UPDATE,
        XPTOUCH_ON_CHARACTER_LEFT_EYE_UPDATE,
        XPTOUCH_ON_CHARACTER_RIGHT_EYE_UPDATE,
        XPTOUCH_ON_CHARACTER_LEFT_EYE_POSITION_X_UPDATE,
        XPTOUCH_ON_CHARACTER_RIGHT_EYE_POSITION_X_UPDATE,
        XPTOUCH_ON_CHARACTER_MOUTH_UPDATE,

        XPTOUCH_PREHEAT_BUTTON1,
        XPTOUCH_PREHEAT_BUTTON2,
        XPTOUCH_PREHEAT_BUTTON3,

        XPTOUCH_ON_OTHER_PRINTER_UPDATE,
        /** Printers 画面でサムネイル全スロット取得をスケジュールせよ（コンポーネント表示時。購読は xtouch） */
        XPTOUCH_PRINTERS_SCHEDULE_THUMB_FETCH,
        /** Printers 入室直前: 行と slot の対応を取り直す（dsc/cache フラグを捨て、現在の task_id で path・必要なら即デコード）。購読は xtouch */
        XPTOUCH_PRINTERS_THUMB_REBIND,
        /** サムネイルタイマー開始（Printers 画面表示時。購読は xtouch） */
        XPTOUCH_PRINTERS_THUMB_TIMER_START,
        /** サムネイルタイマー停止（Printers 画面離脱時。購読は xtouch） */
        XPTOUCH_PRINTERS_THUMB_TIMER_STOP,
        /** Printers 一覧の表示を更新せよ（初期表示・再描画。購読は画面側） */
        XPTOUCH_PRINTERS_LIST_REFRESH,
        /** Printers 画面: 行クリックでメイン操作先を一時切替え。data = 行 0..（1 以降＝他プリンタ）。購読は mqtt.h */
        XPTOUCH_PRINTERS_TEMP_FOCUS_ROW,
        /** History 画面: Cloud から履歴取得を依頼（購読は xtouch） */
        XPTOUCH_HISTORY_FETCH,
        /** History: 一覧は触らずカバー画像だけ再キュー（再入室のリスト更新は XPTOUCH_HISTORY_FETCH。任意経路用） */
        XPTOUCH_HISTORY_COVER_RETRY,
        /** History 一覧の表示を更新せよ（購読は画面側） */
        XPTOUCH_HISTORY_LIST_REFRESH,
        /** History 再印刷: payload data = 履歴行インデックス（0-based） */
        XPTOUCH_HISTORY_REPRINT,
        /** History 再印刷 API 成功後に送信。購読側で Home へ遷移する */
        XPTOUCH_HISTORY_REPRINT_DONE,
        /** History リプリント設定画面からの確定: data=履歴行インデックス, data2=プリンタスロット(0=自機,1以降other) */
        XPTOUCH_HISTORY_REPRINT_WITH_OPTIONS,
        /** History リプリント設定画面: 選択タスクの amsDetailMapping 詳細取得を依頼（購読は xtouch） */
        XPTOUCH_HISTORY_REPRINT_DETAIL_FETCH,
        /** History リプリント設定画面: amsDetailMapping 詳細取得完了（購読は画面側。再描画用） */
        XPTOUCH_HISTORY_REPRINT_DETAIL_READY,
        /** Reprint 画面で印刷先プリンタが変わった（pushall 後に xtouch がデフォルトスロット再計算して DETAIL_READY） */
        XPTOUCH_HISTORY_REPRINT_PRINTER_CHANGED,
        /** History リプリント設定: mapping行に対する AMS スロット選択。data=map_index, data2=(ams_id&0xFF)|((tray_id&0xFF)<<8) */
        XPTOUCH_HISTORY_REPRINT_SLOT_PICKED,
        /** History カバー画像 DL 待ちキューを捨てる（画面遷移時。購読は xtouch。再入場は fetch 完了後に再キュー） */
        XPTOUCH_HISTORY_COVER_DL_CANCEL,
        /** サムネイル全画面非表示オプション変更（settings 保存後に送る。xtouch が購読） */
        XPTOUCH_THUMBNAILS_HIDE_MODE_CHANGED,
    };

    struct XPTOUCH_MESSAGE_DATA
    {
        unsigned long long data;
        unsigned long long data2;
    };

    /** XPTOUCH_ON_OTHER_PRINTER_UPDATE: Home のメイン(slot0)サムネ img を差し替えるべきペイロードか。
     *  lv_msg_send の (slot+1) 直ポインタは 1 のみ。ui_msg_send では data==0（push_status 同期）または data==1（明示 slot0）。 */
    static inline int ui_msg_payload_is_main_thumb_refresh(const void *payload)
    {
        if (!payload)
            return 0;
        if ((uintptr_t)payload < 256u)
            return (intptr_t)payload == 1;
        const struct XPTOUCH_MESSAGE_DATA *d = (const struct XPTOUCH_MESSAGE_DATA *)payload;
        return (d->data == 0ull || d->data == 1ull);
    }

    void ui_msg_send(enum XPTOUCH_MESSAGE msg, unsigned long long data1, unsigned long long data2);

    /** Save 時に送る ams_filament_setting 用。tray_info_idx は setting_id（フル）。filament_id は Cloud の filament_id（例: GFB00）。 */
    struct XPTOUCH_AMS_FILAMENT_SETTING_PAYLOAD
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