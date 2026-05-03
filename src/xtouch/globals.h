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
#ifdef __XTOUCH_PLATFORM_S3__
extern other_printer_status_t otherPrinters[XTOUCH_OTHER_PRINTERS_MAX];
extern char xtouch_other_printer_dev_ids[XTOUCH_OTHER_PRINTERS_MAX][16];
extern int xtouch_other_printer_count;
void xtouch_mqtt_pushall_for_dev_c(const char *dev_id);
void xtouch_mqtt_pushall_main_printer_for_screen_c(void);
/** History/Reprint からそれ以外の画面へ出るとき一覧バッファを捨てる（15↔16 は保持） */
void xtouch_history_clear_tasks_on_leave_c(void);
// Camera 画面表示中は Chamber LED 自動オフタイマーを止める／離脱時に再開
void xtouch_screen_led_off_timer_pause_for_camera_c(void);
void xtouch_screen_led_off_timer_resume_c(void);
#endif
#ifdef __cplusplus
}
#endif

#endif