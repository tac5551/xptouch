#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9341    _panel_instance;
  lgfx::Bus_SPI          _bus_instance;  
  lgfx::Light_PWM        _light_instance;
  lgfx::Touch_XPT2046    _touch_instance;

public:
  LGFX(void)
  {
    { // バス制御の設定を行います。
      auto cfg = _bus_instance.config();           // バス設定用の構造体を取得します。


      cfg.spi_host = HSPI_HOST;                    // 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
      cfg.spi_mode = 0;                            // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 40000000;                   // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
      cfg.freq_read  = 16000000;                   // 受信時のSPIクロック
      cfg.spi_3wire  = false;                      // 受信をMOSIピンで行う場合はtrueを設定
      cfg.use_lock   = false;                       // トランザクションロックを使用する場合はtrueを設定
      cfg.dma_channel = 0;                         // 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
      cfg.pin_mosi = GPIO_NUM_13;                           // SPIのMOSIピン番号を設定
      cfg.pin_miso = GPIO_NUM_12;                           // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_sclk = GPIO_NUM_14;                           // SPIのSCLKピン番号を設定
      cfg.pin_dc   = GPIO_NUM_2;                            // SPIのD/Cピン番号を設定  (-1 = disable)

      _bus_instance.config(cfg);                   // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance);      // バスをパネルにセットします。
    }

    { // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config();         // 表示パネル設定用の構造体を取得します。

      cfg.pin_cs           =    GPIO_NUM_15;                // CSが接続されているピン番号   (-1 = disable)
      cfg.pin_rst          =    GPIO_NUM_NC;                // RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_busy         =    GPIO_NUM_NC;                // BUSYが接続されているピン番号 (-1 = disable)

      cfg.panel_width      =   240;                // 実際に表示可能な幅
      cfg.panel_height     =   320;                // 実際に表示可能な高さ
      cfg.offset_rotation  =     2;                // 回転方向の値のオフセット 0~7 (4~7は上下反転)
      cfg.bus_shared       = false;                // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

      _panel_instance.config(cfg);
    }

    { 
      auto cfg = _light_instance.config();         // バックライト設定用の構造体を取得します。

      cfg.pin_bl = GPIO_NUM_21;                             // バックライトが接続されているピン番号
      cfg.invert = false;                          // バックライトの輝度を反転させる場合 true
      cfg.freq   = 44100;                          // バックライトのPWM周波数
      cfg.pwm_channel = 7;                         // 使用するPWMのチャンネル番号

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);  // バックライトをパネルにセットします。
    }

    { 
      auto cfg = _touch_instance.config();

      cfg.x_min      =  300;
      cfg.x_max      = 3900;
      cfg.y_min      = 3700;
      cfg.y_max      =  200;
      cfg.pin_int    = GPIO_NUM_NC;
      cfg.bus_shared = false;

      cfg.spi_host = -1;                           // -1:use software SPI for XPT2046
      cfg.freq = 1000000;                          // SPIクロックを設定
      cfg.pin_sclk = GPIO_NUM_25;                           // SCLKが接続されているピン番号
      cfg.pin_mosi = GPIO_NUM_32;                           // MOSIが接続されているピン番号
      cfg.pin_miso = GPIO_NUM_39;                           // MISOが接続されているピン番号
      cfg.pin_cs   = GPIO_NUM_33;                           //   CSが接続されているピン番号

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);  // タッチスクリーンをパネルにセットします。
    }


    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};

