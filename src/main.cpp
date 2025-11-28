#include <driver/i2s.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "xtouch/globals.h"
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

#if defined(__XTOUCH_SCREEN_50__)
#include "devices/5.0/screen.h"
#endif

#include "xtouch/cloud.hpp"
#include "xtouch/neopixel.h"
#include "xtouch/led.h"
#include "xtouch/settings.h"
#include "xtouch/net.h"
#include "xtouch/firmware.h"
#include "xtouch/mqtt.h"
#include "xtouch/sensors/chamber.h"
#include "xtouch/events.h"
#include "xtouch/connection.h"
#include "xtouch/coldboot.h"
#include "xtouch/webserver.h"
#include "xtouch/globals.h"


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

  xtouch_firmware_checkFirmwareUpdate();

  xtouch_touch_setup();

  while (!xtouch_wifi_setup())
    ;

  xtouch_firmware_checkOnlineFirmwareUpdate();

  xtouch_screen_setupScreenTimer();
  xtouch_screen_setupLEDOffTimer();
  xtouch_setupGlobalEvents();


  if (!cloud.hasAuthTokens())
  {
    xtouch_webserver_begin();
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

#if defined(__XTOUCH_SCREEN_28__)
  lgfx::boards::board_t board = tft.getBoard();
  if (board == lgfx::boards::board_t::board_ESP32_ESP32E)
      return;

  ConsoleDebug.println("XTOUCH_NEO_PIXEL_INIT");
  if (board == lgfx::boards::board_t::board_Guition_ESP32_2432W328R || board == lgfx::boards::board_t::board_Guition_ESP32_2432W328C)
  {
      xTouchConfig.xTouchNeoPixelPinValue = 21;
  }
  else if (board == lgfx::boards::board_t::board_Sunton_ESP32_2432S028_7789 || board == lgfx::boards::board_t::board_Sunton_ESP32_2432S028_9341)
  {
      xTouchConfig.xTouchNeoPixelPinValue = 27;
  }
#elif defined(__XTOUCH_SCREEN_50__)
  xTouchConfig.xTouchNeoPixelPinValue = 17;
#endif

  xtouch_neo_pixel_timer_init(xTouchConfig.xTouchNeoPixelPinValue);
  
  xtouch_screen_startScreenTimer();
}

void loop()
{
  lv_timer_handler();
  lv_task_handler();
  if (cloud.loggedIn)
    xtouch_mqtt_loop();
  if (xtouch_ota_update_flag)
  {
    xtouch_ota_update_flag = false;
    lv_msg_send(XTOUCH_SETTINGS_OTA_UPDATE_NOW, NULL);
  }
  
  // キャラクターアニメーション処理（millis()ベース、タイマー不要）
  xtouch_events_onCharacterAnimation();
  
  delay(10);
}
