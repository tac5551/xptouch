#include <driver/i2s.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "xtouch/debug.h"
#include "xtouch/paths.h"
#include "xtouch/eeprom.h"
#include "xtouch/types.h"
#include "xtouch/bblp.h"
#include "xtouch/globals.h"
#include "xtouch/filesystem.h"
#include "ui/ui.h"
#include "xtouch/sdcard.h"
#include "xtouch/hms.h"

#if defined(__XTOUCH_SCREEN_28__)
#include "devices/2.8/screen.h"
#endif

#include "xtouch/cloud.hpp"
#include "xtouch/settings.h"
#include "xtouch/net.h"
#include "xtouch/firmware.h"
#include "xtouch/mqtt.h"
#include "xtouch/sensors/chamber.h"
#include "xtouch/events.h"
#include "xtouch/connection.h"
#include "xtouch/coldboot.h"
#include "xtouch/webserver.h"

void xtouch_intro_show(void)
{
  ui_introScreen_screen_init();
  lv_disp_load_scr(introScreen);
  lv_timer_handler();
}

void setup()
{
#if XTOUCH_USE_SERIAL == true || XTOUCH_DEBUG_ERROR == true || XTOUCH_DEBUG_DEBUG == true || XTOUCH_DEBUG_INFO == true
  Serial.begin(115200);
#endif

  xtouch_eeprom_setup();
  xtouch_globals_init();
  xtouch_screen_setup();
  xtouch_intro_show();
  while (!xtouch_sdcard_setup())
    ;

  xtouch_coldboot_check();

  xtouch_settings_loadSettings();

  xtouch_touch_setup();

  while (!xtouch_wifi_setup())
    ;

  xtouch_screen_setupScreenTimer();
  xtouch_setupGlobalEvents();
  xtouch_webserver_begin();

  if (!cloud.hasAuthTokens())
  {
    String gotoCode = "Provision at " + WiFi.localIP().toString();
    lv_label_set_text(introScreenCaption, gotoCode.c_str());
    lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0x00FF00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_timer_handler();
  }
  else
  {
    cloud.loadAuthTokens();
    if (!cloud.isPaired())
    {
      cloud.selectPrinter();
    }
    else
    {
      cloud.loadPair();
    }
    Serial.println("xtouch_mqtt_setup ...");
    xtouch_mqtt_setup();
  }
  Serial.println("xtouch_chamber_timer_init ...");
  xtouch_chamber_timer_init();
}

void loop()
{
  lv_timer_handler();
  lv_task_handler();
  if (cloud.loggedIn)
    xtouch_mqtt_loop();
  delay(10);
}
