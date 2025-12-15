#include <stdio.h>
#include "../ui.h"
#include "../ui_msgs.h"
#include "ui_comp.h"
// COMPONENT utilCalibrationComponent
void ui_event_comp_utilCalibrationComponent_onSaveButtonClick(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        lv_obj_t **children = (lv_obj_t **)lv_event_get_user_data(e);
        lv_obj_t *comp = children[UI_COMP_UTILCALIBRATIONCOMPONENT_UTILCALIBRATIONCOMPONENT];
        
        lv_obj_t *lidarCheckbox = ui_comp_get_child(comp, UI_COMP_UTILCALIBRATIONCOMPONENT_UTILCALIBRATION_LIDAR);
        lv_obj_t *bedCheckbox = ui_comp_get_child(comp, UI_COMP_UTILCALIBRATIONCOMPONENT_UTILCALIBRATION_BED);
        lv_obj_t *vibrationCheckbox = ui_comp_get_child(comp, UI_COMP_UTILCALIBRATIONCOMPONENT_UTILCALIBRATION_VIBRATION);
        lv_obj_t *motorCheckbox = ui_comp_get_child(comp, UI_COMP_UTILCALIBRATIONCOMPONENT_UTILCALIBRATION_MOTOR);
        
        bool lidarCalibration = lv_obj_has_state(lidarCheckbox, LV_STATE_CHECKED);
        bool bedLevelling = lv_obj_has_state(bedCheckbox, LV_STATE_CHECKED);
        bool vibrationCompensation = lv_obj_has_state(vibrationCheckbox, LV_STATE_CHECKED);
        bool motorCancellation = lv_obj_has_state(motorCheckbox, LV_STATE_CHECKED);
        
        printf("Setting: lidarCalibration=%d, bedLevelling=%d, vibrationCompensation=%d, motorCancellation=%d\n", lidarCalibration, bedLevelling, vibrationCompensation, motorCancellation);

        uint8_t bitmask = 0;
        if (lidarCalibration) bitmask |= 1;
        if (bedLevelling) bitmask |= 1 << 1;
        if (vibrationCompensation) bitmask |= 1 << 2;
        if (motorCancellation) bitmask |= 1 << 3;

        xTouchConfig.xTouchUtilCalibrationBitmask = bitmask;
//print_gcode_action
// 1 bet lebeling
// 13 homeing
// 14 nozzle wipe
// 18 lidar calibration
//  3 vibration compensation
// 25 motor noize cancellation
// 255 unknown

        onCalibrationConfirm(e, bitmask);

        onMoveHomeScreen(e);
    }
}

lv_obj_t *ui_utilCalibrationComponent_create(lv_obj_t *comp_parent)
{

    lv_obj_t *cui_utilCalibrationComponent;
    cui_utilCalibrationComponent = lv_obj_create(comp_parent);
    lv_obj_set_height(cui_utilCalibrationComponent, lv_pct(100));
    lv_obj_set_flex_grow(cui_utilCalibrationComponent, 1);
    lv_obj_set_x(cui_utilCalibrationComponent, 385);
    lv_obj_set_y(cui_utilCalibrationComponent, 178);
    lv_obj_set_flex_flow(cui_utilCalibrationComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_utilCalibrationComponent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_radius(cui_utilCalibrationComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_utilCalibrationComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_utilCalibrationComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_utilCalibrationComponent, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_utilCalibrationComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_utilCalibrationComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_utilCalibrationComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_utilCalibrationComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_utilCalibrationComponent, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_utilCalibrationComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(cui_utilCalibrationComponent, LV_SCROLLBAR_MODE_ACTIVE);

    lv_obj_t *cui_utilCalibrationTitle;
    cui_utilCalibrationTitle = lv_label_create(cui_utilCalibrationComponent);
    lv_obj_set_width(cui_utilCalibrationTitle, lv_pct(100));
    lv_obj_set_height(cui_utilCalibrationTitle, LV_SIZE_CONTENT); /// 40
    lv_label_set_text_fmt(cui_utilCalibrationTitle, LV_SYMBOL_SETTINGS " UTILCALIBRATION SELECT");
    lv_obj_set_scrollbar_mode(cui_utilCalibrationTitle, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_utilCalibrationTitle, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_utilCalibrationTitle, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_utilCalibrationTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_utilCalibrationTitle, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_utilCalibrationTitle, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_utilCalibrationTitle, lv_color_hex(0x682AFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_utilCalibrationTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_utilCalibrationTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_utilCalibrationTitle, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_utilCalibrationTitle, lv_color_hex(0x000), LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_t *cui_utilCalibrationBox;
    cui_utilCalibrationBox = lv_obj_create(cui_utilCalibrationComponent);
    lv_obj_set_width(cui_utilCalibrationBox, lv_pct(100));
    lv_obj_set_height(cui_utilCalibrationBox, 100); /// 50
    lv_obj_set_flex_grow(cui_utilCalibrationBox, 1);
    lv_obj_set_flex_flow(cui_utilCalibrationBox, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_utilCalibrationBox, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_scrollbar_mode(cui_utilCalibrationBox, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_utilCalibrationBox, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_utilCalibrationBox, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_utilCalibrationBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_utilCalibrationBox, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_utilCalibrationBox, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_utilCalibrationBox, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_utilCalibrationBox, 16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_utilCalibrationLidar;
    cui_utilCalibrationLidar = lv_checkbox_create(cui_utilCalibrationBox);
    lv_obj_set_width(cui_utilCalibrationLidar, LV_SIZE_CONTENT);
    lv_obj_set_height(cui_utilCalibrationLidar, LV_SIZE_CONTENT);
    lv_checkbox_set_text(cui_utilCalibrationLidar, "Lidar Calibration");
    lv_obj_set_style_text_font(cui_utilCalibrationLidar, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    if (xtouch_bblp_is_p1Series())
    {
        lv_obj_add_flag(cui_utilCalibrationLidar, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_state(cui_utilCalibrationLidar, LV_STATE_CHECKED);
    }


    lv_obj_t *cui_utilCalibrationBed;
    cui_utilCalibrationBed = lv_checkbox_create(cui_utilCalibrationBox);
    lv_obj_set_width(cui_utilCalibrationBed, LV_SIZE_CONTENT);
    lv_obj_set_height(cui_utilCalibrationBed, LV_SIZE_CONTENT);
    lv_checkbox_set_text(cui_utilCalibrationBed, "Bed Levelling");
    lv_obj_set_style_text_font(cui_utilCalibrationBed, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_state(cui_utilCalibrationBed, LV_STATE_CHECKED);


    lv_obj_t *cui_utilCalibrationVibration;
    cui_utilCalibrationVibration = lv_checkbox_create(cui_utilCalibrationBox);
    lv_obj_set_width(cui_utilCalibrationVibration, LV_SIZE_CONTENT);
    lv_obj_set_height(cui_utilCalibrationVibration, LV_SIZE_CONTENT);
    lv_checkbox_set_text(cui_utilCalibrationVibration, "Vibration Compensation");
    lv_obj_set_style_text_font(cui_utilCalibrationVibration, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_state(cui_utilCalibrationVibration, LV_STATE_CHECKED);

    lv_obj_t *cui_utilCalibrationMotor;
    cui_utilCalibrationMotor = lv_checkbox_create(cui_utilCalibrationBox);
    lv_obj_set_width(cui_utilCalibrationMotor, LV_SIZE_CONTENT);
    lv_obj_set_height(cui_utilCalibrationMotor, LV_SIZE_CONTENT);
    lv_checkbox_set_text(cui_utilCalibrationMotor, "Motor Cancellation");
    lv_obj_set_style_text_font(cui_utilCalibrationMotor, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_state(cui_utilCalibrationMotor, LV_STATE_CHECKED);

    lv_obj_t *cui_utilCalibrationButton;
    cui_utilCalibrationButton = lv_label_create(cui_utilCalibrationBox);
    lv_obj_set_width(cui_utilCalibrationButton, lv_pct(100));
    lv_obj_set_height(cui_utilCalibrationButton, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(cui_utilCalibrationButton, LV_ALIGN_CENTER);
    lv_obj_set_style_text_align(cui_utilCalibrationButton, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_utilCalibrationButton, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_utilCalibrationButton, LV_SYMBOL_PLAY " Calibration Start");
    lv_obj_add_flag(cui_utilCalibrationButton, LV_OBJ_FLAG_CLICKABLE);    /// Flags
    lv_obj_clear_flag(cui_utilCalibrationButton, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_scrollbar_mode(cui_utilCalibrationButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_utilCalibrationButton, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_utilCalibrationButton, lv_color_hex(0xFF682A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_utilCalibrationButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_utilCalibrationButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_utilCalibrationButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_utilCalibrationButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_utilCalibrationButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_utilCalibrationButton, lv_color_hex(0x000), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t **children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_UTILCALIBRATIONCOMPONENT_NUM);
    children[UI_COMP_UTILCALIBRATIONCOMPONENT_UTILCALIBRATIONCOMPONENT] = cui_utilCalibrationComponent;
    children[UI_COMP_UTILCALIBRATIONCOMPONENT_UTILCALIBRATION_BOX] = cui_utilCalibrationBox;
    children[UI_COMP_UTILCALIBRATIONCOMPONENT_UTILCALIBRATION_LIDAR] = cui_utilCalibrationLidar;
    children[UI_COMP_UTILCALIBRATIONCOMPONENT_UTILCALIBRATION_BED] = cui_utilCalibrationBed;
    children[UI_COMP_UTILCALIBRATIONCOMPONENT_UTILCALIBRATION_VIBRATION] = cui_utilCalibrationVibration;
    children[UI_COMP_UTILCALIBRATIONCOMPONENT_UTILCALIBRATION_MOTOR] = cui_utilCalibrationMotor;
    children[UI_COMP_UTILCALIBRATIONCOMPONENT_UTILCALIBRATION_RUN] = cui_utilCalibrationButton;

    lv_obj_add_event_cb(cui_utilCalibrationComponent, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(cui_utilCalibrationComponent, del_component_child_event_cb, LV_EVENT_DELETE, children);

    lv_obj_add_event_cb(cui_utilCalibrationButton, ui_event_comp_utilCalibrationComponent_onSaveButtonClick, LV_EVENT_ALL, children);

    ui_comp_utilCalibrationComponent_create_hook(cui_utilCalibrationComponent);
    return cui_utilCalibrationComponent;
}
