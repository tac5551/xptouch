#ifndef _XTOUCH_LV_FS_ARDUINO_SD_H_
#define _XTOUCH_LV_FS_ARDUINO_SD_H_

#ifdef __cplusplus
extern "C" {
#endif

/* LVGL 向け Arduino SD ファイルシステムドライバ。
 * ドライブレター 'S' で S:/path/to/file.png のようにアクセスする。*/

void lv_fs_arduino_sd_init(void);

#ifdef __cplusplus
}
#endif

#endif

