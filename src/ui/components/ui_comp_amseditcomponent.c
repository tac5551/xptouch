/**
 * AMS Edit screen component: フィラメント設定
 * string.h / stdlib.h は ui.h → Arduino.h 経由で届く想定
 */
#include "../ui.h"

#define AMSEDIT_OPTS_BUF_SIZE 512

static lv_obj_t *s_dd_brand;
static lv_obj_t *s_dd_type;
static lv_obj_t *s_lbl_temp_min;
static lv_obj_t *s_lbl_temp_max;
static lv_timer_t *s_ams_edit_sd_timer = NULL;

static void schedule_fetch_for_current_selection(void);
static void update_temp_display(void);

/** SD 読みを非同期で実行。完了後に Brand / Type を更新。保存済み選択（色画面から戻ったとき）があれば復元、なければトレイ種別から設定。 */
static void ams_edit_sd_load_timer_cb(lv_timer_t *t)
{
    static char buf[AMSEDIT_OPTS_BUF_SIZE];
    buf[0] = '\0';
    xtouch_filaments_load_for_current_printer_c();
    xtouch_public_filaments_get_brand_options(buf, sizeof(buf));
    if (s_dd_brand)
        lv_dropdown_set_options(s_dd_brand, buf[0] ? buf : "—");
    lv_dropdown_set_selected(s_dd_brand, 0);
    buf[0] = '\0';
    xtouch_public_filaments_get_type_options_by_display_index(0, buf, sizeof(buf));
    if (s_dd_type)
        lv_dropdown_set_options(s_dd_type, buf[0] ? buf : "—");
    lv_dropdown_set_selected(s_dd_type, 0);

    if (ams_edit_current_brand_index >= 0 && ams_edit_current_type_index >= 0)
    {
        /* 色画面から戻ったなど、前回の選択を復元 */
        int bi = ams_edit_current_brand_index;
        int ti = ams_edit_current_type_index;
        lv_dropdown_set_selected(s_dd_brand, bi);
        buf[0] = '\0';
        xtouch_public_filaments_get_type_options_by_display_index(bi, buf, sizeof(buf));
        if (s_dd_type)
            lv_dropdown_set_options(s_dd_type, buf[0] ? buf : "—");
        lv_dropdown_set_selected(s_dd_type, ti);
    }
    else
    {
        /* 初回表示: setting_id 逆引き優先、なければ tray_type で選択。tray_id は 0–3。 */
        uint8_t aid = (ams_edit_current_ams_id == 255) ? 0 : (uint8_t)ams_edit_current_ams_id;
        uint8_t tray_index = (ams_edit_current_ams_id == 255) ? 0 : (uint8_t)ams_edit_current_tray_id;
        int bi = 0, ti = 0;
        int set = 0;
        if (aid < 8)
        {
            const char *setting_id = get_tray_setting_id(aid, tray_index);
            if (setting_id && setting_id[0] != '\0')
            {
                char brand_buf[24], type_buf[24];
                brand_buf[0] = type_buf[0] = '\0';
                if (xtouch_public_filaments_rev_lookup(setting_id, brand_buf, sizeof(brand_buf), type_buf, sizeof(type_buf)))
                {
                    if (xtouch_public_filaments_find_indices_by_brand_and_type(brand_buf, type_buf, &bi, &ti))
                        set = 1;
                }
            }
        }
        if (set)
        {
            lv_dropdown_set_selected(s_dd_brand, bi);
            buf[0] = '\0';
            xtouch_public_filaments_get_type_options_by_display_index(bi, buf, sizeof(buf));
            if (s_dd_type)
                lv_dropdown_set_options(s_dd_type, buf[0] ? buf : "—");
            lv_dropdown_set_selected(s_dd_type, ti);
        }
    }
    /* 現在の選択を保存（色画面から戻ったときに復元するため） */
    ams_edit_current_brand_index = (int)lv_dropdown_get_selected(s_dd_brand);
    ams_edit_current_type_index = (int)lv_dropdown_get_selected(s_dd_type);
    if (ams_edit_current_brand_index < 0) ams_edit_current_brand_index = 0;
    if (ams_edit_current_type_index < 0) ams_edit_current_type_index = 0;

    schedule_fetch_for_current_selection();
    update_temp_display();
    lv_timer_del(t);
    s_ams_edit_sd_timer = NULL;
}

/** 現在の Brand/Type の tray_info_idx で非同期に API 取得を依頼（lv_msg → device の lv_timer）。 */
static void schedule_fetch_for_current_selection(void)
{
    if (!s_dd_brand || !s_dd_type)
        return;
    int brand_idx = (int)lv_dropdown_get_selected(s_dd_brand);
    int type_idx = (int)lv_dropdown_get_selected(s_dd_type);
    if (brand_idx < 0) brand_idx = 0;
    if (type_idx < 0) type_idx = 0;
    static char id_buf[16];
    id_buf[0] = '\0';
    xtouch_public_filaments_get_selected_id_n(brand_idx, type_idx, id_buf, sizeof(id_buf), NULL, 0, NULL, 0, NULL, NULL);
    if (id_buf[0] != '\0')
        lv_msg_send(XTOUCH_COMMAND_AMS_FETCH_SLICER_TEMP, id_buf);
}

static void on_fetched_temp_msg(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_MSG_RECEIVED)
        return;
    update_temp_display();
}

/** Min/Max 温度表示を現在の選択と fetched 値に応じて更新 */
static void update_temp_display(void)
{
    if (!s_lbl_temp_min || !s_lbl_temp_max)
        return;
    int min_val = 0, max_val = 0;
    if (s_dd_brand && s_dd_type)
    {
        int brand_idx = (int)lv_dropdown_get_selected(s_dd_brand);
        int type_idx = (int)lv_dropdown_get_selected(s_dd_type);
        if (brand_idx < 0) brand_idx = 0;
        if (type_idx < 0) type_idx = 0;
        char id_buf[16];
        id_buf[0] = '\0';
        xtouch_public_filaments_get_selected_id_n(brand_idx, type_idx, id_buf, sizeof(id_buf), NULL, 0, NULL, 0, &min_val, &max_val);
        int id_match = (id_buf[0] != '\0' && strcmp(id_buf, ams_edit_fetched_setting_id) == 0);
        if (id_match && (ams_edit_fetched_min > 0 || ams_edit_fetched_max > 0))
        {
            min_val = ams_edit_fetched_min;
            max_val = ams_edit_fetched_max;
        }
    }
    static char buf_min[24], buf_max[24];
    snprintf(buf_min, sizeof(buf_min), "Min:%d", min_val);
    snprintf(buf_max, sizeof(buf_max), "Max:%d", max_val);
    lv_label_set_text(s_lbl_temp_min, buf_min);
    lv_label_set_text(s_lbl_temp_max, buf_max);
}

static void on_back_clicked(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;
    if (s_ams_edit_sd_timer)
    {
        lv_timer_del(s_ams_edit_sd_timer);
        s_ams_edit_sd_timer = NULL;
    }
    loadScreen(7); /* Back to AMS View */
}

static void on_reset_clicked(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;
    /* tray_info_idx を空で登録してスロット情報をリセットする */
    struct XTOUCH_AMS_FILAMENT_SETTING_PAYLOAD payload;
    payload.ams_id = ams_edit_current_ams_id;
    payload.tray_id = ams_edit_current_tray_id;
    payload.nozzle_temp_min = 0;
    payload.nozzle_temp_max = 0;
    payload.tray_info_idx[0] = '\0';          /* tray_info_idx="" */
    snprintf(payload.tray_color, sizeof(payload.tray_color), "00000000"); /* RRGGBBAA デフォルトは透明 */
    payload.tray_type[0] = '\0';              /* tray_type も空にする */
    lv_msg_send(XTOUCH_COMMAND_AMS_FILAMENT_SETTING, &payload);
    if (s_ams_edit_sd_timer)
    {
        lv_timer_del(s_ams_edit_sd_timer);
        s_ams_edit_sd_timer = NULL;
    }
    loadScreen(7); /* Back to AMS View */
}

static void on_save_clicked(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;
    int brand_idx = (int)lv_dropdown_get_selected(s_dd_brand);
    int type_idx = (int)lv_dropdown_get_selected(s_dd_type);
    if (brand_idx < 0) brand_idx = 0;
    if (type_idx < 0) type_idx = 0;
    char id_buf[16];
    char n_buf[24];
    char type_buf[16];
    int nozzle_temp_min = 0, nozzle_temp_max = 0;
    id_buf[0] = n_buf[0] = type_buf[0] = '\0';
    xtouch_public_filaments_get_selected_id_n(brand_idx, type_idx, id_buf, sizeof(id_buf), n_buf, sizeof(n_buf), type_buf, sizeof(type_buf), &nozzle_temp_min, &nozzle_temp_max);
    /* MQTT に送る温度は Cloud で取ってきた値を使う（id 一致時は ams_edit_fetched_* を優先） */
    int id_match = (id_buf[0] != '\0' && strcmp(id_buf, ams_edit_fetched_setting_id) == 0);
    if (id_match && (ams_edit_fetched_min > 0 || ams_edit_fetched_max > 0))
    {
        nozzle_temp_min = ams_edit_fetched_min;
        nozzle_temp_max = ams_edit_fetched_max;
    }
    /* #region agent log */
    xtouch_debug_log_ams_save(id_buf, ams_edit_fetched_setting_id, id_match, ams_edit_fetched_min, ams_edit_fetched_max, nozzle_temp_min, nozzle_temp_max);
    /* #endregion */
    struct XTOUCH_AMS_FILAMENT_SETTING_PAYLOAD payload;
    payload.ams_id = ams_edit_current_ams_id;
    payload.tray_id = ams_edit_current_tray_id;
    payload.nozzle_temp_min = nozzle_temp_min;
    payload.nozzle_temp_max = nozzle_temp_max;
    /* setting_id（MQTT）は純正同様に _XX を除いた形（GFSL99）で送る。tray_info_idx（MQTT）には filament_id（GFL99）を送る（device.h で payload.filament_id を使用）。 */
    if (id_match && ams_edit_fetched_setting_id[0] != '\0')
        snprintf(payload.tray_info_idx, sizeof(payload.tray_info_idx), "%s", ams_edit_fetched_setting_id);
    else
        snprintf(payload.tray_info_idx, sizeof(payload.tray_info_idx), "%s", id_buf[0] ? id_buf : "GFL03");
    if (ams_edit_fetched_filament_id[0] != '\0')
        snprintf(payload.filament_id, sizeof(payload.filament_id), "%s", ams_edit_fetched_filament_id);
    else
        snprintf(payload.filament_id, sizeof(payload.filament_id), "%s", payload.tray_info_idx);
    snprintf(payload.tray_color, sizeof(payload.tray_color), "%s", ams_edit_current_tray_color);
    snprintf(payload.tray_type, sizeof(payload.tray_type), "%s", type_buf[0] ? type_buf : "PLA");
    lv_msg_send(XTOUCH_COMMAND_AMS_FILAMENT_SETTING, &payload);
    if (s_ams_edit_sd_timer)
    {
        lv_timer_del(s_ams_edit_sd_timer);
        s_ams_edit_sd_timer = NULL;
    }
    loadScreen(7); /* Back to AMS View */
}

static void on_color_btn_clicked(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;
    loadScreen(14); /* 色パレット画面へ */
}

static lv_obj_t *add_row_label(lv_obj_t *parent, const char *text)
{
    lv_obj_t *lbl = lv_label_create(parent);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(lbl, 100);
    return lbl;
}

static void on_filament_selection_changed(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
        return;
    if (s_dd_brand && s_dd_type)
    {
        ams_edit_current_brand_index = (int)lv_dropdown_get_selected(s_dd_brand);
        ams_edit_current_type_index = (int)lv_dropdown_get_selected(s_dd_type);
        if (ams_edit_current_brand_index < 0) ams_edit_current_brand_index = 0;
        if (ams_edit_current_type_index < 0) ams_edit_current_type_index = 0;
    }
    schedule_fetch_for_current_selection();
    update_temp_display();
}

static void on_brand_changed(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
        return;
    lv_obj_t *dd_brand = lv_event_get_target(e);
    lv_obj_t *dd_type = (lv_obj_t *)lv_event_get_user_data(e);
    if (!dd_type)
        return;
    int idx = (int)lv_dropdown_get_selected(dd_brand);
    if (idx < 0)
        idx = 0;
    static char buf[AMSEDIT_OPTS_BUF_SIZE];
    buf[0] = '\0';
    xtouch_public_filaments_get_type_options_by_display_index(idx, buf, sizeof(buf));
    lv_dropdown_set_options(dd_type, buf[0] ? buf : "—");
    lv_dropdown_set_selected(dd_type, 0);
    update_temp_display();
    if (s_dd_brand && s_dd_type)
    {
        ams_edit_current_brand_index = (int)lv_dropdown_get_selected(s_dd_brand);
        ams_edit_current_type_index = 0;
    }
    schedule_fetch_for_current_selection();
}

void ui_amsEditComponent_refresh_filament_options(void)
{
    if (!s_dd_brand || !s_dd_type)
        return;
    lv_dropdown_set_options(s_dd_brand, "—");
    lv_dropdown_set_selected(s_dd_brand, 0);
    lv_dropdown_set_options(s_dd_type, "—");
    lv_dropdown_set_selected(s_dd_type, 0);
    if (s_ams_edit_sd_timer)
    {
        lv_timer_del(s_ams_edit_sd_timer);
        s_ams_edit_sd_timer = NULL;
    }
    s_ams_edit_sd_timer = lv_timer_create(ams_edit_sd_load_timer_cb, 1, NULL);
}

lv_obj_t *ui_amsEditComponent_create(lv_obj_t *comp_parent)
{
    lv_obj_t *cui_amsEditComponent = lv_obj_create(comp_parent);
    lv_obj_set_width(cui_amsEditComponent, lv_pct(100));
    lv_obj_set_height(cui_amsEditComponent, lv_pct(100));
    lv_obj_set_flex_flow(cui_amsEditComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_amsEditComponent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_radius(cui_amsEditComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_amsEditComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_amsEditComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_amsEditComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_amsEditComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_amsEditComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_amsEditComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_amsEditComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* Content area */
    lv_obj_t *content = lv_obj_create(cui_amsEditComponent);
    lv_obj_set_width(content, lv_pct(100));
    lv_obj_set_flex_grow(content, 1);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_color(content, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(content, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(content, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(content, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(content, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(content, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(content, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(content, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* Row: フィラメント(ブランド) + Dropdown */
    lv_obj_t *row1 = lv_obj_create(content);
    lv_obj_set_width(row1, lv_pct(100));
    lv_obj_set_height(row1, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(row1, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(row1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(row1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(row1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(row1, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *dd_brand = lv_dropdown_create(row1);
    s_dd_brand = dd_brand;
    lv_obj_set_width(dd_brand, lv_pct(100));
    lv_obj_set_height(dd_brand, LV_SIZE_CONTENT);
    // {
    //     char buf[AMSEDIT_OPTS_BUF_SIZE];
    //     buf[0] = '\0';
    //     xtouch_public_filaments_get_brand_options(buf, sizeof(buf));
    //     lv_dropdown_set_options(dd_brand, buf[0] ? buf : "—");
    //     lv_dropdown_set_selected(dd_brand, 0);
    // }
    lv_dropdown_set_options(dd_brand, "-");
    lv_dropdown_set_selected(dd_brand, 0);
    lv_obj_set_style_bg_color(dd_brand, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(dd_brand, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(dd_brand, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(lv_dropdown_get_list(dd_brand), lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(lv_dropdown_get_list(dd_brand), lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(lv_dropdown_get_list(dd_brand), 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* Row: 種類 + Dropdown */
    lv_obj_t *row2 = lv_obj_create(content);
    lv_obj_set_width(row2, lv_pct(100));
    lv_obj_set_height(row2, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(row2, lv_color_hex(0x00FF00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(row2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row2, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(row2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(row2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(row2, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *dd_type = lv_dropdown_create(row2);
    s_dd_type = dd_type;
    lv_obj_set_width(dd_type, lv_pct(100));
    lv_obj_set_height(dd_type, LV_SIZE_CONTENT);
    lv_dropdown_set_options(dd_type, "-");
    lv_dropdown_set_selected(dd_type, 0);
    lv_obj_set_style_bg_color(dd_type, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(dd_type, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(dd_type, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(lv_dropdown_get_list(dd_type), lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(lv_dropdown_get_list(dd_type), lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(lv_dropdown_get_list(dd_type), 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(dd_brand, on_brand_changed, LV_EVENT_VALUE_CHANGED, dd_type);
    lv_obj_add_event_cb(dd_type, on_filament_selection_changed, LV_EVENT_VALUE_CHANGED, NULL);

    /* 表示時に最新の構造体を反映するため refresh を呼ぶ（loadScreen から呼ばれる） */
    ui_amsEditComponent_refresh_filament_options();

    /* Row: 色 + 表示エリア + 色設定ボタン */
    lv_obj_t *row3 = lv_obj_create(content);
    lv_obj_set_width(row3, lv_pct(100));
    lv_obj_set_height(row3, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row3, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row3, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(row3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(row3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(row3, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *color_panel = lv_obj_create(row3);
    lv_obj_set_width(color_panel, 48);
    lv_obj_set_height(color_panel, 48);
    lv_obj_set_style_radius(color_panel, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    /* 現在選択中の色を表示（RRGGBBAA の先頭 RRGGBB 部分を使用） */
    {
        const char *hex = ams_edit_current_tray_color;
        unsigned int rgb = 0x000000;
        if (hex && strlen(hex) >= 6)
            (void)sscanf(hex, "%6x", &rgb);
        lv_obj_set_style_bg_color(color_panel, lv_color_hex(rgb & 0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    lv_obj_set_style_bg_opa(color_panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(color_panel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(color_panel, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(color_panel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(color_panel, on_color_btn_clicked, LV_EVENT_CLICKED, NULL); /* パネルタップでもパレットへ */
    lv_obj_t *btn_color = lv_btn_create(row3);
    lv_obj_set_width(btn_color, 48);
    lv_obj_set_height(btn_color, 48);
    lv_obj_set_style_radius(btn_color, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_color, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *lbl_color = lv_label_create(btn_color);
    lv_label_set_text(lbl_color, LV_SYMBOL_EDIT);
    lv_obj_center(lbl_color);
    lv_obj_add_event_cb(btn_color, on_color_btn_clicked, LV_EVENT_CLICKED, NULL);

    /* 色選択の右: Min/Max 温度を2段表示 */
    lv_obj_t *temp_col = lv_obj_create(row3);
    lv_obj_set_width(temp_col, LV_SIZE_CONTENT);
    lv_obj_set_height(temp_col, 48);
    lv_obj_set_flex_flow(temp_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(temp_col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(temp_col, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(temp_col, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(temp_col, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(temp_col, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    s_lbl_temp_min = lv_label_create(temp_col);
    lv_label_set_text(s_lbl_temp_min, "Min:0");
    lv_obj_set_style_text_font(s_lbl_temp_min, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    s_lbl_temp_max = lv_label_create(temp_col);
    lv_label_set_text(s_lbl_temp_max, "Max:0");
    lv_obj_set_style_text_font(s_lbl_temp_max, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    update_temp_display();
    lv_obj_add_event_cb(content, on_fetched_temp_msg, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_AMS_EDIT_FETCHED_TEMP, content, NULL);

    /* Footer: Back, Reset, Save */
    lv_obj_t *footer = lv_obj_create(cui_amsEditComponent);
    lv_obj_set_width(footer, lv_pct(100));
    lv_obj_set_height(footer, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(footer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(footer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(footer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(footer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(footer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(footer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(footer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(footer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(footer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(footer, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *btn_back = lv_btn_create(footer);
    lv_obj_set_width(btn_back, lv_pct(25));
    lv_obj_set_height(btn_back, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(btn_back, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_back, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn_back, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *lbl_back = lv_label_create(btn_back);
    lv_obj_set_style_text_font(lbl_back, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(lbl_back, "Back");
    lv_obj_center(lbl_back);

    lv_obj_t *btn_reset = lv_btn_create(footer);
    lv_obj_set_width(btn_reset, lv_pct(25));
    lv_obj_set_height(btn_reset, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(btn_reset, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_reset, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_reset, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn_reset, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(btn_reset, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(btn_reset, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *lbl_reset = lv_label_create(btn_reset);
    lv_obj_set_style_text_font(lbl_reset, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(lbl_reset, "Reset");
    lv_obj_center(lbl_reset);

    lv_obj_t *btn_save = lv_btn_create(footer);
    lv_obj_set_width(btn_save, lv_pct(25));
    lv_obj_set_height(btn_save, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(btn_save, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_save, lv_color_hex(0x2A7C3E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_save, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn_save, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(btn_save, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(btn_save, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *lbl_save = lv_label_create(btn_save); 
    lv_obj_set_style_text_font(lbl_save, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(lbl_save, "Save");
    lv_obj_center(lbl_save);







    lv_obj_add_event_cb(btn_back, on_back_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_reset, on_reset_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_save, on_save_clicked, LV_EVENT_CLICKED, NULL);

    return cui_amsEditComponent;
}
