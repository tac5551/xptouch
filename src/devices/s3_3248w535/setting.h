#ifndef _XLCD_SETTING
#define _XLCD_SETTING

#define JC3248_LCD_CS 45
#define JC3248_LCD_SCLK 47
#define JC3248_LCD_SDIO0 21
#define JC3248_LCD_SDIO1 48
#define JC3248_LCD_SDIO2 40
#define JC3248_LCD_SDIO3 39
#define JC3248_LCD_RST (-1)
#define JC3248_TFT_BL 1

/* 1: active low backlight, 0: active high. */
#ifndef JC3248_TFT_BL_ACTIVE_LOW
#define JC3248_TFT_BL_ACTIVE_LOW 0
#endif

#define JC3248_LCD_NATIVE_W 320
#define JC3248_LCD_NATIVE_H 480

#define JC3248_TOUCH_SDA 4
#define JC3248_TOUCH_SCL 8
#define JC3248_TOUCH_INT 3
#define JC3248_TOUCH_ADDR 0x3B

#ifndef XTOUCH_TFT_DEFAULT_ROTATION
#define XTOUCH_TFT_DEFAULT_ROTATION 1
#endif

#ifndef XTOUCH_LVGL_DRAW_BUF_DENOM
#define XTOUCH_LVGL_DRAW_BUF_DENOM 8
#endif

int screenWidth = 320;
int screenHeight = 480;

#endif

