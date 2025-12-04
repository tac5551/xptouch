#include <stdio.h>
#include "../ui.h"
#include "../ui_msgs.h"
#include "ui_comp.h"
// COMPONENT nozzleComponent

void ui_event_comp_nozzleComponent_nozzleTypeDropDown(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_nozzleComponent = lv_event_get_user_data(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        // 選択されたインデックスを取得
        uint16_t selected_index = lv_dropdown_get_selected(target);
        
        printf("ui_event_comp_nozzleComponent_nozzleTypeDropDown: selected_index=%d\n", selected_index);

        // 選択された値に基づいて条件分岐
        lv_obj_t *child = ui_comp_get_child(ui_nozzleComponent, UI_COMP_NOZZLECOMPONENT_NOZZLEDEMILITER_DROPDOWN);
        
        // 現在選択されているノズル径の文字列を取得
        char current_diameter_str[64];
        lv_dropdown_get_selected_str(child, current_diameter_str, sizeof(current_diameter_str));
        
        uint16_t new_diameter_index = 0; // デフォルト
        
        if (selected_index == 0) // hardened_steel
        {
            // hardened_steel の場合の処理
            lv_dropdown_set_options(child, "0.4mm\n0.6mm\n0.8mm");
            // 現在選択されている文字列が新しい候補に含まれているか確認
            if (strcmp(current_diameter_str, "0.4mm") == 0)
            {
                new_diameter_index = 0;
            }
            else if (strcmp(current_diameter_str, "0.6mm") == 0)
            {
                new_diameter_index = 1;
            }
            else if (strcmp(current_diameter_str, "0.8mm") == 0)
            {
                new_diameter_index = 2;
            }
            else
            {
                // 含まれていない場合はデフォルト（0.4mm）
                new_diameter_index = 0;
            }
        }
        else if (selected_index == 1) // stainless_steel
        {
            // stainless_steel の場合の処理
            lv_dropdown_set_options(child, "0.2mm\n0.4mm");
            // 現在選択されている文字列が新しい候補に含まれているか確認
            if (strcmp(current_diameter_str, "0.2mm") == 0)
            {
                new_diameter_index = 0;
            }
            else if (strcmp(current_diameter_str, "0.4mm") == 0)
            {
                new_diameter_index = 1;
            }
            else
            {
                // 含まれていない場合はデフォルト（0.4mm）
                new_diameter_index = 1;
            }
        }
        
        lv_dropdown_set_selected(child, new_diameter_index);
    }
}

void ui_event_comp_nozzleComponentt_nozzleDemiliterDropDown(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_nozzleComponent = lv_event_get_user_data(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        // 選択されたインデックスを取得
        uint16_t selected_index = lv_dropdown_get_selected(target);
        
        printf("ui_event_comp_nozzleComponent_nozzleDemiliterDropDown: selected_index=%d\n", selected_index);

    }
}

void ui_event_comp_nozzleComponent_onSaveButtonClick(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_nozzleComponent_onSaveButtonClick\n");
 
        lv_obj_t *NozzleDemiliter = ui_comp_get_child(ui_nozzleComponent, UI_COMP_NOZZLECOMPONENT_NOZZLEDEMILITER_DROPDOWN);
        lv_obj_t *NozzleType = ui_comp_get_child(ui_nozzleComponent, UI_COMP_NOZZLECOMPONENT_NOZZLETYPE_DROPDOWN);

        // Typeの選択値を取得してbambuStatusに設定
        uint16_t type_index = lv_dropdown_get_selected(NozzleType);
        if (type_index == 0)
        {
            strcpy(bambuStatus.nozzle_type, "hardened_steel");
        }
        else if (type_index == 1)
        {
            strcpy(bambuStatus.nozzle_type, "stainless_steel");
        }

        // ノズル径の選択値を取得してbambuStatusに設定
        uint16_t diameter_index = lv_dropdown_get_selected(NozzleDemiliter);
        if (strcmp(bambuStatus.nozzle_type, "stainless_steel") == 0)
        {
            // stainless_steel: 0.2mm(0), 0.4mm(1)
            if (diameter_index == 0)
            {
                bambuStatus.nozzle_diameter = 0.2f;
            }
            else if (diameter_index == 1)
            {
                bambuStatus.nozzle_diameter = 0.4f;
            }
        }
        else
        {
            // hardened_steel: 0.4mm(0), 0.6mm(1), 0.8mm(2)
            if (diameter_index == 0)
            {
                bambuStatus.nozzle_diameter = 0.4f;
            }
            else if (diameter_index == 1)
            {
                bambuStatus.nozzle_diameter = 0.6f;
            }
            else if (diameter_index == 2)
            {
                bambuStatus.nozzle_diameter = 0.8f;
            }
        }
        printf("Setting: nozzle_type=%s, nozzle_diameter=%.1f\n", bambuStatus.nozzle_type, bambuStatus.nozzle_diameter);

        // メッセージを送信
        lv_msg_send(XTOUCH_COMMAND_SET_ACCESSORIES_NOZZLE, NULL);
    }
}

lv_obj_t *ui_nozzleComponent_create(lv_obj_t *comp_parent)
{

    lv_obj_t *cui_nozzleComponent;
    cui_nozzleComponent = lv_obj_create(ui_nozzleScreen);
    lv_obj_set_height(cui_nozzleComponent, lv_pct(100));
    lv_obj_set_flex_grow(cui_nozzleComponent, 1);
    lv_obj_set_x(cui_nozzleComponent, 385);
    lv_obj_set_y(cui_nozzleComponent, 178);
    lv_obj_set_flex_flow(cui_nozzleComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_nozzleComponent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_radius(cui_nozzleComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_nozzleComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_nozzleComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_nozzleComponent, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_nozzleComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_nozzleComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_nozzleComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_nozzleComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_nozzleComponent, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_nozzleComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(cui_nozzleComponent, LV_SCROLLBAR_MODE_ACTIVE);

    lv_obj_t *cui_nozzleTitle;
    cui_nozzleTitle = lv_label_create(cui_nozzleComponent);
    lv_obj_set_width(cui_nozzleTitle, lv_pct(100));
    lv_obj_set_height(cui_nozzleTitle, LV_SIZE_CONTENT); /// 40
    lv_label_set_text_fmt(cui_nozzleTitle, LV_SYMBOL_SETTINGS " NOZZLE SELECT");
    lv_obj_set_scrollbar_mode(cui_nozzleTitle, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_nozzleTitle, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_nozzleTitle, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_nozzleTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_nozzleTitle, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_nozzleTitle, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_nozzleTitle, lv_color_hex(0x682AFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_nozzleTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_nozzleTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_nozzleTitle, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_nozzleTitle, lv_color_hex(0x000), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_nozzleTypeSelect;
    cui_nozzleTypeSelect = lv_obj_create(cui_nozzleComponent);
    lv_obj_set_width(cui_nozzleTypeSelect, lv_pct(100));
    lv_obj_set_height(cui_nozzleTypeSelect, 100); /// 50
    lv_obj_set_flex_grow(cui_nozzleTypeSelect, 1);
    lv_obj_set_flex_flow(cui_nozzleTypeSelect, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_nozzleTypeSelect, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(cui_nozzleTypeSelect, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_nozzleTypeSelect, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_nozzleTypeSelect, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_nozzleTypeSelect, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_nozzleTypeSelect, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_nozzleTypeSelect, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_nozzleTypeSelect, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_nozzleTypeSelect, 16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_nozzleTypeSelectDropDown;
    cui_nozzleTypeSelectDropDown = lv_dropdown_create(cui_nozzleTypeSelect);
    lv_dropdown_set_options(cui_nozzleTypeSelectDropDown, "hardened_steel\nstainless_steel");
    lv_obj_set_width(cui_nozzleTypeSelectDropDown, lv_pct(100));
    lv_obj_set_height(cui_nozzleTypeSelectDropDown, LV_SIZE_CONTENT);           /// 1
    lv_obj_add_state(cui_nozzleTypeSelectDropDown, LV_STATE_PRESSED);           /// States
    lv_obj_add_flag(cui_nozzleTypeSelectDropDown, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_set_style_bg_color(cui_nozzleTypeSelectDropDown, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_nozzleTypeSelectDropDown, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_nozzleTypeSelectDropDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    // bambuStatus.nozzle_typeの値に基づいて選択を設定
    uint16_t nozzle_type_index = 0; // デフォルトはhardened_steel
    if (strcmp(bambuStatus.nozzle_type, "stainless_steel") == 0)
    {
        nozzle_type_index = 1;
    }
    lv_dropdown_set_selected(cui_nozzleTypeSelectDropDown, nozzle_type_index);

    lv_obj_set_style_text_color(cui_nozzleTypeSelectDropDown, lv_color_hex(0x2aff00), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_nozzleTypeSelectDropDown, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_text_letter_space(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), 32, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), 0, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), 32, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), 255, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), lv_color_hex(0x2aff00), LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), 255, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_PRESSED);
    lv_obj_set_style_text_opa(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), 255, LV_PART_SELECTED | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), lv_color_hex(0x00AA00), LV_PART_SELECTED | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(lv_dropdown_get_list(cui_nozzleTypeSelectDropDown), 255, LV_PART_SELECTED | LV_STATE_PRESSED);

    lv_obj_t *cui_nozzleDemiliterSelectDropDown;
    cui_nozzleDemiliterSelectDropDown = lv_dropdown_create(cui_nozzleTypeSelect);
    
    // Typeの値に基づいて候補を設定
    uint16_t nozzle_diameter_index = 0; // デフォルト
    if (strcmp(bambuStatus.nozzle_type, "stainless_steel") == 0)
    {
        // stainless_steel: 0.2mm, 0.4mm
        lv_dropdown_set_options(cui_nozzleDemiliterSelectDropDown, "0.2mm\n0.4mm");
        // bambuStatus.nozzle_diameterに基づいてインデックスを設定
        if (bambuStatus.nozzle_diameter == 0.2f)
        {
            nozzle_diameter_index = 0;
        }
        else if (bambuStatus.nozzle_diameter == 0.4f)
        {
            nozzle_diameter_index = 1;
        }
        else
        {
            // デフォルトは0.4mm
            nozzle_diameter_index = 1;
        }
    }
    else
    {
        // hardened_steel: 0.4mm, 0.6mm, 0.8mm
        lv_dropdown_set_options(cui_nozzleDemiliterSelectDropDown, "0.4mm\n0.6mm\n0.8mm");
        // bambuStatus.nozzle_diameterに基づいてインデックスを設定
        if (bambuStatus.nozzle_diameter == 0.4f)
        {
            nozzle_diameter_index = 0;
        }
        else if (bambuStatus.nozzle_diameter == 0.6f)
        {
            nozzle_diameter_index = 1;
        }
        else if (bambuStatus.nozzle_diameter == 0.8f)
        {
            nozzle_diameter_index = 2;
        }
        else
        {
            // デフォルトは0.4mm
            nozzle_diameter_index = 0;
        }
    }
    
    lv_obj_set_width(cui_nozzleDemiliterSelectDropDown, lv_pct(100));
    lv_obj_set_height(cui_nozzleDemiliterSelectDropDown, LV_SIZE_CONTENT);           /// 1
    lv_obj_add_state(cui_nozzleDemiliterSelectDropDown, LV_STATE_PRESSED);           /// States
    lv_obj_add_flag(cui_nozzleDemiliterSelectDropDown, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_set_style_bg_color(cui_nozzleDemiliterSelectDropDown, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_nozzleDemiliterSelectDropDown, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_nozzleDemiliterSelectDropDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_dropdown_set_selected(cui_nozzleDemiliterSelectDropDown, nozzle_diameter_index);

    lv_obj_set_style_text_color(cui_nozzleDemiliterSelectDropDown, lv_color_hex(0x2aff00), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_nozzleDemiliterSelectDropDown, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_text_letter_space(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), 32, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), 0, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), 32, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), 255, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), lv_color_hex(0x2aff00), LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), 255, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), lv_color_hex(0x000000), LV_PART_SELECTED | LV_STATE_PRESSED);
    lv_obj_set_style_text_opa(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), 255, LV_PART_SELECTED | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), lv_color_hex(0x00AA00), LV_PART_SELECTED | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(lv_dropdown_get_list(cui_nozzleDemiliterSelectDropDown), 255, LV_PART_SELECTED | LV_STATE_PRESSED);

    lv_obj_t *cui_saveNozzleButton;
    cui_saveNozzleButton = lv_label_create(cui_nozzleTypeSelect);
    lv_obj_set_width(cui_saveNozzleButton, lv_pct(100));
    lv_obj_set_height(cui_saveNozzleButton, LV_SIZE_CONTENT); /// 1
    lv_obj_set_style_text_font(cui_saveNozzleButton, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_saveNozzleButton, LV_SYMBOL_SAVE " Save");
    lv_obj_add_flag(cui_saveNozzleButton, LV_OBJ_FLAG_CLICKABLE);    /// Flags
    lv_obj_clear_flag(cui_saveNozzleButton, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_scrollbar_mode(cui_saveNozzleButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_saveNozzleButton, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_saveNozzleButton, lv_color_hex(0xFF682A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_saveNozzleButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_saveNozzleButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_saveNozzleButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_saveNozzleButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_saveNozzleButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_saveNozzleButton, lv_color_hex(0x000), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t **children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_NOZZLECOMPONENT_NUM);
    children[UI_COMP_NOZZLECOMPONENT_NOZZLECOMPONENT] = cui_nozzleComponent;
    children[UI_COMP_NOZZLECOMPONENT_NOZZLETYPE] = cui_nozzleTypeSelect;
    children[UI_COMP_NOZZLECOMPONENT_NOZZLETYPE_DROPDOWN] = cui_nozzleTypeSelectDropDown;
    children[UI_COMP_NOZZLECOMPONENT_NOZZLEDEMILITER_DROPDOWN] = cui_nozzleDemiliterSelectDropDown;
    children[UI_COMP_NOZZLECOMPONENT_NOZZLE_SAVE] = cui_saveNozzleButton;

    lv_obj_add_event_cb(cui_nozzleComponent, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(cui_nozzleComponent, del_component_child_event_cb, LV_EVENT_DELETE, children);

    lv_obj_add_event_cb(cui_saveNozzleButton, ui_event_comp_nozzleComponent_onSaveButtonClick, LV_EVENT_ALL, children);

    lv_obj_add_event_cb(cui_nozzleTypeSelectDropDown, ui_event_comp_nozzleComponent_nozzleTypeDropDown, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_nozzleDemiliterSelectDropDown, ui_event_comp_nozzleComponentt_nozzleDemiliterDropDown, LV_EVENT_ALL, children);

    ui_comp_nozzleComponent_create_hook(cui_nozzleComponent);
    return cui_nozzleComponent;
}
