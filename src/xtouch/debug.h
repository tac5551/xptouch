#ifndef _XLCD_DEBUG
#define _XLCD_DEBUG

#include <Arduino.h>

/* CORE_DEBUG_LEVEL に応じて ERROR / INFO / DEBUG を段階的に有効化（Arduino/ESP32 の 0=NONE, 1=ERROR, 3=INFO, 4=DEBUG, 5=VERBOSE に合わせる） */
#if defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL >= 1
#define XTOUCH_DEBUG_ERROR true
#else
#define XTOUCH_DEBUG_ERROR false
#endif
#if defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL >= 3
#define XTOUCH_DEBUG_INFO  true
#else
#define XTOUCH_DEBUG_INFO  false
#endif
#if defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL >= 4
#define XTOUCH_DEBUG_DEBUG true
#else
#define XTOUCH_DEBUG_DEBUG false
#endif
/* VERBOSE 層（Arduino/ESP32 の CORE_DEBUG_LEVEL 5 相当） */
#if defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL >= 5
#define XTOUCH_DEBUG_VERBOSE true
#else
#define XTOUCH_DEBUG_VERBOSE false
#endif
/* DETAIL 層（Arduino/ESP32 の CORE_DEBUG_LEVEL 6 相当） */
#if defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL >= 6
#define XTOUCH_DEBUG_DETAIL true
#else
#define XTOUCH_DEBUG_DETAIL false
#endif

#ifdef __cplusplus

#define ConsoleInfo   if (XTOUCH_DEBUG_INFO || XTOUCH_DEBUG_DEBUG || XTOUCH_DEBUG_VERBOSE) Serial
#define ConsoleDebug  if (XTOUCH_DEBUG_DEBUG || XTOUCH_DEBUG_VERBOSE) Serial
#define ConsoleVerbose if (XTOUCH_DEBUG_VERBOSE) Serial

#define ConsoleError  if (XTOUCH_DEBUG_ERROR) Serial
#define ConsoleDetail if (XTOUCH_DEBUG_DETAIL) Serial

#include <ArduinoJson.h>

static inline void xtouch_debug_json(const JsonDocument &doc)
{
#if XTOUCH_DEBUG_DETAIL == true
    String output;
    serializeJsonPretty(doc, output);
    ConsoleDebug.println(output);
#endif
}
#else
/* .c では Serial / F() / Print チェーンが使えないため printf に寄せる */
#include <stdio.h>
#define ConsoleVerbose_Printf(...)                                                                 \
    do                                                                                             \
    {                                                                                              \
        if (XTOUCH_DEBUG_VERBOSE)                                                                  \
            printf(__VA_ARGS__);                                                                   \
    } while (0)
#define ConsoleDetail_Printf(...)                                                                  \
    do                                                                                              \
    {                                                                                               \
        if (XTOUCH_DEBUG_DETAIL)                                                                  \
            printf(__VA_ARGS__);                                                                   \
    } while (0)
#define ConsoleInfo_Printf(...)                                                                    \
    do                                                                                             \
    {                                                                                              \
        if (XTOUCH_DEBUG_INFO || XTOUCH_DEBUG_DEBUG || XTOUCH_DEBUG_VERBOSE)                           \
            printf(__VA_ARGS__);                                                                   \
    } while (0)
#define ConsoleDebug_Printf(...)                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (XTOUCH_DEBUG_DEBUG || XTOUCH_DEBUG_VERBOSE)                                                \
            printf(__VA_ARGS__);                                                                   \
    } while (0)
#define ConsoleError_Printf(...)                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (XTOUCH_DEBUG_ERROR)                                                                    \
            printf(__VA_ARGS__);                                                                   \
    } while (0)

#endif /* __cplusplus */

#endif
