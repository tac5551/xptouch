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

#ifdef __XTOUCH_SCREEN_50__
other_printer_status_t otherPrinters[XTOUCH_OTHER_PRINTERS_MAX];
char xtouch_other_printer_dev_ids[XTOUCH_OTHER_PRINTERS_MAX][16];
int xtouch_other_printer_count = 0;
char xtouch_thumbnail_slot_path[XTOUCH_THUMB_SLOT_MAX][XTOUCH_THUMB_PATH_LEN];
xtouch_history_task_t xtouch_history_tasks[XTOUCH_HISTORY_TASKS_MAX];
int xtouch_history_count = 0;
int xtouch_history_selected_index = -1;
int xtouch_history_selected_detail_index = -1;
int xtouch_history_selected_ams_map_count = -1;
xtouch_history_ams_map_t xtouch_history_selected_ams_map[XTOUCH_HISTORY_AMS_MAP_MAX];
uint8_t xtouch_history_reprint_pick_ams[XTOUCH_HISTORY_AMS_MAP_MAX];
uint8_t xtouch_history_reprint_pick_tray[XTOUCH_HISTORY_AMS_MAP_MAX];
#endif

// グローバル関数の実際の定義
void xtouch_globals_init()
{
    controlMode.inc = 10;
#ifdef __XTOUCH_SCREEN_50__
    /* 起動時は pthumb_N.png を指さない。task_id ベースのパスは getThumbPathForSlot() で設定される */
    for (int i = 0; i < XTOUCH_THUMB_SLOT_MAX; i++)
        xtouch_thumbnail_slot_path[i][0] = '\0';
#endif
} 