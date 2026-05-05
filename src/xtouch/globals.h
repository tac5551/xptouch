#ifndef _XLCD_GLOBALS
#define _XLCD_GLOBALS

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void xptouch_globals_init();
extern bool xptouch_mqtt_light_on;
extern bool xptouch_ota_update_flag;
extern bool xptouch_neopixel_enabled;
extern char xTouchFilamentsPipeBuf[];
extern unsigned int xTouchFilamentsPipeLen;
extern char xptouch_filament_brand_options[XPTOUCH_FILAMENT_OPTS_BUF_SIZE];
extern char xptouch_filament_type_options[XPTOUCH_FILAMENT_OPTS_BUF_SIZE];
extern int xptouch_filament_num_brands;
extern int xptouch_filament_current_brand_index;
extern int xptouch_filament_current_type_count;
extern int xptouch_filament_pipe_holds_brands;
extern int xptouch_filament_use_fixed_brands;
#ifdef __XPTOUCH_PLATFORM_S3__
extern other_printer_status_t otherPrinters[XPTOUCH_OTHER_PRINTERS_MAX];
extern char xptouch_other_printer_dev_ids[XPTOUCH_OTHER_PRINTERS_MAX][16];
extern int xptouch_other_printer_count;
void xptouch_mqtt_pushall_for_dev_c(const char *dev_id);
void xptouch_mqtt_pushall_main_printer_for_screen_c(void);
/** History/Reprint からそれ以外の画面へ出るとき一覧バッファを捨てる（15↔16 は保持） */
void xptouch_history_clear_tasks_on_leave_c(void);
// Camera 画面表示中は Chamber LED 自動オフタイマーを止める／離脱時に再開
void xptouch_screen_led_off_timer_pause_for_camera_c(void);
void xptouch_screen_led_off_timer_resume_c(void);
#endif
#ifdef __cplusplus
}
#endif

#endif