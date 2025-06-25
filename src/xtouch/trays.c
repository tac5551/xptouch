#include "trays.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

char tray_types[max_ams][5][16];
uint16_t tray_temps[max_ams][5];
uint64_t tray_status[max_ams][5];

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

uint16_t get_tray_temp(uint8_t ams_id, uint8_t tray_id)
{
    return tray_temps[ams_id][tray_id + 1];
}
void set_tray_temp(uint8_t ams_id, uint8_t tray_id, uint16_t tray_temp)
{
    tray_temps[ams_id][tray_id] = tray_temp;
}

uint64_t get_tray_status(uint8_t ams_id, uint8_t tray_id)
{
    return tray_status[ams_id][tray_id];
}
void set_tray_status(uint8_t ams_id, uint8_t tray_id, uint64_t tray_state)
{
    tray_status[ams_id][tray_id] = tray_state;
}
