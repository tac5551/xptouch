#ifndef _XLCD_GLOBALS
#define _XLCD_GLOBALS

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void xtouch_globals_init();
extern bool xtouch_mqtt_light_on;
extern bool xtouch_ota_update_flag;
extern bool xtouch_neopixel_enabled;
extern char xTouchFilamentsPipeBuf[];
extern unsigned int xTouchFilamentsPipeLen;
extern char xtouch_filament_brand_options[XTOUCH_FILAMENT_OPTS_BUF_SIZE];
extern char xtouch_filament_type_options[XTOUCH_FILAMENT_OPTS_BUF_SIZE];
extern int xtouch_filament_num_brands;
extern int xtouch_filament_current_brand_index;
extern int xtouch_filament_current_type_count;
extern int xtouch_filament_pipe_holds_brands;
extern int xtouch_filament_use_fixed_brands;
#ifdef __XTOUCH_SCREEN_50__
extern other_printer_status_t otherPrinters[XTOUCH_OTHER_PRINTERS_MAX];
extern char xtouch_other_printer_dev_ids[XTOUCH_OTHER_PRINTERS_MAX][16];
extern int xtouch_other_printer_count;
#endif
#ifdef __cplusplus
}
#endif

#endif