
#include "ui_comp_charactercomponent.h"
#include "../ui.h"

// 関数宣言
void xtouch_events_onCharacterAnimation();
void calc_position(int x_offset, int y_offset);
void xtouch_character_timer_init();
void xtouch_character_timer_stop();
void xtouch_character_timer_handler(lv_timer_t *timer);

// グローバル変数の定義
lv_timer_t *xtouch_character_timer = NULL;
bool xtouch_character_timer_started = false;

// キャラクター制御用変数
bool is_blinking = false;
int eye_blink_counter = 0;
bool is_mouth_animating = false;
int mouth_animation_counter = 0;
int mouth_pakupaku_count = 0; // パクパクの回数カウンター
int position_change_counter = 0; // 位置変更用カウンター

// 座標計算用変数
const int screen_width = 320;
const int screen_height = 240;
const int center_x = screen_width / 2;
const int center_y = screen_height / 2;
const int eye_size = 8;
const int mouth_width = 30;
const int mouth_height = 5;
const int eye_offset_x = 65;
const int eye_offset_y = -30;
const int eye_diff = 120;
const int mouth_offset_x = 45;
const int mouth_offset_y = 30;

int left_eye_x = 0;
int right_eye_x = 0;
int eye_y = 0;
int mouth_x = 0;
int mouth_y = 0;



// キャラクター関連イベント関数
void xtouch_events_onCharacterAnimation()
{
    // screenが9の場合のみ処理を実行
    if (xTouchConfig.currentScreenIndex != 9)
    {
        xtouch_character_timer_stop();
        return;
    }

    struct XTOUCH_MESSAGE_DATA eventData;
    eventData.data = 0;
    eventData.data2 = 0;    
    // 瞬きの処理
    if (is_blinking)
    {
        // 2回のタイマー呼び出し（100ms）で目を開く
        if (eye_blink_counter >= 2)
        {
            eventData.data = 8;
            lv_msg_send(XTOUCH_ON_CHARACTER_LEFT_EYE_UPDATE, &eventData);
            lv_msg_send(XTOUCH_ON_CHARACTER_RIGHT_EYE_UPDATE, &eventData);
            is_blinking = false;
            eye_blink_counter = 0;
        }
    }
    else
    {
        // ランダムで瞬き（約3秒に1回）
        if (eye_blink_counter >= 60 && rand() % 100 < 2)
        {
            is_blinking = true;
            eye_blink_counter = 0;
            // 目を細くする（瞬き開始）
            eventData.data = 2;
            lv_msg_send(XTOUCH_ON_CHARACTER_LEFT_EYE_UPDATE, &eventData);
            lv_msg_send(XTOUCH_ON_CHARACTER_RIGHT_EYE_UPDATE, &eventData);
        }
    }

    // 口のアニメーション処理
    if (is_mouth_animating)
    {
        // パクパクアニメーション（2回のタイマー呼び出しで1回のパクパク）
        if (mouth_animation_counter >= 2)
        {
            mouth_animation_counter = 0;
            mouth_pakupaku_count++;
            
            // 3回パクパクしたら終了
            if (mouth_pakupaku_count >= 3)
            {
                // 口を元のサイズに戻す
                eventData.data = mouth_height;
                lv_msg_send(XTOUCH_ON_CHARACTER_MOUTH_UPDATE, &eventData);

                is_mouth_animating = false;
                mouth_animation_counter = 0;
                mouth_pakupaku_count = 0;
            }
            else
            {
                // 口を閉じる（元のサイズ）
                eventData.data = mouth_height;
                lv_msg_send(XTOUCH_ON_CHARACTER_MOUTH_UPDATE, &eventData);
            }
        }
        else
        {
            // 口を開く（4倍の高さに拡大）
            eventData.data = mouth_height * 4;
            lv_msg_send(XTOUCH_ON_CHARACTER_MOUTH_UPDATE, &eventData);
        }
    }
    else
    {
        // ランダムで口のアニメーション（約5秒に1回）
        if (mouth_animation_counter >= 100 && rand() % 100 < 1)
        {
            is_mouth_animating = true;
            mouth_animation_counter = 0;
            mouth_pakupaku_count = 0;
        }
    }

    // 位置変更の処理
    position_change_counter++;
    // ランダムで位置変更（約10秒に1回）
    if (position_change_counter >= 200 && rand() % 100 < 1)
    {
        // -50から+50の範囲でランダムな位置に移動
        int random_x = (rand() % (320-159)) ; // -50 から +50
        int random_y = (rand() % (240-150)); // -50 から +50
        position_change_counter = 0;
        
        
        // 方法2: 1つのメッセージで data と data2 を同時に送信する場合の例
        eventData.data = random_x;   // data に X座標
        eventData.data2 = random_y;  // data2 に Y座標
        lv_msg_send(XTOUCH_ON_CHARACTER_FACEPOTITION_UPDATE, &eventData);
    }
}

// イベントハンドラー
void ui_event_comp_characterComponent_characterFace(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if (event_code == LV_EVENT_CLICKED)
    {
        onTouchStackChan(e);
    }
}




// タイマー関連の関数
void xtouch_character_timer_init()
{
    if (xtouch_character_timer_started && xtouch_character_timer != NULL)
    {
        xtouch_character_timer_stop();
    }
    xtouch_character_timer = lv_timer_create(xtouch_character_timer_handler, 50, NULL);
    lv_timer_set_repeat_count(xtouch_character_timer, -1);
    xtouch_character_timer_started = true;
}

void xtouch_character_timer_stop()
{
    if(xtouch_character_timer != NULL)lv_timer_del(xtouch_character_timer);
    xtouch_character_timer = NULL;
    xtouch_character_timer_started = false;
}

void xtouch_character_timer_handler(lv_timer_t *timer)
{
    // screenが9の場合のみ処理を実行
    if (xTouchConfig.currentScreenIndex != 9) {
        xtouch_character_timer_stop();
        return;
    }

    eye_blink_counter++;
    mouth_animation_counter++;

    xtouch_events_onCharacterAnimation();
}

void onXTouchCharacterHeightUpdate(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);
    
    struct XTOUCH_MESSAGE_DATA *message = (struct XTOUCH_MESSAGE_DATA *)m->payload;
printf("onXTouchCharacterHeightUpdate %d\n", message->data);   
    lv_obj_set_height(target, message->data);
}

// XとYを同時に更新する例（data2を使用）
void onXTouchCharacterPositionXYUpdate(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);
    
    struct XTOUCH_MESSAGE_DATA *message = (struct XTOUCH_MESSAGE_DATA *)m->payload;
printf("onXTouchCharacterPositionXYUpdate %d %d\n", message->data, message->data2);  

    // data に X座標、data2 に Y座標が入っている
    lv_obj_set_pos(target, message->data, message->data2);  // XとYを同時に設定
}


lv_obj_t *ui_characterComponent_create(lv_obj_t *comp_parent)
{
printf("ui_characterComponent_create\n");
    // 顔のセット
    lv_obj_t *ui_characterComponent = lv_obj_create(comp_parent);
    lv_obj_set_width(ui_characterComponent, 158);
    lv_obj_set_height(ui_characterComponent, 155);
    lv_obj_set_pos(ui_characterComponent, center_x - 79, center_y - 75);
    lv_obj_clear_flag(ui_characterComponent, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_add_flag(ui_characterComponent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(ui_characterComponent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(ui_characterComponent,10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_characterComponent, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_characterComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_characterComponent, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_characterComponent, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(ui_characterComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(ui_characterComponent, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_characterComponent, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_characterDisplay = lv_obj_create(ui_characterComponent);
    lv_obj_set_width(ui_characterDisplay, 128);
    lv_obj_set_height(ui_characterDisplay, 100);
    lv_obj_clear_flag(ui_characterDisplay, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(ui_characterDisplay, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(ui_characterDisplay,10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_characterDisplay, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_characterDisplay, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_characterDisplay, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_characterDisplay, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_characterDisplay, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_characterDisplay, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_characterDisplay, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *sw_box = lv_obj_create(ui_characterComponent);
    lv_obj_set_width(sw_box, 128);
    lv_obj_set_height(sw_box, 20);
    lv_obj_clear_flag(sw_box, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(sw_box, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(sw_box, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(sw_box, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(sw_box, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(sw_box, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(sw_box, LV_FLEX_FLOW_ROW);  // 横に並べる
    lv_obj_set_flex_align(sw_box, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(sw_box, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(sw_box, 5, LV_PART_MAIN | LV_STATE_DEFAULT);  // ボタン間の隙間を5ピクセルに設定

    lv_obj_t *sw1 = lv_obj_create(sw_box);
    lv_obj_set_width(sw1, 128/3-4);
    lv_obj_set_height(sw1, 20);
    lv_obj_clear_flag(sw1, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(sw1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(sw1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(sw1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(sw1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(sw1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *sw2 = lv_obj_create(sw_box);
    lv_obj_set_width(sw2, 128/3-4);
    lv_obj_set_height(sw2, 20);
    lv_obj_clear_flag(sw2, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(sw2, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(sw2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(sw2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(sw2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(sw2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *sw3 = lv_obj_create(sw_box);
    lv_obj_set_width(sw3, 128/3-3);
    lv_obj_set_height(sw3, 20);
    lv_obj_clear_flag(sw3, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(sw3, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(sw3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(sw3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(sw3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(sw3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 左目（白い点）
    lv_obj_t *left_eye = lv_obj_create(ui_characterDisplay);
    lv_obj_set_width(left_eye, eye_size); //8
    lv_obj_set_height(left_eye, eye_size);
    lv_obj_set_x(left_eye, 3);
    lv_obj_set_y(left_eye, 30);
    lv_obj_set_style_radius(left_eye, eye_size / 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(left_eye, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(left_eye, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(left_eye, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(left_eye, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 右目（白い点）
    lv_obj_t *right_eye = lv_obj_create(ui_characterDisplay);
    lv_obj_set_width(right_eye, eye_size); //8
    lv_obj_set_height(right_eye, eye_size);
    lv_obj_set_x(right_eye, 95);
    lv_obj_set_y(right_eye, 30);
    lv_obj_set_style_radius(right_eye, eye_size / 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(right_eye, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(right_eye, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(right_eye, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(right_eye, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 口（白い線）
    lv_obj_t *mouth = lv_obj_create(ui_characterDisplay);
    lv_obj_set_width(mouth, mouth_width);
    lv_obj_set_height(mouth, mouth_height);
    lv_obj_set_x(mouth, 38);
    lv_obj_set_y(mouth, 60);
    lv_obj_set_style_radius(mouth, mouth_height / 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(mouth, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(mouth, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(mouth, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(mouth, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
printf("mouth created\n");
printf("object created\n");
    lv_obj_t **children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_CHARACTORPANEL_NUM);
    children[UI_COMP_CHARACTORPANEL_CHARACTORPANEL] = ui_characterComponent;
    // children[UI_COMP_CHARACTORPANEL_RIGHT_EYE] = right_eye;
    // children[UI_COMP_CHARACTORPANEL_LEFT_EYE] = left_eye;
    // children[UI_COMP_CHARACTORPANEL_MOUSE] = mouth;
printf("ui_event_comp_characterComponent_characterFace\n");
    // イベントハンドラー設定
    lv_obj_add_event_cb(ui_characterComponent, ui_event_comp_characterComponent_characterFace, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_characterComponent, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(ui_characterComponent, del_component_child_event_cb, LV_EVENT_DELETE, children);

    lv_obj_add_event_cb(ui_characterComponent, onXTouchCharacterPositionXYUpdate, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_CHARACTER_FACEPOTITION_UPDATE, ui_characterComponent, NULL);


    lv_obj_add_event_cb(left_eye, onXTouchCharacterHeightUpdate, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_CHARACTER_LEFT_EYE_UPDATE, left_eye, NULL);
    lv_obj_add_event_cb(right_eye, onXTouchCharacterHeightUpdate, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_CHARACTER_RIGHT_EYE_UPDATE, right_eye, NULL);
    lv_obj_add_event_cb(mouth, onXTouchCharacterHeightUpdate, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_CHARACTER_MOUTH_UPDATE, mouth, NULL);

    ui_comp_characterComponent_create_hook(ui_characterComponent);
printf("return ui_characterComponent\n");
    return ui_characterComponent;
}
