#include "ui_comp_cameracomponent.h"
#include "../ui_events.h"

#ifdef __XTOUCH_PLATFORM_S3__

lv_obj_t *ui_cameraComponent_create(lv_obj_t *comp_parent)
{
    lv_obj_t *cui_cameraComponent = lv_obj_create(comp_parent);
    lv_obj_set_width(cui_cameraComponent, lv_pct(100));
    lv_obj_set_height(cui_cameraComponent, lv_pct(100));
    lv_obj_set_flex_grow(cui_cameraComponent, 1);
    lv_obj_set_style_bg_color(cui_cameraComponent, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_cameraComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cui_cameraComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(cui_cameraComponent, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *backBtn = lv_btn_create(cui_cameraComponent);
    lv_obj_set_size(backBtn, 120, 44);
    lv_obj_align(backBtn, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_add_event_cb(backBtn, onMoveHomeScreen, LV_EVENT_CLICKED, NULL);
    lv_obj_t *backLabel = lv_label_create(backBtn);
    lv_label_set_text(backLabel, LV_SYMBOL_LEFT " Back");
    lv_obj_center(backLabel);

    lv_obj_t *previewPanel = lv_obj_create(cui_cameraComponent);
    lv_obj_set_size(previewPanel, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_top(previewPanel, 64, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(previewPanel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(previewPanel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(previewPanel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(previewPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(previewPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(previewPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(previewPanel, LV_OBJ_FLAG_SCROLLABLE);

    ui_cameraPreviewImage = lv_img_create(previewPanel);
    lv_obj_set_size(ui_cameraPreviewImage, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(ui_cameraPreviewImage);
    lv_img_set_src(ui_cameraPreviewImage, "S:/resource/Logo.jpg");
    lv_img_set_size_mode(ui_cameraPreviewImage, LV_IMG_SIZE_MODE_REAL);

    return cui_cameraComponent;
}

#else

lv_obj_t *ui_cameraComponent_create(lv_obj_t *comp_parent)
{
    (void)comp_parent;
    return NULL;
}

#endif
