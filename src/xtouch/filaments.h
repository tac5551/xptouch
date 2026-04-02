#ifndef _XTOUCH_PUBLIC_FILAMENTS_H
#define _XTOUCH_PUBLIC_FILAMENTS_H

#include <string.h>
#include "xtouch/paths.h"
#include "xtouch/types.h"
#include "xtouch/globals.h"
/* 都度組み立て: SD から表示時に読み、brand_options / type_options のみ保持。id/n は必要なときファイルから取得。 */

#ifdef __cplusplus
#include <stdio.h>
#include <FS.h>
#include "xtouch/sdcard.h"
#include "xtouch/bblp.h"

/** path のテキストをパイプバッファに読み込むだけ。パースは呼ばない。 */
static inline bool xtouch_filaments_read_file_to_buf(const char *path) {
    xTouchFilamentsPipeLen = 0;
    if (!path || !path[0] || !xtouch_sdcard_exists(path)) return false;
    File f = xtouch_sdcard_open(path);
    if (!f || !f.available()) { if (f) f.close(); return false; }
    unsigned int n = 0;
    while (f.available() && n < XTOUCH_FILAMENTS_PIPE_BUF_SIZE - 1) {
        int c = f.read();
        if (c < 0) break;
        xTouchFilamentsPipeBuf[n++] = (char)c;
    }
    xTouchFilamentsPipeBuf[n] = '\0';
    xTouchFilamentsPipeLen = n;
    f.close();
    return (n > 0);
}

/** ブランド名をファイル名用に変換（スペース→アンダースコア）。out は少なくとも 16 バイト。 */
static inline void xtouch_filaments_sanitize_brand_for_filename(const char *brand, char *out, size_t out_size) {
    if (!out || out_size == 0) return;
    size_t i = 0;
    while (brand[i] && i < out_size - 1) {
        out[i] = (brand[i] == ' ') ? '_' : brand[i];
        i++;
    }
    out[i] = '\0';
}

/** パイプバッファの id|n|t 行から type_options 文字列だけ組み立てる（id/n は保持しない）。 */
static inline void xtouch_filaments_parse_pipe_into_type_options(void) {
    xtouch_filament_current_type_count = 0;
    xtouch_filament_type_options[0] = '\0';
    const char *p = xTouchFilamentsPipeBuf;
    const char *end = p + xTouchFilamentsPipeLen;
    unsigned int opt_n = 0;
    while (p < end && xtouch_filament_current_type_count < XTOUCH_FILAMENT_MAX_ITEMS_PER_BRAND) {
        const char *line_end = (const char *)memchr(p, '\n', (size_t)(end - p));
        if (!line_end) line_end = end;
        size_t line_len = (size_t)(line_end - p);
        if (line_len >= 2 && p[0] == '%' && p[1] == '%')
            break;
        if (line_len > 0) {
            const char *a = p, *b = (const char *)memchr(a, '|', line_len);
            if (b) {
                a = b + 1;
                b = (const char *)memchr(a, '|', (size_t)(line_end - a));
                if (b) {
                    a = b + 1;
                    size_t t_len = (size_t)(line_end - a);
                    if (opt_n > 0 && opt_n < XTOUCH_FILAMENT_OPTS_BUF_SIZE - 1)
                        xtouch_filament_type_options[opt_n++] = '\n';
                    size_t copy_t = t_len >= 16 ? 15 : t_len;
                    if (opt_n + copy_t >= XTOUCH_FILAMENT_OPTS_BUF_SIZE)
                        copy_t = XTOUCH_FILAMENT_OPTS_BUF_SIZE - opt_n - 1;
                    memcpy(xtouch_filament_type_options + opt_n, a, copy_t);
                    opt_n += (unsigned int)copy_t;
                    xtouch_filament_current_type_count++;
                }
            }
        }
        p = line_end + (line_end < end ? 1 : 0);
    }
    if (opt_n < XTOUCH_FILAMENT_OPTS_BUF_SIZE)
        xtouch_filament_type_options[opt_n] = '\0';
}

/** ファイルが無いときのフォールバック用。 */
static const char* const xtouch_filament_fixed_brand_names[] = { "Bambu Lab", "Generic" };
#define XTOUCH_FILAMENT_FIXED_BRAND_COUNT 2

/** ファイルがあれば全ブランドを SD から読み、なければ固定 2 件でフォールバック。 */
static inline void xtouch_filaments_ensure_brands_loaded_impl(void) {
    bambuStatus.has_public_filaments = 0;
    xtouch_filament_num_brands = 0;
    xtouch_filament_brand_options[0] = '\0';
    xtouch_filament_current_brand_index = -1;
    xtouch_filament_pipe_holds_brands = 0;
    xTouchFilamentsPipeLen = 0;

    /* ノズルサイズは使わず1セット: filaments_brands.txt */
    char path[96];
    snprintf(path, sizeof(path), "%s/filaments_brands.txt", xtouch_paths_filament_dir);
    if (xtouch_sdcard_exists(path) && xtouch_filaments_read_file_to_buf(path)) {
        const char *p = xTouchFilamentsPipeBuf;
        const char *end = p + xTouchFilamentsPipeLen;
        int nb = 0;
        unsigned int opt_n = 0;
        while (p < end && nb < XTOUCH_FILAMENT_MAX_BRANDS) {
            const char *line_end = (const char *)memchr(p, '\n', (size_t)(end - p));
            if (!line_end) line_end = end;
            size_t line_len = (size_t)(line_end - p);
            if (line_len > 0) {
                if (opt_n > 0 && opt_n < XTOUCH_FILAMENT_OPTS_BUF_SIZE - 1)
                    xtouch_filament_brand_options[opt_n++] = '\n';
                size_t copy_len = line_len >= 16 ? 15 : line_len;
                if (opt_n + copy_len >= XTOUCH_FILAMENT_OPTS_BUF_SIZE)
                    copy_len = XTOUCH_FILAMENT_OPTS_BUF_SIZE - opt_n - 1;
                memcpy(xtouch_filament_brand_options + opt_n, p, copy_len);
                opt_n += (unsigned int)copy_len;
                nb++;
            }
            p = line_end + (line_end < end ? 1 : 0);
        }
        if (opt_n < XTOUCH_FILAMENT_OPTS_BUF_SIZE)
            xtouch_filament_brand_options[opt_n] = '\0';
        xtouch_filament_num_brands = nb;
        bambuStatus.has_public_filaments = (nb > 0) ? 1 : 0;
        xtouch_filament_use_fixed_brands = 0;
        xtouch_filament_pipe_holds_brands = 1;
        return;
    }
    /* ファイルがなければフォールバックなし（num_brands=0 のまま。Edit ボタン非表示などに利用可） */
}

/** i 番目のブランド名を取得。固定時は Bambu Lab / Generic を返す。 */
static inline void xtouch_filaments_get_ith_brand_name_impl(int index, char *buf, unsigned int buf_len) {
    if (!buf || buf_len == 0 || index < 0) return;
    buf[0] = '\0';
    if (index >= xtouch_filament_num_brands) return;
    if (xtouch_filament_use_fixed_brands && index < XTOUCH_FILAMENT_FIXED_BRAND_COUNT) {
        const char *s = xtouch_filament_fixed_brand_names[index];
        size_t len = strlen(s);
        if (len >= buf_len) len = buf_len - 1;
        memcpy(buf, s, len + 1);
        return;
    }
    if (!xtouch_filament_pipe_holds_brands) {
        char path[96];
        snprintf(path, sizeof(path), "%s/filaments_brands.txt", xtouch_paths_filament_dir);
        if (!xtouch_filaments_read_file_to_buf(path)) return;
        xtouch_filament_pipe_holds_brands = 1;
    }
    const char *p = xTouchFilamentsPipeBuf;
    const char *end = p + xTouchFilamentsPipeLen;
    int cur = 0;
    while (p < end && cur <= index) {
        const char *line_end = (const char *)memchr(p, '\n', (size_t)(end - p));
        if (!line_end) line_end = end;
        size_t line_len = (size_t)(line_end - p);
        if (line_len > 0) {
            if (cur == index) {
                size_t copy_len = line_len >= (size_t)buf_len - 1 ? (size_t)buf_len - 1 : line_len;
                memcpy(buf, p, copy_len);
                buf[copy_len] = '\0';
                return;
            }
            cur++;
        }
        p = line_end + (line_end < end ? 1 : 0);
    }
}

/** 表示インデックス display_index のブランドの Type 一覧を SD から読んで type_options にセット。 */
static inline void xtouch_filaments_load_type_options_for_display_index_impl(int display_index) {
    if (display_index < 0 || display_index >= xtouch_filament_num_brands) {
        xtouch_filament_current_brand_index = -1;
        xtouch_filament_current_type_count = 0;
        xtouch_filament_type_options[0] = '\0';
        return;
    }
    xtouch_filament_current_brand_index = display_index;
    char brand[16];
    xtouch_filaments_get_ith_brand_name_impl(display_index, brand, sizeof(brand));
    char fname[20];
    xtouch_filaments_sanitize_brand_for_filename(brand, fname, sizeof(fname));
    /* ノズルなし1セット: filaments_{Brand}.txt（1行＝filament_idごと先頭のsetting_id） */
    char path[96];
    snprintf(path, sizeof(path), "%s/filaments_%s.txt", xtouch_paths_filament_dir, fname);
    xtouch_filament_pipe_holds_brands = 0;
    if (xtouch_filaments_read_file_to_buf(path))
        xtouch_filaments_parse_pipe_into_type_options();
    else {
        xtouch_filament_current_type_count = 0;
        xtouch_filament_type_options[0] = '\0';
    }
}

extern "C" void xtouch_filaments_ensure_brands_loaded(void) {
    xtouch_filaments_ensure_brands_loaded_impl();
}
extern "C" void xtouch_filaments_load_type_options_for_display_index(int display_index) {
    xtouch_filaments_load_type_options_for_display_index_impl(display_index);
}
extern "C" void xtouch_filaments_load_for_current_printer_c(void) {
    xtouch_filaments_ensure_brands_loaded_impl();
}
extern "C" void xtouch_filaments_get_brand_name_at_index(int index, char *buf, unsigned int buf_len) {
    xtouch_filaments_get_ith_brand_name_impl(index, buf, buf_len);
}

/** 指定ブランドの type_display_index 番目の行から取得。行形式: id|n|t または id|n|t|nozzle_temp_min|nozzle_temp_max（Chrome 等が API 取得後に追記）。type_buf / out_min / out_max は省略可（NULL）。 */
static inline void xtouch_filaments_get_id_n_for_brand_type_index_impl(int brand_display_index, int type_display_index, char *id_buf, unsigned int id_len, char *n_buf, unsigned int n_len, char *type_buf, unsigned int type_len, int *out_nozzle_temp_min, int *out_nozzle_temp_max) {
    if (id_buf && id_len > 0) id_buf[0] = '\0';
    if (n_buf && n_len > 0) n_buf[0] = '\0';
    if (type_buf && type_len > 0) type_buf[0] = '\0';
    if (out_nozzle_temp_min) *out_nozzle_temp_min = 0;
    if (out_nozzle_temp_max) *out_nozzle_temp_max = 0;
    if (brand_display_index < 0 || type_display_index < 0) return;
    char brand[16];
    xtouch_filaments_get_ith_brand_name_impl(brand_display_index, brand, sizeof(brand));
    if (!brand[0]) return;
    char fname[20];
    xtouch_filaments_sanitize_brand_for_filename(brand, fname, sizeof(fname));
    char path[96];
    snprintf(path, sizeof(path), "%s/filaments_%s.txt", xtouch_paths_filament_dir, fname);
    if (!xtouch_filaments_read_file_to_buf(path)) return;
    const char *p = xTouchFilamentsPipeBuf;
    const char *end = p + xTouchFilamentsPipeLen;
    int cur = 0;
    while (p < end && cur <= type_display_index) {
        const char *line_end = (const char *)memchr(p, '\n', (size_t)(end - p));
        if (!line_end) line_end = end;
        size_t line_len = (size_t)(line_end - p);
        if (line_len >= 2 && p[0] == '%' && p[1] == '%') break;
        if (line_len > 0) {
            const char *a = p, *b = (const char *)memchr(a, '|', line_len);
            if (b) {
                const char *b2 = (const char *)memchr(b + 1, '|', (size_t)(line_end - (b + 1)));
                if (b2 && cur == type_display_index) {
                    size_t id_sz = (size_t)(b - a);
                    if (id_sz >= id_len) id_sz = id_len - 1;
                    if (id_buf && id_len > 0) {
                        memcpy(id_buf, a, id_sz);
                        id_buf[id_sz] = '\0';
                    }
                    size_t n_sz = (size_t)(b2 - (b + 1));
                    if (n_sz >= n_len) n_sz = n_len - 1;
                    if (n_buf && n_len > 0) {
                        memcpy(n_buf, b + 1, n_sz);
                        n_buf[n_sz] = '\0';
                    }
                    const char *b3 = (const char *)memchr(b2 + 1, '|', (size_t)(line_end - (b2 + 1)));
                    size_t t_sz = b3 ? (size_t)(b3 - (b2 + 1)) : (size_t)(line_end - (b2 + 1));
                    if (type_buf && type_len > 0) {
                        if (t_sz >= type_len) t_sz = type_len - 1;
                        memcpy(type_buf, b2 + 1, t_sz);
                        type_buf[t_sz] = '\0';
                    }
                    if (b3 && out_nozzle_temp_min) {
                        const char *b4 = (const char *)memchr(b3 + 1, '|', (size_t)(line_end - (b3 + 1)));
                        char num[12];
                        size_t len = b4 ? (size_t)(b4 - (b3 + 1)) : (size_t)(line_end - (b3 + 1));
                        if (len >= sizeof(num)) len = sizeof(num) - 1;
                        memcpy(num, b3 + 1, len);
                        num[len] = '\0';
                        *out_nozzle_temp_min = atoi(num);
                        if (out_nozzle_temp_max && b4) {
                            len = (size_t)(line_end - (b4 + 1));
                            if (len >= sizeof(num)) len = sizeof(num) - 1;
                            memcpy(num, b4 + 1, len);
                            num[len] = '\0';
                            *out_nozzle_temp_max = atoi(num);
                        }
                    }
                    return;
                }
                cur++;
            }
        }
        p = line_end + (line_end < end ? 1 : 0);
    }
}
extern "C" void xtouch_filaments_get_id_n_for_brand_type_index(int brand_display_index, int type_display_index, char *id_buf, unsigned int id_len, char *n_buf, unsigned int n_len, char *type_buf, unsigned int type_len, int *out_nozzle_temp_min, int *out_nozzle_temp_max) {
    xtouch_filaments_get_id_n_for_brand_type_index_impl(brand_display_index, type_display_index, id_buf, id_len, n_buf, n_len, type_buf, type_len, out_nozzle_temp_min, out_nozzle_temp_max);
}

#endif /* __cplusplus */

/** 都度組み立て用クリア（必要なら UI から呼ぶ）。 */
void xtouch_public_filaments_load(void)
{
    /* オプション文字列とキャッシュをクリア。bambuStatus.has_public_filaments は ensure で上書きされる。 */
}

/* 以下は filaments_options.c で実装。UI は types.h の宣言のみ参照。 */
void xtouch_public_filaments_get_brand_options(char *buf, unsigned int buf_len);
void xtouch_public_filaments_get_type_options(int brand_idx, char *buf, unsigned int buf_len);
void xtouch_public_filaments_get_selected_id_n(int brand_display_index, int type_display_index, char *id_buf, unsigned int id_len, char *n_buf, unsigned int n_len, char *type_buf, unsigned int type_len, int *out_nozzle_temp_min, int *out_nozzle_temp_max);

#endif
