/**
 * AMS Edit 色選択コンポーネント: パレットから色を選び、Edit に戻る
 */
#include "../ui.h"
#include <stdio.h>

/* 5x5 パレット色（0xRRGGBB）。画像の「その他」パレットに近い並び */
static const uint32_t s_palette_colors[25] = {
    0xffffff, 0xfff144, 0xdcf478, 0x0acc38, 0x057748, /* row1 */
    0x0d6284, 0x0ee2a0, 0x76d9f4, 0x46a8f9, 0x2850e0, /* row2 */
    0x443089, 0x443089, 0xa03cf7, 0xd4b1dd, 0xf95d73, /* row3 */
    0xf72323, 0x7c4b00, 0xf98c36, 0xfcecd6, 0xd3c5a3, /* row4 */
    0xaf7933, 0x898989, 0xbcbcbc, 0x161616, 0x000000  /* row5 */
};

static void on_palette_clicked(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;
    uint32_t idx = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
    if (idx >= 25)
        return;
    uint32_t rgb = s_palette_colors[idx];
    char hex8[12];
    snprintf(hex8, sizeof(hex8), "%06lXFF", (unsigned long)(rgb & 0xFFFFFF)); /* RRGGBBAA, AA=FF */
    ams_edit_set_tray_color(hex8);
    loadScreen(13); /* AMS Edit に戻る */
}

static void on_back_clicked(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;
    loadScreen(13); /* 色はそのまま、AMS Edit に戻る */
}

lv_obj_t *ui_amsEditColorComponent_create(lv_obj_t *comp_parent)
{
    lv_obj_t *content = lv_obj_create(comp_parent);
    lv_obj_set_width(content, lv_pct(100));
    lv_obj_set_height(content, lv_pct(100));
    lv_obj_set_flex_grow(content, 1);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_color(content, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(content, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(content, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(content, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(content, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);
    lv_obj_add_flag(content, LV_OBJ_FLAG_SCROLLABLE);



    /* 5x5 パレット */
    for (int row = 0; row < 5; row++)
    {
        lv_obj_t *row_obj = lv_obj_create(content);
        lv_obj_set_width(row_obj, LV_SIZE_CONTENT);
        lv_obj_set_height(row_obj, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(row_obj, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row_obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
        lv_obj_set_style_bg_opa(row_obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(row_obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_column(row_obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);

        for (int col = 0; col < 5; col++)
        {
            int idx = row * 5 + col;
            lv_obj_t *chip = lv_obj_create(row_obj);
            lv_obj_set_size(chip, 36, 36);
            lv_obj_set_style_radius(chip, 18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(chip, lv_color_hex(s_palette_colors[idx] & 0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(chip, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(chip, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(chip, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(chip, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(chip, on_palette_clicked, LV_EVENT_CLICKED, (void *)(uintptr_t)idx);
        }
    }

    /* Back */
    lv_obj_t *btn_back = lv_btn_create(comp_parent);
    lv_obj_set_width(btn_back, 120);
    lv_obj_set_height(btn_back, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(btn_back, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_back, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn_back, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *lbl_back = lv_label_create(btn_back);
    lv_label_set_text(lbl_back, "Back");
    lv_obj_set_style_text_font(lbl_back, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(lbl_back);
    lv_obj_add_event_cb(btn_back, on_back_clicked, LV_EVENT_CLICKED, NULL);

    return content;
}
