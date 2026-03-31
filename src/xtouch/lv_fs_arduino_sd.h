#ifndef _XTOUCH_LV_FS_ARDUINO_SD_H_
#define _XTOUCH_LV_FS_ARDUINO_SD_H_

#include "lvgl.h"
#include "debug.h"
#include <SD.h>
#include <Arduino.h>
#include "xtouch/sdcard_status.h"

/* シンプルな Arduino SD ベースの LVGL ファイルシステムドライバ。
 * - 読み込み専用
 * - キャッシュなし
 * - ドライブレターは 'S' （S:/path.png） */

static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode)
{
    LV_UNUSED(drv);
    if (!path)
        return nullptr;

    if (!xtouch_sdcard_is_present_cached())
    {
        ConsoleDetail.printf("[LVFS] open skipped (SD not present) path=\"%s\"\n", path);
        return nullptr;
    }

    const char *open_mode;
    if (mode == LV_FS_MODE_WR)
        open_mode = FILE_WRITE;   // "w"
    else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD))
        open_mode = FILE_WRITE;   // 読み書きはとりあえず書き込みモード扱い
    else
        open_mode = FILE_READ;    // "r"

    // LVGL からは "S:/foo/bar.png" 形式で来るので、"S:" を剥がして SD に渡す
    const char *sd_path = path;
    if (path[0] == 'S' && path[1] == ':')
        sd_path = (path[2] == '/') ? path + 3 : path + 2;

    File *f = new File(SD.open(sd_path, open_mode));
    if (!f || !*f)
    {
        ConsoleDetail.printf("[LVFS] SD.open FAIL path=\"%s\" sd_path=\"%s\"\n", path, sd_path);
        if (f) delete f;
        return nullptr;
    }
    ConsoleDetail.printf("[LVFS] SD.open OK path=\"%s\" sd_path=\"%s\"\n", path, sd_path);
    return (void *)f;
}

static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p)
{
    LV_UNUSED(drv);
    if (!file_p)
        return LV_FS_RES_OK;

    File *f = (File *)file_p;
    f->close();
    delete f;
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br)
{
    LV_UNUSED(drv);
    if (!file_p || !buf || !br)
        return LV_FS_RES_INV_PARAM;

    File *f = (File *)file_p;
    *br = f->read((uint8_t *)buf, btr);
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_write(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw)
{
    LV_UNUSED(drv);
    LV_UNUSED(file_p);
    LV_UNUSED(buf);
    LV_UNUSED(btw);
    LV_UNUSED(bw);
    return LV_FS_RES_NOT_IMP;  // 今回は書き込み不要
}

static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence)
{
    LV_UNUSED(drv);
    if (!file_p)
        return LV_FS_RES_INV_PARAM;

    File *f = (File *)file_p;
    SeekMode mode = SeekSet;
    if (whence == LV_FS_SEEK_CUR)
        mode = SeekCur;
    else if (whence == LV_FS_SEEK_END)
        mode = SeekEnd;

    if (!f->seek(pos, mode))
        return LV_FS_RES_UNKNOWN;
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p)
{
    LV_UNUSED(drv);
    if (!file_p || !pos_p)
        return LV_FS_RES_INV_PARAM;

    File *f = (File *)file_p;
    *pos_p = (uint32_t)f->position();
    return LV_FS_RES_OK;
}

inline void lv_fs_arduino_sd_init(void)
{
    static lv_fs_drv_t drv;
    lv_fs_drv_init(&drv);
    drv.letter = 'S';
    drv.open_cb = fs_open;
    drv.close_cb = fs_close;
    drv.read_cb = fs_read;
    drv.write_cb = fs_write;
    drv.seek_cb = fs_seek;
    drv.tell_cb = fs_tell;
    drv.dir_open_cb = nullptr;
    drv.dir_read_cb = nullptr;
    drv.dir_close_cb = nullptr;
    lv_fs_drv_register(&drv);
}


#endif

