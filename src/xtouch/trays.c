#include "trays.h"
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

// AMS:3 Tray:1  user_id = 301
// AMS:3 Tray:2  user_id = 302
// AMS:3 Tray:3  user_id = 303
// AMS:3 Tray:4  user_id = 304

char *get_tray_type(uint8_t ams_id, uint8_t tray_id)
{
    return tray_types[ams_id][tray_id + 1];
}

void set_tray_type(uint8_t ams_id, uint8_t tray_id, char *tray_type)
{
    strcpy(tray_types[ams_id][tray_id + 1], tray_type);
}

const char *get_tray_color(uint8_t ams_id, uint8_t tray_id)
{
    if (ams_id >= max_ams || tray_id > 3)
        return "";
    return tray_colors[ams_id][tray_id + 1];
}

void set_tray_color(uint8_t ams_id, uint8_t tray_id, const char *color)
{
    if (ams_id >= max_ams || tray_id > 3 || !color)
        return;
    strncpy(tray_colors[ams_id][tray_id + 1], color, TRAY_COLOR_LEN - 1);
    tray_colors[ams_id][tray_id + 1][TRAY_COLOR_LEN - 1] = '\0';
}

uint16_t get_tray_temp(uint8_t ams_id, uint8_t tray_id)
{
    return tray_temps[ams_id][tray_id + 1];
}
void set_tray_temp(uint8_t ams_id, uint8_t tray_id, uint16_t tray_temp)
{
    tray_temps[ams_id][tray_id] = tray_temp;
}

uint16_t get_tray_temp_min(uint8_t ams_id, uint8_t tray_id)
{
    return tray_temp_min[ams_id][tray_id + 1];
}
uint16_t get_tray_temp_max(uint8_t ams_id, uint8_t tray_id)
{
    return tray_temp_max[ams_id][tray_id + 1];
}
void set_tray_temp_min_max(uint8_t ams_id, uint8_t tray_id, uint16_t t_min, uint16_t t_max)
{
    tray_temp_min[ams_id][tray_id] = t_min;
    tray_temp_max[ams_id][tray_id] = t_max;
}

uint64_t get_tray_status(uint8_t ams_id, uint8_t tray_id)
{
    return tray_status[ams_id][tray_id];
}
void set_tray_status(uint8_t ams_id, uint8_t tray_id, uint64_t tray_state)
{
    tray_status[ams_id][tray_id] = tray_state;
}

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
