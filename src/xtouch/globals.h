#ifndef _XLCD_GLOBALS
#define _XLCD_GLOBALS

#ifdef __cplusplus
extern "C" {
#endif

void xtouch_globals_init();
extern bool xtouch_mqtt_light_on;
extern bool xtouch_ota_update_flag;
#ifdef __cplusplus
}
#endif

#endif