
#include "ui_comp_charactercomponent.h"
#include "../ui.h"

// 関数宣言
void xtouch_events_onCharacterAnimation();
void calc_position(int x_offset, int y_offset);
void xtouch_character_init();  // 初期化関数（タイマー不要）

// キャラクター制御用変数
bool is_blinking = false;
unsigned long last_blink_time = 0;  // 最後に瞬きした時刻
unsigned long blink_start_time = 0;  // 瞬き開始時刻
bool is_mouth_animating = false;
unsigned long last_mouth_time = 0;   // 最後に口を動かした時刻
unsigned long mouth_animation_start_time = 0;  // 口アニメーション開始時刻
unsigned long last_mouth_update_time = 0;  // 最後に口の状態を更新した時刻
bool mouth_is_open = false;  // 口が開いているかどうか
int mouth_pakupaku_count = 0; // パクパクの回数カウンター
unsigned long last_position_time = 0;  // 最後に位置変更した時刻
int eye_position = 0;  // 目の基準位置からのオフセット（-8～8）

// 座標計算用変数
#if defined(__XTOUCH_SCREEN_28__)
const int screen_width = 320;
const int screen_height = 240;
#elif defined(__XTOUCH_SCREEN_50__)
const int screen_width = 800;
const int screen_height = 480;
#endif

const int center_x = screen_width / 2;
const int center_y = screen_height / 2;
const int character_width = 158;
const int character_height = 155;
const int character_padding = 10;
const int character_padding_low = 5;
const int character_border_width = 2;
const int charactor_display_width = character_width - character_padding*2 - character_border_width*2;

const int sw_box_height = 20;
const int sw_box_padding_column = 5;
const int eye_size = 8;
const int eye_diff = 92;
const int eye_position_x = (charactor_display_width - (eye_diff + eye_size *2)) / 2;//センタリング片方の目は考慮に入れない
const int eye_position_y = 30;

const int mouth_width = 30;
const int mouth_height = 5;
const int mouth_position_x = (charactor_display_width - (mouth_width) - character_border_width*2) / 2;//センタリング
const int mouth_position_y = eye_position_y + 30;

// キャラクター関連イベント関数
void xtouch_events_onCharacterAnimation()
{
    // screenが9の場合のみ処理を実行
    if (xTouchConfig.currentScreenIndex != 9)
    {
        return;
    }

    unsigned long current_time = millis();
    struct XTOUCH_MESSAGE_DATA eventData;
    eventData.data = 0;
    eventData.data2 = 0;

    // 瞬きの処理
    if (is_blinking)
    {
        // 100ms経過で目を開く
        if (current_time - blink_start_time >= 100)
        {
            eventData.data = 8;
            lv_msg_send(XTOUCH_ON_CHARACTER_LEFT_EYE_UPDATE, &eventData);
            lv_msg_send(XTOUCH_ON_CHARACTER_RIGHT_EYE_UPDATE, &eventData);
            is_blinking = false;
            last_blink_time = current_time;
        }
    }
    else
    {
        // 約3秒（3000ms）経過してランダムで瞬き
        if (current_time - last_blink_time >= 3000 && rand() % 100 < 2)
        {
            // 目を細くする（瞬き開始）
            eventData.data = 2;
            eventData.data2 = 0;
            lv_msg_send(XTOUCH_ON_CHARACTER_LEFT_EYE_UPDATE, &eventData);
            lv_msg_send(XTOUCH_ON_CHARACTER_RIGHT_EYE_UPDATE, &eventData);

            is_blinking = true;
            blink_start_time = current_time;
                        
            // 瞬きのタイミングでランダムに左右に向くアクション
            // -8～8の間で乱数を取得し、基準位置に加減算
            eye_position = (rand() % 17) - 8;  // -8から8の範囲

            // 左目のX位置を更新
            eventData.data = eye_position_x + eye_position;
            eventData.data2 = 0;
            lv_msg_send(XTOUCH_ON_CHARACTER_LEFT_EYE_POSITION_X_UPDATE, &eventData);
            
            // 右目のX位置を更新
            eventData.data = eye_position_x + eye_diff + eye_position;
            eventData.data2 = 0;
            lv_msg_send(XTOUCH_ON_CHARACTER_RIGHT_EYE_POSITION_X_UPDATE, &eventData);

        }
    }

    // 口のアニメーション処理
    if (is_mouth_animating)
    {
        // パクパクアニメーション（100ms間隔で開閉）
        if (current_time - last_mouth_update_time >= 100)
        {
            last_mouth_update_time = current_time;
            mouth_is_open = !mouth_is_open;  // 開閉を切り替え
            
            if (mouth_is_open)
            {
                // 口を開く（4倍の高さに拡大）
                eventData.data = mouth_height * 4;
                lv_msg_send(XTOUCH_ON_CHARACTER_MOUTH_UPDATE, &eventData);
                mouth_pakupaku_count++;
            }
            else
            {
                // 口を閉じる（元のサイズ）
                eventData.data = mouth_height;
                lv_msg_send(XTOUCH_ON_CHARACTER_MOUTH_UPDATE, &eventData);
            }
            
            // 3回パクパク（6回の状態変更）したら終了
            if (mouth_pakupaku_count >= 6)
            {
                is_mouth_animating = false;
                mouth_pakupaku_count = 0;
                last_mouth_time = current_time;
                // 口を閉じる（元のサイズ）
                eventData.data = mouth_height;
            lv_msg_send(XTOUCH_ON_CHARACTER_MOUTH_UPDATE, &eventData);
                mouth_is_open = false;
            }
        }
    }
    else
    {
        // 約5秒（5000ms）経過してランダムで口のアニメーション
        if (current_time - last_mouth_time >= 5000 && rand() % 100 < 1)
        {
            is_mouth_animating = true;
            mouth_animation_start_time = current_time;
            last_mouth_update_time = current_time;
            mouth_pakupaku_count = 0;
            mouth_is_open = false;
        }
    }

    // 位置変更の処理
    // 約10秒（10000ms）経過してランダムで位置変更
    if (current_time - last_position_time >= 10000 && rand() % 100 < 1)
    {
        // ランダムな位置に移動
        int random_x = (rand() % (screen_width-character_width));
        int random_y = (rand() % (screen_height-character_height));
        last_position_time = current_time;
        
        // 1つのメッセージで data と data2 を同時に送信
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




// キャラクター初期化関数（タイマー不要、loop()で直接呼び出す）
void xtouch_character_init()
{
    // millis()ベースの変数を初期化
    unsigned long current_time = millis();
    last_blink_time = current_time;
    last_mouth_time = current_time;
    last_position_time = current_time;
    is_blinking = false;
    is_mouth_animating = false;
    mouth_pakupaku_count = 0;
    mouth_is_open = false;
    eye_position = 0;  // 目の位置オフセットを初期化
}

void onXTouchCharacterHeightUpdate(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);
    
    // メッセージIDをチェック（高さ更新メッセージのみ処理）
    uint32_t msg_id = lv_msg_get_id(m);
    if (msg_id != XTOUCH_ON_CHARACTER_LEFT_EYE_UPDATE && 
        msg_id != XTOUCH_ON_CHARACTER_RIGHT_EYE_UPDATE &&
        msg_id != XTOUCH_ON_CHARACTER_MOUTH_UPDATE) {
        return;  // 該当するメッセージでない場合は処理しない
    }
    
    struct XTOUCH_MESSAGE_DATA *message = (struct XTOUCH_MESSAGE_DATA *)lv_msg_get_payload(m);
    lv_obj_set_height(target, message->data);
}

// 目のX位置を更新するイベントハンドラー
void onXTouchCharacterEyePositionXUpdate(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);
    
    // メッセージIDをチェック（位置更新メッセージのみ処理）
    uint32_t msg_id = lv_msg_get_id(m);
    if (msg_id != XTOUCH_ON_CHARACTER_LEFT_EYE_POSITION_X_UPDATE && 
        msg_id != XTOUCH_ON_CHARACTER_RIGHT_EYE_POSITION_X_UPDATE) {
        return;  // 該当するメッセージでない場合は処理しない
    }
    
    struct XTOUCH_MESSAGE_DATA *message = (struct XTOUCH_MESSAGE_DATA *)lv_msg_get_payload(m);
    lv_obj_set_x(target, message->data);
}

// XとYを同時に更新する例（data2を使用）
void onXTouchCharacterPositionXYUpdate(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);
    
    struct XTOUCH_MESSAGE_DATA *message = (struct XTOUCH_MESSAGE_DATA *)m->payload;

    // data に X座標、data2 に Y座標が入っている
    lv_obj_set_pos(target, message->data, message->data2);  // XとYを同時に設定
}


lv_obj_t *ui_characterComponent_create(lv_obj_t *comp_parent)
{
    // 顔のセット
    lv_obj_t *ui_characterComponent = lv_obj_create(comp_parent);
    lv_obj_set_width(ui_characterComponent, character_width);
    lv_obj_set_height(ui_characterComponent, character_height);
    lv_obj_set_pos(ui_characterComponent, center_x - character_width, center_y/2 - character_height/2);
    lv_obj_clear_flag(ui_characterComponent, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_add_flag(ui_characterComponent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(ui_characterComponent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(ui_characterComponent,10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_characterComponent, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_characterComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_characterComponent, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_characterComponent, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(ui_characterComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_left(ui_characterComponent, character_padding, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_characterComponent, character_padding, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_characterComponent, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_characterComponent, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_characterComponent, character_padding_low, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_characterComponent, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_characterDisplay = lv_obj_create(ui_characterComponent);
    lv_obj_set_width(ui_characterDisplay, charactor_display_width);
    lv_obj_set_height(ui_characterDisplay, character_height - character_padding*2 - sw_box_height - character_padding_low);
    lv_obj_clear_flag(ui_characterDisplay, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(ui_characterDisplay, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(ui_characterDisplay,10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_characterDisplay, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_characterDisplay, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_characterDisplay, character_border_width, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_characterDisplay, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_characterDisplay, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_characterDisplay, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_characterDisplay, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_characterDisplay, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *sw_box = lv_obj_create(ui_characterComponent);
    lv_obj_set_width(sw_box, charactor_display_width);
    lv_obj_set_height(sw_box, sw_box_height);
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
    lv_obj_set_width(sw1, (charactor_display_width-sw_box_padding_column*2)/3);
    lv_obj_set_height(sw1, sw_box_height);
    lv_obj_clear_flag(sw1, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(sw1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(sw1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(sw1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(sw1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(sw1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(sw1,2, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *sw2 = lv_obj_create(sw_box);
    lv_obj_set_width(sw2, (charactor_display_width-sw_box_padding_column*2)/3);
    lv_obj_set_height(sw2, sw_box_height);
    lv_obj_clear_flag(sw2, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(sw2, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(sw2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(sw2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(sw2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(sw2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(sw2,2, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *sw3 = lv_obj_create(sw_box);
    lv_obj_set_width(sw3, (charactor_display_width-sw_box_padding_column*2)/3);
    lv_obj_set_height(sw3, sw_box_height);
    lv_obj_clear_flag(sw3, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(sw3, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(sw3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(sw3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(sw3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(sw3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(sw3,2, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 左目（白い点）
    lv_obj_t *left_eye = lv_obj_create(ui_characterDisplay);
    lv_obj_set_width(left_eye, eye_size); //8
    lv_obj_set_height(left_eye, eye_size);
    lv_obj_set_x(left_eye, eye_position_x);
    lv_obj_set_y(left_eye, eye_position_y);
    lv_obj_set_style_radius(left_eye, eye_size / 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(left_eye, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(left_eye, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(left_eye, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(left_eye, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 右目（白い点）
    lv_obj_t *right_eye = lv_obj_create(ui_characterDisplay);
    lv_obj_set_width(right_eye, eye_size); //8
    lv_obj_set_height(right_eye, eye_size);
    lv_obj_set_x(right_eye, eye_position_x + eye_diff);
    lv_obj_set_y(right_eye, eye_position_y);
    lv_obj_set_style_radius(right_eye, eye_size / 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(right_eye, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(right_eye, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(right_eye, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(right_eye, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 口（白い線）
    lv_obj_t *mouth = lv_obj_create(ui_characterDisplay);
    lv_obj_set_width(mouth, mouth_width);
    lv_obj_set_height(mouth, mouth_height);
    lv_obj_set_x(mouth, mouth_position_x);
    lv_obj_set_y(mouth, mouth_position_y);
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

    lv_obj_add_event_cb(left_eye, onXTouchCharacterEyePositionXUpdate, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_CHARACTER_LEFT_EYE_POSITION_X_UPDATE, left_eye, NULL);

    lv_obj_add_event_cb(right_eye, onXTouchCharacterEyePositionXUpdate, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_CHARACTER_RIGHT_EYE_POSITION_X_UPDATE, right_eye, NULL);



    ui_comp_characterComponent_create_hook(ui_characterComponent);
printf("return ui_characterComponent\n");
    return ui_characterComponent;
}
