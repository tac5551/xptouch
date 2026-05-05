#include <stdbool.h>
#include <stdio.h>
#include "globals.h"

/* グローバル変数の実際の定義（型は globals.h → types.h） */
bool xptouch_mqtt_light_on = false;
bool xptouch_ota_update_flag = false;
bool xptouch_neopixel_enabled = true;
char xTouchFilamentsPipeBuf[XPTOUCH_FILAMENTS_PIPE_BUF_SIZE];
unsigned int xTouchFilamentsPipeLen = 0;
char xptouch_filament_brand_options[XPTOUCH_FILAMENT_OPTS_BUF_SIZE];
char xptouch_filament_type_options[XPTOUCH_FILAMENT_OPTS_BUF_SIZE];
int xptouch_filament_num_brands = 0;
int xptouch_filament_current_brand_index = -1;
int xptouch_filament_current_type_count = 0;
int xptouch_filament_pipe_holds_brands = 0;
int xptouch_filament_use_fixed_brands = 1;

#ifdef __XPTOUCH_PLATFORM_S3__
other_printer_status_t otherPrinters[XPTOUCH_OTHER_PRINTERS_MAX];
char xptouch_other_printer_dev_ids[XPTOUCH_OTHER_PRINTERS_MAX][16];
char xptouch_current_printer_dev_product_name[XPTOUCH_DEV_PRODUCT_NAME_LEN];
char xptouch_other_printer_dev_product_names[XPTOUCH_OTHER_PRINTERS_MAX][XPTOUCH_DEV_PRODUCT_NAME_LEN];
int xptouch_other_printer_count = 0;
char xptouch_thumbnail_slot_path[XPTOUCH_THUMB_SLOT_MAX][XPTOUCH_THUMB_PATH_LEN];
xptouch_history_task_t xptouch_history_tasks[XPTOUCH_HISTORY_TASKS_MAX];
int xptouch_history_count = 0;
char xptouch_history_reprint_task_id[XPTOUCH_HISTORY_TASK_ID_LEN] = { 0 };
int xptouch_history_reprint_task_id_valid = 0;
xptouch_history_task_t xptouch_history_reprint_task_basic = { 0 };
int xptouch_history_reprint_task_basic_valid = 0;
int xptouch_history_selected_ams_map_count = -1;
int xptouch_history_reprint_detail_fetch_inflight = 0;
int xptouch_history_reprint_detail_fetch_done = 0;
xptouch_history_ams_map_t xptouch_history_selected_ams_map[XPTOUCH_HISTORY_AMS_MAP_MAX];
uint8_t xptouch_history_reprint_pick_ams[XPTOUCH_HISTORY_AMS_MAP_MAX];
uint8_t xptouch_history_reprint_pick_tray[XPTOUCH_HISTORY_AMS_MAP_MAX];
xptouch_other_printer_tray_cell_t xptouch_other_printer_trays[XPTOUCH_OTHER_PRINTERS_MAX][XPTOUCH_BAMBU_AMS_UNITS][XPTOUCH_BAMBU_AMS_SLOTS_PER_UNIT];
long xptouch_other_printer_tray_ams_exist_bits[XPTOUCH_OTHER_PRINTERS_MAX];
int xptouch_history_reprint_printer_dd_slot = 0;
#endif

// グローバル関数の実際の定義
void xptouch_globals_init()
{
    controlMode.inc = 10;
#ifdef __XPTOUCH_PLATFORM_S3__
    /* 起動時は pthumb_N.png を指さない。task_id ベースのパスは getThumbPathForSlot() で設定される */
    for (int i = 0; i < XPTOUCH_THUMB_SLOT_MAX; i++)
        xptouch_thumbnail_slot_path[i][0] = '\0';
#endif
} 