#include "../ui.h"

#ifdef __XTOUCH_PLATFORM_S3__
static lv_obj_t *s_video_img = NULL;

static void ui_event_video_frame_update(lv_event_t *e)
{
    lv_obj_t *img = lv_event_get_target(e);
    if (!img || !xtouch_p1s_video_last_path[0])
        return;
    char src[40];
    snprintf(src, sizeof(src), "S:%s", xtouch_p1s_video_last_path);
    lv_img_cache_invalidate_src(src);
    lv_img_set_src(img, src);
}

static void ui_event_video_back(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
        onMoveHomeScreen(e);
}
#endif

lv_obj_t *ui_videoComponent_create(lv_obj_t *parent)
{
    lv_obj_t *root = lv_obj_create(parent);
    lv_obj_set_width(root, lv_pct(100));
    lv_obj_set_height(root, lv_pct(100));
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(root, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(root, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(root, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(root, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(root, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *header = lv_obj_create(root);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, "P1S Live View");
    lv_obj_set_style_text_font(title, lv_font_middle, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *back = lv_label_create(header);
    lv_label_set_text(back, LV_SYMBOL_LEFT " Back");
    lv_obj_add_flag(back, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_text_font(back, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(back, ui_event_video_back, LV_EVENT_CLICKED, NULL);

    lv_obj_t *panel = lv_obj_create(root);
    lv_obj_set_width(panel, lv_pct(100));
    lv_obj_set_flex_grow(panel, 1);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(panel, 4, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *img = lv_img_create(panel);
    lv_obj_center(img);
    lv_obj_set_size(img, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(img, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_img_set_size_mode(img, LV_IMG_SIZE_MODE_REAL);
    lv_obj_t *hint = lv_label_create(panel);
    lv_label_set_text(hint, "Waiting for stream...");
    lv_obj_align(hint, LV_ALIGN_CENTER, 0, 0);

#ifdef __XTOUCH_PLATFORM_S3__
    s_video_img = img;
    lv_msg_subsribe_obj(XTOUCH_ON_P1S_VIDEO_FRAME, img, NULL);
    lv_obj_add_event_cb(img, ui_event_video_frame_update, LV_EVENT_MSG_RECEIVED, NULL);
#endif

    return root;
}
