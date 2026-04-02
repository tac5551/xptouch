#include "trays.h"
#include "types.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

char tray_types[max_ams][5][16];
#define TRAY_COLOR_LEN 12
char tray_colors[max_ams][5][TRAY_COLOR_LEN];
uint16_t tray_temps[max_ams][5];
uint16_t tray_temp_min[max_ams][5];
uint16_t tray_temp_max[max_ams][5];
uint64_t tray_status[max_ams][5];
char tray_setting_id[max_ams][5][TRAY_SETTING_ID_LEN];

//ExTernal 
// AMS:0 Tray:0 user_id = 000

// AMS:0 Tray:1  user_id = 001
// AMS:0 Tray:2  user_id = 002
// AMS:0 Tray:3  user_id = 003
// AMS:0 Tray:4  user_id = 004

// AMS:1 Tray:1  user_id = 101
// AMS:1 Tray:2  user_id = 102
// AMS:1 Tray:3  user_id = 103
// AMS:1 Tray:4  user_id = 104

// AMS:2 Tray:1  user_id = 201
// AMS:2 Tray:2  user_id = 202
// AMS:2 Tray:3  user_id = 203
// AMS:2 Tray:4  user_id = 204

// AMS:3 Tray:0~3  user_id = 300~303

/** 統一: tray_id 254=External, 0-3=AMS slot。配列 [0]=254, [1]=slot0, [2]=slot1, [3]=slot2, [4]=slot3 */
#define TRAY_ID_EXTERNAL 254
static inline int tray_id_to_index(uint8_t tray_id)
{
    if (tray_id == TRAY_ID_EXTERNAL)
        return 0;
    if (tray_id <= 3)
        return (int)tray_id + 1;
    return -1;
}

char *get_tray_type(uint8_t ams_id, uint8_t tray_id)
{
    int i = tray_id_to_index(tray_id);
    if (ams_id >= max_ams || i < 0)
        return (char *)"";
    return tray_types[ams_id][i];
}

void set_tray_type(uint8_t ams_id, uint8_t tray_id, char *tray_type)
{
    int i = tray_id_to_index(tray_id);
    if (ams_id >= max_ams || i < 0 || !tray_type)
        return;
    strncpy(tray_types[ams_id][i], tray_type, 15);
    tray_types[ams_id][i][15] = '\0';
}

const char *get_tray_color(uint8_t ams_id, uint8_t tray_id)
{
    int i = tray_id_to_index(tray_id);
    if (ams_id >= max_ams || i < 0)
        return "";
    return tray_colors[ams_id][i];
}

void set_tray_color(uint8_t ams_id, uint8_t tray_id, const char *color)
{
    int i = tray_id_to_index(tray_id);
    if (ams_id >= max_ams || i < 0 || !color)
        return;
    strncpy(tray_colors[ams_id][i], color, TRAY_COLOR_LEN - 1);
    tray_colors[ams_id][i][TRAY_COLOR_LEN - 1] = '\0';
}

uint16_t get_tray_temp(uint8_t ams_id, uint8_t tray_id)
{
    int i = tray_id_to_index(tray_id);
    if (ams_id >= max_ams || i < 0)
        return 0;
    return tray_temps[ams_id][i];
}
void set_tray_temp(uint8_t ams_id, uint8_t tray_id, uint16_t tray_temp)
{
    int i = tray_id_to_index(tray_id);
    if (ams_id >= max_ams || i < 0)
        return;
    tray_temps[ams_id][i] = tray_temp;
}

uint16_t get_tray_temp_min(uint8_t ams_id, uint8_t tray_id)
{
    int i = tray_id_to_index(tray_id);
    if (ams_id >= max_ams || i < 0)
        return 0;
    return tray_temp_min[ams_id][i];
}
uint16_t get_tray_temp_max(uint8_t ams_id, uint8_t tray_id)
{
    int i = tray_id_to_index(tray_id);
    if (ams_id >= max_ams || i < 0)
        return 0;
    return tray_temp_max[ams_id][i];
}
void set_tray_temp_min_max(uint8_t ams_id, uint8_t tray_id, uint16_t t_min, uint16_t t_max)
{
    int i = tray_id_to_index(tray_id);
    if (ams_id >= max_ams || i < 0)
        return;
    tray_temp_min[ams_id][i] = t_min;
    tray_temp_max[ams_id][i] = t_max;
}

uint64_t get_tray_status(uint8_t ams_id, uint8_t tray_id)
{
    int i = tray_id_to_index(tray_id);
    if (ams_id >= max_ams || i < 0)
        return 0;
    return tray_status[ams_id][i];
}
void set_tray_status(uint8_t ams_id, uint8_t tray_id, uint64_t tray_state)
{
    int i = tray_id_to_index(tray_id);
    if (ams_id >= max_ams || i < 0)
        return;
    tray_status[ams_id][i] = tray_state;
}

/* setting_id: tray_id 0-3=AMS slot のみ（External は使わない） */
const char *get_tray_setting_id(uint8_t ams_id, uint8_t tray_id)
{
    if (ams_id >= max_ams || tray_id > 3)
        return "";
    return tray_setting_id[ams_id][tray_id + 1];
}

void set_tray_setting_id(uint8_t ams_id, uint8_t tray_id, const char *setting_id)
{
    if (ams_id >= max_ams || tray_id > 3 || !setting_id)
        return;
    strncpy(tray_setting_id[ams_id][tray_id + 1], setting_id, TRAY_SETTING_ID_LEN - 1);
    tray_setting_id[ams_id][tray_id + 1][TRAY_SETTING_ID_LEN - 1] = '\0';
}

#if defined(__XTOUCH_PLATFORM_S3__ )

static int xtouch_reprint_other_array_index(void)
{
    int dd = xtouch_history_reprint_printer_dd_slot;
    if (dd <= 0)
        return -1;
    int o = dd - 1;
    if (o < 0 || o >= xtouch_other_printer_count || o >= XTOUCH_OTHER_PRINTERS_MAX)
        return -1;
    return o;
}

long xtouch_reprint_ams_exist_bits(void)
{
    int o = xtouch_reprint_other_array_index();
    if (o < 0)
        return bambuStatus.ams_exist_bits;
    return xtouch_other_printer_tray_ams_exist_bits[o];
}

uint64_t get_tray_status_reprint(uint8_t ams_id, uint8_t tray_id)
{
    int o = xtouch_reprint_other_array_index();
    if (o < 0)
        return get_tray_status(ams_id, tray_id);
    if (tray_id == TRAY_ID_EXTERNAL)
        return 0;
    if (ams_id >= XTOUCH_BAMBU_AMS_UNITS || tray_id >= XTOUCH_BAMBU_AMS_SLOTS_PER_UNIT)
        return 0;
    return xtouch_other_printer_trays[o][ams_id][tray_id].tray_status;
}

char *get_tray_type_reprint(uint8_t ams_id, uint8_t tray_id)
{
    int o = xtouch_reprint_other_array_index();
    if (o < 0)
        return get_tray_type(ams_id, tray_id);
    if (tray_id == TRAY_ID_EXTERNAL)
        return (char *)"";
    if (ams_id >= XTOUCH_BAMBU_AMS_UNITS || tray_id >= XTOUCH_BAMBU_AMS_SLOTS_PER_UNIT)
        return (char *)"";
    return (char *)xtouch_other_printer_trays[o][ams_id][tray_id].tray_type;
}

const char *get_tray_color_reprint(uint8_t ams_id, uint8_t tray_id)
{
    int o = xtouch_reprint_other_array_index();
    if (o < 0)
        return get_tray_color(ams_id, tray_id);
    if (tray_id == TRAY_ID_EXTERNAL)
        return "808080FF";
    if (ams_id >= XTOUCH_BAMBU_AMS_UNITS || tray_id >= XTOUCH_BAMBU_AMS_SLOTS_PER_UNIT)
        return "808080FF";
    const char *c = xtouch_other_printer_trays[o][ams_id][tray_id].tray_color;
    if (c && c[0] && strlen(c) >= 6)
        return c;
    return "808080FF";
}

const char *get_tray_setting_id_reprint(uint8_t ams_id, uint8_t tray_id)
{
    int o = xtouch_reprint_other_array_index();
    if (o < 0)
        return get_tray_setting_id(ams_id, tray_id);
    if (tray_id == TRAY_ID_EXTERNAL)
        return "";
    if (ams_id >= XTOUCH_BAMBU_AMS_UNITS || tray_id > 3)
        return "";
    return xtouch_other_printer_trays[o][ams_id][tray_id].tray_setting_id;
}
#endif /* __XTOUCH_PLATFORM_S3__ */
