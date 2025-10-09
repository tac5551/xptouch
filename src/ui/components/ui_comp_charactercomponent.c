
#include "ui_comp_charactercomponent.h"
#include "../ui.h"

// キャラクターコンポーネントの内部構造
typedef struct {
    lv_obj_t *face;
    lv_obj_t *left_ear;
    lv_obj_t *right_ear;
    lv_obj_t *left_ear_inner;
    lv_obj_t *right_ear_inner;
    lv_obj_t *left_eye;
    lv_obj_t *right_eye;
    lv_obj_t *nose;
    lv_obj_t *mouth;
    lv_obj_t *tail;
    lv_obj_t *status_label;
    lv_obj_t *mood_label;
    
    character_mood_t current_mood;
    character_color_t current_color;
    bool animation_running;
    int blink_state;
    int animation_frame;
} character_component_t;

static character_component_t *char_comp = NULL;
static lv_timer_t *blink_timer = NULL;
static lv_timer_t *expression_timer = NULL;
static lv_timer_t *tail_timer = NULL;

// プライベート関数
static void character_blink_timer_cb(lv_timer_t *timer);
static void character_expression_timer_cb(lv_timer_t *timer);
static void character_tail_timer_cb(lv_timer_t *timer);
static void update_character_expression(void);
static void update_character_color(void);
static void update_character_ears(void);
static void yawn_timer_cb(lv_timer_t *timer);
static void meow_timer_cb(lv_timer_t *timer);


void ui_characterComponent_set_mood(character_mood_t mood)
{
    if (!char_comp) return;
    
    char_comp->current_mood = mood;
    update_character_expression();
    
    // 気分ラベル更新
    const char* mood_texts[] = {
        "Normal", "Happy", "Sleepy", "Alert", "Sad", "Angry"
    };
    lv_label_set_text(char_comp->mood_label, mood_texts[mood]);
}

void ui_characterComponent_set_color(character_color_t color)
{
    if (!char_comp) return;
    
    char_comp->current_color = color;
    update_character_color();
}

void ui_characterComponent_set_status_text(const char* text)
{
    if (!char_comp) return;
    
    lv_label_set_text(char_comp->status_label, text);
}

void ui_characterComponent_start_animation(void)
{
    if (!char_comp) return;
    
    char_comp->animation_running = true;
    
    // まばたきタイマー
    if (!blink_timer) {
        blink_timer = lv_timer_create(character_blink_timer_cb, 200, NULL);
    }
    
    // 表情タイマー
    if (!expression_timer) {
        expression_timer = lv_timer_create(character_expression_timer_cb, 1000, NULL);
    }
    
    // しっぽタイマー
    if (!tail_timer) {
        tail_timer = lv_timer_create(character_tail_timer_cb, 100, NULL);
    }
}

void ui_characterComponent_stop_animation(void)
{
    if (!char_comp) return;
    
    char_comp->animation_running = false;
    
    if (blink_timer) {
        lv_timer_del(blink_timer);
        blink_timer = NULL;
    }
    
    if (expression_timer) {
        lv_timer_del(expression_timer);
        expression_timer = NULL;
    }
    
    if (tail_timer) {
        lv_timer_del(tail_timer);
        tail_timer = NULL;
    }
}

void ui_characterComponent_blink(void)
{
    if (!char_comp) return;
    
    char_comp->blink_state = 3; // 3フレームまばたき
}

void ui_characterComponent_yawn(void)
{
    if (!char_comp) return;
    
    // あくびアニメーション
    lv_obj_set_size(char_comp->mouth, 30, 30);
    lv_arc_set_angles(char_comp->mouth, 0, 360);
    
    // 2秒後に元に戻す
    lv_timer_t *yawn_timer = lv_timer_create(yawn_timer_cb, 2000, NULL);
    lv_timer_set_repeat_count(yawn_timer, 1);
}

void ui_characterComponent_meow(void)
{
    if (!char_comp) return;
    
    // 鳴き声アニメーション
    lv_obj_set_size(char_comp->mouth, 25, 25);
    lv_arc_set_angles(char_comp->mouth, 0, 360);
    
    // 1秒後に元に戻す
    lv_timer_t *meow_timer = lv_timer_create(meow_timer_cb, 1000, NULL);
    lv_timer_set_repeat_count(meow_timer, 1);
}

// プライベート関数の実装
static void character_blink_timer_cb(lv_timer_t *timer)
{
    if (!char_comp || !char_comp->animation_running) return;
    
    if (char_comp->blink_state > 0) {
        char_comp->blink_state--;
        // 目を閉じる
        lv_obj_set_size(char_comp->left_eye, 12, 2);
        lv_obj_set_size(char_comp->right_eye, 12, 2);
    } else {
        // 目を開く
        lv_obj_set_size(char_comp->left_eye, 12, 20);
        lv_obj_set_size(char_comp->right_eye, 12, 20);
        
        // ランダムでまばたき
        if (rand() % 100 < 5) {
            char_comp->blink_state = 2;
        }
    }
}

static void character_expression_timer_cb(lv_timer_t *timer)
{
    if (!char_comp || !char_comp->animation_running) return;
    
    char_comp->animation_frame++;
    
    // 10秒ごとに気分をランダムに変更
    if (char_comp->animation_frame >= 10) {
        char_comp->current_mood = (character_mood_t)(rand() % 6);
        char_comp->animation_frame = 0;
        update_character_expression();
    }
}

static void character_tail_timer_cb(lv_timer_t *timer)
{
    if (!char_comp || !char_comp->animation_running) return;
    
    static int tail_angle = 0;
    static int tail_direction = 1;
    
    tail_angle += tail_direction * 3;
    
    if (tail_angle > 20) tail_direction = -1;
    if (tail_angle < -20) tail_direction = 1;
    
    // しっぽの回転
    lv_obj_set_style_transform_angle(char_comp->tail, tail_angle * 10, LV_PART_MAIN);
}

static void update_character_expression(void)
{
    if (!char_comp) return;
    
    switch(char_comp->current_mood) {
        case CHARACTER_MOOD_NORMAL:
            lv_arc_set_angles(char_comp->mouth, 0, 180);
            lv_obj_set_size(char_comp->left_eye, 12, 20);
            lv_obj_set_size(char_comp->right_eye, 12, 20);
            break;
            
        case CHARACTER_MOOD_HAPPY:
            lv_arc_set_angles(char_comp->mouth, 0, 180);
            lv_obj_set_style_bg_color(char_comp->face, lv_color_hex(0xFFD700), LV_PART_MAIN);
            break;
            
        case CHARACTER_MOOD_SLEEPY:
            lv_obj_set_size(char_comp->left_eye, 12, 8);
            lv_obj_set_size(char_comp->right_eye, 12, 8);
            lv_arc_set_angles(char_comp->mouth, 0, 90);
            break;
            
        case CHARACTER_MOOD_ALERT:
            lv_obj_set_size(char_comp->left_ear, 20, 30);
            lv_obj_set_size(char_comp->right_ear, 20, 30);
            lv_arc_set_angles(char_comp->mouth, 0, 180);
            break;
            
        case CHARACTER_MOOD_SAD:
            lv_arc_set_angles(char_comp->mouth, 180, 360);
            lv_obj_set_style_bg_color(char_comp->face, lv_color_hex(0x87CEEB), LV_PART_MAIN);
            break;
            
        case CHARACTER_MOOD_ANGRY:
            lv_obj_set_size(char_comp->left_eye, 12, 15);
            lv_obj_set_size(char_comp->right_eye, 12, 15);
            lv_arc_set_angles(char_comp->mouth, 0, 180);
            lv_obj_set_style_bg_color(char_comp->face, lv_color_hex(0xFF4500), LV_PART_MAIN);
            break;
    }
}

static void update_character_color(void)
{
    if (!char_comp) return;
    
    switch(char_comp->current_color) {
        case CHARACTER_COLOR_ORANGE:
            lv_obj_set_style_bg_color(char_comp->face, lv_color_hex(0xFFA500), LV_PART_MAIN);
            lv_obj_set_style_bg_color(char_comp->left_ear, lv_color_hex(0xFF8C00), LV_PART_MAIN);
            lv_obj_set_style_bg_color(char_comp->right_ear, lv_color_hex(0xFF8C00), LV_PART_MAIN);
            lv_obj_set_style_bg_color(char_comp->tail, lv_color_hex(0xFF8C00), LV_PART_MAIN);
            break;
            
        case CHARACTER_COLOR_WHITE:
            lv_obj_set_style_bg_color(char_comp->face, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
            lv_obj_set_style_bg_color(char_comp->left_ear, lv_color_hex(0xF0F0F0), LV_PART_MAIN);
            lv_obj_set_style_bg_color(char_comp->right_ear, lv_color_hex(0xF0F0F0), LV_PART_MAIN);
            lv_obj_set_style_bg_color(char_comp->tail, lv_color_hex(0xF0F0F0), LV_PART_MAIN);
            break;
            
        case CHARACTER_COLOR_BLACK:
            lv_obj_set_style_bg_color(char_comp->face, lv_color_hex(0x2F2F2F), LV_PART_MAIN);
            lv_obj_set_style_bg_color(char_comp->left_ear, lv_color_hex(0x1C1C1C), LV_PART_MAIN);
            lv_obj_set_style_bg_color(char_comp->right_ear, lv_color_hex(0x1C1C1C), LV_PART_MAIN);
            lv_obj_set_style_bg_color(char_comp->tail, lv_color_hex(0x1C1C1C), LV_PART_MAIN);
            break;
            
        case CHARACTER_COLOR_CALICO:
            lv_obj_set_style_bg_color(char_comp->face, lv_color_hex(0xFFD700), LV_PART_MAIN);
            lv_obj_set_style_bg_color(char_comp->left_ear, lv_color_hex(0xFF8C00), LV_PART_MAIN);
            lv_obj_set_style_bg_color(char_comp->right_ear, lv_color_hex(0xFF8C00), LV_PART_MAIN);
            lv_obj_set_style_bg_color(char_comp->tail, lv_color_hex(0xFF8C00), LV_PART_MAIN);
            break;
    }
}

// コールバック関数の実装
static void yawn_timer_cb(lv_timer_t *timer)
{
    if (char_comp) {
        lv_obj_set_size(char_comp->mouth, 20, 20);
        lv_arc_set_angles(char_comp->mouth, 0, 180);
    }
    lv_timer_del(timer);
}

static void meow_timer_cb(lv_timer_t *timer)
{
    if (char_comp) {
        lv_obj_set_size(char_comp->mouth, 20, 20);
        lv_arc_set_angles(char_comp->mouth, 0, 180);
    }
    lv_timer_del(timer);
}

// イベントハンドラー
void ui_event_comp_characterComponent_characterFace(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if (event_code == LV_EVENT_CLICKED) {
        onTouchStackChan(e);
    }
}


lv_obj_t *ui_characterComponent_create(lv_obj_t *comp_parent)
{
    // メモリ確保
    char_comp = (character_component_t*)malloc(sizeof(character_component_t));
    if (!char_comp) return NULL;
    memset(char_comp, 0, sizeof(character_component_t));
    
    // メインコンテナ
    lv_obj_t *ui_characterComponent = lv_obj_create(comp_parent);
    lv_obj_set_width(ui_characterComponent, 200);
    lv_obj_set_height(ui_characterComponent, 180);
    lv_obj_set_style_bg_opa(ui_characterComponent, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_opa(ui_characterComponent, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(ui_characterComponent, 0, LV_PART_MAIN);
    
    // 猫の顔（楕円形）
    char_comp->face = lv_obj_create(ui_characterComponent);
    lv_obj_set_size(char_comp->face, 120, 100);
    lv_obj_set_pos(char_comp->face, 40, 20);
    lv_obj_set_style_radius(char_comp->face, 50, LV_PART_MAIN);
    lv_obj_set_style_bg_color(char_comp->face, lv_color_hex(0xFFA500), LV_PART_MAIN);
    lv_obj_set_style_border_opa(char_comp->face, LV_OPA_TRANSP, LV_PART_MAIN);
    
    // 左耳
    char_comp->left_ear = lv_obj_create(char_comp->face);
    lv_obj_set_size(char_comp->left_ear, 20, 25);
    lv_obj_set_pos(char_comp->left_ear, 10, -5);
    lv_obj_set_style_radius(char_comp->left_ear, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(char_comp->left_ear, lv_color_hex(0xFF8C00), LV_PART_MAIN);
    lv_obj_set_style_border_opa(char_comp->left_ear, LV_OPA_TRANSP, LV_PART_MAIN);
    
    // 右耳
    char_comp->right_ear = lv_obj_create(char_comp->face);
    lv_obj_set_size(char_comp->right_ear, 20, 25);
    lv_obj_set_pos(char_comp->right_ear, 90, -5);
    lv_obj_set_style_radius(char_comp->right_ear, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(char_comp->right_ear, lv_color_hex(0xFF8C00), LV_PART_MAIN);
    lv_obj_set_style_border_opa(char_comp->right_ear, LV_OPA_TRANSP, LV_PART_MAIN);
    
    // 左耳の内側
    char_comp->left_ear_inner = lv_obj_create(char_comp->left_ear);
    lv_obj_set_size(char_comp->left_ear_inner, 10, 15);
    lv_obj_set_pos(char_comp->left_ear_inner, 5, 5);
    lv_obj_set_style_radius(char_comp->left_ear_inner, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(char_comp->left_ear_inner, lv_color_hex(0xFFB6C1), LV_PART_MAIN);
    lv_obj_set_style_border_opa(char_comp->left_ear_inner, LV_OPA_TRANSP, LV_PART_MAIN);
    
    // 右耳の内側
    char_comp->right_ear_inner = lv_obj_create(char_comp->right_ear);
    lv_obj_set_size(char_comp->right_ear_inner, 10, 15);
    lv_obj_set_pos(char_comp->right_ear_inner, 5, 5);
    lv_obj_set_style_radius(char_comp->right_ear_inner, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(char_comp->right_ear_inner, lv_color_hex(0xFFB6C1), LV_PART_MAIN);
    lv_obj_set_style_border_opa(char_comp->right_ear_inner, LV_OPA_TRANSP, LV_PART_MAIN);
    
    // 左目（縦長の楕円）
    char_comp->left_eye = lv_obj_create(char_comp->face);
    lv_obj_set_size(char_comp->left_eye, 12, 20);
    lv_obj_set_pos(char_comp->left_eye, 30, 25);
    lv_obj_set_style_radius(char_comp->left_eye, 6, LV_PART_MAIN);
    lv_obj_set_style_bg_color(char_comp->left_eye, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_opa(char_comp->left_eye, LV_OPA_TRANSP, LV_PART_MAIN);
    
    // 右目（縦長の楕円）
    char_comp->right_eye = lv_obj_create(char_comp->face);
    lv_obj_set_size(char_comp->right_eye, 12, 20);
    lv_obj_set_pos(char_comp->right_eye, 78, 25);
    lv_obj_set_style_radius(char_comp->right_eye, 6, LV_PART_MAIN);
    lv_obj_set_style_bg_color(char_comp->right_eye, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_opa(char_comp->right_eye, LV_OPA_TRANSP, LV_PART_MAIN);
    
    // 鼻（三角形）
    char_comp->nose = lv_obj_create(char_comp->face);
    lv_obj_set_size(char_comp->nose, 8, 6);
    lv_obj_set_pos(char_comp->nose, 56, 40);
    lv_obj_set_style_radius(char_comp->nose, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(char_comp->nose, lv_color_hex(0xFF69B4), LV_PART_MAIN);
    lv_obj_set_style_border_opa(char_comp->nose, LV_OPA_TRANSP, LV_PART_MAIN);
    
    // 口（弧）
    char_comp->mouth = lv_arc_create(char_comp->face);
    lv_obj_set_size(char_comp->mouth, 20, 20);
    lv_obj_set_pos(char_comp->mouth, 50, 50);
    lv_arc_set_bg_angles(char_comp->mouth, 0, 180);
    lv_arc_set_angles(char_comp->mouth, 0, 180);
    lv_obj_set_style_arc_width(char_comp->mouth, 3, LV_PART_MAIN);
    lv_obj_set_style_arc_color(char_comp->mouth, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_arc_width(char_comp->mouth, 0, LV_PART_INDICATOR);
    
    // しっぽ
    char_comp->tail = lv_obj_create(ui_characterComponent);
    lv_obj_set_size(char_comp->tail, 8, 60);
    lv_obj_set_pos(char_comp->tail, 160, 30);
    lv_obj_set_style_radius(char_comp->tail, 4, LV_PART_MAIN);
    lv_obj_set_style_bg_color(char_comp->tail, lv_color_hex(0xFF8C00), LV_PART_MAIN);
    lv_obj_set_style_border_opa(char_comp->tail, LV_OPA_TRANSP, LV_PART_MAIN);
    
    // ステータスラベル
    char_comp->status_label = lv_label_create(ui_characterComponent);
    lv_label_set_text(char_comp->status_label, "Ready");
    lv_obj_set_pos(char_comp->status_label, 10, 140);
    lv_obj_set_style_text_font(char_comp->status_label, &lv_font_montserrat_14, LV_PART_MAIN);
    
    // 気分ラベル
    char_comp->mood_label = lv_label_create(ui_characterComponent);
    lv_label_set_text(char_comp->mood_label, "Normal");
    lv_obj_set_pos(char_comp->mood_label, 10, 160);
    lv_obj_set_style_text_font(char_comp->mood_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(char_comp->mood_label, lv_color_hex(0x666666), LV_PART_MAIN);
    
    // 初期化
    char_comp->current_mood = CHARACTER_MOOD_NORMAL;
    char_comp->current_color = CHARACTER_COLOR_ORANGE;
    char_comp->animation_running = false;
    char_comp->blink_state = 0;
    char_comp->animation_frame = 0;
    
    // イベントハンドラー設定
    lv_obj_add_event_cb(char_comp->face, ui_event_comp_characterComponent_characterFace, LV_EVENT_ALL, NULL);
    
    // 初期化処理
    ui_characterComponent_start_animation();
    ui_characterComponent_set_mood(CHARACTER_MOOD_NORMAL);
    ui_characterComponent_set_color(CHARACTER_COLOR_ORANGE);
    ui_characterComponent_set_status_text("Ready");
    
    ui_comp_characterComponent_create_hook(ui_characterComponent);
    return ui_characterComponent;
}

