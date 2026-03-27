
#include <stdint.h>
#include "../ui/ui.h"

#ifndef _TRAYS_INCLUDE_GUARD
#define _TRAYS_INCLUDE_GUARD
#endif

/** 統一: tray_id 254=External, 0-3=AMS slot */
#define TRAY_ID_EXTERNAL 254

#ifdef __cplusplus
extern "C"
{
#endif
#define max_ams 8
#define TRAY_SETTING_ID_LEN 16

    char *get_tray_type(uint8_t ams_id, uint8_t tray_id);

    void set_tray_type(uint8_t ams_id, uint8_t tray_id, char *tray_type);

    const char *get_tray_color(uint8_t ams_id, uint8_t tray_id);
    void set_tray_color(uint8_t ams_id, uint8_t tray_id, const char *color);

    uint16_t get_tray_temp(uint8_t ams_id, uint8_t tray_id);

    void set_tray_temp(uint8_t ams_id, uint8_t tray_id, uint16_t tray_temp);

    uint16_t get_tray_temp_min(uint8_t ams_id, uint8_t tray_id);
    uint16_t get_tray_temp_max(uint8_t ams_id, uint8_t tray_id);
    void set_tray_temp_min_max(uint8_t ams_id, uint8_t tray_id, uint16_t t_min, uint16_t t_max);

    uint64_t get_tray_status(uint8_t ams_id, uint8_t tray_id);

    void set_tray_status(uint8_t ams_id, uint8_t tray_id, uint64_t tray_state);

    /** MQTT で取得した setting_id（tray_info_idx）。tray_id は 0=0番トレイ。空の場合は ""。 */
    const char *get_tray_setting_id(uint8_t ams_id, uint8_t tray_id);
    void set_tray_setting_id(uint8_t ams_id, uint8_t tray_id, const char *setting_id);

#if defined(__XTOUCH_SCREEN_50__)
    /** Reprint: xtouch_history_reprint_printer_dd_slot（0=自機）に応じ bambuStatus または xtouch_other_printer_trays を参照 */
    long xtouch_reprint_ams_exist_bits(void);
    uint64_t get_tray_status_reprint(uint8_t ams_id, uint8_t tray_id);
    char *get_tray_type_reprint(uint8_t ams_id, uint8_t tray_id);
    const char *get_tray_color_reprint(uint8_t ams_id, uint8_t tray_id);
    const char *get_tray_setting_id_reprint(uint8_t ams_id, uint8_t tray_id);
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif


