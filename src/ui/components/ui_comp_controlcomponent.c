#include "../ui.h"

void ui_event_comp_controlComponent_controlScreenRange(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_controlComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        if (controlMode.inc==1)  controlMode.inc =10;
        else if (controlMode.inc==10)  controlMode.inc =100;
        else if (controlMode.inc==100)  controlMode.inc =1;
        onControlRange(e);
    }
}
void ui_event_comp_controlComponent_controlScreenLeft(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_controlComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onControlLeft(e);
    }
}
void ui_event_comp_controlComponent_controlScreenUp(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_controlComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onControlUp(e);
    }
}

void ui_event_comp_controlComponent_controlScreenHomeConfirm() { onControlHome(NULL); }
void ui_event_comp_controlComponent_controlScreenMotorUnlockConfirm(void) { onControlMotorUnlock(NULL); }
void ui_event_comp_controlComponent_controlScreenHome(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_controlComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        ui_confirmPanel_show(LV_SYMBOL_WARNING " Start Homing Process?", ui_event_comp_controlComponent_controlScreenHomeConfirm);
    }
}
void ui_event_comp_controlComponent_controlScreenDown(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_controlComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onControlDown(e);
    }
}
void ui_event_comp_controlComponent_controlScreenBedUp(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_controlComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onControlBedUp(e);
    }
}
void ui_event_comp_controlComponent_controlScreenBedDown(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_controlComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onControlBedDown(e);
    }
}
void ui_event_comp_controlComponent_controlScreenRight(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_controlComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        onControlRight(e);
    }
}

void ui_event_comp_controlComponent_controlScreenMotorUnlock(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t **comp_controlComponent = lv_event_get_user_data(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        ui_confirmPanel_show(LV_SYMBOL_WARNING " Disable stepper motors?\n(M18)", ui_event_comp_controlComponent_controlScreenMotorUnlockConfirm);
    }
}

void ui_event_comp_controlComponent_nozzleTemp(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);
    struct XTOUCH_MESSAGE_DATA *message = (struct XTOUCH_MESSAGE_DATA *)m->payload;
    char value[10];
    itoa(message->data, value, 10);
    lv_label_set_text(target, value);
    lv_obj_set_style_text_color(target, message->data < 170 ? lv_color_hex(0x39a1fd) : lv_color_hex(0xfaa61e), LV_PART_MAIN | LV_STATE_DEFAULT);
}

void ui_event_comp_controlComponent_nozzleUpClick(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
        onNozzleUp(e);
}

void ui_event_comp_controlComponent_nozzleDownClick(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
        onNozzleDown(e);
}

void ui_event_comp_controlComponent_nozzleTempClick(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
        onHomeNozzleTemp(e, 2);
}

void onXtouchRangeChange(lv_event_t *e)
{

    lv_obj_t *target = lv_event_get_target(e);
    switch (controlMode.inc){
        case 1: lv_label_set_text(target, "1");break;
        case 10: lv_label_set_text(target, "10");break;
        case 100: lv_label_set_text(target, "100");break;
    }
    
}

/* AXIS リング: デザイン基準 CONTROL_AXIS_BASE に対し、ColB の短辺に比例してスケール */
#define CONTROL_AXIS_BASE 236
/* ColB 短辺に対するリング直径の割合（100=いっぱい、小さくすると円が小さくなる） */
#define CONTROL_AXIS_RING_FILL_PCT 88

static struct {
    lv_obj_t *axisPad;
    lv_obj_t *rangeBtn;
    lv_obj_t *motorUnlock;
    lv_obj_t *home;
    lv_obj_t *xyIcon;
} s_control_axis;

static lv_coord_t control_axis_sc(lv_coord_t side, int x)
{
    return (lv_coord_t)(((int32_t)side * x) / CONTROL_AXIS_BASE);
}

/* axisPad の子の並び（作成順固定）: 外弧4 → 方向4 → 内弧4 → 内ヒット4 */
static void control_axis_apply_scale(lv_coord_t side_in)
{
    lv_obj_t *ap = s_control_axis.axisPad;
    if (!ap || side_in < 55)
        return;
    lv_coord_t side = (lv_coord_t)(((int32_t)side_in * CONTROL_AXIS_RING_FILL_PCT) / 100);
    if (side < 48)
        return;

    lv_obj_set_size(ap, side, side);

    lv_coord_t sc230 = control_axis_sc(side, 230);
    lv_coord_t sc58 = control_axis_sc(side, 58);
    for (int i = 0; i < 4; i++) {
        lv_obj_t *o = lv_obj_get_child(ap, i);
        if (!o)
            continue;
        lv_obj_set_size(o, sc230, sc230);
        lv_obj_center(o);
        lv_obj_set_style_arc_width(o, sc58, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    lv_coord_t sc56 = control_axis_sc(side, 56);
    /* 外周から中心へ 5px 相当（基準236では ±1） */
    lv_coord_t scIn = control_axis_sc(side, 1);
    lv_obj_t *b0 = lv_obj_get_child(ap, 4);
    lv_obj_t *b1 = lv_obj_get_child(ap, 5);
    lv_obj_t *b2 = lv_obj_get_child(ap, 6);
    lv_obj_t *b3 = lv_obj_get_child(ap, 7);
    if (b0) {
        lv_obj_set_size(b0, sc56, sc56);
        lv_obj_align(b0, LV_ALIGN_TOP_MID, 0, scIn);
    }
    if (b1) {
        lv_obj_set_size(b1, sc56, sc56);
        lv_obj_align(b1, LV_ALIGN_LEFT_MID, scIn, 0);
    }
    if (b2) {
        lv_obj_set_size(b2, sc56, sc56);
        lv_obj_align(b2, LV_ALIGN_RIGHT_MID, -scIn, 0);
    }
    if (b3) {
        lv_obj_set_size(b3, sc56, sc56);
        lv_obj_align(b3, LV_ALIGN_BOTTOM_MID, 0, -scIn);
    }

    lv_coord_t sc126 = control_axis_sc(side, 126);
    lv_coord_t sc36 = control_axis_sc(side, 36);
    for (int i = 8; i < 12; i++) {
        lv_obj_t *o = lv_obj_get_child(ap, i);
        if (!o)
            continue;
        lv_obj_set_size(o, sc126, sc126);
        lv_obj_center(o);
        lv_obj_set_style_arc_width(o, sc36, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    lv_coord_t sc62 = control_axis_sc(side, 62);
    lv_coord_t sc44 = control_axis_sc(side, 44);
    lv_coord_t sc30 = control_axis_sc(side, 30);
    lv_obj_t *h0 = lv_obj_get_child(ap, 12);
    lv_obj_t *h1 = lv_obj_get_child(ap, 13);
    lv_obj_t *h2 = lv_obj_get_child(ap, 14);
    lv_obj_t *h3 = lv_obj_get_child(ap, 15);
    if (h0) {
        lv_obj_set_size(h0, sc62, sc44);
        lv_obj_align(h0, LV_ALIGN_TOP_MID, 0, sc30);
    }
    if (h1) {
        lv_obj_set_size(h1, sc44, sc62);
        lv_obj_align(h1, LV_ALIGN_LEFT_MID, sc30, 0);
    }
    if (h2) {
        lv_obj_set_size(h2, sc44, sc62);
        lv_obj_align(h2, LV_ALIGN_RIGHT_MID, -sc30, 0);
    }
    if (h3) {
        lv_obj_set_size(h3, sc62, sc44);
        lv_obj_align(h3, LV_ALIGN_BOTTOM_MID, 0, -sc30);
    }

    lv_coord_t sc55 = control_axis_sc(side, 55);
    if (s_control_axis.rangeBtn)
        lv_obj_set_size(s_control_axis.rangeBtn, sc55, sc55);
    if (s_control_axis.motorUnlock)
        lv_obj_set_size(s_control_axis.motorUnlock, sc55, sc55);
    if (s_control_axis.home)
        lv_obj_set_size(s_control_axis.home, sc55, sc55);
    if (s_control_axis.xyIcon)
        lv_obj_set_size(s_control_axis.xyIcon, sc55, sc55);
}

static void control_colb_axis_size_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_SIZE_CHANGED)
        return;
    lv_obj_t *colb = lv_event_get_target(e);
    lv_coord_t w = lv_obj_get_content_width(colb);
    lv_coord_t h = lv_obj_get_content_height(colb);
    if (w <= 0 || h <= 0)
        return;
    lv_coord_t side = w < h ? w : h;
    control_axis_apply_scale(side);
}

// COMPONENT controlComponent

lv_obj_t *ui_controlComponent_create(lv_obj_t *comp_parent)
{
    lv_coord_t side_button_col_w = 48; /* base: 2.8" (240x320) */
    lv_coord_t axis_outer_pad = 2;     /* 2.8" は円周り余白を詰める */
    lv_coord_t axis_side_gap = 4;      /* 左列と円の間隔 */
    lv_coord_t rw = lv_disp_get_hor_res(NULL);
    lv_coord_t rh = lv_disp_get_ver_res(NULL);
    lv_coord_t long_side = (rw > rh) ? rw : rh;
    lv_coord_t short_side = (rw > rh) ? rh : rw;
    if (long_side >= 800)
    {
        side_button_col_w = 108; /* 5" : 480x800 */
        axis_outer_pad = 4;
        axis_side_gap = 8;
    }
    else if (long_side >= 480)
    {
        side_button_col_w = 72;  /* 3.5" : 320x480 */
        axis_outer_pad = 4;
        axis_side_gap = 8;
    }

    lv_obj_t *cui_controlComponent;
    cui_controlComponent = lv_obj_create(comp_parent);
    lv_obj_set_height(cui_controlComponent, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlComponent, 1);
    lv_obj_set_x(cui_controlComponent, 386);
    lv_obj_set_y(cui_controlComponent, 178);
    lv_obj_set_flex_flow(cui_controlComponent, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_controlComponent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_controlComponent, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlComponent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_controlComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_controlComponent, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_controlComponent, axis_outer_pad, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_controlComponent, axis_outer_pad, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_controlComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    /* 基本列間は上下の間隔感に合わせる。広げたい箇所はスペーサー列で追加調整 */
    lv_obj_set_style_pad_column(cui_controlComponent, axis_outer_pad, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlComponent, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_controlComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 左端に HOTEND の左3要素（Up / Temp / Down）を統合 */
    lv_obj_t *cui_nozzleCol;
    cui_nozzleCol = lv_obj_create(cui_controlComponent);
    lv_obj_set_height(cui_nozzleCol, lv_pct(100));
    lv_obj_set_flex_grow(cui_nozzleCol, 0);
    lv_obj_set_width(cui_nozzleCol, side_button_col_w);
    lv_obj_set_flex_flow(cui_nozzleCol, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_nozzleCol, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_nozzleCol, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(cui_nozzleCol, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(cui_nozzleCol, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_nozzleCol, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_nozzleCol, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_nozzleCol, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_nozzleCol, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_nozzleCol, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_nozzleCol, 4, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 1-2間を広げるスペーサー */
    lv_obj_t *cui_spacer_12 = lv_obj_create(cui_controlComponent);
    lv_obj_set_width(cui_spacer_12, axis_side_gap);
    lv_obj_set_height(cui_spacer_12, lv_pct(100));
    lv_obj_set_flex_grow(cui_spacer_12, 0);
    lv_obj_clear_flag(cui_spacer_12, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_set_scrollbar_mode(cui_spacer_12, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(cui_spacer_12, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_spacer_12, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cui_spacer_12, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_nozzleUp = lv_obj_create(cui_nozzleCol);
    lv_obj_set_width(cui_nozzleUp, lv_pct(100));
    lv_obj_set_flex_grow(cui_nozzleUp, 2);
    lv_obj_set_flex_flow(cui_nozzleUp, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_nozzleUp, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_nozzleUp, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(cui_nozzleUp, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_nozzleUp, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_nozzleUp, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_nozzleUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_nozzleUp, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_nozzleUp, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_nozzleUp, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_nozzleUp, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_nozzleUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_nozzleUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_nozzleUp, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_nozzleUp, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_nozzleUp, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_nozzleUp, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_t *cui_nozzleUpIcon = lv_label_create(cui_nozzleUp);
    lv_obj_set_height(cui_nozzleUpIcon, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(cui_nozzleUpIcon, 1);
    lv_obj_set_align(cui_nozzleUpIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_nozzleUpIcon, "s");
    lv_obj_clear_flag(cui_nozzleUpIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(cui_nozzleUpIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_nozzleUpIcon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_nozzleUpIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_nozzleTempBtn = lv_obj_create(cui_nozzleCol);
    lv_obj_set_width(cui_nozzleTempBtn, lv_pct(100));
    /* Bed列（Up/Dummy/Down=2:2:2）と高さを揃えるため中央も2にする */
    lv_obj_set_flex_grow(cui_nozzleTempBtn, 2);
    lv_obj_clear_flag(cui_nozzleTempBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(cui_nozzleTempBtn, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_nozzleTempBtn, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_nozzleTempBtn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_nozzleTempBtn, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_nozzleTempBtn, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_nozzleTempBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_nozzleTempBtn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_nozzleTempBtn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_nozzleTempBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_nozzleTempBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *cui_nozzleTempIcon = lv_label_create(cui_nozzleTempBtn);
    lv_label_set_text(cui_nozzleTempIcon, "p");
    lv_obj_set_align(cui_nozzleTempIcon, LV_ALIGN_LEFT_MID);
    lv_obj_set_style_text_font(cui_nozzleTempIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *cui_nozzleTempVal = lv_label_create(cui_nozzleTempBtn);
    lv_label_set_text(cui_nozzleTempVal, "");
    lv_obj_set_align(cui_nozzleTempVal, LV_ALIGN_RIGHT_MID);
#if defined(__XTOUCH_SCREEN_S3_050__) || defined(__XTOUCH_SCREEN_S3_3248__)
    lv_obj_set_style_text_font(cui_nozzleTempVal, lv_font_middle, LV_PART_MAIN | LV_STATE_DEFAULT);
#else
    /* default は 2.8 側 */
    lv_obj_set_style_text_font(cui_nozzleTempVal, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif

    lv_obj_t *cui_nozzleDown = lv_obj_create(cui_nozzleCol);
    lv_obj_set_width(cui_nozzleDown, lv_pct(100));
    lv_obj_set_flex_grow(cui_nozzleDown, 2);
    lv_obj_set_flex_flow(cui_nozzleDown, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_nozzleDown, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_nozzleDown, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(cui_nozzleDown, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_nozzleDown, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_nozzleDown, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_nozzleDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_nozzleDown, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_nozzleDown, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_nozzleDown, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_nozzleDown, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_nozzleDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_nozzleDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_nozzleDown, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_nozzleDown, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_nozzleDown, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_nozzleDown, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_t *cui_nozzleDownIcon = lv_label_create(cui_nozzleDown);
    lv_obj_set_height(cui_nozzleDownIcon, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(cui_nozzleDownIcon, 1);
    lv_obj_set_align(cui_nozzleDownIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_nozzleDownIcon, "t");
    lv_obj_clear_flag(cui_nozzleDownIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scrollbar_mode(cui_nozzleDownIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_nozzleDownIcon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_nozzleDownIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_controlScreenColA;
    cui_controlScreenColA = lv_obj_create(cui_controlComponent);
    lv_obj_set_height(cui_controlScreenColA, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenColA, 1);
    lv_obj_set_x(cui_controlScreenColA, 386);
    lv_obj_set_y(cui_controlScreenColA, 178);
    lv_obj_set_flex_flow(cui_controlScreenColA, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_controlScreenColA, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_controlScreenColA, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenColA, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenColA, lv_color_hex(0x55FF55), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenColA, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenColA, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_controlScreenColA, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_controlScreenColA, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_controlScreenColA, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlScreenColA, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenColA, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_controlScreenColA, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenColA, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_controlScreenColA, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_controlScreenRange;
    cui_controlScreenRange = lv_obj_create(cui_controlScreenColA);
    lv_obj_set_width(cui_controlScreenRange, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenRange, 2);
    lv_obj_set_x(cui_controlScreenRange, 386);
    lv_obj_set_y(cui_controlScreenRange, 178);
    lv_obj_set_flex_flow(cui_controlScreenRange, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_controlScreenRange, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_controlScreenRange, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenRange, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenRange, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenRange, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenRange, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_controlScreenRange, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_controlScreenRange, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_controlScreenRange, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlScreenRange, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenRange, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_controlScreenRange, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenRange, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_controlScreenRange, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_controlScreenRange, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_controlScreenRange, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_controlScreenRangeIcon;
    cui_controlScreenRangeIcon = lv_label_create(cui_controlScreenRange);
    lv_obj_set_width(cui_controlScreenRangeIcon, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(cui_controlScreenRangeIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(cui_controlScreenRangeIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_controlScreenRangeIcon, "k");
    lv_obj_clear_flag(cui_controlScreenRangeIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenRangeIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_controlScreenRangeIcon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_controlScreenRangeIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_controlScreenRangeValue;
    cui_controlScreenRangeValue = lv_label_create(cui_controlScreenRange);
    lv_obj_set_height(cui_controlScreenRangeValue, LV_SIZE_CONTENT); /// 1
    lv_obj_set_flex_grow(cui_controlScreenRangeValue, 1);
    lv_obj_set_align(cui_controlScreenRangeValue, LV_ALIGN_CENTER);
    lv_label_set_text(cui_controlScreenRangeValue, "-");
    lv_obj_clear_flag(cui_controlScreenRangeValue, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenRangeValue, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_controlScreenRangeValue, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_controlScreenRangeValue, lv_font_middle, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_controlScreenLeft;
    cui_controlScreenLeft = lv_obj_create(cui_controlScreenColA);
    lv_obj_set_width(cui_controlScreenLeft, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenLeft, 2);
    lv_obj_set_x(cui_controlScreenLeft, 386);
    lv_obj_set_y(cui_controlScreenLeft, 178);
    lv_obj_set_flex_flow(cui_controlScreenLeft, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_controlScreenLeft, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_controlScreenLeft, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenLeft, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenLeft, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenLeft, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenLeft, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_controlScreenLeft, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_controlScreenLeft, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_controlScreenLeft, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlScreenLeft, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenLeft, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_controlScreenLeft, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenLeft, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_controlScreenLeft, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_controlScreenLeft, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_controlScreenLeft, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_controlScreenLeft, 0, LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_text_color(cui_controlScreenLeft, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);

    lv_obj_t *cui_controlScreenLeftIcon;
    cui_controlScreenLeftIcon = lv_label_create(cui_controlScreenLeft);
    lv_obj_set_height(cui_controlScreenLeftIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_flex_grow(cui_controlScreenLeftIcon, 1);
    lv_obj_set_align(cui_controlScreenLeftIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_controlScreenLeftIcon, "u");
    lv_obj_clear_flag(cui_controlScreenLeftIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenLeftIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_controlScreenLeftIcon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_controlScreenLeftIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenLeftIcon, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenLeftIcon, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);

    /* 左下: モーター解除（左矢印ボタンと同デザイン。アイコンは xlcd の "w" = 鍵/ロック系） */
    lv_obj_t *cui_controlScreenMotorUnlock;
    cui_controlScreenMotorUnlock = lv_obj_create(cui_controlScreenColA);
    lv_obj_set_width(cui_controlScreenMotorUnlock, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenMotorUnlock, 2);
    lv_obj_set_x(cui_controlScreenMotorUnlock, 386);
    lv_obj_set_y(cui_controlScreenMotorUnlock, 178);
    lv_obj_set_flex_flow(cui_controlScreenMotorUnlock, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_controlScreenMotorUnlock, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_controlScreenMotorUnlock, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenMotorUnlock, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenMotorUnlock, lv_color_hex(0xCC7A00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenMotorUnlock, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenMotorUnlock, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_controlScreenMotorUnlock, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_controlScreenMotorUnlock, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_controlScreenMotorUnlock, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlScreenMotorUnlock, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenMotorUnlock, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_controlScreenMotorUnlock, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenMotorUnlock, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_controlScreenMotorUnlock, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_controlScreenMotorUnlock, lv_color_hex(0xE08A00), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_controlScreenMotorUnlock, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_controlScreenMotorUnlockIcon;
    cui_controlScreenMotorUnlockIcon = lv_label_create(cui_controlScreenMotorUnlock);
    lv_obj_set_height(cui_controlScreenMotorUnlockIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_flex_grow(cui_controlScreenMotorUnlockIcon, 1);
    lv_obj_set_align(cui_controlScreenMotorUnlockIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_controlScreenMotorUnlockIcon, "A");
    lv_obj_clear_flag(cui_controlScreenMotorUnlockIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenMotorUnlockIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_controlScreenMotorUnlockIcon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_controlScreenMotorUnlockIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenMotorUnlockIcon, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_controlScreenColB;
    cui_controlScreenColB = lv_obj_create(cui_controlComponent);
    lv_obj_set_height(cui_controlScreenColB, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenColB, 1);
    lv_obj_set_x(cui_controlScreenColB, 386);
    lv_obj_set_y(cui_controlScreenColB, 178);
    lv_obj_set_flex_flow(cui_controlScreenColB, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_controlScreenColB, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_controlScreenColB, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenColB, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenColB, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenColB, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenColB, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_controlScreenColB, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_controlScreenColB, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_controlScreenColB, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlScreenColB, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenColB, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_controlScreenColB, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenColB, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_controlScreenColB, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_controlScreenUp;
    cui_controlScreenUp = lv_obj_create(cui_controlScreenColB);
    lv_obj_set_width(cui_controlScreenUp, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenUp, 2);
    lv_obj_set_x(cui_controlScreenUp, 386);
    lv_obj_set_y(cui_controlScreenUp, 178);
    lv_obj_set_flex_flow(cui_controlScreenUp, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_controlScreenUp, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_controlScreenUp, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenUp, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenUp, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenUp, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_controlScreenUp, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_controlScreenUp, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_controlScreenUp, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlScreenUp, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_controlScreenUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenUp, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_controlScreenUp, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_controlScreenUp, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_controlScreenUp, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_controlScreenUpIcon;
    cui_controlScreenUpIcon = lv_label_create(cui_controlScreenUp);
    lv_obj_set_height(cui_controlScreenUpIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_flex_grow(cui_controlScreenUpIcon, 1);
    lv_obj_set_align(cui_controlScreenUpIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_controlScreenUpIcon, "s");
    lv_obj_clear_flag(cui_controlScreenUpIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenUpIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_controlScreenUpIcon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_controlScreenUpIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_controlScreenHome;
    cui_controlScreenHome = lv_obj_create(cui_controlScreenColB);
    lv_obj_set_width(cui_controlScreenHome, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenHome, 2);
    lv_obj_set_x(cui_controlScreenHome, 386);
    lv_obj_set_y(cui_controlScreenHome, 178);
    lv_obj_set_flex_flow(cui_controlScreenHome, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_controlScreenHome, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_controlScreenHome, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenHome, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenHome, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenHome, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenHome, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_controlScreenHome, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_controlScreenHome, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_controlScreenHome, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlScreenHome, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenHome, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_controlScreenHome, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenHome, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_controlScreenHome, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_controlScreenHome, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_controlScreenHome, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_controlScreenHomeIcon;
    cui_controlScreenHomeIcon = lv_label_create(cui_controlScreenHome);
    lv_obj_set_height(cui_controlScreenHomeIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_flex_grow(cui_controlScreenHomeIcon, 1);
    lv_obj_set_align(cui_controlScreenHomeIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_controlScreenHomeIcon, "a");
    lv_obj_clear_flag(cui_controlScreenHomeIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenHomeIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_controlScreenHomeIcon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_controlScreenHomeIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_controlScreenDown;
    cui_controlScreenDown = lv_obj_create(cui_controlScreenColB);
    lv_obj_set_width(cui_controlScreenDown, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenDown, 2);
    lv_obj_set_x(cui_controlScreenDown, 386);
    lv_obj_set_y(cui_controlScreenDown, 178);
    lv_obj_set_flex_flow(cui_controlScreenDown, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_controlScreenDown, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_controlScreenDown, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenDown, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenDown, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenDown, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_controlScreenDown, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_controlScreenDown, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_controlScreenDown, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlScreenDown, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_controlScreenDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenDown, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_controlScreenDown, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_controlScreenDown, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_controlScreenDown, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_controlScreenDownIcon;
    cui_controlScreenDownIcon = lv_label_create(cui_controlScreenDown);
    lv_obj_set_height(cui_controlScreenDownIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_flex_grow(cui_controlScreenDownIcon, 1);
    lv_obj_set_align(cui_controlScreenDownIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_controlScreenDownIcon, "t");
    lv_obj_clear_flag(cui_controlScreenDownIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenDownIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_controlScreenDownIcon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_controlScreenDownIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* AXIS円形パッド（試験実装）: 既存十字は残しつつ非表示にして、新規5ボタンを重ねる */
    lv_obj_add_flag(cui_controlScreenLeft, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cui_controlScreenUp, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cui_controlScreenHome, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cui_controlScreenDown, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cui_controlScreenMotorUnlock, LV_OBJ_FLAG_HIDDEN);
    /* 旧AXIS列は退避して、円形パッド側に面積を寄せる */
    lv_obj_add_flag(cui_controlScreenColA, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_flex_grow(cui_controlScreenColA, 0);
    lv_obj_set_width(cui_controlScreenColA, 0);
    lv_obj_set_flex_grow(cui_controlScreenColB, 3);
    /* 非表示にした旧十字が Flex 高さ配分を奪わないようにする */
    lv_obj_set_flex_grow(cui_controlScreenUp, 0);
    lv_obj_set_flex_grow(cui_controlScreenHome, 0);
    lv_obj_set_flex_grow(cui_controlScreenDown, 0);
    lv_obj_set_height(cui_controlScreenUp, 0);
    lv_obj_set_height(cui_controlScreenHome, 0);
    lv_obj_set_height(cui_controlScreenDown, 0);
    lv_obj_set_style_pad_top(cui_controlScreenColB, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlScreenColB, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenColB, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    /* 左列で range が伸びすぎないよう、サイズを固定して上端に維持 */
    lv_obj_set_flex_grow(cui_controlScreenRange, 0);
    lv_obj_set_height(cui_controlScreenRange, 56);
    lv_obj_add_flag(cui_controlScreenRange, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *cui_controlScreenColC;
    cui_controlScreenColC = lv_obj_create(cui_controlComponent);
    lv_obj_set_height(cui_controlScreenColC, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenColC, 1);
    lv_obj_set_x(cui_controlScreenColC, 386);
    lv_obj_set_y(cui_controlScreenColC, 178);
    lv_obj_set_flex_flow(cui_controlScreenColC, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_controlScreenColC, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_controlScreenColC, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenColC, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenColC, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenColC, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenColC, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_controlScreenColC, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_controlScreenColC, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_controlScreenColC, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlScreenColC, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenColC, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_controlScreenColC, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenColC, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_controlScreenColC, 255, LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_t *cui_controlScreenDummy3;
    cui_controlScreenDummy3 = lv_obj_create(cui_controlScreenColC);
    lv_obj_set_width(cui_controlScreenDummy3, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenDummy3, 2);
    lv_obj_set_x(cui_controlScreenDummy3, 386);
    lv_obj_set_y(cui_controlScreenDummy3, 178);
    lv_obj_set_flex_flow(cui_controlScreenDummy3, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_controlScreenDummy3, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_controlScreenDummy3, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenDummy3, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenDummy3, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenDummy3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenDummy3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_controlScreenDummy3, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_controlScreenDummy3, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_controlScreenDummy3, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlScreenDummy3, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenDummy3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_controlScreenDummy3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenDummy3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_controlScreenDummy3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_controlScreenDummy3, lv_color_hex(0x008800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_controlScreenDummy3, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_controlScreenDummy3Icon;
    cui_controlScreenDummy3Icon = lv_label_create(cui_controlScreenDummy3);
    lv_obj_set_width(cui_controlScreenDummy3Icon, LV_SIZE_CONTENT);
    lv_obj_set_height(cui_controlScreenDummy3Icon, LV_SIZE_CONTENT);
    lv_obj_set_align(cui_controlScreenDummy3Icon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_controlScreenDummy3Icon, "l"); /* icon_l_006C */
    lv_obj_clear_flag(cui_controlScreenDummy3Icon, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(cui_controlScreenDummy3Icon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_controlScreenDummy3Icon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_controlScreenDummy3Icon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_controlScreenRight;
    cui_controlScreenRight = lv_obj_create(cui_controlScreenColC);
    lv_obj_set_width(cui_controlScreenRight, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenRight, 2);
    lv_obj_set_x(cui_controlScreenRight, 386);
    lv_obj_set_y(cui_controlScreenRight, 178);
    lv_obj_set_flex_flow(cui_controlScreenRight, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_controlScreenRight, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_controlScreenRight, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenRight, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenRight, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenRight, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_controlScreenRight, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_controlScreenRight, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_controlScreenRight, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlScreenRight, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_controlScreenRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenRight, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_controlScreenRight, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_controlScreenRight, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_controlScreenRight, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_controlScreenRight, 0, LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_text_color(cui_controlScreenRight, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);

    lv_obj_t *cui_controlScreenRightIcon;
    cui_controlScreenRightIcon = lv_label_create(cui_controlScreenRight);
    lv_obj_set_height(cui_controlScreenRightIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_flex_grow(cui_controlScreenRightIcon, 1);
    lv_obj_set_align(cui_controlScreenRightIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_controlScreenRightIcon, "v");
    lv_obj_clear_flag(cui_controlScreenRightIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenRightIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_controlScreenRightIcon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_controlScreenRightIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenRightIcon, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenRightIcon, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_add_flag(cui_controlScreenRight, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cui_controlScreenColC, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_flex_grow(cui_controlScreenColC, 0);
    lv_obj_set_width(cui_controlScreenColC, 0);

    lv_obj_t *cui_controlScreenDummy1;
    cui_controlScreenDummy1 = lv_obj_create(cui_controlScreenColC);
    lv_obj_set_width(cui_controlScreenDummy1, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenDummy1, 2);
    lv_obj_set_x(cui_controlScreenDummy1, 386);
    lv_obj_set_y(cui_controlScreenDummy1, 178);
    lv_obj_set_flex_flow(cui_controlScreenDummy1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_controlScreenDummy1, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_controlScreenDummy1, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenDummy1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenDummy1, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenDummy1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenDummy1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_controlScreenDummy1, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_controlScreenDummy1, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_controlScreenDummy1, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlScreenDummy1, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenDummy1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_controlScreenDummy1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenDummy1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_controlScreenDummy1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_controlScreenDummy1, lv_color_hex(0x008800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_controlScreenDummy1, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    /* 4-5間を広げるスペーサー */
    lv_obj_t *cui_spacer_45 = lv_obj_create(cui_controlComponent);
    lv_obj_set_width(cui_spacer_45, 8);
    lv_obj_set_height(cui_spacer_45, lv_pct(100));
    lv_obj_set_flex_grow(cui_spacer_45, 0);
    lv_obj_clear_flag(cui_spacer_45, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_set_scrollbar_mode(cui_spacer_45, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(cui_spacer_45, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_spacer_45, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cui_spacer_45, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(cui_spacer_45, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_width(cui_spacer_45, 0);

    lv_obj_t *cui_controlScreenColD;
    cui_controlScreenColD = lv_obj_create(cui_controlComponent);
    lv_obj_set_height(cui_controlScreenColD, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenColD, 0);
    lv_obj_set_width(cui_controlScreenColD, side_button_col_w);
    lv_obj_set_x(cui_controlScreenColD, 386);
    lv_obj_set_y(cui_controlScreenColD, 178);
    lv_obj_set_flex_flow(cui_controlScreenColD, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_controlScreenColD, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_controlScreenColD, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenColD, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenColD, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenColD, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenColD, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_controlScreenColD, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_controlScreenColD, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_controlScreenColD, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlScreenColD, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenColD, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_controlScreenColD, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenColD, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_controlScreenColD, 255, LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_t *cui_controlScreenBedUp;
    cui_controlScreenBedUp = lv_obj_create(cui_controlScreenColD);
    lv_obj_set_width(cui_controlScreenBedUp, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenBedUp, 2);
    lv_obj_set_x(cui_controlScreenBedUp, 386);
    lv_obj_set_y(cui_controlScreenBedUp, 178);
    lv_obj_set_flex_flow(cui_controlScreenBedUp, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_controlScreenBedUp, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_controlScreenBedUp, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenBedUp, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenBedUp, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenBedUp, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenBedUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_controlScreenBedUp, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_controlScreenBedUp, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_controlScreenBedUp, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlScreenBedUp, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenBedUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_controlScreenBedUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenBedUp, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_controlScreenBedUp, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_controlScreenBedUp, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_controlScreenBedUp, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_controlScreenBedUpIcon;
    cui_controlScreenBedUpIcon = lv_label_create(cui_controlScreenBedUp);
    lv_obj_set_height(cui_controlScreenBedUpIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_flex_grow(cui_controlScreenBedUpIcon, 1);
    lv_obj_set_align(cui_controlScreenBedUpIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_controlScreenBedUpIcon, "s");
    lv_obj_clear_flag(cui_controlScreenBedUpIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenBedUpIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_controlScreenBedUpIcon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_controlScreenBedUpIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_t *cui_controlScreenDummy2;
    cui_controlScreenDummy2 = lv_obj_create(cui_controlScreenColD);
    lv_obj_set_width(cui_controlScreenDummy2, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenDummy2, 2);
    lv_obj_set_x(cui_controlScreenDummy2, 386);
    lv_obj_set_y(cui_controlScreenDummy2, 178);
    lv_obj_set_flex_flow(cui_controlScreenDummy2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_controlScreenDummy2, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_controlScreenDummy2, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenDummy2, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenDummy2, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenDummy2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenDummy2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_controlScreenDummy2, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_controlScreenDummy2, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_controlScreenDummy2, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlScreenDummy2, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenDummy2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_controlScreenDummy2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenDummy2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_controlScreenDummy2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_controlScreenDummy2, lv_color_hex(0x008800), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_controlScreenDummy2, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_controlScreenDummy2Icon;
    cui_controlScreenDummy2Icon = lv_label_create(cui_controlScreenDummy2);
    lv_obj_set_width(cui_controlScreenDummy2Icon, LV_SIZE_CONTENT);
    lv_obj_set_height(cui_controlScreenDummy2Icon, LV_SIZE_CONTENT);
    lv_obj_set_align(cui_controlScreenDummy2Icon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_controlScreenDummy2Icon, "l"); /* icon_m_006D */
    lv_obj_clear_flag(cui_controlScreenDummy2Icon, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(cui_controlScreenDummy2Icon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_controlScreenDummy2Icon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_controlScreenDummy2Icon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_controlScreenBedDown;
    cui_controlScreenBedDown = lv_obj_create(cui_controlScreenColD);
    lv_obj_set_width(cui_controlScreenBedDown, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenBedDown, 2);
    lv_obj_set_x(cui_controlScreenBedDown, 386);
    lv_obj_set_y(cui_controlScreenBedDown, 178);
    lv_obj_set_flex_flow(cui_controlScreenBedDown, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_controlScreenBedDown, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_controlScreenBedDown, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenBedDown, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenBedDown, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenBedDown, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenBedDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_controlScreenBedDown, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_controlScreenBedDown, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_controlScreenBedDown, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_controlScreenBedDown, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenBedDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_controlScreenBedDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenBedDown, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_controlScreenBedDown, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_controlScreenBedDown, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_controlScreenBedDown, 255, LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_t *cui_controlScreenBedDownIcon;
    cui_controlScreenBedDownIcon = lv_label_create(cui_controlScreenBedDown);
    lv_obj_set_height(cui_controlScreenBedDownIcon, LV_SIZE_CONTENT); /// 1
    lv_obj_set_flex_grow(cui_controlScreenBedDownIcon, 1);
    lv_obj_set_align(cui_controlScreenBedDownIcon, LV_ALIGN_CENTER);
    lv_label_set_text(cui_controlScreenBedDownIcon, "t");
    lv_obj_clear_flag(cui_controlScreenBedDownIcon, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_controlScreenBedDownIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_controlScreenBedDownIcon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_controlScreenBedDownIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* AXISエリアを一度全削除して、円形メニューとして作り直す */
    lv_obj_del(cui_controlScreenColA);
    lv_obj_del(cui_controlScreenColB);
    lv_obj_del(cui_controlScreenColC);
    lv_obj_del(cui_spacer_45);

    cui_controlScreenColA = lv_obj_create(cui_controlComponent);
    lv_obj_set_height(cui_controlScreenColA, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenColA, 0);
    lv_obj_set_width(cui_controlScreenColA, 0);
    lv_obj_set_flex_flow(cui_controlScreenColA, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_controlScreenColA, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_controlScreenColA, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(cui_controlScreenColA, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(cui_controlScreenColA, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenColA, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cui_controlScreenColA, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_controlScreenColA, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(cui_controlScreenColA, LV_OBJ_FLAG_HIDDEN);

    /* ColA系は互換維持のため最小プレースホルダだけ残す（実体は ColB オーバーレイへ） */
    cui_controlScreenRange = lv_obj_create(cui_controlScreenColA);
    lv_obj_set_size(cui_controlScreenRange, 1, 1);
    lv_obj_set_style_bg_opa(cui_controlScreenRange, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenRange, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    cui_controlScreenRangeIcon = lv_label_create(cui_controlScreenRange);
    lv_label_set_text(cui_controlScreenRangeIcon, "");
    cui_controlScreenRangeValue = lv_label_create(cui_controlScreenRange);
    lv_label_set_text(cui_controlScreenRangeValue, "");

    cui_controlScreenMotorUnlock = lv_obj_create(cui_controlScreenColA);
    lv_obj_set_size(cui_controlScreenMotorUnlock, 1, 1);
    lv_obj_set_style_bg_opa(cui_controlScreenMotorUnlock, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenMotorUnlock, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(cui_controlScreenMotorUnlock, LV_OBJ_FLAG_CLICKABLE);
    cui_controlScreenMotorUnlockIcon = lv_label_create(cui_controlScreenMotorUnlock);
    lv_label_set_text(cui_controlScreenMotorUnlockIcon, "");

    cui_controlScreenColB = lv_obj_create(cui_controlComponent);
    lv_obj_set_height(cui_controlScreenColB, lv_pct(100));
    /* 中央リングを主役にする */
    lv_obj_set_flex_grow(cui_controlScreenColB, 3);
    lv_obj_set_flex_flow(cui_controlScreenColB, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_controlScreenColB, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_controlScreenColB, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(cui_controlScreenColB, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(cui_controlScreenColB, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenColB, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cui_controlScreenColB, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *axisPad = lv_obj_create(cui_controlScreenColB);
    lv_obj_set_size(axisPad, 236, 236);
    lv_obj_set_align(axisPad, LV_ALIGN_CENTER);
    lv_obj_clear_flag(axisPad, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(axisPad, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(axisPad, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(axisPad, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *axisSegUp2 = lv_arc_create(axisPad);
    lv_obj_set_size(axisSegUp2, 230, 230);
    lv_obj_center(axisSegUp2);
    lv_obj_remove_style(axisSegUp2, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(axisSegUp2, 58, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(axisSegUp2, false, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(axisSegUp2, lv_color_hex(0x686868), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(axisSegUp2, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(axisSegUp2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(axisSegUp2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_arc_set_bg_angles(axisSegUp2, 315, 45);
    lv_obj_clear_flag(axisSegUp2, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *axisSegRight2 = lv_arc_create(axisPad);
    lv_obj_set_size(axisSegRight2, 230, 230);
    lv_obj_center(axisSegRight2);
    lv_obj_remove_style(axisSegRight2, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(axisSegRight2, 58, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(axisSegRight2, false, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(axisSegRight2, lv_color_hex(0x5E5E5E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(axisSegRight2, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(axisSegRight2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(axisSegRight2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_arc_set_bg_angles(axisSegRight2, 45, 135);
    lv_obj_clear_flag(axisSegRight2, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *axisSegDown2 = lv_arc_create(axisPad);
    lv_obj_set_size(axisSegDown2, 230, 230);
    lv_obj_center(axisSegDown2);
    lv_obj_remove_style(axisSegDown2, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(axisSegDown2, 58, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(axisSegDown2, false, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(axisSegDown2, lv_color_hex(0x686868), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(axisSegDown2, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(axisSegDown2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(axisSegDown2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_arc_set_bg_angles(axisSegDown2, 135, 225);
    lv_obj_clear_flag(axisSegDown2, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *axisSegLeft2 = lv_arc_create(axisPad);
    lv_obj_set_size(axisSegLeft2, 230, 230);
    lv_obj_center(axisSegLeft2);
    lv_obj_remove_style(axisSegLeft2, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(axisSegLeft2, 58, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(axisSegLeft2, false, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(axisSegLeft2, lv_color_hex(0x5E5E5E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(axisSegLeft2, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(axisSegLeft2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(axisSegLeft2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_arc_set_bg_angles(axisSegLeft2, 225, 315);
    lv_obj_clear_flag(axisSegLeft2, LV_OBJ_FLAG_CLICKABLE);

 

    cui_controlScreenUp = lv_btn_create(axisPad);
    lv_obj_set_size(cui_controlScreenUp, 56, 56);
    lv_obj_align(cui_controlScreenUp, LV_ALIGN_TOP_MID, 0, 1);
    lv_obj_set_style_bg_opa(cui_controlScreenUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    cui_controlScreenUpIcon = lv_label_create(cui_controlScreenUp);
    lv_obj_center(cui_controlScreenUpIcon);
    lv_label_set_text(cui_controlScreenUpIcon, "s");
    lv_obj_set_style_text_font(cui_controlScreenUpIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    cui_controlScreenLeft = lv_btn_create(axisPad);
    lv_obj_set_size(cui_controlScreenLeft, 56, 56);
    lv_obj_align(cui_controlScreenLeft, LV_ALIGN_LEFT_MID, 1, 0);
    lv_obj_set_style_bg_opa(cui_controlScreenLeft, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenLeft, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    cui_controlScreenLeftIcon = lv_label_create(cui_controlScreenLeft);
    lv_obj_center(cui_controlScreenLeftIcon);
    lv_label_set_text(cui_controlScreenLeftIcon, "u");
    lv_obj_set_style_text_font(cui_controlScreenLeftIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    cui_controlScreenRight = lv_btn_create(axisPad);
    lv_obj_set_size(cui_controlScreenRight, 56, 56);
    lv_obj_align(cui_controlScreenRight, LV_ALIGN_RIGHT_MID, -1, 0);
    lv_obj_set_style_bg_opa(cui_controlScreenRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    cui_controlScreenRightIcon = lv_label_create(cui_controlScreenRight);
    lv_obj_center(cui_controlScreenRightIcon);
    lv_label_set_text(cui_controlScreenRightIcon, "v");
    lv_obj_set_style_text_font(cui_controlScreenRightIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    cui_controlScreenDown = lv_btn_create(axisPad);
    lv_obj_set_size(cui_controlScreenDown, 56, 56);
    lv_obj_align(cui_controlScreenDown, LV_ALIGN_BOTTOM_MID, 0, -1);
    lv_obj_set_style_bg_opa(cui_controlScreenDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    cui_controlScreenDownIcon = lv_label_create(cui_controlScreenDown);
    lv_obj_center(cui_controlScreenDownIcon);
    lv_label_set_text(cui_controlScreenDownIcon, "t");
    lv_obj_set_style_text_font(cui_controlScreenDownIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 内側リング: 暗めカラーの4扇ボタン（クリック可能） */
    lv_obj_t *axisInnerUp = lv_arc_create(axisPad);
    lv_obj_set_size(axisInnerUp, 126, 126);
    lv_obj_center(axisInnerUp);
    lv_obj_remove_style(axisInnerUp, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(axisInnerUp, 36, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(axisInnerUp, lv_color_hex(0x4C4C4C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(axisInnerUp, lv_color_hex(0x6A6A6A), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_arc_rounded(axisInnerUp, false, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(axisInnerUp, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(axisInnerUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(axisInnerUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_arc_set_bg_angles(axisInnerUp, 315, 45);
    lv_obj_clear_flag(axisInnerUp, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *axisInnerRight = lv_arc_create(axisPad);
    lv_obj_set_size(axisInnerRight, 126, 126);
    lv_obj_center(axisInnerRight);
    lv_obj_remove_style(axisInnerRight, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(axisInnerRight, 36, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(axisInnerRight, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(axisInnerRight, lv_color_hex(0x6A6A6A), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_arc_rounded(axisInnerRight, false, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(axisInnerRight, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(axisInnerRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(axisInnerRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_arc_set_bg_angles(axisInnerRight, 45, 135);
    lv_obj_clear_flag(axisInnerRight, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *axisInnerDown = lv_arc_create(axisPad);
    lv_obj_set_size(axisInnerDown, 126, 126);
    lv_obj_center(axisInnerDown);
    lv_obj_remove_style(axisInnerDown, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(axisInnerDown, 36, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(axisInnerDown, lv_color_hex(0x4C4C4C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(axisInnerDown, lv_color_hex(0x6A6A6A), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_arc_rounded(axisInnerDown, false, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(axisInnerDown, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(axisInnerDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(axisInnerDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_arc_set_bg_angles(axisInnerDown, 135, 225);
    lv_obj_clear_flag(axisInnerDown, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *axisInnerLeft = lv_arc_create(axisPad);
    lv_obj_set_size(axisInnerLeft, 126, 126);
    lv_obj_center(axisInnerLeft);
    lv_obj_remove_style(axisInnerLeft, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(axisInnerLeft, 36, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(axisInnerLeft, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(axisInnerLeft, lv_color_hex(0x6A6A6A), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_arc_rounded(axisInnerLeft, false, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(axisInnerLeft, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(axisInnerLeft, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(axisInnerLeft, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_arc_set_bg_angles(axisInnerLeft, 225, 315);
    lv_obj_clear_flag(axisInnerLeft, LV_OBJ_FLAG_CLICKABLE);

    /* 内側方向タッチは専用の透明ボタンで判定する */
    lv_obj_t *axisInnerHitUp = lv_btn_create(axisPad);
    lv_obj_set_size(axisInnerHitUp, 62, 44);
    lv_obj_align(axisInnerHitUp, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_style_radius(axisInnerHitUp, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(axisInnerHitUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(axisInnerHitUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(axisInnerHitUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(axisInnerHitUp, LV_OBJ_FLAG_ADV_HITTEST);

    lv_obj_t *axisInnerHitLeft = lv_btn_create(axisPad);
    lv_obj_set_size(axisInnerHitLeft, 44, 62);
    lv_obj_align(axisInnerHitLeft, LV_ALIGN_LEFT_MID, 30, 0);
    lv_obj_set_style_radius(axisInnerHitLeft, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(axisInnerHitLeft, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(axisInnerHitLeft, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(axisInnerHitLeft, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(axisInnerHitLeft, LV_OBJ_FLAG_ADV_HITTEST);

    lv_obj_t *axisInnerHitRight = lv_btn_create(axisPad);
    lv_obj_set_size(axisInnerHitRight, 44, 62);
    lv_obj_align(axisInnerHitRight, LV_ALIGN_RIGHT_MID, -30, 0);
    lv_obj_set_style_radius(axisInnerHitRight, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(axisInnerHitRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(axisInnerHitRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(axisInnerHitRight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(axisInnerHitRight, LV_OBJ_FLAG_ADV_HITTEST);

    lv_obj_t *axisInnerHitDown = lv_btn_create(axisPad);
    lv_obj_set_size(axisInnerHitDown, 62, 44);
    lv_obj_align(axisInnerHitDown, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_set_style_radius(axisInnerHitDown, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(axisInnerHitDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(axisInnerHitDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(axisInnerHitDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(axisInnerHitDown, LV_OBJ_FLAG_ADV_HITTEST);

    /* 内側リング4＋ヒット4 は一旦非表示（外側リングのみ） */
    lv_obj_add_flag(axisInnerUp, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(axisInnerRight, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(axisInnerDown, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(axisInnerLeft, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(axisInnerHitUp, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(axisInnerHitLeft, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(axisInnerHitRight, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(axisInnerHitDown, LV_OBJ_FLAG_HIDDEN);

    /* 1-10-100 / MotorUnlock は ColB 上でオーバーレイ配置 */
    cui_controlScreenRange = lv_obj_create(cui_controlScreenColB);
    lv_obj_add_flag(cui_controlScreenRange, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_set_size(cui_controlScreenRange, 55, 55);
    lv_obj_align(cui_controlScreenRange, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_scrollbar_mode(cui_controlScreenRange, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenRange, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenRange, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenRange, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_controlScreenRange, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cui_controlScreenRange, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_controlScreenRange, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_controlScreenRange, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_move_foreground(cui_controlScreenRange);
    cui_controlScreenRangeIcon = lv_label_create(cui_controlScreenRange);
    lv_label_set_text(cui_controlScreenRangeIcon, "k");
    lv_obj_center(cui_controlScreenRangeIcon);
    lv_obj_set_style_text_font(cui_controlScreenRangeIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_controlScreenRangeIcon, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    cui_controlScreenRangeValue = lv_label_create(cui_controlScreenColB);
    lv_obj_add_flag(cui_controlScreenRangeValue, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_label_set_text(cui_controlScreenRangeValue, "-");
    /* 旧ホーム位置（リング中央）— タップは下の方向ボタンへ通す */
    lv_obj_align(cui_controlScreenRangeValue, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(cui_controlScreenRangeValue, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_text_align(cui_controlScreenRangeValue, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_controlScreenRangeValue, lv_font_middle, LV_PART_MAIN | LV_STATE_DEFAULT);

    cui_controlScreenMotorUnlock = lv_obj_create(cui_controlScreenColB);
    lv_obj_add_flag(cui_controlScreenMotorUnlock, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_set_size(cui_controlScreenMotorUnlock, 55, 55);
    lv_obj_align(cui_controlScreenMotorUnlock, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_flex_flow(cui_controlScreenMotorUnlock, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_controlScreenMotorUnlock, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(cui_controlScreenMotorUnlock, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_controlScreenMotorUnlock, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenMotorUnlock, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_controlScreenMotorUnlock, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_controlScreenMotorUnlock, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_controlScreenMotorUnlock, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_controlScreenMotorUnlock, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cui_controlScreenMotorUnlock, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_move_foreground(cui_controlScreenMotorUnlock);
    cui_controlScreenMotorUnlockIcon = lv_label_create(cui_controlScreenMotorUnlock);
    lv_label_set_text(cui_controlScreenMotorUnlockIcon, "A");
    lv_obj_set_style_text_font(cui_controlScreenMotorUnlockIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);


   /* 右上 XY（表示のみ・背景は透明のまま円形で角見切れを防ぐ） */
    lv_obj_t *axisXyIcon = lv_obj_create(cui_controlScreenColB);
    lv_obj_add_flag(axisXyIcon, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_set_size(axisXyIcon, 55, 55);
    lv_obj_align(axisXyIcon, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_flex_flow(axisXyIcon, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(axisXyIcon, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(axisXyIcon, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(axisXyIcon, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(axisXyIcon, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(axisXyIcon, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(axisXyIcon, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(axisXyIcon, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(axisXyIcon, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *axisXyIconLabel = lv_label_create(axisXyIcon);
    lv_label_set_text(axisXyIconLabel, "m");
    lv_obj_set_style_text_font(axisXyIconLabel, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(axisXyIconLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_move_foreground(axisXyIcon);

    /* Home は ColB オーバーレイ（右下・押せるボタン） */
    cui_controlScreenHome = lv_btn_create(cui_controlScreenColB);
    lv_obj_add_flag(cui_controlScreenHome, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_set_size(cui_controlScreenHome, 55, 55);
    lv_obj_align(cui_controlScreenHome, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_scrollbar_mode(cui_controlScreenHome, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_controlScreenHome, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_controlScreenHome, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_controlScreenHome, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_controlScreenHome, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_controlScreenHome, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_controlScreenHome, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cui_controlScreenHome, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_move_foreground(cui_controlScreenHome);
    cui_controlScreenHomeIcon = lv_label_create(cui_controlScreenHome);
    lv_obj_center(cui_controlScreenHomeIcon);
    lv_label_set_text(cui_controlScreenHomeIcon, "a");
    lv_obj_set_style_text_font(cui_controlScreenHomeIcon, lv_icon_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_move_foreground(cui_controlScreenRangeValue);

    s_control_axis.axisPad = axisPad;
    s_control_axis.rangeBtn = cui_controlScreenRange;
    s_control_axis.motorUnlock = cui_controlScreenMotorUnlock;
    s_control_axis.home = cui_controlScreenHome;
    s_control_axis.xyIcon = axisXyIcon;

    cui_controlScreenColC = lv_obj_create(cui_controlComponent);
    lv_obj_set_width(cui_controlScreenColC, 0);
    lv_obj_set_height(cui_controlScreenColC, lv_pct(100));
    lv_obj_set_flex_grow(cui_controlScreenColC, 0);
    lv_obj_set_style_bg_opa(cui_controlScreenColC, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenColC, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cui_controlScreenColC, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(cui_controlScreenColC, LV_OBJ_FLAG_HIDDEN);
    cui_controlScreenDummy3 = lv_obj_create(cui_controlScreenColC);
    lv_obj_set_size(cui_controlScreenDummy3, 1, 1);
    lv_obj_set_style_bg_opa(cui_controlScreenDummy3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenDummy3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    cui_controlScreenDummy1 = lv_obj_create(cui_controlScreenColC);
    lv_obj_set_size(cui_controlScreenDummy1, 1, 1);
    lv_obj_set_style_bg_opa(cui_controlScreenDummy1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_controlScreenDummy1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 並びを固定: Left(nozzle) - Center(ring) - Right(Z) */
    int32_t colD_index = lv_obj_get_index(cui_controlScreenColD);
    lv_obj_move_to_index(cui_controlScreenColA, colD_index);
    lv_obj_move_to_index(cui_controlScreenColB, colD_index + 1);
    lv_obj_move_to_index(cui_controlScreenColC, colD_index + 2);

    lv_obj_update_layout(cui_controlComponent);
    {
        lv_coord_t cw = lv_obj_get_content_width(cui_controlScreenColB);
        lv_coord_t ch = lv_obj_get_content_height(cui_controlScreenColB);
        if (cw > 0 && ch > 0)
            control_axis_apply_scale(cw < ch ? cw : ch);
    }
    lv_obj_add_event_cb(cui_controlScreenColB, control_colb_axis_size_cb, LV_EVENT_SIZE_CHANGED, NULL);

    lv_obj_t **children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_CONTROLCOMPONENT_NUM);
    children[UI_COMP_CONTROLCOMPONENT_CONTROLCOMPONENT] = cui_controlComponent;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLA] = cui_controlScreenColA;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLA_CONTROLSCREENRANGE] = cui_controlScreenRange;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLA_CONTROLSCREENRANGE_CONTROLSCREENRANGEICON] = cui_controlScreenRangeIcon;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLA_CONTROLSCREENRANGE_CONTROLSCREENRANGEVALUE] = cui_controlScreenRangeValue;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLA_CONTROLSCREENLEFT] = cui_controlScreenLeft;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLA_CONTROLSCREENLEFT_CONTROLSCREENLEFTICON] = cui_controlScreenLeftIcon;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLA_CONTROLSCREENMOTORUNLOCK] = cui_controlScreenMotorUnlock;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLA_CONTROLSCREENMOTORUNLOCK_CONTROLSCREENMOTORUNLOCKICON] = cui_controlScreenMotorUnlockIcon;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLB] = cui_controlScreenColB;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLB_CONTROLSCREENUP] = cui_controlScreenUp;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLB_CONTROLSCREENUP_CONTROLSCREENUPICON] = cui_controlScreenUpIcon;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLB_CONTROLSCREENHOME] = cui_controlScreenHome;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLB_CONTROLSCREENHOME_CONTROLSCREENHOMEICON] = cui_controlScreenHomeIcon;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLB_CONTROLSCREENDOWN] = cui_controlScreenDown;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLB_CONTROLSCREENDOWN_CONTROLSCREENDOWNICON] = cui_controlScreenDownIcon;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLC] = cui_controlScreenColC;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLC_CONTROLSCREENDUMMY3] = cui_controlScreenDummy3;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLC_CONTROLSCREENRIGHT] = cui_controlScreenRight;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLC_CONTROLSCREENRIGHT_CONTROLSCREENRIGHTICON] = cui_controlScreenRightIcon;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLC_CONTROLSCREENDUMMY1] = cui_controlScreenDummy1;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLD] = cui_controlScreenColD;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLD_CONTROLSCREENBEDUP] = cui_controlScreenBedUp;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLD_CONTROLSCREENBEDUP_CONTROLSCREENBEDUPICON] = cui_controlScreenBedUpIcon;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLD_CONTROLSCREENDUMMY2] = cui_controlScreenDummy2;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLD_CONTROLSCREENBEDDOWN] = cui_controlScreenBedDown;
    children[UI_COMP_CONTROLCOMPONENT_CONTROLSCREENCOLD_CONTROLSCREENBEDDOWN_CONTROLSCREENBEDDOWNICON] = cui_controlScreenBedDownIcon;

    lv_obj_add_event_cb(cui_controlComponent, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(cui_controlComponent, del_component_child_event_cb, LV_EVENT_DELETE, children);
    lv_obj_add_event_cb(cui_controlScreenRange, ui_event_comp_controlComponent_controlScreenRange, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_controlScreenLeft, ui_event_comp_controlComponent_controlScreenLeft, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_controlScreenUp, ui_event_comp_controlComponent_controlScreenUp, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_controlScreenHome, ui_event_comp_controlComponent_controlScreenHome, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_controlScreenDown, ui_event_comp_controlComponent_controlScreenDown, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_controlScreenBedUp, ui_event_comp_controlComponent_controlScreenBedUp, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_controlScreenBedDown, ui_event_comp_controlComponent_controlScreenBedDown, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_controlScreenRight, ui_event_comp_controlComponent_controlScreenRight, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_controlScreenMotorUnlock, ui_event_comp_controlComponent_controlScreenMotorUnlock, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(axisInnerHitLeft, ui_event_comp_controlComponent_controlScreenLeft, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(axisInnerHitUp, ui_event_comp_controlComponent_controlScreenUp, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(axisInnerHitDown, ui_event_comp_controlComponent_controlScreenDown, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(axisInnerHitRight, ui_event_comp_controlComponent_controlScreenRight, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_nozzleUp, ui_event_comp_controlComponent_nozzleUpClick, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_nozzleDown, ui_event_comp_controlComponent_nozzleDownClick, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_nozzleTempBtn, ui_event_comp_controlComponent_nozzleTempClick, LV_EVENT_ALL, children);
    lv_obj_add_event_cb(cui_nozzleTempVal, ui_event_comp_controlComponent_nozzleTemp, LV_EVENT_MSG_RECEIVED, NULL);

    lv_obj_add_event_cb(cui_controlScreenRangeValue, onXtouchRangeChange, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_CONTROL_INC_SWITCH, cui_controlScreenRangeValue, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_NOZZLE_TEMP, cui_nozzleTempVal, NULL);

    ui_comp_controlComponent_create_hook(cui_controlComponent);
    return cui_controlComponent;
}