#ifndef _XLCD_PATHS
#define _XLCD_PATHS

const char *xtouch_paths_eeprom = "/eeprom.bin";
const char *xtouch_paths_root = "/xtouch";
const char *xtouch_paths_config = "/provisioning.json";
const char *xtouch_paths_timezones = "/timezones.json";

const char *xtouch_paths_settings = "/xtouch/settings.json";
const char *xtouch_paths_printers = "/xtouch/printer.json";
const char *xtouch_paths_pair = "/xtouch/printer-pair.json";
const char *xtouch_paths_touch = "/xtouch/touch.json";


const char *xtouch_paths_firmware_ota_json = "/xtouch/ota.json";
#ifdef __XTOUCH_SCREEN_50__
const char *xtouch_paths_firmware_ota_file = "https://tac-lab.tech/xptouch-bin/5.0/ota/ota.json";
#else
const char *xtouch_paths_firmware_ota_file = "https://tac-lab.tech/xptouch-bin/2.8/ota/ota.json";
#endif
const char *xtouch_paths_firmware_ota_fw = "/firmware.bin";

const char *xtouch_paths_hms_key_db = "/xtouch/device_hms.key.db";
const char *xtouch_paths_hms_value_db = "/xtouch/device_hms.value.db";

const char *xtouch_paths_public_filaments = "/xtouch/public_filaments.json";
const char *xtouch_paths_private_filaments = "/xtouch/private_filaments.json";
const char *xtouch_paths_private_filaments_flat = "/xtouch/private_filaments_flat.json";

#endif