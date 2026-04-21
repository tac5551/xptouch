/* Platform macro: auto-detect ESP32-S3 target */
#if (defined(CONFIG_IDF_TARGET_ESP32S3) || defined(__XTOUCH_SCREEN_S3_028__) || defined(__XTOUCH_SCREEN_S3_050__) || defined(__XTOUCH_SCREEN_S3_3248__)) && !defined(__XTOUCH_PLATFORM_S3__)
#define __XTOUCH_PLATFORM_S3__
#endif

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


#if defined(__XTOUCH_SCREEN_S3_050__) 
#include "devices/5.0/screen.h"
#elif defined(__XTOUCH_SCREEN_S3_3248__)
#include "devices/s3_3248w535/screen.h"
#elif defined(__XTOUCH_SCREEN_S3_028__)
#include "devices/s3_2.8/screen.h"
#else
#include "devices/2.8/screen.h"
#endif

#include "xtouch/cloud.hpp"
#include "xtouch/neopixel.h"
#include "xtouch/led.h"
#include "xtouch/settings.h"
#include "xtouch/net.h"
#include "xtouch/firmware.h"
#include "xtouch/mqtt.h"
#if defined(__XTOUCH_PLATFORM_S3__)
#include "xtouch/thumbnail.h"
#include "xtouch/history.h"
#include "xtouch/p1s_video.h"
#include "xtouch/lv_fs_arduino_sd.h"
#include "xtouch/lcd_json.h"
#endif
#include "xtouch/sensors/chamber.h"
#include "xtouch/events.h"
#include "xtouch/connection.h"
#include "xtouch/coldboot.h"
#include "xtouch/webserver.h"
#include "xtouch/globals.h"
#include "xtouch/filaments.h"
#include "xtouch/ams_edit_temp.h"
#include "xtouch/filaments_rev.h"
#include "ui/ui_events.h"



// #if defined(__XTOUCH_SCREEN_28__)
// #include "xtouch/m5Stack.h"
// #endif

void xtouch_intro_show(void)
{
  ui_introScreen_screen_init();
  lv_disp_load_scr(introScreen);
  lv_timer_handler();
}

void setup()
{
  Serial.begin(115200);
  /* platformio で -DXTOUCH_BOOT_SERIAL_DELAY_MS=... を付けたときだけ（主に S3 USB CDC 再接続用） */
#if defined(XTOUCH_BOOT_SERIAL_DELAY_MS) && (XTOUCH_BOOT_SERIAL_DELAY_MS > 0)
  delay(XTOUCH_BOOT_SERIAL_DELAY_MS);
#endif
#if defined(__XTOUCH_SCREEN_S3_3248__)
  Serial.println("\r\n[xPTouch] boot JC3248 (USB CDC)");
#endif
#if defined(XTOUCH_DEBUG) || XTOUCH_USE_SERIAL == true || XTOUCH_DEBUG_ERROR == true || XTOUCH_DEBUG_DEBUG == true || XTOUCH_DEBUG_INFO == true|| XTOUCH_DEBUG_VERBOSE == true
  Serial.begin(115200);
  ConsoleDebug.println("Serial started");
#endif

  xtouch_eeprom_setup();
#if defined(__XTOUCH_SCREEN_S3_050__)
  Serial.println("[xPTouch] enter xtouch_eeprom_rgb_pclk_heal_invalid_storage");
  xtouch_eeprom_rgb_pclk_heal_invalid_storage();
#endif
  xtouch_globals_init();
#if defined(__XTOUCH_SCREEN_S3_3248__)
  Serial.println("[xPTouch] enter xtouch_screen_setup (LCD+LVGL)");
#endif

  xtouch_screen_setup();
#if defined(__XTOUCH_SCREEN_S3_3248__)
  Serial.println("[xPTouch] leave xtouch_screen_setup OK");
#endif

#ifdef __XTOUCH_SCREEN_S3_050__
  lv_font_small_set(&lv_font_montserrat_28);
  lv_font_middle_set(&lv_font_montserrat_32);
  lv_font_big_set(&lv_font_montserrat_48);
  lv_icon_font_small_set(&ui_font_xlcd48);
#elif defined(__XTOUCH_SCREEN_S3_3248__)
  /* 3.5": TEMP / Control 等のアイコンは xlcd（2.8" の min より大きいグリッド） */
  lv_font_small_set(&lv_font_montserrat_14);
  lv_font_middle_set(&lv_font_montserrat_24);
  lv_font_big_set(&lv_font_montserrat_28);
  lv_icon_font_small_set(&ui_font_xlcd);
#elif defined(__XTOUCH_PLATFORM_S3__)
  /* S3 2.8" 等（5" / 3.5" 以外） */
  lv_font_small_set(&lv_font_montserrat_14);
  lv_font_middle_set(&lv_font_montserrat_24);
  lv_font_big_set(&lv_font_montserrat_28);
  lv_icon_font_small_set(&ui_font_xlcdmin);
#else
  lgfx::boards::board_t board = tft.getBoard();

  if (board == lgfx::boards::board_t::board_Sunton_ESP32_3248S035C ||
      board == lgfx::boards::board_t::board_Sunton_ESP32_3248S035R)
  {
    ConsoleDebug.println("Found Sunton_ESP32_3248S035C or Sunton_ESP32_3248S035R");
    lv_font_small_set(&lv_font_montserrat_24);
    lv_font_middle_set(&lv_font_montserrat_28);
    lv_font_big_set(&lv_font_montserrat_32);
    lv_icon_font_small_set(&ui_font_xlcd);
  }
#endif
  xtouch_intro_show();
#if defined(__XTOUCH_SCREEN_S3_3248__)
  for (int i = 0; i < 80; i++)
  {
    lv_timer_handler();
    if ((i % 16) == 0)
      delay(2);
  }
#endif
  int8_t sd_cs_pin = -1;
  bool sd_mode_1bit = true;
#if defined(__XTOUCH_SCREEN_S3_028__)
  sd_mode_1bit = false;
#else
#if defined(__XTOUCH_SCREEN_S3_050__)
  sd_cs_pin = 10; /* TF CS。SCK/MOSI/MISO は devices/5.0/sd_spi_pins.h */
#elif defined(__XTOUCH_SCREEN_28__)
  if (tft.getBoard() == (lgfx::boards::board_t)2)
    sd_cs_pin = 4;
  else
    sd_cs_pin = 5;
  sd_mode_1bit = true;
#endif
#endif
  while (!xtouch_sdcard_setup(sd_cs_pin, sd_mode_1bit))
    ;

#if defined(__XTOUCH_PLATFORM_S3__)
  xtouch_lcd_json_apply_from_sd_and_reboot();
  lv_fs_arduino_sd_init(); 
  xtouch_thumbnail_subscribe_events();
  xtouch_history_subscribe_events();
  xtouch_p1s_video_subscribe_events();
#endif

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
    if(xtouch_filesystem_exist(xtouch_sdcard_fs(), xtouch_paths_provisioning)){
        ConsoleDebug.println("Provisioning mode initialize");
        xTouchConfig.xTouchProvisioningMode = true;
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
          xtouch_cloud_mqtt_setup();
        }

    }else if(xtouch_filesystem_exist(xtouch_sdcard_fs(), xtouch_paths_config)){
        ConsoleDebug.println("Lan only mode initialize");
        xTouchConfig.xTouchLanOnlyMode = true;
        xtouch_local_mqtt_setup();
    }


#if defined(__XTOUCH_SCREEN_28__)
  ConsoleDebug.println("XTOUCH_NEO_PIXEL_INIT");
  if (board == lgfx::boards::board_t::board_Guition_ESP32_2432W328R || 
      board == lgfx::boards::board_t::board_Guition_ESP32_2432W328C || 
      board == lgfx::boards::board_t::board_Sunton_ESP32_3248S035C ||
      board == lgfx::boards::board_t::board_Sunton_ESP32_3248S035R)
  {
    ConsoleDebug.println("LED 21");
    xTouchConfig.xTouchNeoPixelPinValue = 21;
  }
  else if (board == lgfx::boards::board_t::board_Sunton_ESP32_2432S028_7789 ||
           board == lgfx::boards::board_t::board_Sunton_ESP32_2432S028_9341 )
  {
    ConsoleDebug.println("LED 27");
    xTouchConfig.xTouchNeoPixelPinValue = 27;
  }
#elif defined(__XTOUCH_SCREEN_S3_050__)
  ConsoleDebug.println("LED 17");
  xTouchConfig.xTouchNeoPixelPinValue = 17;
#elif defined(__XTOUCH_SCREEN_S3_3248__)
  ConsoleDebug.println("LED 17");
  xTouchConfig.xTouchNeoPixelPinValue = 17;
#elif defined(__XTOUCH_PLATFORM_S3__)
  /* 2432S028（LGFX）: FT5x06 の INT=GPIO17。5" と同じ 17 を Neo に使うとタッチと競合する。
   * ピンは printer.json の neoPixelPin のみ（未設定 0 のときは Neo タイマーは起動しない）。 */
  ConsoleDebug.println("[NeoPixel] S3 (non-5\"): use neoPixelPin from settings (not GPIO17)");
#endif

  xtouch_neo_pixel_timer_init(xTouchConfig.xTouchNeoPixelPinValue);
  xtouch_chamber_timer_init();
  xtouch_screen_startScreenTimer();

// #if defined(__XTOUCH_SCREEN_28__)
//   xtouch_m5stack_buttons_setup((int)tft.getBoard());
// #endif
}

#ifdef XTOUCH_DEBUG_VERBOSE
static uint32_t s_last_heap_log_ms = 0;
#define HEAP_LOG_INTERVAL_MS 5000
#endif

void loop()
{
  lv_timer_handler();
  lv_task_handler();
  /* Video画面(17)では負荷軽減のため MQTT ループを止める */
  if ((xTouchConfig.xTouchLanOnlyMode || cloud.loggedIn) && xTouchConfig.currentScreenIndex != 17)
    xtouch_cloud_mqtt_loop();

  if (xtouch_ota_update_flag)
  {
    xtouch_ota_update_flag = false;
    lv_msg_send(XTOUCH_SETTINGS_OTA_UPDATE_NOW, NULL);
  }
  
  // キャラクターアニメーション処理（millis()ベース、タイマー不要）
  xtouch_events_onCharacterAnimation();

// #if defined(__XTOUCH_SCREEN_28__)
//   xtouch_m5stack_buttons_loop();
// #endif

#ifdef XTOUCH_DEBUG_VERBOSE
  if (millis() - s_last_heap_log_ms >= HEAP_LOG_INTERVAL_MS)
  {
    s_last_heap_log_ms = millis();
    ConsoleVerbose.printf("[xPTouch][V][Heap] free=%u max_alloc=%u\n", (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getMaxAllocHeap());
  }
#endif

  delay(10);
}
