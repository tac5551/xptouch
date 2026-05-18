/* Platform macro: auto-detect ESP32-S3 target */
#if (defined(CONFIG_IDF_TARGET_ESP32S3) || defined(__XPTOUCH_SCREEN_S3_028__) || defined(__XPTOUCH_SCREEN_S3_050__) || defined(__XPTOUCH_SCREEN_S3_3248__)) && !defined(__XPTOUCH_PLATFORM_S3__)
#define __XPTOUCH_PLATFORM_S3__
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


#if defined(__XPTOUCH_SCREEN_S3_050__) 
#include "devices/5.0/screen.h"
#elif defined(__XPTOUCH_SCREEN_S3_3248__)
#include "devices/s3_3248w535/screen.h"
#elif defined(__XPTOUCH_SCREEN_S3_028__)
#include "devices/s3_2.8/screen.h"
#else
#include "devices/2.8/screen.h"
#endif

#if defined(__XPTOUCH_PLATFORM_S3__)
extern "C" void xptouch_screen_led_off_timer_pause_for_camera_c(void)
{
  xptouch_screen_stopLEDOffTimer();
}

extern "C" void xptouch_screen_led_off_timer_resume_c(void)
{
  xptouch_screen_startLEDOffTimer();
}
#endif

#include "xtouch/cloud.hpp"
#include "xtouch/neopixel.h"
#include "xtouch/led.h"
#include "xtouch/settings.h"
#include "xtouch/net.h"
#include "xtouch/firmware.h"
#include "xtouch/mqtt.h"
#if defined(__XPTOUCH_PLATFORM_S3__)
#include "xtouch/thumbnail.h"
#include "xtouch/history.h"
#include "xtouch/lv_fs_arduino_sd.h"
#include "xtouch/lcd_json.h"
#include "xtouch/camera_stream.h"
#include <algorithm>
#endif
#include "xtouch/sensors/chamber.h"
#include "xtouch/events.h"
#include "xtouch/connection.h"
#include "xtouch/demo.h"
#include "xtouch/coldboot.h"
#include "xtouch/webserver.h"
#include "xtouch/globals.h"
#include "xtouch/filaments.h"
#include "xtouch/ams_edit_temp.h"
#include "xtouch/filaments_rev.h"
#include "ui/ui_events.h"



// #if defined(__XPTOUCH_SCREEN_28__)
// #include "xtouch/m5Stack.h"
// #endif

void xptouch_intro_show(void)
{
  ui_introScreen_screen_init();
  lv_disp_load_scr(introScreen);
  lv_timer_handler();
}

void setup()
{
  Serial.begin(115200);
  /* platformio で -DXPTOUCH_BOOT_SERIAL_DELAY_MS=... を付けたときだけ（主に S3 USB CDC 再接続用） */
#if defined(XPTOUCH_BOOT_SERIAL_DELAY_MS) && (XPTOUCH_BOOT_SERIAL_DELAY_MS > 0)
  delay(XPTOUCH_BOOT_SERIAL_DELAY_MS);
#endif
#if defined(__XPTOUCH_SCREEN_S3_3248__)
  Serial.println("\r\n[xPTouch] boot JC3248 (USB CDC)");
#endif
#if defined(XPTOUCH_DEBUG) || XPTOUCH_USE_SERIAL == true || XPTOUCH_DEBUG_ERROR == true || XPTOUCH_DEBUG_DEBUG == true || XPTOUCH_DEBUG_INFO == true|| XPTOUCH_DEBUG_VERBOSE == true
  Serial.begin(115200);
  ConsoleDebug.println("Serial started");
#endif

  xptouch_eeprom_setup();
#if defined(__XPTOUCH_SCREEN_S3_050__)
  Serial.println("[xPTouch] enter xptouch_eeprom_rgb_pclk_heal_invalid_storage");
  xptouch_eeprom_rgb_pclk_heal_invalid_storage();
#endif
  xptouch_globals_init();
#if defined(__XPTOUCH_SCREEN_S3_3248__)
  Serial.println("[xPTouch] enter xptouch_screen_setup (LCD+LVGL)");
#endif

  xptouch_screen_setup();
#if defined(__XPTOUCH_SCREEN_S3_3248__)
  Serial.println("[xPTouch] leave xptouch_screen_setup OK");
#endif

#ifdef __XPTOUCH_SCREEN_S3_050__
  lv_font_small_set(&lv_font_montserrat_28);
  lv_font_middle_set(&lv_font_montserrat_32);
  lv_font_big_set(&lv_font_montserrat_48);
  lv_icon_font_small_set(&ui_font_xlcd48);
#elif defined(__XPTOUCH_SCREEN_S3_3248__)
  /* 3.5": TEMP / Control 等のアイコンは xlcd（2.8" の min より大きいグリッド） */
  lv_font_small_set(&lv_font_montserrat_14);
  lv_font_middle_set(&lv_font_montserrat_24);
  lv_font_big_set(&lv_font_montserrat_28);
  lv_icon_font_small_set(&ui_font_xlcd);
#elif defined(__XPTOUCH_PLATFORM_S3__)
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
  xptouch_intro_show();
#if defined(__XPTOUCH_SCREEN_S3_3248__)
  for (int i = 0; i < 80; i++)
  {
    lv_timer_handler();
    if ((i % 16) == 0)
      delay(2);
  }
#endif
  int8_t sd_cs_pin = -1;
  bool sd_mode_1bit = true;
#if defined(__XPTOUCH_SCREEN_S3_028__)
  sd_mode_1bit = false;
#else
#if defined(__XPTOUCH_SCREEN_S3_050__)
  sd_cs_pin = 10; /* TF CS。SCK/MOSI/MISO は devices/5.0/sd_spi_pins.h */
#elif defined(__XPTOUCH_SCREEN_28__)
  if (tft.getBoard() == (lgfx::boards::board_t)2)
    sd_cs_pin = 4;
  else
    sd_cs_pin = 5;
  sd_mode_1bit = true;
#endif
#endif
  while (!xptouch_sdcard_setup(sd_cs_pin, sd_mode_1bit))
    ;

#if defined(__XPTOUCH_PLATFORM_S3__)
  xptouch_lcd_json_apply_from_sd_and_reboot();
  lv_fs_arduino_sd_init(); 
  xptouch_thumbnail_subscribe_events();
  xptouch_history_subscribe_events();
#endif

  xptouch_coldboot_check();

  xptouch_settings_loadSettings();

  xptouch_firmware_checkFirmwareUpdate();

  xptouch_touch_setup();

  xptouch_demo_detect_flag();

  while (!xptouch_wifi_setup())
    ;

  if (!xPTouchConfig.xTouchDemoMode)
    xptouch_firmware_checkOnlineFirmwareUpdate();

  xptouch_screen_setupScreenTimer();
  xptouch_screen_setupLEDOffTimer();
  xptouch_setupGlobalEvents();

  if (xPTouchConfig.xTouchDemoMode)
  {
    xptouch_demo_setup();
  }
  else if (xptouch_filesystem_exist(xptouch_sdcard_fs(), xptouch_paths_provisioning))
  {
        ConsoleDebug.println("Provisioning mode initialize");
        xPTouchConfig.xTouchProvisioningMode = true;
        if (!cloud.hasAuthTokens()) 
        {
          xptouch_webserver_begin();
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
          xptouch_cloud_mqtt_setup();
        }

  }
  else if (xptouch_filesystem_exist(xptouch_sdcard_fs(), xptouch_paths_config))
  {
        ConsoleDebug.println("Lan only mode initialize");
        xPTouchConfig.xTouchLanOnlyMode = true;
        xptouch_local_mqtt_setup();
    }


#if defined(__XPTOUCH_SCREEN_28__)
  ConsoleDebug.println("XPTOUCH_NEO_PIXEL_INIT");
  if (board == lgfx::boards::board_t::board_Guition_ESP32_2432W328R || 
      board == lgfx::boards::board_t::board_Guition_ESP32_2432W328C || 
      board == lgfx::boards::board_t::board_Sunton_ESP32_3248S035C ||
      board == lgfx::boards::board_t::board_Sunton_ESP32_3248S035R)
  {
    ConsoleDebug.println("LED 21");
    xPTouchConfig.xTouchNeoPixelPinValue = 21;
  }
  else if (board == lgfx::boards::board_t::board_Sunton_ESP32_2432S028_7789 ||
           board == lgfx::boards::board_t::board_Sunton_ESP32_2432S028_9341 )
  {
    ConsoleDebug.println("LED 27");
    xPTouchConfig.xTouchNeoPixelPinValue = 27;
  }
#elif defined(__XPTOUCH_SCREEN_S3_050__)
  ConsoleDebug.println("LED 17");
  xPTouchConfig.xTouchNeoPixelPinValue = 17;
#elif defined(__XPTOUCH_SCREEN_S3_3248__)
  ConsoleDebug.println("LED 17");
  xPTouchConfig.xTouchNeoPixelPinValue = 17;
#elif defined(__XPTOUCH_SCREEN_S3_028__)
  /* S3 2.8: UART0(TX0/RX0)=GPIO43/44 を外部I/Oに回す運用に合わせ、NeoPixel は GPIO43 固定 */
  ConsoleDebug.println("[NeoPixel] S3 2.8 LED 43");
  xPTouchConfig.xTouchNeoPixelPinValue = 43;
#elif defined(__XPTOUCH_PLATFORM_S3__)
  /* 2432S028（LGFX）: FT5x06 の INT=GPIO17。5" と同じ 17 を Neo に使うとタッチと競合する。
   * ピンは printer.json の neoPixelPin のみ（未設定 0 のときは Neo タイマーは起動しない）。 */
  ConsoleDebug.println("[NeoPixel] S3 (non-5\"): use neoPixelPin from settings (not GPIO17)");
#endif

  xptouch_neo_pixel_init(xPTouchConfig.xTouchNeoPixelPinValue);
  xptouch_chamber_timer_init();
  xptouch_screen_startScreenTimer();

// #if defined(__XPTOUCH_SCREEN_28__)
//   xptouch_m5stack_buttons_setup((int)tft.getBoard());
// #endif
}

#ifdef XPTOUCH_DEBUG_VERBOSE
static uint32_t s_last_heap_log_ms = 0;
#define HEAP_LOG_INTERVAL_MS 5000
#endif

#if defined(__XPTOUCH_PLATFORM_S3__) && !defined(__XPTOUCH_SCREEN_S3_3248__)
static void xptouch_camera_render_latest_frame_if_any()
{
  if (xPTouchConfig.currentScreenIndex != 17)
    return;
  const uint8_t *jpg = nullptr;
  size_t jpg_len = 0;
  const bool got = xPTouchConfig.xTouchDemoMode
                       ? xptouch_camera_stream::consume_demo_still_frame(&jpg, &jpg_len)
                       : xptouch_camera_stream::consume_latest_frame(&jpg, &jpg_len);
  if (!got)
    return;

  const int sidebar_w = (screenWidth >= 800) ? (screenWidth * 10 / 100) : (screenWidth * 13 / 100);
  const int preview_x = sidebar_w;
  int preview_w = screenWidth - sidebar_w;
  int preview_h = (screenHeight * 8) / 10;
  int preview_y = (screenHeight - preview_h) / 2;
  if (preview_h <= 0 || preview_w <= 0)
    return;

  const float src_w = 1280.0f;
  const float src_h = 720.0f;
  float scale_w = (float)preview_w / src_w;
  float scale = scale_w;
  if (scale <= 0.01f)
    scale = 0.25f;

  tft.drawJpg(jpg, jpg_len, preview_x, preview_y, preview_w, preview_h, 0, 0, scale);
}
#endif

void loop()
{
  lv_timer_handler();
  lv_task_handler();
#if defined(__XPTOUCH_PLATFORM_S3__)
  if (xPTouchConfig.currentScreenIndex == 17 && xPTouchConfig.xTouchP1sCameraStreamEnabled)
  {
    if (!xPTouchConfig.xTouchDemoMode)
      xptouch_camera_stream::start_if_needed();
  }
  else
  {
    xptouch_camera_stream::stop_if_needed();
    if (xPTouchConfig.xTouchDemoMode)
      xptouch_camera_stream::reset_demo_still_cache();
  }
  if (!xPTouchConfig.xTouchDemoMode)
    xptouch_camera_stream::loop_once();
#if !defined(__XPTOUCH_SCREEN_S3_3248__)
  xptouch_camera_render_latest_frame_if_any();
#endif
#endif
  /* Video画面(17)では負荷軽減のため MQTT ループを止める */
  if (!xPTouchConfig.xTouchDemoMode &&
      (xPTouchConfig.xTouchLanOnlyMode || cloud.loggedIn) &&
      xPTouchConfig.currentScreenIndex != 17)
    xptouch_cloud_mqtt_loop();

  if (xptouch_ota_update_flag)
  {
    xptouch_ota_update_flag = false;
    lv_msg_send(XPTOUCH_SETTINGS_OTA_UPDATE_NOW, NULL);
  }
  
  // キャラクターアニメーション処理（millis()ベース、タイマー不要）
  xptouch_events_onCharacterAnimation();

// #if defined(__XPTOUCH_SCREEN_28__)
//   xptouch_m5stack_buttons_loop();
// #endif

#ifdef XPTOUCH_DEBUG_VERBOSE
  if (millis() - s_last_heap_log_ms >= HEAP_LOG_INTERVAL_MS)
  {
    s_last_heap_log_ms = millis();
    ConsoleVerbose.printf("[xPTouch][V][Heap] free=%u max_alloc=%u\n", (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getMaxAllocHeap());
  }
#endif

  delay(10);
}
