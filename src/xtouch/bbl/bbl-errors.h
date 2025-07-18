#ifndef _XLCD_BBL_ERRORS
#define _XLCD_BBL_ERRORS

#ifdef __cplusplus
extern "C"
{
#endif

#include <pgmspace.h>

// Legacy HMS error arrays
extern int hms_error_length;
extern const char *hms_error_keys[] PROGMEM;
extern const char *hms_error_values[] PROGMEM;

// Legacy Device error arrays
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