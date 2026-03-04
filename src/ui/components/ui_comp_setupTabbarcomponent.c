#include "../ui.h"
#include "ui_comp_setupTabbarcomponent.h"

typedef struct {
    lv_obj_t *general_panel;
    lv_obj_t *option_panel;
    lv_obj_t *system_panel;
} setup_tabbar_panels_t;

static void on_tab_click(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED)
        return;
    int index = (int)(intptr_t)lv_event_get_user_data(e);
    if (index < 0 || index > 2)
        return;
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *comp = lv_obj_get_parent(btn);
    setup_tabbar_panels_t *panels = (setup_tabbar_panels_t *)lv_obj_get_user_data(comp);
    if (!panels || !panels->general_panel || !panels->option_panel || !panels->system_panel)
        return;
    lv_obj_add_flag(panels->general_panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(panels->option_panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(panels->system_panel, LV_OBJ_FLAG_HIDDEN);
    if (index == 0)
        lv_obj_clear_flag(panels->general_panel, LV_OBJ_FLAG_HIDDEN);
    else if (index == 1)
        lv_obj_clear_flag(panels->option_panel, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_clear_flag(panels->system_panel, LV_OBJ_FLAG_HIDDEN);
    ui_setupTabbarComponent_set_active(comp, index);
}

static void on_delete(lv_event_t *e)
{
    lv_obj_t *comp = lv_event_get_target(e);
    setup_tabbar_panels_t *panels = (setup_tabbar_panels_t *)lv_obj_get_user_data(comp);
    if (panels)
        lv_mem_free(panels);
}

static lv_obj_t *create_tab_button(lv_obj_t *parent, const char *label, int index, lv_obj_t *tabbar_comp)
{
    lv_obj_t *btn = lv_obj_create(parent);
    lv_obj_set_flex_grow(btn, 1);
    lv_obj_set_height(btn, lv_pct(100));
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_set_style_radius(btn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(btn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x2aff00), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(btn, 255, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x008800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(btn, lv_color_hex(0x111111), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_pad_left(btn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(btn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(btn, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(btn, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, label);
    lv_obj_center(lbl);
    lv_obj_set_style_text_font(lbl, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(btn, on_tab_click, LV_EVENT_CLICKED, (void *)(intptr_t)index);
    (void)tabbar_comp;
    return btn;
}

void ui_setupTabbarComponent_set_active(lv_obj_t *comp, int index)
{
    if (index < 0 || index > 2)
        return;
    for (int i = 0; i < 3; i++)
    {
        lv_obj_t *btn = ui_comp_get_child(comp, (uint32_t)(UI_COMP_SETUPTABBARCOMPONENT_BTN_GENERAL + i));
        if (btn)
        {
            if (i == index)
                lv_obj_add_state(btn, LV_STATE_CHECKED);
            else
                lv_obj_clear_state(btn, LV_STATE_CHECKED);
        }
    }
}

lv_obj_t *ui_setupTabbarComponent_create(lv_obj_t *comp_parent, lv_obj_t *general_panel, lv_obj_t *option_panel, lv_obj_t *system_panel, int active_tab_index)
{
    lv_obj_t *cui = lv_obj_create(comp_parent);
    lv_obj_set_width(cui, lv_pct(100));
    lv_obj_set_height(cui, lv_pct(15));
    lv_obj_set_flex_flow(cui, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(cui, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    setup_tabbar_panels_t *panels = (setup_tabbar_panels_t *)lv_mem_alloc(sizeof(setup_tabbar_panels_t));
    panels->general_panel = general_panel;
    panels->option_panel = option_panel;
    panels->system_panel = system_panel;
    lv_obj_set_user_data(cui, panels);
    lv_obj_add_event_cb(cui, on_delete, LV_EVENT_DELETE, NULL);

    lv_obj_t **children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_SETUPTABBARCOMPONENT_NUM);
    children[UI_COMP_SETUPTABBARCOMPONENT_SETUPTABBARCOMPONENT] = cui;
    children[UI_COMP_SETUPTABBARCOMPONENT_BTN_GENERAL] = create_tab_button(cui, "General", 0, cui);
    children[UI_COMP_SETUPTABBARCOMPONENT_BTN_OPTION]  = create_tab_button(cui, "Option",  1, cui);
    children[UI_COMP_SETUPTABBARCOMPONENT_BTN_SYSTEM]  = create_tab_button(cui, "System",  2, cui);

    lv_obj_add_event_cb(cui, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(cui, del_component_child_event_cb, LV_EVENT_DELETE, children);

    lv_obj_add_flag(option_panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(system_panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(general_panel, LV_OBJ_FLAG_HIDDEN);
    if (active_tab_index == 1) {
        lv_obj_add_flag(general_panel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(option_panel, LV_OBJ_FLAG_HIDDEN);
        ui_setupTabbarComponent_set_active(cui, 1);
    } else if (active_tab_index == 2) {
        lv_obj_add_flag(general_panel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(system_panel, LV_OBJ_FLAG_HIDDEN);
        ui_setupTabbarComponent_set_active(cui, 2);
    } else {
        ui_setupTabbarComponent_set_active(cui, 0);
    }
    return cui;
}
