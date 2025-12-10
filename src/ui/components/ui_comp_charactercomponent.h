#ifndef _UI_COMP_CHARACTERCOMPONENT_H
#define _UI_COMP_CHARACTERCOMPONENT_H

#include "../ui.h"

#ifdef __cplusplus
extern "C"
{
#endif

// COMPONENT characterComponent
#define UI_COMP_CHARACTERCOMPONENT_CHARACTERCOMPONENT 0
#define UI_COMP_CHARACTERCOMPONENT_CHARACTERFACE 1
#define UI_COMP_CHARACTERCOMPONENT_CHARACTERFACE_FACE 2
#define UI_COMP_CHARACTERCOMPONENT_CHARACTERFACE_LEFTEAR 3
#define UI_COMP_CHARACTERCOMPONENT_CHARACTERFACE_RIGHTEAR 4
#define UI_COMP_CHARACTERCOMPONENT_CHARACTERFACE_LEFTEARINNER 5
#define UI_COMP_CHARACTERCOMPONENT_CHARACTERFACE_RIGHTEARINNER 6
#define UI_COMP_CHARACTERCOMPONENT_CHARACTERFACE_LEFTEYE 7
#define UI_COMP_CHARACTERCOMPONENT_CHARACTERFACE_RIGHTEYE 8
#define UI_COMP_CHARACTERCOMPONENT_CHARACTERFACE_NOSE 9
#define UI_COMP_CHARACTERCOMPONENT_CHARACTERFACE_MOUTH 10
#define UI_COMP_CHARACTERCOMPONENT_CHARACTERFACE_TAIL 11
#define UI_COMP_CHARACTERCOMPONENT_CHARACTERFACE_STATUSLABEL 12
#define UI_COMP_CHARACTERCOMPONENT_CHARACTERFACE_MOODLABEL 13
#define _UI_COMP_CHARACTERCOMPONENT_NUM 14

// キャラクターの状態
typedef enum {
    CHARACTER_MOOD_NORMAL = 0,
    CHARACTER_MOOD_HAPPY,
    CHARACTER_MOOD_SLEEPY,
    CHARACTER_MOOD_ALERT,
    CHARACTER_MOOD_SAD,
    CHARACTER_MOOD_ANGRY
} character_mood_t;

// キャラクターの色
typedef enum {
    CHARACTER_COLOR_ORANGE = 0,
    CHARACTER_COLOR_WHITE,
    CHARACTER_COLOR_BLACK,
    CHARACTER_COLOR_CALICO
} character_color_t;

enum CharactorPanelComponents
{
UI_COMP_CHARACTORPANEL_CHARACTORPANEL,
UI_COMP_CHARACTORPANEL_RIGHT_EYE,
UI_COMP_CHARACTORPANEL_LEFT_EYE,
UI_COMP_CHARACTORPANEL_MOUSE,
_UI_COMP_CHARACTORPANEL_NUM
};


lv_obj_t *ui_characterComponent_create(lv_obj_t *comp_parent);
void ui_characterComponent_set_mood(character_mood_t mood);
void ui_characterComponent_set_color(character_color_t color);
void ui_characterComponent_set_status_text(const char* text);
void ui_characterComponent_start_animation(void);
void ui_characterComponent_stop_animation(void);
void ui_characterComponent_blink(void);
void ui_characterComponent_yawn(void);
void ui_characterComponent_meow(void);
void xtouch_character_init(void);  // キャラクター初期化（タイマー不要）
void xtouch_events_onCharacterAnimation(void);  // アニメーション処理（loop()から呼び出す）
void inir_face_position(lv_obj_t *comp_parent);

// イベントハンドラー
void ui_event_comp_characterComponent_characterFace(lv_event_t *e);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
