
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

// 座標計算関数
void calc_position(int x_offset, int y_offset)
{


    // 座標計算 (左目基準)
    left_eye_x = center_x - eye_offset_x + x_offset;
    right_eye_x = left_eye_x + eye_diff;
    eye_y = center_y - eye_offset_y + y_offset;

    mouth_x = left_eye_x + mouth_offset_x;
    mouth_y = eye_y + mouth_offset_y;

    // 既存オブジェクトがあれば即時反映
    if (left_eye) {
        lv_obj_set_x(left_eye, left_eye_x);
        lv_obj_set_y(left_eye, eye_y);
    }
    if (right_eye) {
        lv_obj_set_x(right_eye, right_eye_x);
        lv_obj_set_y(right_eye, eye_y);
    }
    if (mouth) {
        lv_obj_set_x(mouth, mouth_x);
        lv_obj_set_y(mouth, mouth_y);
    }
}

// キャラクター関連イベント関数
void xtouch_events_onCharacterAnimation()
{
    // screenが9の場合のみ処理を実行
    if (xTouchConfig.currentScreenIndex != 9)
    {
        xtouch_character_timer_stop();
        return;
    }

    if (!left_eye || !right_eye || !mouth)
        return;

    // 瞬きの処理
    if (is_blinking)
    {
        // 2回のタイマー呼び出し（100ms）で目を開く
        if (eye_blink_counter >= 2)
        {
            lv_obj_set_height(left_eye, 8);
            lv_obj_set_height(right_eye, 8);
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
            lv_obj_set_height(left_eye, 2);
            lv_obj_set_height(right_eye, 2);
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
                lv_obj_set_height(mouth, mouth_height);
                is_mouth_animating = false;
                mouth_animation_counter = 0;
                mouth_pakupaku_count = 0;
            }
            else
            {
                // 口を閉じる（元のサイズ）
                lv_obj_set_height(mouth, mouth_height);
            }
        }
        else
        {
            // 口を開く（4倍の高さに拡大）
            lv_obj_set_height(mouth, mouth_height * 4);
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
        int random_x = (rand() % 101) - 50; // -50 から +50
        int random_y = (rand() % 101) - 50; // -50 から +50
        calc_position(random_x, random_y);
        position_change_counter = 0;
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
    if (!xtouch_character_timer_started)
    {
        xtouch_character_timer = lv_timer_create(xtouch_character_timer_handler, 50, NULL);
        lv_timer_set_repeat_count(xtouch_character_timer, -1);
        xtouch_character_timer_started = true;
    }
}

void xtouch_character_timer_stop()
{
    if (xtouch_character_timer_started && xtouch_character_timer)
    {
        lv_timer_del(xtouch_character_timer);
        xtouch_character_timer = NULL;
        xtouch_character_timer_started = false;
    }
}

void xtouch_character_timer_handler(lv_timer_t *timer)
{
    // screenが9の場合のみ処理を実行
    if (xTouchConfig.currentScreenIndex != 9) {
        return;
    }

    if (!left_eye || !right_eye || !mouth)
        return;

    eye_blink_counter++;
    mouth_animation_counter++;

    xtouch_events_onCharacterAnimation();
}


lv_obj_t *ui_characterComponent_create(lv_obj_t *comp_parent)
{
    // メインコンテナ
    lv_obj_t *ui_characterComponent = lv_obj_create(comp_parent);
    lv_obj_set_width(ui_characterComponent, 200);
    lv_obj_set_height(ui_characterComponent, 200);
    lv_obj_align(ui_characterComponent, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(ui_characterComponent, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_add_flag(ui_characterComponent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(ui_characterComponent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(ui_characterComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_characterComponent, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_border_width(ui_characterComponent, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_pad_left(ui_characterScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_pad_right(ui_characterScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_pad_top(ui_characterScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_pad_bottom(ui_characterScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_pad_row(ui_characterScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_pad_column(ui_characterScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //    lv_obj_set_style_bg_opa(ui_characterComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    calc_position(0, 0);

    // 左目（白い点）
    left_eye = lv_obj_create(ui_characterScreen);
    lv_obj_set_width(left_eye, eye_size);
    lv_obj_set_height(left_eye, eye_size);
    lv_obj_set_x(left_eye, left_eye_x);
    lv_obj_set_y(left_eye, eye_y);
    lv_obj_set_style_radius(left_eye, eye_size / 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(left_eye, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(left_eye, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(left_eye, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(left_eye, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 右目（白い点）
    right_eye = lv_obj_create(ui_characterScreen);
    lv_obj_set_width(right_eye, eye_size);
    lv_obj_set_height(right_eye, eye_size);
    lv_obj_set_x(right_eye, right_eye_x);
    lv_obj_set_y(right_eye, eye_y);
    lv_obj_set_style_radius(right_eye, eye_size / 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(right_eye, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(right_eye, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(right_eye, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(right_eye, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 口（白い線）
    mouth = lv_obj_create(ui_characterScreen);
    lv_obj_set_width(mouth, mouth_width);
    lv_obj_set_height(mouth, mouth_height);
    lv_obj_set_x(mouth, mouth_x);
    lv_obj_set_y(mouth, mouth_y);
    lv_obj_set_style_radius(mouth, mouth_height / 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(mouth, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(mouth, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(mouth, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(mouth, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // イベントハンドラー設定
    lv_obj_add_event_cb(ui_characterComponent, ui_event_comp_characterComponent_characterFace, LV_EVENT_ALL, NULL);

    xtouch_character_timer_init();

    ui_comp_characterComponent_create_hook(ui_characterComponent);
    return ui_characterComponent;
}
