#ifndef _XLCD_AMS
#define _XLCD_AMS

#ifdef __cplusplus
extern "C"
{
#endif

#include <Arduino.h>
#include "xtouch/bbl/bbl-errors.h"
#include <pgmspace.h>
    // Function to retrieve a value by key
    extern void xptouch_ams_parse_tray_now(const char *tray_now);
    extern void xptouch_ams_parse_status(int ams_status);
    extern bool xptouch_has_ams();
    extern bool xptouch_can_load_filament();
    extern bool xptouch_can_unload_filament();

#ifdef __cplusplus
}
#endif

#endif