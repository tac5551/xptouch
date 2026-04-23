#include "ui_comp_historyreprintcomponent.h"
#include "xtouch/globals.h"
#include "../ui_msgs.h"
#include "../ui_helpers.h"
#include "../ui_events.h"
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdint.h>

#ifdef __XTOUCH_PLATFORM_S3__

/* History 一覧（ui_comp_historycomponent.c の ROW_LEFT_*）とサムネ枠を同一にする */
#if defined(__XTOUCH_SCREEN_S3_050__)
#define HRPRINT_HISTORY_THUMB_W 150
#define HRPRINT_HISTORY_THUMB_H 150
#else
#define HRPRINT_HISTORY_THUMB_W 75
#define HRPRINT_HISTORY_THUMB_H 75
#endif

/* Reprint レイアウト定数（解像度ベースで初期化） */
static int HRPRINT_SET_BTN_W = 56;
static int HRPRINT_SET_BTN_H = 34;
static int HRPRINT_SRC_SWATCH_W = 20;
static int HRPRINT_SRC_SWATCH_H = 14;
static int HRPRINT_SLOT_CELL_W = 22;
static int HRPRINT_SLOT_SQ_H = 24;
static int HRPRINT_LEFT_TARGET_H = 22;
static int HRPRINT_CARD_HDR_H = 20;
static int HRPRINT_THUMB_W = HRPRINT_HISTORY_THUMB_W;
static int HRPRINT_THUMB_H = HRPRINT_HISTORY_THUMB_H;
static int HRPRINT_TOP_ROW_H = HRPRINT_HISTORY_THUMB_H;
/* 上段はサムネイル基準。行内/行外の余白を足して実効高さが膨らまないようにする */
static int HRPRINT_TOP_ROW_BOTTOM_PAD = 0;
static int HRPRINT_LEFT_PANE_W_PCT = 30;
static int HRPRINT_RIGHT_PANE_W_PCT = 70;
static int HRPRINT_UI_PAD = 0;
static int HRPRINT_UI_GAP = 0;
static int HRPRINT_UI_FOOT_BTN_V = 4;
static int HRPRINT_FOOT_BTN_MIN_H = 34;
static int HRPRINT_FOOT_BTN_PAD_V = 10;
static int HRPRINT_RIGHT_UI_PAD = 0;
static int HRPRINT_SET_BTN_PAD = 12;

static void hrprint_apply_metrics_by_resolution(void)
{
    int hor = (int)lv_disp_get_hor_res(NULL);
    int ver = (int)lv_disp_get_ver_res(NULL);
    int long_edge = (hor > ver) ? hor : ver;

    /* 密度は long_edge、サムネ枠は History 行と同じ（画面プロファイルマクロ） */
    if (long_edge >= 800)
    {
        HRPRINT_SET_BTN_W = 108;
        HRPRINT_SET_BTN_H = 68;
        HRPRINT_SRC_SWATCH_W = 36;
        HRPRINT_SRC_SWATCH_H = 24;
        HRPRINT_SLOT_CELL_W = 40;
        HRPRINT_SLOT_SQ_H = 40;
        HRPRINT_LEFT_TARGET_H = 32;
        HRPRINT_CARD_HDR_H = 32;
        HRPRINT_THUMB_W = HRPRINT_HISTORY_THUMB_W;
        HRPRINT_THUMB_H = HRPRINT_HISTORY_THUMB_H;
        HRPRINT_TOP_ROW_H = HRPRINT_THUMB_H;
        HRPRINT_LEFT_PANE_W_PCT = 33;
        HRPRINT_RIGHT_PANE_W_PCT = 67;
        HRPRINT_UI_PAD = 4;
        HRPRINT_UI_GAP = 8;
        HRPRINT_UI_FOOT_BTN_V = 4;
        HRPRINT_FOOT_BTN_MIN_H = 0;
        HRPRINT_FOOT_BTN_PAD_V = 10;
        HRPRINT_RIGHT_UI_PAD = 2;
        HRPRINT_SET_BTN_PAD = 4;
    }
    else
    {
        HRPRINT_SET_BTN_W = 56;
        HRPRINT_SET_BTN_H = 34;
        HRPRINT_SRC_SWATCH_W = 20;
        HRPRINT_SRC_SWATCH_H = 14;
        HRPRINT_SLOT_CELL_W = 22;
        HRPRINT_SLOT_SQ_H = 24;
        HRPRINT_LEFT_TARGET_H = 22;
        HRPRINT_CARD_HDR_H = 20;
        HRPRINT_THUMB_W = HRPRINT_HISTORY_THUMB_W;
        HRPRINT_THUMB_H = HRPRINT_HISTORY_THUMB_H;
        HRPRINT_TOP_ROW_H = HRPRINT_THUMB_H;
        HRPRINT_LEFT_PANE_W_PCT = 30;
        HRPRINT_RIGHT_PANE_W_PCT = 70;
        HRPRINT_UI_PAD = 0;
        HRPRINT_UI_GAP = 6;
        HRPRINT_UI_FOOT_BTN_V = 4;
        HRPRINT_FOOT_BTN_MIN_H = 34;
        HRPRINT_RIGHT_UI_PAD = 0;
        HRPRINT_SET_BTN_PAD = 4;
    }
}

#define MAP_DD_MAX XTOUCH_HISTORY_AMS_MAP_MAX
#define MAP_OPT_MAX 24
#define MAP_OPT_BUF 384

static lv_obj_t *s_printer_dd = NULL;
static lv_timer_t *s_printer_list_style_timer = NULL;
static lv_timer_t *s_printer_changed_timer = NULL;
static lv_obj_t *s_cover_img = NULL;
static lv_obj_t *s_title_lbl = NULL;
static lv_obj_t *s_info_lbl = NULL;
static lv_obj_t *s_start_btn = NULL;
static lv_obj_t *s_form_obj = NULL;
static lv_coord_t s_form_scroll_y = 0;
static uint8_t s_map_pick_ams[MAP_DD_MAX][MAP_OPT_MAX];
static uint8_t s_map_pick_tray[MAP_DD_MAX][MAP_OPT_MAX];
static uint8_t s_map_pick_loaded[MAP_DD_MAX][MAP_OPT_MAX];
static uint8_t s_map_pick_selectable[MAP_DD_MAX][MAP_OPT_MAX];
static lv_obj_t *s_map_slot_obj[MAP_DD_MAX][MAP_OPT_MAX];
static lv_obj_t *s_map_left_card_obj[MAP_DD_MAX];
static lv_obj_t *s_map_left_target_obj[MAP_DD_MAX];
static lv_obj_t *s_map_left_target_lbl[MAP_DD_MAX];
static lv_obj_t *s_map_right_panel = NULL;
static int s_active_map_idx = 0;
static int s_map_nopt[MAP_DD_MAX];
static int s_map_row_count;
static uint8_t s_pending_pick_ams[MAP_DD_MAX];
static uint8_t s_pending_pick_tray[MAP_DD_MAX];
static uint8_t s_pending_pick_valid[MAP_DD_MAX];
static int s_printer_dd_to_slot[XTOUCH_MULTI_PRINTER_MAX];
static int s_printer_dd_count = 0;

static void rebuild_right_selector(void);
static lv_obj_t *reprint_make_color_rect(lv_obj_t *parent, lv_color_t col, int w, int h, int border_w, lv_color_t border_col);

static const char *get_tray_color_safe(uint8_t ams, uint8_t tray)
{
    const char *c = get_tray_color_reprint(ams, tray);
    return (c && strlen(c) >= 6) ? c : "808080FF";
}

static void format_ams_slot_text(char *dst, size_t dst_sz, uint8_t ams, uint8_t tray)
{
    static const char ams_unit_lbl[] = { 'A', 'B', 'C', 'D' };
    char ul = (ams < XTOUCH_BAMBU_AMS_UNITS) ? ams_unit_lbl[ams] : '?';
    (void)snprintf(dst, dst_sz, "AMS-%c Slot-%d", ul, (int)tray + 1);
}

static void format_ams_slot_short(char *dst, size_t dst_sz, uint8_t ams, uint8_t tray)
{
    static const char ams_unit_lbl[] = { 'A', 'B', 'C', 'D' };
    char ul = (ams < XTOUCH_BAMBU_AMS_UNITS) ? ams_unit_lbl[ams] : '?';
    (void)snprintf(dst, dst_sz, "%c-%d", ul, (int)tray + 1);
}

static int is_a1mini_name(const char *s)
{
    if (!s || !s[0])
        return 0;
    return (strcasecmp(s, "A1 Mini") == 0 || strcasecmp(s, "A1Mini") == 0 || strcasecmp(s, "A1-Mini") == 0);
}

static int is_history_group_compatible(const char *printer_product, const char *task_device_model)
{
    if (!task_device_model || !task_device_model[0] || !printer_product || !printer_product[0])
        return 1;
    if (is_a1mini_name(printer_product) || is_a1mini_name(task_device_model))
        return strcasecmp(printer_product, task_device_model) == 0;

    static const char *const k_grp[] = { "X1 Carbon", "P1S", "P1P", "P2S", "A1", NULL };
    int in_p = 0, in_t = 0;
    for (int i = 0; k_grp[i]; i++)
    {
        if (strcasecmp(printer_product, k_grp[i]) == 0)
            in_p = 1;
        if (strcasecmp(task_device_model, k_grp[i]) == 0)
            in_t = 1;
    }
    return (in_p && in_t) ? 1 : (strcasecmp(printer_product, task_device_model) == 0);
}

static void populate_printer_dropdown(lv_obj_t *dd)
{
    if (!dd)
        return;

    lv_dropdown_clear_options(dd);

    char buf[256];
    buf[0] = '\0';
    s_printer_dd_count = 0;
    for (int i = 0; i < XTOUCH_MULTI_PRINTER_MAX; i++)
        s_printer_dd_to_slot[i] = 0;

    const char *task_model = xtouch_history_reprint_task_basic_valid ? xtouch_history_reprint_task_basic.device_model : NULL;

    if (is_history_group_compatible(xtouch_current_printer_dev_product_name, task_model) &&
        s_printer_dd_count < XTOUCH_MULTI_PRINTER_MAX)
    {
        s_printer_dd_to_slot[s_printer_dd_count++] = 0;
    }

    for (int i = 0; i < xtouch_other_printer_count && i < XTOUCH_OTHER_PRINTERS_MAX; i++)
    {
        if (!is_history_group_compatible(xtouch_other_printer_dev_product_names[i], task_model))
            continue;
        if (s_printer_dd_count >= XTOUCH_MULTI_PRINTER_MAX)
            break;
        s_printer_dd_to_slot[s_printer_dd_count++] = i + 1;
    }
    if (s_printer_dd_count == 0)
    {
        s_printer_dd_to_slot[0] = 0;
        s_printer_dd_count = 1;
    }

    for (int di = 0; di < s_printer_dd_count; di++)
    {
        size_t len = strlen(buf);
        if (len + 3 >= sizeof(buf))
            break;
        if (di > 0)
        {
            buf[len++] = '\n';
            buf[len] = '\0';
        }
        int slot = s_printer_dd_to_slot[di];
        if (slot == 0)
        {
            strlcpy(buf + len, xTouchConfig.xTouchPrinterName[0] ? xTouchConfig.xTouchPrinterName : "This printer", sizeof(buf) - len);
        }
        else
        {
            int oi = slot - 1;
            const other_printer_status_t *p = &otherPrinters[oi];
            strlcpy(buf + len, p->name[0] ? p->name : "Printer", sizeof(buf) - len);
        }
    }

    lv_dropdown_set_options(dd, buf);
    int selected_idx = 0;
    for (int di = 0; di < s_printer_dd_count; di++)
    {
        if (s_printer_dd_to_slot[di] == xtouch_history_reprint_printer_dd_slot)
        {
            selected_idx = di;
            break;
        }
    }
    lv_dropdown_set_selected(dd, selected_idx);
    /* コンボボックス表示部（main）側のフォントを固定 */
    lv_obj_set_style_text_font(dd, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* プルダウン list が既に存在している場合は先に適用（未確定でも後段で上書きされる） */
    lv_obj_t *list = lv_dropdown_get_list(dd);
    if (list)
        lv_obj_set_style_text_font(list, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static lv_color_t printer_row_bg_by_status(int status)
{
    switch (status)
    {
    case XTOUCH_PRINT_STATUS_RUNNING:
        return lv_color_hex(0x1a5c2a);
    case XTOUCH_PRINT_STATUS_PAUSED:
        return lv_color_hex(0x5c5a1a);
    case XTOUCH_PRINT_STATUS_PREPARE:
        return lv_color_hex(0x3a4a2a);
    case XTOUCH_PRINT_STATUS_FINISHED:
        return lv_color_hex(0x2a3555);
    case XTOUCH_PRINT_STATUS_FAILED:
        return lv_color_hex(0x5c2525);
    default:
        return lv_color_hex(0x444444);
    }
}

static void apply_printer_dropdown_list_colors(lv_obj_t *dd)
{
    if (!dd)
        return;
    lv_obj_t *list = lv_dropdown_get_list(dd);
    if (!list || lv_obj_has_flag(list, LV_OBJ_FLAG_HIDDEN))
        return;

    /* 未指定フォント回避: list/row/子要素すべてにフォントを付ける */
    lv_obj_set_style_text_font(list, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    int n_printers = s_printer_dd_count;

    uint32_t nchild = lv_obj_get_child_cnt(list);
    for (uint32_t i = 0; i < nchild && (int)i < n_printers; i++)
    {
        lv_obj_t *row = lv_obj_get_child(list, i);
        if (!row)
            continue;
        int st = XTOUCH_PRINT_STATUS_IDLE;
        int slot = s_printer_dd_to_slot[i];
        if (slot == 0)
            st = bambuStatus.print_status;
        else if (slot - 1 < xtouch_other_printer_count && otherPrinters[slot - 1].valid)
            st = otherPrinters[slot - 1].print_status;

        lv_color_t bg = printer_row_bg_by_status(st);
        lv_obj_set_style_bg_color(row, bg, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(row, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(row, lv_color_hex(0xF0F0F0), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(row, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
        uint32_t nc = lv_obj_get_child_cnt(row);
        for (uint32_t j = 0; j < nc; j++)
        {
            lv_obj_t *ch = lv_obj_get_child(row, j);
            lv_obj_set_style_text_color(ch, lv_color_hex(0xF0F0F0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(ch, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
}

static void printer_list_style_timer_cb(lv_timer_t *t)
{
    lv_obj_t *dd = (lv_obj_t *)t->user_data;
    s_printer_list_style_timer = NULL;
    apply_printer_dropdown_list_colors(dd);
    lv_timer_del(t);
}

static void on_printer_dd_clicked(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;
    lv_obj_t *dd = lv_event_get_target(e);
    if (dd != s_printer_dd)
        return;
    if (s_printer_list_style_timer)
    {
        lv_timer_del(s_printer_list_style_timer);
        s_printer_list_style_timer = NULL;
    }
    s_printer_list_style_timer = lv_timer_create(printer_list_style_timer_cb, 45, dd);
    lv_timer_set_repeat_count(s_printer_list_style_timer, 1);
}

static void printer_changed_deferred_cb(lv_timer_t *t)
{
    (void)t;
    s_printer_changed_timer = NULL;
    ui_msg_send(XTOUCH_HISTORY_REPRINT_PRINTER_CHANGED, 0, 0);
}

static void on_printer_dd_value_changed(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
        return;
    lv_obj_t *dd = lv_event_get_target(e);
    if (dd != s_printer_dd)
        return;
    int sel = (int)lv_dropdown_get_selected(dd);
    if (sel < 0 || sel >= s_printer_dd_count)
        sel = 0;
    int printer_slot = s_printer_dd_to_slot[sel];
    xtouch_history_reprint_printer_dd_slot = printer_slot;
    const char *dev = xTouchConfig.xTouchSerialNumber;
    if (printer_slot > 0 && printer_slot - 1 < xtouch_other_printer_count)
        dev = xtouch_other_printer_dev_ids[printer_slot - 1];
    if (dev && dev[0])
        xtouch_mqtt_pushall_for_dev_c(dev);
    if (s_printer_changed_timer)
    {
        lv_timer_del(s_printer_changed_timer);
        s_printer_changed_timer = NULL;
    }
    s_printer_changed_timer = lv_timer_create(printer_changed_deferred_cb, 750, NULL);
    lv_timer_set_repeat_count(s_printer_changed_timer, 1);
}

static void populate_summary_panel(void)
{
    if (!s_title_lbl || !s_info_lbl)
        return;
    if (!xtouch_history_reprint_task_basic_valid)
    {
        lv_label_set_text(s_title_lbl, "-");
        lv_label_set_text(s_info_lbl, "");
        if (s_cover_img)
        {
            lv_obj_add_flag(s_cover_img, LV_OBJ_FLAG_HIDDEN);
            lv_obj_t *left = lv_obj_get_parent(s_cover_img);
            if (left)
                lv_obj_add_flag(left, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }

    const xtouch_history_task_t *t = &xtouch_history_reprint_task_basic;
    lv_label_set_text(s_title_lbl, t->title[0] ? t->title : "-");

    char info[96];
    if (t->device_name[0])
        snprintf(info, sizeof(info), "%s  |  Plate %d", t->device_name, t->plate_index);
    else
        snprintf(info, sizeof(info), "Plate %d", t->plate_index);
    lv_label_set_text(s_info_lbl, info);

    if (s_cover_img)
    {
        lv_obj_t *cover_left = lv_obj_get_parent(s_cover_img);
        if (xTouchConfig.xTouchHideAllThumbnails)
        {
            lv_obj_add_flag(s_cover_img, LV_OBJ_FLAG_HIDDEN);
            if (cover_left)
                lv_obj_add_flag(cover_left, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            if (cover_left)
                lv_obj_clear_flag(cover_left, LV_OBJ_FLAG_HIDDEN);
            if (xtouch_history_reprint_cover_dsc != NULL)
            {
                lv_img_set_src(s_cover_img, (const lv_img_dsc_t *)xtouch_history_reprint_cover_dsc);
                if (cover_left)
                    ui_img_zoom_to_fit_box(s_cover_img, lv_obj_get_width(cover_left), lv_obj_get_height(cover_left));
                else
                    ui_img_zoom_to_fit_box(s_cover_img, HRPRINT_THUMB_W, HRPRINT_THUMB_H);
                lv_obj_clear_flag(s_cover_img, LV_OBJ_FLAG_HIDDEN);
                lv_obj_invalidate(s_cover_img);
            }
            else
            {
                lv_obj_add_flag(s_cover_img, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

static void append_slot_line(char *buf, size_t buf_sz, int *n, int max_opts,
                             uint8_t *out_ams, uint8_t *out_tray, uint8_t *out_loaded, uint8_t *out_selectable,
                             int ams_id, int tray_id, char *tt, int is_loaded, int is_selectable)
{
    if (*n >= max_opts || strlen(buf) + 48 >= buf_sz)
        return;
    out_ams[*n] = (uint8_t)ams_id;
    out_tray[*n] = (uint8_t)tray_id;
    out_loaded[*n] = is_loaded ? 1u : 0u;
    out_selectable[*n] = is_selectable ? 1u : 0u;
    size_t len = strlen(buf);
    if (*n > 0)
    {
        buf[len++] = '\n';
        buf[len] = '\0';
    }
    static const char ams_unit_lbl[] = { 'A', 'B', 'C', 'D' };
    char ul = (ams_id >= 0 && ams_id < XTOUCH_BAMBU_AMS_UNITS) ? ams_unit_lbl[ams_id] : '?';
    char line[56];
    if (is_loaded)
        snprintf(line, sizeof(line), "AMS-%c Slot-%d %s", ul, tray_id + 1, (tt && tt[0]) ? tt : "?");
    else
        snprintf(line, sizeof(line), "AMS-%c Slot-%d X", ul, tray_id + 1);
    strlcpy(buf + len, line, buf_sz - len);
    (*n)++;
}

/** Reprint 用スロット候補:
 *  - EXT(254) は表示しない
 *  - AMS ごとに S1..S4 を固定表示（未装填は X）
 *  - 表示順は AMS1(4個) -> AMS2(4個) -> ... */
static int build_slot_options(const char *type_filter, char *buf, size_t buf_sz,
                              uint8_t *out_ams, uint8_t *out_tray, uint8_t *out_loaded, uint8_t *out_selectable, int max_opts)
{
    int n = 0;
    buf[0] = '\0';

    long ams_exist_bits = xtouch_reprint_ams_exist_bits();
    if (ams_exist_bits != 0)
    {
        for (int ams_id = 0; ams_id < XTOUCH_BAMBU_AMS_UNITS; ams_id++)
        {
            if (((ams_exist_bits >> ams_id) & 1) == 0)
                continue;
            for (int tray_id = 0; tray_id < XTOUCH_BAMBU_AMS_SLOTS_PER_UNIT; tray_id++)
            {
                uint32_t st = (uint32_t)get_tray_status_reprint((uint8_t)ams_id, (uint8_t)tray_id);
                int loaded = ((st & 1) != 0);
                char *tt = get_tray_type_reprint((uint8_t)ams_id, (uint8_t)tray_id);
                int selectable = 0;
                if (loaded)
                {
                    if (!type_filter || !type_filter[0])
                        selectable = 1;
                    else if (tt && tt[0] && strcmp(tt, "null") != 0 && strcasecmp(tt, type_filter) == 0)
                        selectable = 1;
                }
                append_slot_line(buf, buf_sz, &n, max_opts, out_ams, out_tray, out_loaded, out_selectable,
                                 ams_id, tray_id, tt, loaded, selectable);
            }
        }
    }

    if (n == 0)
    {
        strlcpy(buf, "(no slot)", buf_sz);
        out_ams[0] = 0;
        out_tray[0] = 0;
        out_loaded[0] = 0;
        out_selectable[0] = 0;
        return 1;
    }
    return n;
}

static lv_color_t reprint_color_from_rrggbbaa(const char *hex)
{
    unsigned rgb = 0;
    if (hex && strlen(hex) >= 6)
        (void)sscanf(hex, "%6x", &rgb);
    return lv_color_hex(rgb & 0xFFFFFFu);
}

static lv_color_t reprint_text_color_for_rrggbb_bg(const char *hex)
{
    unsigned rgb = 0;
    if (hex && strlen(hex) >= 6)
        (void)sscanf(hex, "%6x", &rgb);
    unsigned r = (rgb >> 16) & 0xFFu;
    unsigned g = (rgb >> 8) & 0xFFu;
    unsigned b = rgb & 0xFFu;
    /* Perceived luminance (0..255) */
    unsigned y = (299u * r + 587u * g + 114u * b) / 1000u;
    return (y < 128u) ? lv_color_hex(0xFFFFFF) : lv_color_hex(0x111111);
}

static void on_map_slot_box_clicked(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;
    uintptr_t ud = (uintptr_t)lv_event_get_user_data(e);
    int mi = (int)((ud >> 16) & 0x1F);
    uint8_t ams = (uint8_t)((ud >> 8) & 0xFFu);
    uint8_t tray = (uint8_t)(ud & 0xFFu);
    s_pending_pick_ams[mi] = ams;
    s_pending_pick_tray[mi] = tray;
    s_pending_pick_valid[mi] = 1;

    /* Set押下までは仮選択として右ペインのみ更新 */
    if (mi == s_active_map_idx)
        rebuild_right_selector();
}

static void on_map_set_clicked(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;
    int mi = s_active_map_idx;
    if (mi < 0 || mi >= s_map_row_count || !s_pending_pick_valid[mi])
        return;

    uint8_t ams = s_pending_pick_ams[mi];
    uint8_t tray = s_pending_pick_tray[mi];
    struct XTOUCH_MESSAGE_DATA p;
    p.data = (unsigned long long)mi;
    p.data2 = (unsigned long long)(ams & 0xFFu) | ((unsigned long long)(tray & 0xFFu) << 8);
    lv_msg_send(XTOUCH_HISTORY_REPRINT_SLOT_PICKED, &p);

    if (s_map_left_target_obj[mi])
    {
        lv_obj_set_style_bg_color(
            s_map_left_target_obj[mi],
            reprint_color_from_rrggbbaa(get_tray_color_safe(ams, tray)),
            LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (s_map_left_target_lbl[mi])
    {
        char pick_txt[12];
        format_ams_slot_short(pick_txt, sizeof(pick_txt), ams, tray);
        lv_label_set_text(s_map_left_target_lbl[mi], pick_txt);
    }
    rebuild_right_selector();
}

static void on_map_left_card_clicked(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;
    int mi = (int)(intptr_t)lv_event_get_user_data(e);
    if (mi < 0 || mi >= s_map_row_count)
        return;

    s_active_map_idx = mi;
    /* 左カード切替では pending を確定値で上書きしない（右AMSの選択表示リセットを防ぐ） */
    for (int i = 0; i < s_map_row_count && i < MAP_DD_MAX; i++)
    {
        if (!s_map_left_card_obj[i])
            continue;
        int sel = (i == s_active_map_idx);
        lv_obj_set_style_border_color(s_map_left_card_obj[i], sel ? lv_color_hex(0xFF6666) : lv_color_hex(0xAA3333),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(s_map_left_card_obj[i], sel ? 2 : 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    rebuild_right_selector();
}

static void rebuild_right_selector(void)
{
    if (!s_map_right_panel)
        return;
    lv_obj_clean(s_map_right_panel);
    if (s_active_map_idx < 0 || s_active_map_idx >= s_map_row_count)
        return;

    int mi = s_active_map_idx;
    if (!s_pending_pick_valid[mi])
    {
        s_pending_pick_ams[mi] = xtouch_history_reprint_pick_ams[mi];
        s_pending_pick_tray[mi] = xtouch_history_reprint_pick_tray[mi];
        s_pending_pick_valid[mi] = 1;
    }

    char title_buf[80];
    const xtouch_history_ams_map_t *mapm = NULL;
    if (xtouch_history_selected_ams_map_count > 0 && mi < xtouch_history_selected_ams_map_count)
        mapm = &xtouch_history_selected_ams_map[mi];
    /* プリンタ切替や push_status 反映直後でも、右ペインの候補は都度最新状態で再計算する */
    {
        const char *want_type = (mapm && mapm->filamentType[0]) ? mapm->filamentType : NULL;
        static char s_right_opt_buf[MAP_OPT_BUF];
        s_map_nopt[mi] = build_slot_options(
            want_type,
            s_right_opt_buf,
            sizeof(s_right_opt_buf),
            s_map_pick_ams[mi],
            s_map_pick_tray[mi],
            s_map_pick_loaded[mi],
            s_map_pick_selectable[mi],
            MAP_OPT_MAX);
    }
    snprintf(title_buf, sizeof(title_buf), "color-%d %s", mi + 1, (mapm && mapm->filamentType[0]) ? mapm->filamentType : "");
    lv_obj_t *top_wrap = lv_obj_create(s_map_right_panel);
    lv_obj_set_width(top_wrap, lv_pct(100));
    lv_obj_set_height(top_wrap, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(top_wrap, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_wrap, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(top_wrap, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(top_wrap, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(top_wrap, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(top_wrap, HRPRINT_UI_PAD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(top_wrap, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *info = lv_obj_create(top_wrap);
    lv_obj_set_flex_grow(info, 1);
    lv_obj_set_height(info, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(info, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(info, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(info, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(info, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *row1 = lv_obj_create(info);
    lv_obj_set_width(row1, lv_pct(100));
    lv_obj_set_height(row1, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(row1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(row1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(row1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(row1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *title = lv_label_create(row1);
    lv_obj_set_width(title, lv_pct(100));
    lv_obj_set_height(title, LV_SIZE_CONTENT);
    lv_label_set_text(title, title_buf);
    lv_obj_set_style_text_color(title, lv_color_hex(0xDDDDDD), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(title, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_long_mode(title, LV_LABEL_LONG_SCROLL_CIRCULAR);

    lv_obj_t *row2 = lv_obj_create(info);
    lv_obj_set_width(row2, lv_pct(100));
    lv_obj_set_height(row2, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row2, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(row2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(row2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(row2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(row2, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(row2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *sl = lv_label_create(row2);
    lv_label_set_text(sl, "Selected:");
#if !defined(__XTOUCH_SCREEN_S3_050__)
    lv_obj_add_flag(sl, LV_OBJ_FLAG_HIDDEN); /* 2.8" は幅のため文言を出さない */
#endif
    lv_obj_set_style_text_color(sl, lv_color_hex(0xBBBBBB), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(sl, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *selected_swatch = reprint_make_color_rect(
        row2,
        reprint_color_from_rrggbbaa((mapm && mapm->sourceColor[0]) ? mapm->sourceColor : "808080FF"),
        HRPRINT_SRC_SWATCH_W, HRPRINT_SLOT_SQ_H, 1, lv_color_hex(0x999999));
    /* 候補セルと同様、横幅は1/4基準に合わせる */
    lv_obj_set_width(selected_swatch, lv_pct(25));

    lv_obj_t *set_btn = lv_btn_create(top_wrap);
    lv_obj_set_width(set_btn, HRPRINT_SET_BTN_W);
    lv_obj_set_height(set_btn, HRPRINT_SET_BTN_H);
    lv_obj_set_style_pad_left(set_btn, HRPRINT_SET_BTN_PAD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(set_btn, HRPRINT_SET_BTN_PAD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(set_btn, lv_color_hex(0xAA3333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(set_btn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(set_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(set_btn, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(set_btn, on_map_set_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_t *set_lbl = lv_label_create(set_btn);
    lv_label_set_text(set_lbl, "SET");
    lv_obj_set_style_text_color(set_lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(set_lbl, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(set_lbl);

    lv_obj_t *grid = lv_obj_create(s_map_right_panel);
    lv_obj_set_width(grid, lv_pct(100));
    lv_obj_set_flex_grow(grid, 1);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(grid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(grid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(grid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(grid, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(grid, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(grid, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *ams_panel[XTOUCH_BAMBU_AMS_UNITS] = {0};
    lv_obj_t *ams_slots[XTOUCH_BAMBU_AMS_UNITS] = {0};
    static const char ams_unit_lbl[] = {'A', 'B', 'C', 'D'};

    /* AMSごとの枠を先に作る（横幅100%で縦に積む）。存在ビットが無い場合は後段で必要分だけ作る。 */
    for (int ams = 0; ams < XTOUCH_BAMBU_AMS_UNITS; ams++)
    {
        if (((xtouch_reprint_ams_exist_bits() >> ams) & 1u) == 0)
            continue;
        lv_obj_t *panel = lv_obj_create(grid);
        lv_obj_set_width(panel, lv_pct(100));
        lv_obj_set_height(panel, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_bg_color(panel, lv_color_hex(0x202020), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(panel, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(panel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(panel, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_row(panel, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

        char ams_text[10];
        snprintf(ams_text, sizeof(ams_text), "AMS-%c", ams_unit_lbl[ams]);
        lv_obj_t *pl = lv_label_create(panel);
        lv_label_set_text(pl, ams_text);
        lv_obj_set_style_text_color(pl, lv_color_hex(0xDDDDDD), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(pl, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *slots = lv_obj_create(panel);
        lv_obj_set_width(slots, lv_pct(100));
        lv_obj_set_height(slots, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(slots, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(slots, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_bg_opa(slots, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(slots, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(slots, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_column(slots, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_row(slots, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(slots, LV_OBJ_FLAG_SCROLLABLE);

        ams_panel[ams] = panel;
        ams_slots[ams] = slots;
    }

    for (int si = 0; si < s_map_nopt[mi] && si < MAP_OPT_MAX; si++)
    {
        uint8_t ams = s_map_pick_ams[mi][si];
        uint8_t tray = s_map_pick_tray[mi][si];
        uint8_t selectable = s_map_pick_selectable[mi][si];
        const char *tc = get_tray_color_reprint(ams, tray);
        lv_color_t sc = reprint_color_from_rrggbbaa(tc && strlen(tc) >= 6 ? tc : "808080FF");
        int sel = selectable && (s_pending_pick_ams[mi] == ams && s_pending_pick_tray[mi] == tray);
        lv_color_t bcol = sel ? lv_color_hex(0xFF4444) : lv_color_hex(0x666666);
        int bw = sel ? 3 : 2;
        if (ams < XTOUCH_BAMBU_AMS_UNITS && !ams_panel[ams])
        {
            lv_obj_t *panel = lv_obj_create(grid);
            lv_obj_set_width(panel, lv_pct(100));
            lv_obj_set_height(panel, LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
            lv_obj_set_style_bg_color(panel, lv_color_hex(0x202020), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(panel, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(panel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_all(panel, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_row(panel, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

            char ams_text[10];
            char au = ams_unit_lbl[ams];
            snprintf(ams_text, sizeof(ams_text), "AMS-%c", au);
            lv_obj_t *pl = lv_label_create(panel);
            lv_label_set_text(pl, ams_text);
            lv_obj_set_style_text_color(pl, lv_color_hex(0xDDDDDD), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(pl, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

            lv_obj_t *slots = lv_obj_create(panel);
            lv_obj_set_width(slots, lv_pct(100));
            lv_obj_set_height(slots, LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(slots, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(slots, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
            lv_obj_set_style_bg_opa(slots, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(slots, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_all(slots, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_column(slots, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_row(slots, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_flag(slots, LV_OBJ_FLAG_SCROLLABLE);

            ams_panel[ams] = panel;
            ams_slots[ams] = slots;
        }

        lv_obj_t *parent_slots = (ams < XTOUCH_BAMBU_AMS_UNITS && ams_slots[ams]) ? ams_slots[ams] : grid;
        lv_obj_t *cell = lv_obj_create(parent_slots);
        /* 常に4分割幅を基準にし、候補数が4未満でも1/4より広げない */
        lv_obj_set_width(cell, lv_pct(25));
        lv_obj_set_height(cell, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(cell, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(cell, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_bg_opa(cell, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(cell, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(cell, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_row(cell, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *sq = reprint_make_color_rect(cell, sc, HRPRINT_SRC_SWATCH_W, HRPRINT_SLOT_SQ_H, bw, bcol);
        s_map_slot_obj[mi][si] = sq;
        /* 色■をセル幅いっぱいにし、タッチはセル全体で受ける */
        lv_obj_set_width(sq, lv_pct(100));
        lv_obj_set_height(sq, HRPRINT_SLOT_SQ_H);
        /* lv_obj は既定で CLICKABLE。子が全面を覆うとターゲットが sq になり親 cell の LV_EVENT_CLICKED が届かない */
        lv_obj_clear_flag(sq, LV_OBJ_FLAG_CLICKABLE);
        if (selectable)
        {
            uintptr_t ud = ((uintptr_t)(mi & 0x1F) << 16) | ((uintptr_t)ams << 8) | (uintptr_t)tray;
            lv_obj_add_flag(cell, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(cell, on_map_slot_box_clicked, LV_EVENT_CLICKED, (void *)ud);
        }
        else
        {
            lv_obj_t *xlab = lv_label_create(sq);
            lv_label_set_text(xlab, "X");
            lv_obj_set_style_text_color(xlab, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(xlab, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_center(xlab);
        }

    }

}

static lv_obj_t *reprint_make_color_square(lv_obj_t *parent, lv_color_t col, int side, int border_w, lv_color_t border_col)
{
    lv_obj_t *box = lv_obj_create(parent);
    lv_obj_set_size(box, side, side);
    lv_obj_set_style_radius(box, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(box, col, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(box, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(box, border_w, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(box, border_col, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(box, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
    return box;
}

static lv_obj_t *reprint_make_color_rect(lv_obj_t *parent, lv_color_t col, int w, int h, int border_w, lv_color_t border_col)
{
    lv_obj_t *box = lv_obj_create(parent);
    lv_obj_set_size(box, w, h);
    lv_obj_set_style_radius(box, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(box, col, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(box, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(box, border_w, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(box, border_col, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(box, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
    return box;
}

static void on_reprint_start(lv_event_t *e)
{
    (void)e;
    if (!xtouch_history_reprint_task_id_valid)
        return;

    if (xtouch_history_selected_ams_map_count <= 0 && s_map_row_count <= 0)
        return;

    int printer_slot = 0;
    if (s_printer_dd)
    {
        int sel = (int)lv_dropdown_get_selected(s_printer_dd);
        if (sel >= 0 && sel < s_printer_dd_count)
            printer_slot = s_printer_dd_to_slot[sel];
    }

    struct XTOUCH_MESSAGE_DATA payload;
    payload.data = 0; /* task_id 経路で再印刷するので index は不要 */
    payload.data2 = (unsigned long long)printer_slot;
    lv_msg_send(XTOUCH_HISTORY_REPRINT_WITH_OPTIONS, &payload);
}

static void on_reprint_cancel(lv_event_t *e)
{
    (void)e;
    loadScreen(15);
}

lv_obj_t *ui_historyReprintComponent_create(lv_obj_t *comp_parent)
{
    hrprint_apply_metrics_by_resolution();
    s_map_row_count = 0;
    memset(s_map_slot_obj, 0, sizeof(s_map_slot_obj));

    lv_obj_t *root = lv_obj_create(comp_parent);
    lv_obj_set_width(root, lv_pct(100));
    lv_obj_set_height(root, lv_pct(100));
    lv_obj_set_flex_grow(root, 1);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(root, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(root, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(root, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(root, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(root, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    {
        lv_obj_t *row = lv_obj_create(root);
        lv_obj_set_width(row, lv_pct(100));
        lv_obj_set_height(row, HRPRINT_TOP_ROW_H + 8);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        /* サムネとタイトル行頭を揃える */
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_bg_opa(row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(row, 4, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *left = lv_obj_create(row);
        lv_obj_set_width(left, HRPRINT_THUMB_W);
        lv_obj_set_height(left, HRPRINT_THUMB_H);
        lv_obj_set_style_bg_color(left, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(left, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(left, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(left, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_pad_all(left, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        s_cover_img = lv_img_create(left);
        lv_obj_center(s_cover_img);
        /* History 行と同じ: VIRTUAL + 枠と同寸でアスペクトを保ちつつ枠内に収める（zoom は付けない） */
        lv_img_set_size_mode(s_cover_img, LV_IMG_SIZE_MODE_VIRTUAL);
        lv_obj_set_size(s_cover_img, HRPRINT_THUMB_W, HRPRINT_THUMB_H);
        lv_obj_add_flag(s_cover_img, LV_OBJ_FLAG_HIDDEN);

        lv_obj_t *right = lv_obj_create(row);
        lv_obj_set_flex_grow(right, 1);
        lv_obj_set_height(right, lv_pct(100));
        lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(right, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_bg_opa(right, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(right, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(right, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *title = lv_label_create(right);
        s_title_lbl = title;
        lv_label_set_text(title, "Reprint");
        lv_obj_set_width(title, lv_pct(100));
        lv_obj_set_height(title, LV_SIZE_CONTENT);
#if defined(__XTOUCH_SCREEN_S3_050__)
        lv_obj_set_style_min_height(title, 34, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(title, &lv_font_notosans_28, LV_PART_MAIN | LV_STATE_DEFAULT);
#else
        lv_obj_set_style_min_height(title, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(title, &lv_font_notosans_14, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif
        lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_line_space(title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        /* 長いジョブ名: Home / History 行と同じ lv_label long_mode（API 名は LV_LABEL_LONG_SCROLL_CIRCULAR） */
        lv_label_set_long_mode(title, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_style_pad_all(title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *info = lv_label_create(right);
        s_info_lbl = info;
        lv_label_set_text(info, "");
        lv_obj_set_width(info, lv_pct(100));
        lv_obj_set_height(info, LV_SIZE_CONTENT);
        lv_label_set_long_mode(info, LV_LABEL_LONG_SCROLL_CIRCULAR);
#if defined(__XTOUCH_SCREEN_S3_050__)
        lv_obj_set_style_text_font(info, &lv_font_notosans_28, LV_PART_MAIN | LV_STATE_DEFAULT);
#else
        lv_obj_set_style_text_font(info, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif
        lv_obj_set_style_text_color(info, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    lv_obj_t *form = lv_obj_create(root);
    lv_obj_set_width(form, lv_pct(100));
    lv_obj_set_flex_grow(form, 1);
    lv_obj_set_flex_flow(form, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(form, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(form, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(form, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(form, HRPRINT_RIGHT_UI_PAD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(form, HRPRINT_RIGHT_UI_PAD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(form, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(form, LV_OBJ_FLAG_SCROLLABLE);
    s_form_obj = form;
    if (s_form_scroll_y != 0)
        lv_obj_scroll_to_y(form, s_form_scroll_y, LV_ANIM_OFF);

    static char opt_buf[MAP_DD_MAX][MAP_OPT_BUF];
    /* -1=取得中, >=0=取得完了（0件も「完了」として Loading をやめる） */
    const int map_cnt = xtouch_history_selected_ams_map_count;
    int nrows = 1;
    if (map_cnt > 0)
        nrows = map_cnt;
    if (nrows > MAP_DD_MAX)
        nrows = MAP_DD_MAX;
    s_map_row_count = nrows;

    lv_obj_t *maps_split = lv_obj_create(form);
    lv_obj_set_width(maps_split, lv_pct(100));
    lv_obj_set_flex_grow(maps_split, 1);
    lv_obj_set_flex_flow(maps_split, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(maps_split, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(maps_split, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(maps_split, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(maps_split, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(maps_split, HRPRINT_RIGHT_UI_PAD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(maps_split, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *left_pane = lv_obj_create(maps_split);
    /* 左:右 = 1:2（右のスロット選択が潰れないように） */
    lv_obj_set_width(left_pane, lv_pct(HRPRINT_LEFT_PANE_W_PCT));
    lv_obj_set_height(left_pane, lv_pct(100));
    lv_obj_set_flex_flow(left_pane, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(left_pane, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(left_pane, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(left_pane, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(left_pane, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(left_pane, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(left_pane, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(left_pane, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(left_pane, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *right_pane = lv_obj_create(maps_split);
    lv_obj_set_width(right_pane, lv_pct(HRPRINT_RIGHT_PANE_W_PCT));
    lv_obj_set_height(right_pane, lv_pct(100));
    lv_obj_set_flex_flow(right_pane, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(right_pane, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_color(right_pane, lv_color_hex(0x262626), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(right_pane, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(right_pane, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(right_pane, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(right_pane, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(right_pane, HRPRINT_RIGHT_UI_PAD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(right_pane, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(right_pane, HRPRINT_RIGHT_UI_PAD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(right_pane, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *right_body = lv_obj_create(right_pane);
    lv_obj_set_width(right_body, lv_pct(100));
    lv_obj_set_flex_grow(right_body, 1);
    lv_obj_set_flex_flow(right_body, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(right_body, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(right_body, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(right_body, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(right_body, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(right_body, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(right_body, LV_OBJ_FLAG_SCROLLABLE);
    s_map_right_panel = right_body;

    lv_obj_t *right_footer = lv_obj_create(right_pane);
    lv_obj_set_width(right_footer, lv_pct(100));
    lv_obj_set_height(right_footer, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(right_footer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(right_footer, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(right_footer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(right_footer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(right_footer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(right_footer, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(right_footer, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(right_footer, HRPRINT_RIGHT_UI_PAD, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(right_footer, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *cancelBtn = lv_btn_create(right_footer);
#if defined(__XTOUCH_SCREEN_S3_050__)
    lv_obj_set_width(cancelBtn, LV_SIZE_CONTENT);
    lv_obj_set_height(cancelBtn, LV_SIZE_CONTENT);
    lv_obj_set_style_min_width(cancelBtn, 136, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_hor(cancelBtn, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_ver(cancelBtn, HRPRINT_FOOT_BTN_PAD_V, LV_PART_MAIN | LV_STATE_DEFAULT);
#else
    lv_obj_set_width(cancelBtn, lv_pct(42));
    lv_obj_set_height(cancelBtn, LV_SIZE_CONTENT);
    lv_obj_set_style_min_height(cancelBtn, HRPRINT_FOOT_BTN_MIN_H, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cancelBtn, HRPRINT_UI_FOOT_BTN_V, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cancelBtn, HRPRINT_UI_FOOT_BTN_V, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif
    lv_obj_add_event_cb(cancelBtn, on_reprint_cancel, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_border_width(cancelBtn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(cancelBtn, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *cancelLbl = lv_label_create(cancelBtn);
    lv_label_set_text(cancelLbl, "Cancel");
    lv_obj_set_style_text_font(cancelLbl, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cancelLbl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(cancelLbl);

    lv_obj_t *okBtn = lv_btn_create(right_footer);
#if defined(__XTOUCH_SCREEN_S3_050__)
    lv_obj_set_width(okBtn, LV_SIZE_CONTENT);
    lv_obj_set_height(okBtn, LV_SIZE_CONTENT);
    lv_obj_set_style_min_width(okBtn, 152, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_hor(okBtn, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_ver(okBtn, HRPRINT_FOOT_BTN_PAD_V, LV_PART_MAIN | LV_STATE_DEFAULT);
#else
    lv_obj_set_width(okBtn, lv_pct(42));
    lv_obj_set_height(okBtn, LV_SIZE_CONTENT);
    lv_obj_set_style_min_height(okBtn, HRPRINT_FOOT_BTN_MIN_H, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(okBtn, HRPRINT_UI_FOOT_BTN_V, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(okBtn, HRPRINT_UI_FOOT_BTN_V, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif
    lv_obj_add_event_cb(okBtn, on_reprint_start, LV_EVENT_CLICKED, NULL);
    s_start_btn = okBtn;
    lv_obj_set_style_border_width(okBtn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(okBtn, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *okLbl = lv_label_create(okBtn);
    lv_label_set_text(okLbl, "Start");
    lv_obj_set_style_text_font(okLbl, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(okLbl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(okLbl);

    for (int mi = 0; mi < nrows; mi++)
    {
        const char *want_type = NULL;
        const xtouch_history_ams_map_t *mapm = NULL;
        if (map_cnt > 0)
        {
            mapm = &xtouch_history_selected_ams_map[mi];
            want_type = mapm->filamentType[0] ? mapm->filamentType : NULL;
        }

        lv_obj_t *card = lv_obj_create(left_pane);
        /* 左ペインは画面幅の一部のみ。23% だと実幅が極細になるため 2.8" は1列フル、5" は2列 */
#if defined(__XTOUCH_SCREEN_S3_050__)
        lv_obj_set_width(card, lv_pct(48));
#else
        lv_obj_set_width(card, lv_pct(100));
#endif
        lv_obj_set_height(card, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_all(card, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_row(card, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(card, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(card, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(card, lv_color_hex(0xAA3333), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(card, lv_color_hex(0x1a1a1a), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(card, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

        if (map_cnt > 0 && mapm)
        {
            lv_color_t hdr_bg = reprint_color_from_rrggbbaa(mapm->sourceColor);
            lv_obj_t *hdr = lv_obj_create(card);
            lv_obj_set_width(hdr, lv_pct(100));
            lv_obj_set_height(hdr, HRPRINT_CARD_HDR_H);
            lv_obj_set_style_radius(hdr, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(hdr, hdr_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(hdr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(hdr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(hdr, HRPRINT_UI_PAD, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_flag(hdr, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_add_flag(hdr, LV_OBJ_FLAG_EVENT_BUBBLE);
            lv_obj_t *ht = lv_label_create(hdr);
            lv_label_set_text(ht, mapm->filamentType[0] ? mapm->filamentType : "?");
            lv_obj_set_style_text_color(ht, reprint_text_color_for_rrggbb_bg(mapm->sourceColor), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(ht, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_align(ht, LV_ALIGN_LEFT_MID, 0, 0);

            s_map_nopt[mi] = build_slot_options(want_type, opt_buf[mi], sizeof(opt_buf[mi]),
                                                s_map_pick_ams[mi], s_map_pick_tray[mi], s_map_pick_loaded[mi], s_map_pick_selectable[mi], MAP_OPT_MAX);
            lv_obj_t *body = lv_obj_create(card);
            lv_obj_set_width(body, lv_pct(100));
            lv_obj_set_height(body, LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(body, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(body, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
            lv_obj_set_style_pad_column(body, HRPRINT_UI_PAD, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_all(body, HRPRINT_UI_PAD, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_row(body, HRPRINT_UI_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(body, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(body, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_flag(body, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_add_flag(body, LV_OBJ_FLAG_EVENT_BUBBLE);
            const char *picked = get_tray_color_safe(xtouch_history_reprint_pick_ams[mi], xtouch_history_reprint_pick_tray[mi]);
            s_map_left_target_obj[mi] = reprint_make_color_rect(body, reprint_color_from_rrggbbaa(picked), 0, HRPRINT_LEFT_TARGET_H, 1, lv_color_hex(0x888888));
            lv_obj_set_width(s_map_left_target_obj[mi], lv_pct(100));
            lv_obj_set_height(s_map_left_target_obj[mi], HRPRINT_LEFT_TARGET_H);
            lv_obj_set_style_min_height(s_map_left_target_obj[mi], HRPRINT_LEFT_TARGET_H, LV_PART_MAIN | LV_STATE_DEFAULT);
            char pick_txt[12];
            format_ams_slot_short(pick_txt, sizeof(pick_txt), xtouch_history_reprint_pick_ams[mi], xtouch_history_reprint_pick_tray[mi]);
            lv_obj_t *pl = lv_label_create(s_map_left_target_obj[mi]);
            lv_label_set_text(pl, pick_txt);
            lv_obj_set_style_text_color(pl, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(pl, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_center(pl);
            s_map_left_target_lbl[mi] = pl;
            s_pending_pick_ams[mi] = xtouch_history_reprint_pick_ams[mi];
            s_pending_pick_tray[mi] = xtouch_history_reprint_pick_tray[mi];
            s_pending_pick_valid[mi] = 1;

            lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(card, on_map_left_card_clicked, LV_EVENT_CLICKED, (void *)(intptr_t)mi);
            s_map_left_card_obj[mi] = card;
            int sel_card = (mi == s_active_map_idx);
            lv_obj_set_style_border_color(card, sel_card ? lv_color_hex(0xFF6666) : lv_color_hex(0xAA3333),
                                          LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(card, sel_card ? 2 : 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        else
        {
            lv_obj_t *lab = lv_label_create(card);
            if (map_cnt < 0)
                lv_label_set_text(lab, "Loading...");
            else
                lv_label_set_text(lab, "No filament data in task detail.");
            lv_obj_set_style_text_font(lab, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_all(lab, HRPRINT_UI_PAD, LV_PART_MAIN | LV_STATE_DEFAULT);
            s_map_nopt[mi] = 0;
        }
    }
    rebuild_right_selector();

    if (xtouch_other_printer_count > 0)
    {
        lv_obj_t *row = lv_obj_create(form);
        lv_obj_set_width(row, lv_pct(100));
        lv_obj_set_height(row, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_bg_opa(row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(row, HRPRINT_RIGHT_UI_PAD, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(row, HRPRINT_RIGHT_UI_PAD, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *lbl = lv_label_create(row);
        lv_label_set_text(lbl, "Printer");
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xDDDDDD), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(lbl, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *dd = lv_dropdown_create(row);
        lv_obj_set_width(dd, lv_pct(60));
        s_printer_dd = dd;
        populate_printer_dropdown(dd);
        lv_obj_add_event_cb(dd, on_printer_dd_clicked, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(dd, on_printer_dd_value_changed, LV_EVENT_VALUE_CHANGED, NULL);
        {
            lv_obj_t *lst = lv_dropdown_get_list(dd);
            if (lst)
                lv_obj_set_style_text_font(lst, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    else
    {
        s_printer_dd = NULL;
    }

    populate_summary_panel();

    return root;
}

#else

lv_obj_t *ui_historyReprintComponent_create(lv_obj_t *comp_parent)
{
    (void)comp_parent;
    return NULL;
}

#endif
