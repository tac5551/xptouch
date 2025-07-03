#include <stdbool.h>
#include "globals.h"
#include "types.h"

// グローバル変数の実際の定義
bool xtouch_mqtt_light_on = false;
bool xtouch_ota_update_flag = false;
// グローバル関数の実際の定義
void xtouch_globals_init()
{
    controlMode.inc = 10;
    controlMode.axis = ControlAxisXY;
} 