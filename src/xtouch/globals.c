#include <stdbool.h>
#include <stdio.h>
#include "globals.h"

/* グローバル変数の実際の定義（型は globals.h → types.h） */
bool xtouch_mqtt_light_on = false;
bool xtouch_ota_update_flag = false;
bool xtouch_neopixel_enabled = true;
char xTouchFilamentsPipeBuf[XTOUCH_FILAMENTS_PIPE_BUF_SIZE];
unsigned int xTouchFilamentsPipeLen = 0;
char xtouch_filament_brand_options[XTOUCH_FILAMENT_OPTS_BUF_SIZE];
char xtouch_filament_type_options[XTOUCH_FILAMENT_OPTS_BUF_SIZE];
int xtouch_filament_num_brands = 0;
int xtouch_filament_current_brand_index = -1;
int xtouch_filament_current_type_count = 0;
int xtouch_filament_pipe_holds_brands = 0;
int xtouch_filament_use_fixed_brands = 1;

#ifdef __XTOUCH_PLATFORM_S3__
other_printer_status_t otherPrinters[XTOUCH_OTHER_PRINTERS_MAX];
char xtouch_other_printer_dev_ids[XTOUCH_OTHER_PRINTERS_MAX][16];
char xtouch_current_printer_dev_product_name[XTOUCH_DEV_PRODUCT_NAME_LEN];
char xtouch_other_printer_dev_product_names[XTOUCH_OTHER_PRINTERS_MAX][XTOUCH_DEV_PRODUCT_NAME_LEN];
int xtouch_other_printer_count = 0;
char xtouch_thumbnail_slot_path[XTOUCH_THUMB_SLOT_MAX][XTOUCH_THUMB_PATH_LEN];
xtouch_history_task_t xtouch_history_tasks[XTOUCH_HISTORY_TASKS_MAX];
int xtouch_history_count = 0;
char xtouch_history_reprint_task_id[XTOUCH_HISTORY_TASK_ID_LEN] = { 0 };
int xtouch_history_reprint_task_id_valid = 0;
xtouch_history_task_t xtouch_history_reprint_task_basic = { 0 };
int xtouch_history_reprint_task_basic_valid = 0;
int xtouch_history_selected_ams_map_count = -1;
int xtouch_history_reprint_detail_fetch_inflight = 0;
int xtouch_history_reprint_detail_fetch_done = 0;
xtouch_history_ams_map_t xtouch_history_selected_ams_map[XTOUCH_HISTORY_AMS_MAP_MAX];
uint8_t xtouch_history_reprint_pick_ams[XTOUCH_HISTORY_AMS_MAP_MAX];
uint8_t xtouch_history_reprint_pick_tray[XTOUCH_HISTORY_AMS_MAP_MAX];
xtouch_other_printer_tray_cell_t xtouch_other_printer_trays[XTOUCH_OTHER_PRINTERS_MAX][XTOUCH_BAMBU_AMS_UNITS][XTOUCH_BAMBU_AMS_SLOTS_PER_UNIT];
long xtouch_other_printer_tray_ams_exist_bits[XTOUCH_OTHER_PRINTERS_MAX];
int xtouch_history_reprint_printer_dd_slot = 0;
#endif

// グローバル関数の実際の定義
void xtouch_globals_init()
{
    controlMode.inc = 10;
#ifdef __XTOUCH_PLATFORM_S3__
    /* 起動時は pthumb_N.png を指さない。task_id ベースのパスは getThumbPathForSlot() で設定される */
    for (int i = 0; i < XTOUCH_THUMB_SLOT_MAX; i++)
        xtouch_thumbnail_slot_path[i][0] = '\0';
#endif
} 