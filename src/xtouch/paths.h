#ifndef _XLCD_PATHS
#define _XLCD_PATHS

/* Constants only. No functions here. */

/* SD カード上のルートディレクトリ。すべての /xtouch パスはここから組み立てる。 */
#define XPTOUCH_PATH_ROOT "/xtouch"

const char *xptouch_paths_eeprom = "/eeprom.bin";
const char *xptouch_paths_root   = XPTOUCH_PATH_ROOT;

// for lan only mode
const char *xptouch_paths_config = "/xtouch.json";

//for global settings
const char *xptouch_paths_settings = XPTOUCH_PATH_ROOT "/settings.json";
const char *xptouch_paths_touch    = XPTOUCH_PATH_ROOT "/touch.json";
/** 5インチ: SD ルートの lcd.json で RGB PCLK を EEPROM に書き換えて再起動（検出後に削除） */
const char *xptouch_paths_lcd_json = "/lcd.json";

//for cloud
const char *xptouch_paths_provisioning = "/provisioning.json";
const char *xptouch_paths_printers     = XPTOUCH_PATH_ROOT "/printer.json";
/** printer.json の DynamicJsonDocument 容量。デフォルト 1024 では複数台・長いフィールドでパース失敗する */
#define XPTOUCH_PRINTERS_JSON_DOC_CAP 12288
const char *xptouch_paths_pair         = XPTOUCH_PATH_ROOT "/printer-pair.json";

//for OTA Update
#define XPTOUCH_OTA_HOST "tac-lab.tech"
const char *xptouch_paths_firmware_ota_host = XPTOUCH_OTA_HOST;
const char *xptouch_paths_firmware_ota_json = XPTOUCH_PATH_ROOT "/ota.json";
#if defined(__XPTOUCH_SCREEN_S3_050__)
const char *xptouch_paths_firmware_ota_file = "https://" XPTOUCH_OTA_HOST "/xptouch-bin/5.0/ota/ota.json";
#elif defined(__XPTOUCH_SCREEN_S3_3248__)
const char *xptouch_paths_firmware_ota_file = "https://" XPTOUCH_OTA_HOST "/xptouch-bin/s3_3.5/ota/ota.json";
#elif defined(__XPTOUCH_SCREEN_S3_028__)
const char *xptouch_paths_firmware_ota_file = "https://" XPTOUCH_OTA_HOST "/xptouch-bin/s3_2.8/ota/ota.json";
#else
const char *xptouch_paths_firmware_ota_file = "https://" XPTOUCH_OTA_HOST "/xptouch-bin/2.8/ota/ota.json";
#endif
const char *xptouch_paths_firmware_ota_fw = "/firmware.bin";

/** フィラメント一覧・温度プリセット等。Chrome 拡張で生成。Load from SD at runtime. */
const char *xptouch_paths_filament_dir      = XPTOUCH_PATH_ROOT "/filament";
/** GFL99 等 → b/t 逆引き。Chrome 拡張で生成。 */
const char *xptouch_paths_filaments_rev     = XPTOUCH_PATH_ROOT "/filament/filaments_rev.json";

/** フィラメント設定 JSON（Extention が生成）。/xtouch/filament/json/<setting_id>.json */
const char *xptouch_paths_filament_json_dir = XPTOUCH_PATH_ROOT "/filament/json";

#endif