/**
 * フィラメント Brand/Type オプション文字列取得。
 * 都度組み立て: ensure_brands_loaded / load_type_options_for_display_index を呼び、オプション文字列バッファをコピー。
 */
#include <stdbool.h>
#include <string.h>
#include "xtouch/types.h"
#include "xtouch/globals.h"

void xtouch_public_filaments_get_brand_options(char *buf, unsigned int buf_len)
{
    buf[0] = '\0';
    if (!buf || buf_len == 0)
        return;
    xtouch_filaments_ensure_brands_loaded();
    if (!bambuStatus.has_public_filaments)
        return;
    size_t len = strlen(xtouch_filament_brand_options);
    if (len >= buf_len)
        len = buf_len - 1;
    memcpy(buf, xtouch_filament_brand_options, len);
    buf[len] = '\0';
}

void xtouch_public_filaments_get_type_options(int brand_idx, char *buf, unsigned int buf_len)
{
    buf[0] = '\0';
    if (!buf || buf_len == 0)
        return;
    xtouch_filaments_ensure_brands_loaded();
    if (brand_idx < 0 || brand_idx >= xtouch_filament_num_brands)
        return;
    xtouch_filaments_load_type_options_for_display_index(brand_idx);
    size_t len = strlen(xtouch_filament_type_options);
    if (len >= buf_len)
        len = buf_len - 1;
    memcpy(buf, xtouch_filament_type_options, len);
    buf[len] = '\0';
}

void xtouch_public_filaments_get_type_options_by_name(const char *brand_name, char *buf, unsigned int buf_len)
{
    buf[0] = '\0';
    if (!buf || buf_len == 0 || !brand_name || !brand_name[0])
        return;
    xtouch_filaments_ensure_brands_loaded();
    if (!bambuStatus.has_public_filaments)
        return;
    char tmp[16];
    for (int i = 0; i < xtouch_filament_num_brands; i++) {
        xtouch_filaments_get_brand_name_at_index(i, tmp, sizeof(tmp));
        if (strcmp(tmp, brand_name) == 0) {
            xtouch_filaments_load_type_options_for_display_index(i);
            size_t len = strlen(xtouch_filament_type_options);
            if (len >= buf_len)
                len = buf_len - 1;
            memcpy(buf, xtouch_filament_type_options, len);
            buf[len] = '\0';
            return;
        }
    }
}

void xtouch_public_filaments_get_type_options_by_display_index(int display_index, char *buf, unsigned int buf_len)
{
    buf[0] = '\0';
    if (!buf || buf_len == 0 || display_index < 0)
        return;
    xtouch_filaments_ensure_brands_loaded();
    if (!bambuStatus.has_public_filaments)
        return;
    xtouch_filaments_load_type_options_for_display_index(display_index);
    size_t len = strlen(xtouch_filament_type_options);
    if (len >= buf_len)
        len = buf_len - 1;
    memcpy(buf, xtouch_filament_type_options, len);
    buf[len] = '\0';
}

void xtouch_public_filaments_get_selected_id_n(int brand_display_index, int type_display_index, char *id_buf, unsigned int id_len, char *n_buf, unsigned int n_len, char *type_buf, unsigned int type_len, int *out_nozzle_temp_min, int *out_nozzle_temp_max)
{
    if (id_buf && id_len > 0)
        id_buf[0] = '\0';
    if (n_buf && n_len > 0)
        n_buf[0] = '\0';
    if (type_buf && type_len > 0)
        type_buf[0] = '\0';
    if (brand_display_index < 0 || type_display_index < 0)
        return;
    xtouch_filaments_ensure_brands_loaded();
    if (brand_display_index >= xtouch_filament_num_brands)
        return;
    xtouch_filaments_get_id_n_for_brand_type_index(brand_display_index, type_display_index, id_buf, id_len, n_buf, n_len, type_buf, type_len, out_nozzle_temp_min, out_nozzle_temp_max);
}

/** brand_str と type_str（例: "Bambu Lab", "ABS"）に一致する表示インデックスを返す。見つかれば 1、なければ 0。 */
int xtouch_public_filaments_find_indices_by_brand_and_type(const char *brand_str, const char *type_str, int *out_brand_idx, int *out_type_idx)
{
    if (!brand_str || !type_str || brand_str[0] == '\0' || type_str[0] == '\0' || !out_brand_idx || !out_type_idx)
        return 0;
    xtouch_filaments_ensure_brands_loaded();
    if (!bambuStatus.has_public_filaments || xtouch_filament_num_brands <= 0)
        return 0;
    char brand_buf[16];
    char type_buf[16];
    for (int bi = 0; bi < xtouch_filament_num_brands; bi++)
    {
        xtouch_filaments_get_brand_name_at_index(bi, brand_buf, sizeof(brand_buf));
        if (strcmp(brand_buf, brand_str) != 0)
            continue;
        for (int ti = 0; ti < 128; ti++)
        {
            type_buf[0] = '\0';
            xtouch_filaments_get_id_n_for_brand_type_index(bi, ti, NULL, 0, NULL, 0, type_buf, sizeof(type_buf), NULL, NULL);
            if (type_buf[0] == '\0')
                break;
            if (strcmp(type_buf, type_str) == 0)
            {
                *out_brand_idx = bi;
                *out_type_idx = ti;
                return 1;
            }
        }
    }
    return 0;
}
