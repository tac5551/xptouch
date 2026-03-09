#include "ams_edit_temp.h"
#include <string.h>
#include <stdio.h>
#include <Arduino.h>

char ams_edit_fetched_setting_id[16] = {0};
int ams_edit_fetched_min = 0;
int ams_edit_fetched_max = 0;
char ams_edit_fetched_filament_id[16] = {0};
int ams_edit_current_ams_id = 0;
int ams_edit_current_tray_id = 1;
/* フォーマット: RRGGBBAA（AA は FF 固定） */
char ams_edit_current_tray_color[12] = "00000000";
int ams_edit_current_brand_index = -1;
int ams_edit_current_type_index = -1;

void ams_edit_set_editing_slot(int ams_id, int tray_id)
{
    ams_edit_current_ams_id = ams_id;   /* 0-3=AMS1..4, 255=External */
    ams_edit_current_tray_id = tray_id; /* 0-3=スロット, 254=External */
    ams_edit_current_brand_index = -1;  /* 次回ロードでトレイ種別から設定 */
    ams_edit_current_type_index = -1;
}

void ams_edit_set_tray_color(const char *hex8)
{
    if (!hex8 || hex8[0] == '\0')
    {
        snprintf(ams_edit_current_tray_color, sizeof(ams_edit_current_tray_color), "00000000");
        return;
    }
    size_t len = strlen(hex8);
    if (len >= 8)
        snprintf(ams_edit_current_tray_color, sizeof(ams_edit_current_tray_color), "%.8s", hex8);
    else if (len >= 6)
        snprintf(ams_edit_current_tray_color, sizeof(ams_edit_current_tray_color), "%.6sFF", hex8);
    else
        snprintf(ams_edit_current_tray_color, sizeof(ams_edit_current_tray_color), "00000000");
}

void ams_edit_set_fetched_temps(const char *id, int min_val, int max_val, const char *filament_id)
{
    if (id)
    {
        size_t n = 15;
        strncpy(ams_edit_fetched_setting_id, id, n);
        ams_edit_fetched_setting_id[n] = '\0';
    }
    else
        ams_edit_fetched_setting_id[0] = '\0';
    ams_edit_fetched_min = min_val;
    ams_edit_fetched_max = max_val;
    if (filament_id && filament_id[0] != '\0')
    {
        strncpy(ams_edit_fetched_filament_id, filament_id, 15);
        ams_edit_fetched_filament_id[15] = '\0';
    }
    else
        ams_edit_fetched_filament_id[0] = '\0';
}

void xtouch_debug_log_ams_save(const char *id_buf, const char *fetched_id, int id_match, int fetched_min, int fetched_max, int payload_min, int payload_max)
{
    printf("[AMS Edit Save] id_buf=%s fetched_id=%s id_match=%d fetched_min=%d fetched_max=%d -> payload_min=%d payload_max=%d\n",
        id_buf, fetched_id, id_match, fetched_min, fetched_max, payload_min, payload_max);
}
