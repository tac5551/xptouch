
#include <stdint.h>
#include "../ui/ui.h"

#ifndef _TRAYS_INCLUDE_GUARD
#define _TRAYS_INCLUDE_GUARD
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#define max_ams 8

    char *get_tray_type(uint8_t ams_id, uint8_t tray_id);

    void set_tray_type(uint8_t ams_id, uint8_t tray_id, char *tray_type);

    uint16_t get_tray_temp(uint8_t ams_id, uint8_t tray_id);

    void set_tray_temp(uint8_t ams_id, uint8_t tray_id, uint16_t tray_temp);

    uint64_t get_tray_status(uint8_t ams_id, uint8_t tray_id);

    void set_tray_status(uint8_t ams_id, uint8_t tray_id, uint64_t tray_state);

#ifdef __cplusplus
} /*extern "C"*/
#endif


