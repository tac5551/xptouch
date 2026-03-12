#ifndef _XLCD_DEBUG
#define _XLCD_DEBUG

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
#if defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL >= 5
#define XTOUCH_DEBUG_LOG   true
#else
#define XTOUCH_DEBUG_LOG   false
#endif

#define ConsoleLog    if (XTOUCH_DEBUG_LOG) Serial
#define ConsoleInfo   if (XTOUCH_DEBUG_INFO || XTOUCH_DEBUG_DEBUG || XTOUCH_DEBUG_LOG) Serial
#define ConsoleDebug  if (XTOUCH_DEBUG_DEBUG || XTOUCH_DEBUG_LOG) Serial
#define ConsoleError  if (XTOUCH_DEBUG_ERROR) Serial

#include <Arduino.h>
#include <ArduinoJson.h>

void xtouch_debug_json(const JsonDocument &doc)
{
#if XTOUCH_DEBUG_DEBUG == true
    String output;
    serializeJsonPretty(doc, output);
    ConsoleDebug.println(output);
#endif
}

#endif
