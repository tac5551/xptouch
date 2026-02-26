#include <stdbool.h>
#include "globals.h"

/* グローバル変数の実際の定義（型は globals.h → types.h） */
bool xtouch_mqtt_light_on = false;
bool xtouch_ota_update_flag = false;
bool neopixel_enabled = true;
char xTouchFilamentsPipeBuf[XTOUCH_FILAMENTS_PIPE_BUF_SIZE];
unsigned int xTouchFilamentsPipeLen = 0;
char xtouch_filament_brand_options[XTOUCH_FILAMENT_OPTS_BUF_SIZE];
char xtouch_filament_type_options[XTOUCH_FILAMENT_OPTS_BUF_SIZE];
int xtouch_filament_num_brands = 0;
int xtouch_filament_current_brand_index = -1;
int xtouch_filament_current_type_count = 0;
int xtouch_filament_pipe_holds_brands = 0;
int xtouch_filament_use_fixed_brands = 1;

// グローバル関数の実際の定義
void xtouch_globals_init()
{
    controlMode.inc = 10;
    controlMode.axis = ControlAxisXY;
} 