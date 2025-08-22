#ifndef _XLCD_BBL_ERRORS_OLD
#define _XLCD_BBL_ERRORS_OLD

#ifdef __cplusplus
extern "C"
{
#endif

#include <pgmspace.h>

// Old HMS error arrays (for comparison)
extern int hms_error_length;
extern const char *hms_error_keys[] PROGMEM;
extern const char *hms_error_values[] PROGMEM;

// Old Device error arrays (for comparison)
extern int device_error_length;
extern const char *device_error_keys[] PROGMEM;
extern const char *device_error_values[] PROGMEM;

// Retry and Done messages
extern int message_containing_retry_total;
extern const char *message_containing_retry[] PROGMEM;

extern int message_containing_done_total;
extern const char *message_containing_done[] PROGMEM;

#ifdef __cplusplus
}
#endif

#endif