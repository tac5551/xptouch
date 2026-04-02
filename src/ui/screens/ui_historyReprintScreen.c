#include "../ui.h"
#include "../ui_msgs.h"
#include "../ui_helpers.h"

#ifdef __XTOUCH_PLATFORM_S3__

static lv_timer_t *s_reprint_detail_fetch_timer = NULL;
static lv_timer_t *s_reprint_detail_reload_timer = NULL;
static lv_obj_t *s_historyReprintComponentRoot = NULL;

static void ui_history_reprint_detail_reload_timer_cb(lv_timer_t *t)
{
    (void)t;
    s_reprint_detail_reload_timer = NULL;
    if (xTouchConfig.currentScreenIndex == 16)
    {
        /* 全画面 loadScreen しない。
         * マッピング数が変わるため component だけ作り直す（screen_init が再実行されて再取得ループしない）。 */
        if (s_historyReprintComponentRoot)
        {
            lv_obj_del(s_historyReprintComponentRoot);
            s_historyReprintComponentRoot = NULL;
        }
        s_historyReprintComponentRoot = ui_historyReprintComponent_create(ui_historyReprintScreen);
    }
    lv_timer_del(t);
}

static void ui_event_history_reprint_on_detail_ready(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_MSG_RECEIVED)
        return;
    /* mapping 取得完了で再描画（再印刷オプション行数が変わるため） */
    /* lv_msg_send の notify 中に loadScreen(=obj delete) すると購読リストが書き換わり crash するため、タイマーで遅延実行 */
    if (xTouchConfig.currentScreenIndex == 16)
    {
        if (s_reprint_detail_reload_timer)
            lv_timer_del(s_reprint_detail_reload_timer);
        s_reprint_detail_reload_timer = lv_timer_create(ui_history_reprint_detail_reload_timer_cb, 1, NULL);
        lv_timer_set_repeat_count(s_reprint_detail_reload_timer, 1);
    }
}

static void ui_history_reprint_detail_fetch_timer_cb(lv_timer_t *t)
{
    (void)t;
    s_reprint_detail_fetch_timer = NULL;
    /* 画面遷移直後のメモリ圧迫を避けるため少し遅延して取得開始 */
    ui_msg_send(XTOUCH_HISTORY_REPRINT_DETAIL_FETCH,
                 0, 0);
    lv_timer_del(t);
}

void ui_historyReprintScreen_screen_init(void)
{
    ui_historyReprintScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_historyReprintScreen, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_set_scrollbar_mode(ui_historyReprintScreen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(ui_historyReprintScreen, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_historyReprintScreen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(ui_historyReprintScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_historyReprintScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_historyReprintScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_historyReprintScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_historyReprintScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_historyReprintScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_sidebarComponent = ui_sidebarComponent_create(ui_historyReprintScreen);
    lv_obj_set_x(ui_sidebarComponent, 387);
    lv_obj_set_y(ui_sidebarComponent, 178);

    /* loadScreen() が前画面（この screen 自身も含む）を lv_obj_clean/lv_obj_del で破棄しているため、
     * static で保持している s_historyReprintComponentRoot は dangling pointer になり得る。
     * ここで lv_obj_del すると二重削除になって LVGL 側で panic するので、参照だけ NULL 化する。 */
    s_historyReprintComponentRoot = NULL;
    s_historyReprintComponentRoot = ui_historyReprintComponent_create(ui_historyReprintScreen);

    lv_msg_subsribe_obj(XTOUCH_HISTORY_REPRINT_DETAIL_READY, ui_historyReprintScreen, NULL);
    lv_obj_add_event_cb(ui_historyReprintScreen, ui_event_history_reprint_on_detail_ready, LV_EVENT_MSG_RECEIVED, NULL);

    /* 画面表示タイミングで即取得せず、描画を挟んでから詳細取得を開始する */
    if (!xtouch_history_reprint_detail_fetch_done)
    {
        if (s_reprint_detail_fetch_timer)
            lv_timer_del(s_reprint_detail_fetch_timer);
        s_reprint_detail_fetch_timer = lv_timer_create(ui_history_reprint_detail_fetch_timer_cb, 350, NULL);
        lv_timer_set_repeat_count(s_reprint_detail_fetch_timer, 1);
    }
}

#else

void ui_historyReprintScreen_screen_init(void)
{
    (void)0;
}

#endif

