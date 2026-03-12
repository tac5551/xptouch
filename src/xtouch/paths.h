#ifndef _XLCD_PATHS
#define _XLCD_PATHS

/* Constants only. No functions here. */

const char *xtouch_paths_eeprom = "/eeprom.bin";
const char *xtouch_paths_root = "/xtouch";

// for lan only mode
const char *xtouch_paths_config = "/xtouch.json";

//for global settings
const char *xtouch_paths_settings = "/xtouch/settings.json";
const char *xtouch_paths_touch = "/xtouch/touch.json";

//for cloud
const char *xtouch_paths_provisioning = "/provisioning.json";
const char *xtouch_paths_printers = "/xtouch/printer.json";
const char *xtouch_paths_pair = "/xtouch/printer-pair.json";

//for OTA Update
const char *xtouch_paths_firmware_ota_json = "/xtouch/ota.json";
#ifdef __XTOUCH_SCREEN_50__
const char *xtouch_paths_firmware_ota_file = "https://tac-lab.tech/xptouch-bin/5.0/ota/ota.json";
#else
const char *xtouch_paths_firmware_ota_file = "https://tac-lab.tech/xptouch-bin/2.8/ota/ota.json";
#endif
const char *xtouch_paths_firmware_ota_fw = "/firmware.bin";

/** フィラメント一覧・温度プリセット等。Chrome 拡張で生成。Load from SD at runtime. */
const char *xtouch_paths_filament_dir = "/xtouch/filament";
/** GFL99 等 → b/t 逆引き。Chrome 拡張で生成。 */
const char *xtouch_paths_filaments_rev = "/xtouch/filament/filaments_rev.json";

/** フィラメント設定 JSON（Extention が生成）。/xtouch/filament/json/<setting_id>.json */
const char *xtouch_paths_filament_json_dir = "/xtouch/filament/json";

#endif