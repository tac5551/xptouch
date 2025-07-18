#ifndef _XLCD_BBL_ERRORS_OPTIMIZED
#define _XLCD_BBL_ERRORS_OPTIMIZED

#ifdef __cplusplus
extern "C"
{
#endif

#include <pgmspace.h>

typedef struct {
    const char* ecode;
    const char* pattern;
    const char* placeholders[8];
    int placeholder_count;
} error_pattern_t;

// Optimized error patterns
extern int hms_error_patterns_length;
extern const error_pattern_t hms_error_patterns[] PROGMEM;

extern int device_error_patterns_length;
extern const error_pattern_t device_error_patterns[] PROGMEM;

// Retry and Done messages
extern int message_containing_retry_total;
extern const char *message_containing_retry[] PROGMEM;

extern int message_containing_done_total;
extern const char *message_containing_done[] PROGMEM;

#ifdef __cplusplus
}
#endif

#endif