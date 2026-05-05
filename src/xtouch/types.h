#ifndef _XLCD_TYPES
#define _XLCD_TYPES

#include <stddef.h>
#include <stdint.h>


#define XPTOUCH_LCD_MIN_SLEEP_TIME 5
#define XPTOUCH_LIGHT_MIN_SLEEP_TIME 5

/** バックライト。範囲外・欠損時は MAX に寄せる。5" は 170〜255、2.8" は 10〜255。 */
#if defined(__XPTOUCH_SCREEN_S3_050__)
#define XPTOUCH_BACKLIGHT_SLIDER_MIN 170
#define XPTOUCH_BACKLIGHT_SLIDER_MAX 255
#else
#define XPTOUCH_BACKLIGHT_SLIDER_MIN 10
#define XPTOUCH_BACKLIGHT_SLIDER_MAX 255
#endif
#define XPTOUCH_BACKLIGHT_SLIDER_DEFAULT XPTOUCH_BACKLIGHT_SLIDER_MAX

#ifdef __cplusplus
extern "C"
{
#endif

    enum XTouchPrinterSeries {
        SERIES_X1 = 0,
        SERIES_P1P,
        SERIES_UNKNOWN,
    };

    struct XTouchPanelConfig
    {
        float xCalM;
        float yCalM;
        float xCalC;
        float yCalC;
    };

    enum XTouchPrintStatus
    {
        XPTOUCH_PRINT_STATUS_IDLE,
        XPTOUCH_PRINT_STATUS_RUNNING,
        XPTOUCH_PRINT_STATUS_PAUSED,
        XPTOUCH_PRINT_STATUS_FINISHED,
        XPTOUCH_PRINT_STATUS_PREPARE,
        XPTOUCH_PRINT_STATUS_FAILED,
    };

    enum XTouchPrintingSpeedLevel
    {
        XPTOUCH_SPEED_LEVEL_INVALID = 0,
        XPTOUCH_SPEED_LEVEL_SILENCE = 1,
        XPTOUCH_SPEED_LEVEL_NORMAL = 2,
        XPTOUCH_SPEED_LEVEL_RAPID = 3,
        XPTOUCH_SPEED_LEVEL_RAMPAGE = 4,
        XPTOUCH_SPEED_LEVEL_COUNT
    };

    enum XTouchAmsStatusMain
    {
        AMS_STATUS_MAIN_IDLE = 0x00,
        AMS_STATUS_MAIN_FILAMENT_CHANGE = 0x01,
        AMS_STATUS_MAIN_RFID_IDENTIFYING = 0x02,
        AMS_STATUS_MAIN_ASSIST = 0x03,
        AMS_STATUS_MAIN_CALIBRATION = 0x04,
        AMS_STATUS_MAIN_SELF_CHECK = 0x10,
        AMS_STATUS_MAIN_DEBUG = 0x20,
        AMS_STATUS_MAIN_UNKNOWN = 0xFF,
    };

    typedef struct BambuMQTTPayload
    {
        char print_type[32];
        int home_flag;
        int hw_switch_state;
        int mc_left_time;
        int mc_print_percent;
        int mc_print_sub_stage;
        int mc_print_stage;
        int mc_print_error_code;
        int mc_print_line_number;
        int print_error;
        char printer_type[32];
        char subtask_name[32];
        int current_layer;
        int total_layers;
        int print_status;
        int queue_number;
        int gcode_file_prepare_percent;
        char obj_subtask_id[32];
        char project_id_[32];
        char profile_id_[32];
        char subtask_id_[32];
        char gcode_file[128];
        int plate_index;
        char task_id[32];
        double bed_temper;
        double bed_target_temper;
        double frame_temp;
        double nozzle_temper;
        double nozzle_target_temper;
        double chamber_temper;
        int wifi_signal;
        int cooling_fan_speed;
        int big_fan1_speed;
        int big_fan2_speed;
        int printing_speed_lvl;
        int printing_speed_mag;
        int print_gcode_action;
        int print_real_action;
        bool chamberLed;
        float nozzle_diameter;
        char nozzle_type[32];  // "stainless_steel" or "hardened_steel"
        bool camera_recording_when_printing;
        bool camera_timelapse;
        bool has_ipcam;
        long int ams_exist_bits;
        bool ams;
        int ams_status_sub;
        int ams_status_main;
        long int ams_version;
        bool ams_support_use_ams;
        int ams_rfid_status;
        int ams_humidity[4];
        float ams_temperature[4];
        int ams_user_setting_hold_count;
        bool ams_insert_flag;
        bool ams_power_on_flag;
        bool ams_calibrate_remain_flag;
        bool ams_support_virtual_tray;
        bool is_ams_need_update;
        long tray_exist_bits;
        long tray_is_bbl_bits;
        long tray_read_done_bits;
        long tray_reading_bits;
        int m_ams_id;   // local ams  : "0" ~ "3"
        int m_tray_id;  // local tray id : "0" ~ "3"
        int m_tray_now; // tray_now : "0" ~ "15" or "254", "255"
        int m_tray_pre; // tray_now : "0" ~ "15" or "254", "255"
        int m_humidity; // humidity : "1" ~ "5"
        int m_tray_tar; // tray_tar : "0" ~ "15" or "255",
        char image_url[1024]; /* 印刷サムネイル URL（S3 署名付きで長いので 1KB 確保） */
        int has_public_filaments;
    } XTouchBambuStatus;

    XTouchBambuStatus bambuStatus;

    /* Public filaments: 都度組み立て。SD から表示時に読み、オプション文字列と「現在ブランドの items」だけ保持。 */
#define XPTOUCH_FILAMENT_MAX_BRANDS 8
#define XPTOUCH_FILAMENT_MAX_ITEMS_PER_BRAND 20
#define XPTOUCH_FILAMENT_OPTS_BUF_SIZE 256
    typedef struct
    {
        char id[12];  /* setting_id */
        char n[24];   /* name */
        char t[16];   /* type */
    } XTouchFilamentItem;

    /** 都度組み立て用: ブランド一覧オプション文字列 "B1\nB2\n" */
    extern char xptouch_filament_brand_options[XPTOUCH_FILAMENT_OPTS_BUF_SIZE];
    /** 都度組み立て用: Type オプション文字列 "T1\nT2\n" */
    extern char xptouch_filament_type_options[XPTOUCH_FILAMENT_OPTS_BUF_SIZE];
    extern int xptouch_filament_num_brands;
    /** 最後に load_type_options したブランドの表示インデックス。−1 は未ロード。 */
    extern int xptouch_filament_current_brand_index;
    extern int xptouch_filament_current_type_count;
    /** 1＝パイプバッファに _brands.txt が入っている（get_ith_brand で再読しない）。0＝ブランドファイル等が入っている。 */
    extern int xptouch_filament_pipe_holds_brands;
    /** 1＝ブランドは固定（Bambu Lab, Generic の2つのみ。_brands.txt は読まない）。 */
    extern int xptouch_filament_use_fixed_brands;

    /** パイプ形式テキスト用バッファ（SD /xtouch/filament/ から読み込み）。DRAM 節約のため 3064。 */
#define XPTOUCH_FILAMENTS_PIPE_BUF_SIZE 1024
    extern char xTouchFilamentsPipeBuf[XPTOUCH_FILAMENTS_PIPE_BUF_SIZE];
    extern unsigned int xTouchFilamentsPipeLen;

    typedef struct XTouchControlModeStruct
    {
        int inc;
        int target_bed_temper;
        int target_nozzle_temper;
    } XtouchControlMode;

    XtouchControlMode controlMode;

    typedef struct XTouchTouchConfigStruct
    {
        bool xTouchAuxFanEnabled;
        bool xTouchChamberFanEnabled;
        bool xTouchOTAEnabled;
        bool xTouchTFTFlip;
        bool xTouchTFTInvert;
        char xTouchHost[16];
        char xTouchAccessCode[9];
        /** PushAll(print.net) から得た Main プリンタのカメラ接続先IP。 */
        char xTouchCameraHost[16];
        /** PushAll(print.net) から得た Main プリンタの access code（無ければ設定値を使用）。 */
        char xTouchCameraAccessCode[9];
        char xTouchSerialNumber[16];
        /** pair.json の paired と一致。一時的な操作プリンタ切替えでは変えない（再起動で loadPair が xTouchSerialNumber を戻す）。 */
        char xTouchPairedSerialNumber[16];
        char xTouchPrinterModel[32];
        char xTouchPrinterName[32];
        int xTouchBacklightLevel;
        int xTouchTFTOFFValue;
        int xTouchLEDOffValue;
        bool xTouchWakeOnPrint;
        bool xTouchWakeDuringPrint;
        /** スリープ復帰時に Chamber LED が Off なら On にする（General 設定） */
        bool xTouchChamberLedOnWake;
        int currentScreenIndex;
 
        bool xTouchStackChanEnabled;
        bool xTouchPreheatEnabled;
        /** Multi Printer Monitor / Printers 画面を有効にする（デフォルト true）。5inch のみ。 */
        bool xTouchMultiPrinterMonitorEnabled;
        /** History 画面を有効にする（デフォルト false）。5inch のみ。 */
        bool xTouchHistoryEnabled;
        /** true のとき Home / Printers / History のサムネイルを表示しない（DL も行わない）。5inch のみ。 */
        bool xTouchHideAllThumbnails;
        /** true のとき P1S Camera Stream を有効化。false ならカメラ画面でもストリーム接続しない。 */
        bool xTouchP1sCameraStreamEnabled;

        int xTouchNeoPixelNumValue;
        int xTouchNeoPixelBrightnessValue;
        int xTouchNeoPixelPinValue;

        int xTouchAlarmTimeoutValue;
        bool xTouchIdleLEDEnabled;

        bool xTouchChamberSensorEnabled;
        int xTouchChamberSensorReadingDiff;
        int xTouchUtilCalibrationBitmask;
        bool xTouchLanOnlyMode;
        bool xTouchProvisioningMode;
    } xPTouchConfigParam;

    xPTouchConfigParam xPTouchConfig;
    

    /* HMS */

    enum ModuleID
    {
        MODULE_UKNOWN = 0x00,
        MODULE_01 = 0x01,
        MODULE_02 = 0x02,
        MODULE_MC = 0x03,
        MODULE_04 = 0x04,
        MODULE_MAINBOARD = 0x05,
        MODULE_06 = 0x06,
        MODULE_AMS = 0x07,
        MODULE_TH = 0x08,
        MODULE_09 = 0x09,
        MODULE_10 = 0x0A,
        MODULE_11 = 0x0B,
        MODULE_XCAM = 0x0C,
        MODULE_13 = 0x0D,
        MODULE_14 = 0x0E,
        MODULE_15 = 0x0F,
        MODULE_MAX = 0x10
    };

    enum HMSMessageLevel
    {
        HMS_UNKNOWN = 0,
        HMS_FATAL = 1,
        HMS_SERIOUS = 2,
        HMS_COMMON = 3,
        HMS_INFO = 4,
        HMS_MSG_LEVEL_MAX,
    };

    typedef struct
    {
        int module_id;
        unsigned module_num;
        unsigned part_id;
        unsigned reserved;
        int msg_level;
        int msg_code;
    } HMSItem;

    typedef struct
    {
        char subtask_id[32];
        int print_error;
    } ClearErrorMessage;

#ifdef __XPTOUCH_PLATFORM_S3__
#define XPTOUCH_MULTI_PRINTER_MAX 5
#define XPTOUCH_OTHER_PRINTERS_MAX (XPTOUCH_MULTI_PRINTER_MAX - 1)
#define XPTOUCH_DEV_PRODUCT_NAME_LEN 24

    typedef struct
    {
        char dev_id[16];
        char name[32];
        int print_status;
        int mc_print_percent;
        int mc_left_time;
        char subtask_name[32];
        char image_url[1024]; /* 印刷サムネイル URL（S3 署名付きで長いので 1KB 確保） */
        char task_id[32];    /* Cloud task 用 ID (/user/task/{id}) */
        int current_layer;
        int total_layers;
        unsigned char valid;
        /** printer.json の online。false のとき MQTT が来なくても IDLE と誤表示しない。push_status 受信で 1 に戻す */
        unsigned char online;
    } other_printer_status_t;

    extern other_printer_status_t otherPrinters[XPTOUCH_OTHER_PRINTERS_MAX];
    extern char xptouch_other_printer_dev_ids[XPTOUCH_OTHER_PRINTERS_MAX][16];
    extern char xptouch_current_printer_dev_product_name[XPTOUCH_DEV_PRODUCT_NAME_LEN];
    extern char xptouch_other_printer_dev_product_names[XPTOUCH_OTHER_PRINTERS_MAX][XPTOUCH_DEV_PRODUCT_NAME_LEN];
    extern int xptouch_other_printer_count;

    /** サムネイル表示用: スロット番号ごとの SD パス（"S:/tmp/{task_id}.png"）。UI はこれを参照するだけ。xtouch が init で埋める。 */
#define XPTOUCH_THUMB_SLOT_MAX 5
#define XPTOUCH_THUMB_PATH_LEN 64
    extern char xptouch_thumbnail_slot_path[XPTOUCH_THUMB_SLOT_MAX][XPTOUCH_THUMB_PATH_LEN];
    /** LGFX デコード済みサムネイルの descriptor ポインタ（スロット毎）。UI は lv_img_set_src(img, (lv_img_dsc_t*)xptouch_thumbnail_slot_dsc[slot]) で表示。 */
    extern void *xptouch_thumbnail_slot_dsc[XPTOUCH_THUMB_SLOT_MAX];

    /** Cloud 印刷履歴（user-service/my/tasks）。件数・文字列長・API/ワーカー・サムネ decode はすべてここで揃える。 */
#define XPTOUCH_HISTORY_TASKS_MAX 10
#define XPTOUCH_HISTORY_UI_ROW_SLOTS XPTOUCH_HISTORY_TASKS_MAX
#define XPTOUCH_HISTORY_COVER_SLOTS XPTOUCH_HISTORY_TASKS_MAX
#define XPTOUCH_HISTORY_TITLE_LEN 64
#define XPTOUCH_HISTORY_COVER_URL_LEN 1024
#define XPTOUCH_HISTORY_DEVICE_NAME_LEN 32
#define XPTOUCH_HISTORY_DEVICE_MODEL_LEN 24
/** Cloud / MQTT の task id と揃える（History 行と Home の /tmp/{id}.png を一致させる） */
#define XPTOUCH_HISTORY_TASK_ID_LEN 32
#define XPTOUCH_HISTORY_MODEL_ID_LEN 32
#define XPTOUCH_HISTORY_TIME_LEN 32
/** GET /my/tasks の 1 回の limit（API 取得件数。画面・xptouch_history_tasks[] は XPTOUCH_HISTORY_TASKS_MAX まで） */
#define XPTOUCH_HISTORY_FETCH_PAGE_LIMIT 15
/** カバー DL 完了キュー深度 */
#define XPTOUCH_HISTORY_COVER_QUEUE_LEN (XPTOUCH_HISTORY_TASKS_MAX)
#define XPTOUCH_HISTORY_COVER_TASK_STACK_WORDS 6144
#define XPTOUCH_HISTORY_COVER_POLL_MS 150
/** Printers→History 直後の SSL と競合しにくくする取得開始遅延 */
#define XPTOUCH_HISTORY_FETCH_DELAY_MS 900
#define XPTOUCH_HISTORY_FETCH_RETRY_DELAY_MS 2500
#define XPTOUCH_HISTORY_FETCH_RETRY_MAX 3
/** LIST_REFRESH 後にサムネ処理を始めるまでの待ち ms */
#define XPTOUCH_HISTORY_COVER_DEFER_AFTER_LIST_MS 30
/** History 一覧カバー画像のデコード解像度（thumbnail の LGFX パスと一致。5" のみ 150px） */
#if defined(__XPTOUCH_SCREEN_S3_050__)
#define XPTOUCH_HISTORY_COVER_W 150
#define XPTOUCH_HISTORY_COVER_H 150
#else
#define XPTOUCH_HISTORY_COVER_W 75
#define XPTOUCH_HISTORY_COVER_H 75
#endif
    /** Bambu 想定: AMS 最大 4 ユニット × 各 4 トレイ = 16 スロット。amsDetailMapping も最大 16 要素 */
#define XPTOUCH_BAMBU_AMS_UNITS 4
#define XPTOUCH_BAMBU_AMS_SLOTS_PER_UNIT 4
#define XPTOUCH_HISTORY_AMS_MAP_MAX ((XPTOUCH_BAMBU_AMS_UNITS) * (XPTOUCH_BAMBU_AMS_SLOTS_PER_UNIT))
#define XPTOUCH_OTHER_TRAY_SETTING_ID_LEN 16
    /** 他プリンタ用: push_status の AMS を自機の trays[] とは別に保持（Reprint のスロット表示・API 用） */
    typedef struct
    {
        uint64_t tray_status;
        char tray_color[16];
        char tray_type[24];
        char tray_setting_id[XPTOUCH_OTHER_TRAY_SETTING_ID_LEN];
    } xptouch_other_printer_tray_cell_t;
    extern xptouch_other_printer_tray_cell_t xptouch_other_printer_trays[XPTOUCH_OTHER_PRINTERS_MAX][XPTOUCH_BAMBU_AMS_UNITS][XPTOUCH_BAMBU_AMS_SLOTS_PER_UNIT];
    extern long xptouch_other_printer_tray_ams_exist_bits[XPTOUCH_OTHER_PRINTERS_MAX];
    extern int xptouch_history_reprint_printer_dd_slot;
    typedef struct
    {
        int ams;
        int amsId;
        int slotId;
        int nozzleId;
        double weight;
        char filamentId[16];
        char filamentType[20];
        char sourceColor[16];
        char targetColor[16];
        char targetFilamentType[20];
    } xptouch_history_ams_map_t;
    typedef struct
    {
        char task_id[XPTOUCH_HISTORY_TASK_ID_LEN];  /* API の id を文字列で */
        char title[XPTOUCH_HISTORY_TITLE_LEN];
        char cover_url[XPTOUCH_HISTORY_COVER_URL_LEN];
        char device_name[XPTOUCH_HISTORY_DEVICE_NAME_LEN];
        char device_model[XPTOUCH_HISTORY_DEVICE_MODEL_LEN]; /* API deviceModel（deviceName 空時の表示用） */
        char start_time[XPTOUCH_HISTORY_TIME_LEN];
        char end_time[XPTOUCH_HISTORY_TIME_LEN];
        char model_id[XPTOUCH_HISTORY_MODEL_ID_LEN]; /* create_task 用 */
        int profile_id;   /* create_task 用 (profileId) */
        int plate_index;  /* create_task 用 (plateIndex) */
        int status;      /* 2=失敗, 3=完了 等 */
        int is_printable; /* 1=再印刷可 */
        /** 履歴一覧取得時点で amsDetailMapping を含むか（詳細は Reprint 押下時に別取得して展開） */
        unsigned char has_ams_mapping;
        unsigned char valid;
    } xptouch_history_task_t;
    extern xptouch_history_task_t xptouch_history_tasks[XPTOUCH_HISTORY_TASKS_MAX];
    extern int xptouch_history_count;
    /** History 画面 行別: LGFX デコード済み cover 画像の descriptor。UI は lv_img_set_src(img, (lv_img_dsc_t*)xptouch_history_cover_dsc[idx]) で表示。 */
    extern void *xptouch_history_cover_dsc[XPTOUCH_HISTORY_COVER_SLOTS];
    /** History リプリント対象の task_id を保持する（tasks 一覧が空でも再印刷可能にする）。空なら無効。 */
    extern char xptouch_history_reprint_task_id[XPTOUCH_HISTORY_TASK_ID_LEN];
    /** xptouch_history_reprint_task_id が有効か（空文字列判定の代わり）。 */
    extern int xptouch_history_reprint_task_id_valid;
    /** Reprint 上半分（タイトル/デバイス/Plate/サムネ）表示用の task 基本情報 */
    extern xptouch_history_task_t xptouch_history_reprint_task_basic;
    extern int xptouch_history_reprint_task_basic_valid;
    /** Reprint 用カバー画像デコード済み descriptor（History一覧の行とは独立） */
    extern void *xptouch_history_reprint_cover_dsc;
    /** Reprint 対象タスクの amsDetailMapping を展開した共有バッファ（必要時のみ取得）。count<0 は取得中、0 は無し */
    extern int xptouch_history_selected_ams_map_count;
    /** HistoryReprint の task_id detail fetch を多重実行しないためのフラグ */
    extern int xptouch_history_reprint_detail_fetch_inflight;
    /** HistoryReprint セッションの task_id detail fetch 完了フラグ（再初期化でも再取得しない） */
    extern int xptouch_history_reprint_detail_fetch_done;
    extern xptouch_history_ams_map_t xptouch_history_selected_ams_map[XPTOUCH_HISTORY_AMS_MAP_MAX];
    /** リプリント確定時: ams_map[i] に割り当てる AMS ユニット(0..XPTOUCH_BAMBU_AMS_UNITS-1)とトレイ(0..3)。External は ams=0 tray=254。 */
    extern uint8_t xptouch_history_reprint_pick_ams[XPTOUCH_HISTORY_AMS_MAP_MAX];
    extern uint8_t xptouch_history_reprint_pick_tray[XPTOUCH_HISTORY_AMS_MAP_MAX];
#endif

    /** フィラメント Brand/Type ドロップダウン用。都度 SD から組み立てたオプション文字列を返す。実装は filaments_options.c。 */
    void xptouch_public_filaments_get_brand_options(char *buf, unsigned int buf_len);
    void xptouch_public_filaments_get_type_options(int brand_idx, char *buf, unsigned int buf_len);
    void xptouch_public_filaments_get_type_options_by_name(const char *brand_name, char *buf, unsigned int buf_len);
    /** 表示インデックス（Brand ドロップダウンの選択番号）で Type 候補を取得。get_brand_options と同じ並びで対応。 */
    void xptouch_public_filaments_get_type_options_by_display_index(int display_index, char *buf, unsigned int buf_len);
    /** ブランド一覧を SD から読んでオプション文字列を組み立てる。get_brand_options の前に呼ぶ。 */
    void xptouch_filaments_ensure_brands_loaded(void);
    /** 指定表示インデックスのブランドの Type 一覧を SD から読んで組み立てる。get_type_options_by_display_index の前に呼ぶ。 */
    void xptouch_filaments_load_type_options_for_display_index(int display_index);
    /** AMS 編集画面を開くときに呼ぶ（ensure_brands_loaded のエイリアス）。 */
    void xptouch_filaments_load_for_current_printer_c(void);
    /** i 番目のブランド名を _brands.txt から都度取得。buf に最大 buf_len-1 文字＋NUL。 */
    void xptouch_filaments_get_brand_name_at_index(int index, char *buf, unsigned int buf_len);
    /** 指定ブランド・Type の setting_id / name / type / 温度をファイルから取得。行は id|n|t または id|n|t|min|max。out_min/out_max は NULL 可。 */
    void xptouch_filaments_get_id_n_for_brand_type_index(int brand_display_index, int type_display_index, char *id_buf, unsigned int id_len, char *n_buf, unsigned int n_len, char *type_buf, unsigned int type_len, int *out_nozzle_temp_min, int *out_nozzle_temp_max);
    /** 選択中の Brand/Type の setting_id / name / type / 温度を取得。Save 実装用。 */
    void xptouch_public_filaments_get_selected_id_n(int brand_display_index, int type_display_index, char *id_buf, unsigned int id_len, char *n_buf, unsigned int n_len, char *type_buf, unsigned int type_len, int *out_nozzle_temp_min, int *out_nozzle_temp_max);
    /** filaments_rev.json で filament_id → b/t 逆引き。見つかれば 1、なければ 0。 */
    int xptouch_public_filaments_rev_lookup(const char *filament_id, char *out_brand, size_t out_brand_len, char *out_type, size_t out_type_len);
    /** brand_str と type_str に一致する表示インデックスを返す。見つかれば 1、なければ 0。 */
    int xptouch_public_filaments_find_indices_by_brand_and_type(const char *brand_str, const char *type_str, int *out_brand_idx, int *out_type_idx);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif