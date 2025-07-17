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

extern int hms_error_patterns_length;
extern const error_pattern_t hms_error_patterns[] PROGMEM;

#ifdef __cplusplus
}
#endif

#endif