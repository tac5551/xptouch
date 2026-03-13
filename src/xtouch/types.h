#ifndef _XLCD_TYPES
#define _XLCD_TYPES

#include <stddef.h>

#define XTOUCH_LCD_MIN_SLEEP_TIME 5
#define XTOUCH_LIGHT_MIN_SLEEP_TIME 5

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
        XTOUCH_PRINT_STATUS_IDLE,
        XTOUCH_PRINT_STATUS_RUNNING,
        XTOUCH_PRINT_STATUS_PAUSED,
        XTOUCH_PRINT_STATUS_FINISHED,
        XTOUCH_PRINT_STATUS_PREPARE,
        XTOUCH_PRINT_STATUS_FAILED,
    };

    enum XTouchPrintingSpeedLevel
    {
        XTOUCH_SPEED_LEVEL_INVALID = 0,
        XTOUCH_SPEED_LEVEL_SILENCE = 1,
        XTOUCH_SPEED_LEVEL_NORMAL = 2,
        XTOUCH_SPEED_LEVEL_RAPID = 3,
        XTOUCH_SPEED_LEVEL_RAMPAGE = 4,
        XTOUCH_SPEED_LEVEL_COUNT
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
#define XTOUCH_FILAMENT_MAX_BRANDS 8
#define XTOUCH_FILAMENT_MAX_ITEMS_PER_BRAND 20
#define XTOUCH_FILAMENT_OPTS_BUF_SIZE 256
    typedef struct
    {
        char id[12];  /* setting_id */
        char n[24];   /* name */
        char t[16];   /* type */
    } XTouchFilamentItem;

    /** 都度組み立て用: ブランド一覧オプション文字列 "B1\nB2\n" */
    extern char xtouch_filament_brand_options[XTOUCH_FILAMENT_OPTS_BUF_SIZE];
    /** 都度組み立て用: Type オプション文字列 "T1\nT2\n" */
    extern char xtouch_filament_type_options[XTOUCH_FILAMENT_OPTS_BUF_SIZE];
    extern int xtouch_filament_num_brands;
    /** 最後に load_type_options したブランドの表示インデックス。−1 は未ロード。 */
    extern int xtouch_filament_current_brand_index;
    extern int xtouch_filament_current_type_count;
    /** 1＝パイプバッファに _brands.txt が入っている（get_ith_brand で再読しない）。0＝ブランドファイル等が入っている。 */
    extern int xtouch_filament_pipe_holds_brands;
    /** 1＝ブランドは固定（Bambu Lab, Generic の2つのみ。_brands.txt は読まない）。 */
    extern int xtouch_filament_use_fixed_brands;

    /** パイプ形式テキスト用バッファ（SD /xtouch/filament/ から読み込み）。DRAM 節約のため 3064。 */
#define XTOUCH_FILAMENTS_PIPE_BUF_SIZE 1024
    extern char xTouchFilamentsPipeBuf[XTOUCH_FILAMENTS_PIPE_BUF_SIZE];
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
        char xTouchSerialNumber[16];
        char xTouchPrinterModel[32];
        char xTouchPrinterName[32];
        int xTouchBacklightLevel;
        int xTouchTFTOFFValue;
        int xTouchLEDOffValue;
        bool xTouchWakeOnPrint;
        bool xTouchWakeDuringPrint;
        int currentScreenIndex;
 
        bool xTouchStackChanEnabled;
        bool xTouchPreheatEnabled;
        /** Multi Printer Monitor / Printers 画面を有効にする（デフォルト true）。5inch のみ。 */
        bool xTouchMultiPrinterMonitorEnabled;

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
    } XTouchConfig;

    XTouchConfig xTouchConfig;
    

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

#ifdef __XTOUCH_SCREEN_50__
#define XTOUCH_MULTI_PRINTER_MAX 5
#define XTOUCH_OTHER_PRINTERS_MAX (XTOUCH_MULTI_PRINTER_MAX - 1)

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
    } other_printer_status_t;

    extern other_printer_status_t otherPrinters[XTOUCH_OTHER_PRINTERS_MAX];
    extern char xtouch_other_printer_dev_ids[XTOUCH_OTHER_PRINTERS_MAX][16];
    extern int xtouch_other_printer_count;

    /** サムネイル表示用: スロット番号ごとの SD パス（"S:/tmp/pthumb_N.png"）。UI はこれを参照するだけ。xtouch が init で埋める。 */
#define XTOUCH_THUMB_SLOT_MAX 5
#define XTOUCH_THUMB_PATH_LEN 64
    extern char xtouch_thumbnail_slot_path[XTOUCH_THUMB_SLOT_MAX][XTOUCH_THUMB_PATH_LEN];
    /** LGFX デコード済みサムネイルの descriptor ポインタ（スロット毎）。UI は lv_img_set_src(img, (lv_img_dsc_t*)xtouch_thumbnail_slot_dsc[slot]) で表示。 */
    extern void *xtouch_thumbnail_slot_dsc[XTOUCH_THUMB_SLOT_MAX];
#endif

    /** フィラメント Brand/Type ドロップダウン用。都度 SD から組み立てたオプション文字列を返す。実装は filaments_options.c。 */
    void xtouch_public_filaments_get_brand_options(char *buf, unsigned int buf_len);
    void xtouch_public_filaments_get_type_options(int brand_idx, char *buf, unsigned int buf_len);
    void xtouch_public_filaments_get_type_options_by_name(const char *brand_name, char *buf, unsigned int buf_len);
    /** 表示インデックス（Brand ドロップダウンの選択番号）で Type 候補を取得。get_brand_options と同じ並びで対応。 */
    void xtouch_public_filaments_get_type_options_by_display_index(int display_index, char *buf, unsigned int buf_len);
    /** ブランド一覧を SD から読んでオプション文字列を組み立てる。get_brand_options の前に呼ぶ。 */
    void xtouch_filaments_ensure_brands_loaded(void);
    /** 指定表示インデックスのブランドの Type 一覧を SD から読んで組み立てる。get_type_options_by_display_index の前に呼ぶ。 */
    void xtouch_filaments_load_type_options_for_display_index(int display_index);
    /** AMS 編集画面を開くときに呼ぶ（ensure_brands_loaded のエイリアス）。 */
    void xtouch_filaments_load_for_current_printer_c(void);
    /** i 番目のブランド名を _brands.txt から都度取得。buf に最大 buf_len-1 文字＋NUL。 */
    void xtouch_filaments_get_brand_name_at_index(int index, char *buf, unsigned int buf_len);
    /** 指定ブランド・Type の setting_id / name / type / 温度をファイルから取得。行は id|n|t または id|n|t|min|max。out_min/out_max は NULL 可。 */
    void xtouch_filaments_get_id_n_for_brand_type_index(int brand_display_index, int type_display_index, char *id_buf, unsigned int id_len, char *n_buf, unsigned int n_len, char *type_buf, unsigned int type_len, int *out_nozzle_temp_min, int *out_nozzle_temp_max);
    /** 選択中の Brand/Type の setting_id / name / type / 温度を取得。Save 実装用。 */
    void xtouch_public_filaments_get_selected_id_n(int brand_display_index, int type_display_index, char *id_buf, unsigned int id_len, char *n_buf, unsigned int n_len, char *type_buf, unsigned int type_len, int *out_nozzle_temp_min, int *out_nozzle_temp_max);
    /** filaments_rev.json で filament_id → b/t 逆引き。見つかれば 1、なければ 0。 */
    int xtouch_public_filaments_rev_lookup(const char *filament_id, char *out_brand, size_t out_brand_len, char *out_type, size_t out_type_len);
    /** brand_str と type_str に一致する表示インデックスを返す。見つかれば 1、なければ 0。 */
    int xtouch_public_filaments_find_indices_by_brand_and_type(const char *brand_str, const char *type_str, int *out_brand_idx, int *out_type_idx);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif