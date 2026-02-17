#include "../ui.h"

/* tab index -> loadScreen(screen_index): TEMP=1, AXIS=2, HOTEND=3, UTIL=11 */
static const int tab_to_screen[] = { 1, 2, 3, 11 };

static void on_tab_click(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED)
        return;
    int index = (int)(intptr_t)lv_event_get_user_data(e);
    if (index < 0 || index >= 4)
        return;
    loadScreen(tab_to_screen[index]);
}

static lv_obj_t *create_tab_button(lv_obj_t *parent, const char *label, int index)
{
    lv_obj_t *btn = lv_obj_create(parent);
    lv_obj_set_flex_grow(btn, 1);
    lv_obj_set_height(btn, 32);
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

    lv_obj_add_event_cb(btn, on_tab_click, LV_EVENT_CLICKED, (void *)(intptr_t)index);
    return btn;
}

void ui_utilTabbarComponent_set_active(lv_obj_t *comp, int index)
{
    if (index < 0 || index >= 4)
        return;
    for (int i = 0; i < 4; i++)
    {
        lv_obj_t *btn = ui_comp_get_child(comp, (uint32_t)(i + 1));
        if (btn)
        {
            if (i == index)
                lv_obj_add_state(btn, LV_STATE_CHECKED);
            else
                lv_obj_clear_state(btn, LV_STATE_CHECKED);
        }
    }
}

lv_obj_t *ui_utilTabbarComponent_create(lv_obj_t *comp_parent, int active_tab_index)
{
    lv_obj_t *cui = lv_obj_create(comp_parent);
    lv_obj_set_width(cui, lv_pct(100));
    lv_obj_set_height(cui, 36);
    lv_obj_set_flex_flow(cui, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(cui, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui, 4, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t **children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_UTILTABBARCOMPONENT_NUM);
    children[UI_COMP_UTILTABBARCOMPONENT_UTILTABBARCOMPONENT] = cui;
    children[UI_COMP_UTILTABBARCOMPONENT_BTN_TEMP]    = create_tab_button(cui, "TEMP",    0);
    children[UI_COMP_UTILTABBARCOMPONENT_BTN_AXIS]   = create_tab_button(cui, "AXIS",    1);
    children[UI_COMP_UTILTABBARCOMPONENT_BTN_HOTEND] = create_tab_button(cui, "HOTEND",  2);
    children[UI_COMP_UTILTABBARCOMPONENT_BTN_UTIL]    = create_tab_button(cui, "UTIL",    3);

    lv_obj_add_event_cb(cui, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(cui, del_component_child_event_cb, LV_EVENT_DELETE, children);

    ui_utilTabbarComponent_set_active(cui, (active_tab_index >= 0 && active_tab_index < 4) ? active_tab_index : 0);
    return cui;
}
