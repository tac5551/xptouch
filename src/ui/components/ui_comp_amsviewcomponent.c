#include "../ui.h"
#include "../../xtouch/trays.h"
#include "../../ui/ui_msgs.h"

#define AMS_COUNT 4
#define SLOT_COUNT 5
#define AMS_BORDER 3

/* コンボで選択中の行（0=EXT, 1=AMS1〜4）。リフレッシュでビットコールバックが走ってもこれを優先する */
static uint8_t s_ams_view_selector = UI_AMS_SELECTOR_AMS1;
/* 画面を開いたときの初期選択をまだ適用していないフラグ（コンポーネント作成時に false にリセット） */
static bool s_ams_view_initialized = false;
/* コンボの選択肢インデックス → 論理セレクタ(0=EXT,1=AMS1..4)。接続されているAMSだけ並べるため */
static uint8_t s_dropdown_index_to_selector[5];
static uint8_t s_dropdown_option_count = 0;

void ui_event_comp_amsViewComponent_onAmsHumidity(lv_event_t *e)
{

    lv_obj_t *target = lv_event_get_target(e);
    uintptr_t temp_user_data = (uintptr_t)lv_event_get_user_data(e);
    uint8_t user_data = (uint8_t)temp_user_data;

    uint8_t ams_id = user_data - 1;
    char buffer[100];
    memset(buffer, 0, 100);
    sprintf(buffer, "H\n%d", bambuStatus.ams_humidity[ams_id]);
    // printf("onAmsHumidity %d %d\n", ams_id, bambuStatus.ams_humidity[ams_id]);

    lv_label_set_text(target, buffer);
}

/* 1組の湿度ラベル用: 表示中選択子 s_ams_view_selector に応じて表示を更新 */
static void ui_event_comp_amsViewComponent_onAmsHumidityUnified(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_MSG_RECEIVED)
        return;
    lv_obj_t *target = lv_event_get_target(e);
    char buffer[64];
    memset(buffer, 0, sizeof(buffer));
    if (s_ams_view_selector == UI_AMS_SELECTOR_EXT)
        snprintf(buffer, sizeof(buffer), "H\n-");
    else if (s_ams_view_selector >= UI_AMS_SELECTOR_AMS1 && s_ams_view_selector <= UI_AMS_SELECTOR_AMS4)
        snprintf(buffer, sizeof(buffer), "H\n%d", bambuStatus.ams_humidity[s_ams_view_selector - 1]);
    else
        snprintf(buffer, sizeof(buffer), "H\nX");
    lv_label_set_text(target, buffer);
}

/* 1組のスロットラベル用: user_data=slot_index(0-3)、表示中選択子に応じてスロットIDを決めて更新 */
static void ui_event_comp_amsViewComponent_onAmsUpdateBySlotIndex(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_MSG_RECEIVED)
        return;
    lv_obj_t *target = lv_event_get_target(e);
    uintptr_t ud = (uintptr_t)lv_event_get_user_data(e);
    uint8_t slot_index = (uint8_t)(ud & 3);

    if (!(bambuStatus.ams_status_main == AMS_STATUS_MAIN_IDLE || bambuStatus.ams_status_main == AMS_STATUS_MAIN_ASSIST) || bambuStatus.print_status == XTOUCH_PRINT_STATUS_RUNNING)
        lv_obj_add_state(target, LV_STATE_DISABLED);
    else
        lv_obj_clear_state(target, LV_STATE_DISABLED);

    if (s_ams_view_selector == UI_AMS_SELECTOR_EXT)
    {
        if (slot_index > 0)
            return;
        /* Home画面と同様: EXTは ams_id=0, tray_id=0 (vt_tray) の色・タイプを表示 */
        uint32_t ext_status = get_tray_status(0, 0);
        uint32_t color_code = (uint32_t)((ext_status >> 8) & 0xFFFFFF);
        char *ext_tray_type = get_tray_type(0, 0);
        if (color_code != 0)
        {
            lv_color_t color = lv_color_hex(color_code);
            lv_color_t color_inv = lv_color_hex((0xFFFFFF - color_code) & 0xFFFFFF);
            if (ext_tray_type && ext_tray_type[0] != '\0' && strcmp(ext_tray_type, "null") != 0)
                lv_label_set_text(target, ext_tray_type);
            else
                lv_label_set_text(target, "EXT");
            lv_obj_set_style_bg_color(target, color, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(target, color_inv, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(target, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            if (bambuStatus.m_tray_now == 254)
            {
                lv_obj_set_style_border_color(target, color_inv, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_width(target, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);
            }
        }
        else
        {
            lv_label_set_text(target, "EXT\nSlot");
            lv_obj_set_style_bg_color(target, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(target, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(target, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        return;
    }

    uint8_t tmp_ams_id = s_ams_view_selector - 1;
    uint8_t tmp_tray_id = slot_index + 1;

    uint32_t tray_status = get_tray_status(tmp_ams_id, tmp_tray_id);
    uint16_t tray_id = ((tray_status >> 4) & 0x0F);
    char *tray_type = get_tray_type(tmp_ams_id, tmp_tray_id);

    if (tmp_tray_id == tray_id)
    {
        lv_color_t color = lv_color_hex(tray_status >> 8);
        lv_color_t color_inv = lv_color_hex((0xFFFFFF - (tray_status >> 8)) & 0xFFFFFF);

        if (tray_type && tray_type[0] != '\0' && strcmp(tray_type, "null") != 0)
            lv_label_set_text(target, tray_type);
        else
            lv_label_set_text(target, "x");

        if (tray_id == 0)
            tray_id = 254 + 1;

        lv_obj_set_style_bg_color(target, color, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(target, color_inv, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(target, color, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(target, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        /* 表示中のAMSのスロットのみハイライト。m_tray_now は 0-15 (ams*4+tray) なので、現在AMSかつスロット一致時のみボーダー表示 */
        if (bambuStatus.m_tray_now >= 0 && bambuStatus.m_tray_now <= 15 &&
            (uint8_t)(bambuStatus.m_tray_now >> 2) == tmp_ams_id &&
            (uint8_t)(bambuStatus.m_tray_now & 3) == slot_index)
        {
            lv_obj_set_style_border_color(target, color_inv, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(target, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
}

/* 共通アンロードボタン用: ロック時は無効化 */
static void ui_amsViewComponent_onAmsLockSyncUnloadBtn(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_MSG_RECEIVED)
        return;
    lv_obj_t *btn = lv_event_get_target(e);
    int disabled = (!(bambuStatus.ams_status_main == AMS_STATUS_MAIN_IDLE || bambuStatus.ams_status_main == AMS_STATUS_MAIN_ASSIST) || bambuStatus.print_status == XTOUCH_PRINT_STATUS_RUNNING) ? 1 : 0;
    if (disabled)
        lv_obj_add_state(btn, LV_STATE_DISABLED);
    else
        lv_obj_clear_state(btn, LV_STATE_DISABLED);
}

/* ロック時は EDIT/LOAD も無効化。row2_1 に付け、MSG で全ボタンの DISABLED を更新 */
static void ui_amsViewComponent_onAmsLockSyncButtons(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_MSG_RECEIVED)
        return;
    lv_obj_t *row2 = lv_event_get_target(e);
    int disabled = (!(bambuStatus.ams_status_main == AMS_STATUS_MAIN_IDLE || bambuStatus.ams_status_main == AMS_STATUS_MAIN_ASSIST) || bambuStatus.print_status == XTOUCH_PRINT_STATUS_RUNNING) ? 1 : 0;
    uint32_t n = lv_obj_get_child_cnt(row2);
    for (uint32_t i = 0; i < n; i++)
    {
        lv_obj_t *btn_row = lv_obj_get_child(row2, i);
        uint32_t m = lv_obj_get_child_cnt(btn_row);
        for (uint32_t j = 0; j < m; j++)
        {
            lv_obj_t *btn = lv_obj_get_child(btn_row, j);
            if (disabled)
                lv_obj_add_state(btn, LV_STATE_DISABLED);
            else
                lv_obj_clear_state(btn, LV_STATE_DISABLED);
        }
    }
}

/* 1組のスロットクリック用: user_data=slot_index(0-3)、表示中選択子に応じてスロットIDを渡してLOAD */
static void onAmsSlotClickUnified(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;
    uintptr_t ud = (uintptr_t)lv_event_get_user_data(e);
    uint8_t slot_index = (uint8_t)(ud & 3);
    int slot_id = (s_ams_view_selector == UI_AMS_SELECTOR_EXT) ? 254 : ((int)(s_ams_view_selector - 1) * 100 + slot_index + 1);
    onAmsSlotLoad(e, slot_id);
}

void ui_amsViewComponent_onAMSBitsSlot(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    uintptr_t temp_user_data = (uintptr_t)lv_event_get_user_data(e);
    uint8_t user_data = (uint8_t)temp_user_data;

    /* コンボで選ばれている行以外は常に非表示（リフレッシュで上書きされないようにする） */
    if (s_ams_view_selector == UI_AMS_SELECTOR_EXT)
    {
        lv_obj_add_flag(target, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    if (user_data != s_ams_view_selector)
    {
        lv_obj_add_flag(target, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    uint8_t ams_id = user_data - 1;
    uint8_t check_bit = 0;
    if (ams_id == 0)
        check_bit = 0b00000001;
    if (ams_id == 1)
        check_bit = 0b00000010;
    if (ams_id == 2)
        check_bit = 0b00000100;
    if (ams_id == 3)
        check_bit = 0b00001000;

    if ((bambuStatus.ams_exist_bits & check_bit) == 0)
        lv_obj_add_flag(target, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_clear_flag(target, LV_OBJ_FLAG_HIDDEN);
}

void ui_amsViewComponent_onAMSBitsSlotDummy(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    uintptr_t temp_user_data = (uintptr_t)lv_event_get_user_data(e);
    uint8_t user_data = (uint8_t)temp_user_data;

    /* コンボで選ばれている行以外は常に非表示 */
    if (s_ams_view_selector == UI_AMS_SELECTOR_EXT)
    {
        lv_obj_add_flag(target, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    if (user_data != s_ams_view_selector)
    {
        lv_obj_add_flag(target, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    uint8_t ams_id = user_data - 1;
    uint8_t check_bit = 0;
    if (ams_id == 0)
        check_bit = 0b00000001;
    if (ams_id == 1)
        check_bit = 0b00000010;
    if (ams_id == 2)
        check_bit = 0b00000100;
    if (ams_id == 3)
        check_bit = 0b00001000;

    if ((bambuStatus.ams_exist_bits & check_bit) == 0)
        lv_obj_clear_flag(target, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_add_flag(target, LV_OBJ_FLAG_HIDDEN);
}

void ui_event_comp_amsViewComponent_onAmsUpdate(lv_event_t *e)
{

    lv_obj_t *target = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);
    uintptr_t temp_user_data = (uintptr_t)lv_event_get_user_data(e);
    uint8_t user_data = (uint8_t)temp_user_data;

    if (!(bambuStatus.ams_status_main == AMS_STATUS_MAIN_IDLE || bambuStatus.ams_status_main == AMS_STATUS_MAIN_ASSIST) || bambuStatus.print_status == XTOUCH_PRINT_STATUS_RUNNING)
    {
        lv_obj_add_state(target, LV_STATE_DISABLED);
    }
    else
    {
        lv_obj_clear_state(target, LV_STATE_DISABLED);
    }

    struct XTOUCH_MESSAGE_DATA *message = (struct XTOUCH_MESSAGE_DATA *)m->payload;

    uint8_t tmp_ams_id = user_data / 100;
    uint8_t tmp_tray_id = user_data % 100;

    uint32_t tray_status = get_tray_status(tmp_ams_id, tmp_tray_id);
    uint16_t tray_id = ((tray_status >> 4) & 0x0F);
    uint16_t loaded = ((tray_status) & 0x01);
    char *tray_type = get_tray_type(tmp_ams_id, tmp_tray_id);
    // printf("onAmsUpdate %d %d %d %d\n", tmp_ams_id, tmp_tray_id, tray_id, loaded);

    // for (int i = 0; i < 4; i++)
    // {
    //     printf("--------------------------------\n");
    //     for (int j = 0; j < 5; j++)
    //     {
    //         uint32_t tray_status = get_tray_status(i, j);
    //         uint16_t tray_id = ((tray_status >> 4) & 0x0F);
    //         uint16_t loaded = ((tray_status) & 0x01);

    //         printf("tray dump %d %d %d %d %d %f\n", i, j, tray_id, loaded, bambuStatus.ams_humidity[i], bambuStatus.ams_temperature[i]);
    //     }
    // }
    // printf("--------------------------------\n");

    // lv_obj_t *unload = ui_comp_get_child(target, UI_COMP_amsViewComponent_FILAMENTSCREENFILAMENT_FILAMENTSCREENUNLOAD);

    if (tmp_tray_id == tray_id)
    {
        lv_color_t color = lv_color_hex(tray_status >> 8);
        lv_color_t color_inv = lv_color_hex((0xFFFFFF - (tray_status >> 8)) & 0xFFFFFF);

        // tray_typeがnullポインタでなく、空文字列でもなく、文字列"null"でもない場合、その文字列を設定（優先）
        if (tray_type[0] != '\0' && strcmp(tray_type, "null") != 0) {
            lv_label_set_text(target, tray_type);
        } else {
            lv_label_set_text(target, "x");
        }

        // printf(" tray_now: %d, tray_tar: %d, slot: %d, color: %06llX \n", bambuStatus.m_tray_now, bambuStatus.m_tray_tar, tray_id, message->data >> 8);

        lv_obj_set_style_bg_color(target, color, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(target, color_inv, LV_PART_MAIN | LV_STATE_DEFAULT);

        if (tray_id == 0)
            tray_id = 254 + 1;

        lv_obj_set_style_border_color(target, color, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(target, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        if (bambuStatus.m_tray_now + 1 == tray_id)
        {
            // lv_label_set_text(target, "L");
            lv_obj_set_style_border_color(target, color_inv, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(target, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
}



/* 各スロット下の EDIT / LOAD / UNLOAD ボタン用 */
static void on_ams_btn_load(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;
    int slot = (int)(uintptr_t)lv_event_get_user_data(e);
    onAmsSlotLoad(e, slot);
}
static void on_ams_btn_unload(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;
    lv_msg_send(XTOUCH_COMMAND_AMS_UNLOAD_SLOT, 0);
}
static void on_ams_btn_edit(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;
    loadScreen(3);
}

void ui_event_comp_AMSViewComponent_onAMSSlot1_1Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onAMSSlot1_1Click\n");
        onAmsSlotLoad(e, 1);
    }
}

void ui_event_comp_AMSViewComponent_onAMSSlot1_2Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onAMSSlot1_2Click\n");
        onAmsSlotLoad(e, 2);
    }
}

void ui_event_comp_AMSViewComponent_onAMSSlot1_3Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onAMSSlot1_3Click\n");
        onAmsSlotLoad(e, 3);
    }
}

void ui_event_comp_AMSViewComponent_onAMSSlot1_4Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onAMSSlot1_4Click\n");
        onAmsSlotLoad(e, 4);
    }
}

void ui_event_comp_AMSViewComponent_onAMSSlot2_1Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onAMSSlot2_1Click\n");
        onAmsSlotLoad(e, 101);
    }
}

void ui_event_comp_AMSViewComponent_onAMSSlot2_2Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onAMSSlot2_2Click\n");
        onAmsSlotLoad(e, 102);
    }
}

void ui_event_comp_AMSViewComponent_onAMSSlot2_3Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onAMSSlot2_3Click\n");
        onAmsSlotLoad(e, 103);
    }
}

void ui_event_comp_AMSViewComponent_onAMSSlot2_4Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onAMSSlot2_4Click\n");
        onAmsSlotLoad(e, 104);
    }
}
void ui_event_comp_AMSViewComponent_onAMSSlot3_1Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onAMSSlot3_1Click\n");
        onAmsSlotLoad(e, 201);
    }
}

void ui_event_comp_AMSViewComponent_onAMSSlot3_2Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onAMSSlot3_2Click\n");
        onAmsSlotLoad(e, 202);
    }
}

void ui_event_comp_AMSViewComponent_onAMSSlot3_3Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onAMSSlot3_3Click\n");
        onAmsSlotLoad(e, 203);
    }
}

void ui_event_comp_AMSViewComponent_onAMSSlot3_4Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onAMSSlot3_4Click\n");
        onAmsSlotLoad(e, 204);
    }
}
void ui_event_comp_AMSViewComponent_onAMSSlot4_1Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onAMSSlot4_1Click\n");
        onAmsSlotLoad(e, 301);
    }
}

void ui_event_comp_AMSViewComponent_onAMSSlot4_2Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onAMSSlot4_2Click\n");
        onAmsSlotLoad(e, 302);
    }
}

void ui_event_comp_AMSViewComponent_onAMSSlot4_3Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onAMSSlot4_3Click\n");
        onAmsSlotLoad(e, 303);
    }
}

void ui_event_comp_AMSViewComponent_onAMSSlot4_4Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onAMSSlot4_4Click\n");
        onAmsSlotLoad(e, 304);
    }
}

void ui_event_comp_AMSViewComponent_onButton1Click(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_CLICKED)
    {
        printf("ui_event_comp_AMSViewComponent_onButton1Click\n");
        onMoveUtilNozzleChangeScreen(e);
    }
}

/* 背景色（EXT時ダミー用）。cui_amsViewComponent の bg に合わせる */
#define AMS_DUMMY_BG 0x444444

/* 選択 sel に応じて1組のコンテンツ内でスロット2〜4とボタン群2〜4の表示を切り替える。EXT時はダミー表示で4列レイアウトを維持 */
static void apply_selector_visibility(lv_obj_t *comp, uint8_t sel)
{
    lv_obj_t *content = lv_obj_get_child(comp, 1);
    if (!content)
        return;
    lv_obj_t *row1 = lv_obj_get_child(content, 0);
    lv_obj_t *row2 = lv_obj_get_child(content, 1);
    if (!row1 || !row2)
        return;
    /* row1: 0=humid, 1=slot1, 2=slot2, 3=slot3, 4=slot4. row2: 0=spacer, 1〜4=btn_grp */
    if (sel == UI_AMS_SELECTOR_EXT)
    {
        for (int i = 2; i <= 4; i++)
        {
            lv_obj_t *slot_cell = lv_obj_get_child(row1, i);
            lv_obj_clear_flag(slot_cell, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_bg_color(slot_cell, lv_color_hex(AMS_DUMMY_BG), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(slot_cell, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(slot_cell, "");
        }
        for (int i = 2; i <= 4; i++)
        {
            lv_obj_t *btn_row = lv_obj_get_child(row2, i);
            lv_obj_clear_flag(btn_row, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_bg_color(btn_row, lv_color_hex(AMS_DUMMY_BG), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(btn_row, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            uint32_t n = lv_obj_get_child_cnt(btn_row);
            for (uint32_t j = 0; j < n; j++)
                lv_obj_add_flag(lv_obj_get_child(btn_row, j), LV_OBJ_FLAG_HIDDEN);
        }
    }
    else
    {
        for (int i = 2; i <= 4; i++)
        {
            lv_obj_clear_flag(lv_obj_get_child(row1, i), LV_OBJ_FLAG_HIDDEN);
            /* スロットは XTOUCH_ON_AMS_SLOT_UPDATE で再描画される */
        }
        for (int i = 2; i <= 4; i++)
        {
            lv_obj_t *btn_row = lv_obj_get_child(row2, i);
            lv_obj_clear_flag(btn_row, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_bg_opa(btn_row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            uint32_t n = lv_obj_get_child_cnt(btn_row);
            for (uint32_t j = 0; j < n; j++)
                lv_obj_clear_flag(lv_obj_get_child(btn_row, j), LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void ui_amsViewComponent_on_selector_change(lv_event_t *e)
{
    lv_obj_t *dropdown = lv_event_get_target(e);
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
        return;
    uint16_t idx = lv_dropdown_get_selected(dropdown);
    if (idx < s_dropdown_option_count)
        s_ams_view_selector = s_dropdown_index_to_selector[idx];
    lv_obj_t *comp = lv_obj_get_parent(lv_obj_get_parent(dropdown));
    apply_selector_visibility(comp, s_ams_view_selector);
    /* 1組のウィジェットの表示内容を再描画 */
    struct XTOUCH_MESSAGE_DATA eventData;
    eventData.data = 0;
    lv_msg_send(XTOUCH_ON_AMS_SLOT_UPDATE, &eventData);
    lv_msg_send(XTOUCH_ON_AMS_HUMIDITY_UPDATE, &eventData);
}

/* AMS有無に応じてコンボの選択肢（接続されているAMSのみ）と初期選択を更新 */
static void ui_amsViewComponent_onAMSBitsSyncDropdown(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_MSG_RECEIVED)
        return;
    lv_obj_t *comp = lv_event_get_target(e);
    lv_obj_t *selector_row = lv_obj_get_child(comp, 0);
    lv_obj_t *dropdown = lv_obj_get_child(selector_row, 0);
    if (!dropdown)
        return;

    char options_buf[64];
    options_buf[0] = '\0';
    strcpy(options_buf, "EXT");
    s_dropdown_index_to_selector[0] = UI_AMS_SELECTOR_EXT;
    s_dropdown_option_count = 1;

    if (bambuStatus.ams_exist_bits != 0)
    {
        const char *labels[] = { "AMS1", "AMS2", "AMS3", "AMS4" };
        for (int i = 0; i < 4; i++)
        {
            if ((bambuStatus.ams_exist_bits & (1u << i)) != 0)
            {
                strcat(options_buf, "\n");
                strcat(options_buf, labels[i]);
                s_dropdown_index_to_selector[s_dropdown_option_count] = (uint8_t)(i + 1);
                s_dropdown_option_count++;
            }
        }
    }

    lv_dropdown_set_options(dropdown, options_buf);

    if (!s_ams_view_initialized)
    {
        if (bambuStatus.ams_exist_bits == 0)
        {
            lv_dropdown_set_selected(dropdown, 0);
            s_ams_view_selector = UI_AMS_SELECTOR_EXT;
        }
        else
        {
            lv_dropdown_set_selected(dropdown, 1);
            s_ams_view_selector = s_dropdown_index_to_selector[1];
        }
        s_ams_view_initialized = true;
    }
    else
    {
        int sel_idx = 0;
        for (int i = 0; i < s_dropdown_option_count; i++)
        {
            if (s_dropdown_index_to_selector[i] == s_ams_view_selector)
            {
                sel_idx = i;
                break;
            }
        }
        if (sel_idx < s_dropdown_option_count)
            lv_dropdown_set_selected(dropdown, sel_idx);
        else
        {
            lv_dropdown_set_selected(dropdown, 0);
            s_ams_view_selector = UI_AMS_SELECTOR_EXT;
        }
    }
    apply_selector_visibility(comp, s_ams_view_selector);
}

lv_obj_t *ui_amsViewComponent_create(lv_obj_t *comp_parent)
{
    s_ams_view_initialized = false;
    lv_obj_t *cui_amsViewComponent;
    cui_amsViewComponent = lv_obj_create(comp_parent);
    lv_obj_set_width(cui_amsViewComponent, lv_pct(100));
    lv_obj_set_height(cui_amsViewComponent, lv_pct(100));
    lv_obj_set_flex_grow(cui_amsViewComponent, 1);
    lv_obj_set_x(cui_amsViewComponent, 0);
    lv_obj_set_y(cui_amsViewComponent, 0);
    lv_obj_set_flex_flow(cui_amsViewComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_amsViewComponent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cui_amsViewComponent, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_amsViewComponent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_amsViewComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_amsViewComponent, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_amsViewComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_amsViewComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_amsViewComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_amsViewComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_amsViewComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_amsViewComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_amsViewComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_amsViewComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_amsViewComponent, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_amsViewComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 上部: コンボボックス 25％。残り75％を AmsControl1 内で Row1 25% / Row2 50% に配分 */
    lv_obj_t *cui_amsSelectorRow = lv_obj_create(cui_amsViewComponent);
    lv_obj_set_width(cui_amsSelectorRow, lv_pct(100));
    lv_obj_set_height(cui_amsSelectorRow, 0);
    lv_obj_set_flex_grow(cui_amsSelectorRow, 1);
    lv_obj_set_flex_flow(cui_amsSelectorRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_amsSelectorRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_amsSelectorRow, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(cui_amsSelectorRow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_amsSelectorRow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_amsSelectorRow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_amsSelectorDropDown = lv_dropdown_create(cui_amsSelectorRow);
    lv_dropdown_set_options(cui_amsSelectorDropDown, "EXT\nAMS1\nAMS2\nAMS3\nAMS4");
    lv_obj_set_width(cui_amsSelectorDropDown, lv_pct(40));
    lv_obj_set_height(cui_amsSelectorDropDown, lv_pct(90));
    lv_obj_set_style_bg_color(cui_amsSelectorDropDown, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_amsSelectorDropDown, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_amsSelectorDropDown, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_dropdown_set_selected(cui_amsSelectorDropDown, UI_AMS_SELECTOR_AMS1);
    lv_obj_add_event_cb(cui_amsSelectorDropDown, ui_amsViewComponent_on_selector_change, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_style_text_font(lv_dropdown_get_list(cui_amsSelectorDropDown), lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(lv_dropdown_get_list(cui_amsSelectorDropDown), lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(lv_dropdown_get_list(cui_amsSelectorDropDown), 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 選択コンボの右: 共通アンロードボタン */
    lv_obj_t *cui_amsUnloadBtn = lv_btn_create(cui_amsSelectorRow);
    lv_obj_set_width(cui_amsUnloadBtn, lv_pct(40));
    lv_obj_set_height(cui_amsUnloadBtn, lv_pct(90));
    lv_obj_set_style_radius(cui_amsUnloadBtn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_amsUnloadBtn, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_amsUnloadBtn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(cui_amsUnloadBtn, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_amsUnloadBtn, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_t *cui_amsUnloadBtn_lbl = lv_label_create(cui_amsUnloadBtn);
    lv_label_set_text(cui_amsUnloadBtn_lbl, "UNLOAD");
    lv_obj_center(cui_amsUnloadBtn_lbl);
    lv_obj_set_style_text_font(cui_amsUnloadBtn_lbl, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(cui_amsUnloadBtn, on_ams_btn_unload, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(cui_amsUnloadBtn, ui_amsViewComponent_onAmsLockSyncUnloadBtn, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_amsUnloadBtn, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_STATE_UPDATE, cui_amsUnloadBtn, NULL);

    /* 1組のコンテンツ（EXT/AMS1〜4で使い回し）。コンボ25%の残り75%を取得し、その中で row1:row2 を 1:2（25%:50%）に配分 */
    lv_obj_t *cui_AmsControl1;
    cui_AmsControl1 = lv_obj_create(cui_amsViewComponent);
    lv_obj_set_width(cui_AmsControl1, lv_pct(100));
    lv_obj_set_height(cui_AmsControl1, 0);
    lv_obj_set_flex_grow(cui_AmsControl1, 3);
    lv_obj_set_flex_flow(cui_AmsControl1, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_AmsControl1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(cui_AmsControl1, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsControl1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_AmsControl1, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_AmsControl1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsControl1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsControl1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsControl1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsControl1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsControl1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_AmsControl1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_AmsControl1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsControl1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_AmsControl1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 1段目: 湿度＋スロット1〜4。全体の25% */
    lv_obj_t *row1_1 = lv_obj_create(cui_AmsControl1);
    lv_obj_set_width(row1_1, lv_pct(100));
    lv_obj_set_height(row1_1, 0);
    lv_obj_set_flex_grow(row1_1, 1);
    lv_obj_set_flex_flow(row1_1, LV_FLEX_FLOW_ROW);
    lv_obj_clear_flag(row1_1, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_pad_bottom(row1_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(row1_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(row1_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(row1_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(row1_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsHumid1;
    cui_AmsHumid1 = lv_label_create(row1_1);
    lv_obj_set_width(cui_AmsHumid1, lv_pct(100));
    lv_obj_set_height(cui_AmsHumid1, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsHumid1, 1);
    lv_obj_set_align(cui_AmsHumid1, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsHumid1, "H\nX");
    lv_obj_clear_flag(cui_AmsHumid1, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsHumid1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsHumid1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsHumid1, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsHumid1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsHumid1, lv_color_hex(0x41ADDC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_AmsHumid1, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsHumid1, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_text_color(cui_AmsHumid1, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsHumid1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsHumid1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsHumid1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsHumid1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsHumid1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsHumid1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsHumid1, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsHumid1, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsHumid1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsSlot1_1;
    cui_AmsSlot1_1 = lv_label_create(row1_1);
    lv_obj_set_width(cui_AmsSlot1_1, lv_pct(100));
    lv_obj_set_height(cui_AmsSlot1_1, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsSlot1_1, 2);
    lv_obj_set_align(cui_AmsSlot1_1, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsSlot1_1, "Slot 1");
    lv_obj_add_flag(cui_AmsSlot1_1, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(cui_AmsSlot1_1, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsSlot1_1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsSlot1_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsSlot1_1, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsSlot1_1, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_AmsSlot1_1, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1_1, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsSlot1_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsSlot1_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsSlot1_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsSlot1_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsSlot1_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1_1, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1_1, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsSlot1_1, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsSlot1_2;
    cui_AmsSlot1_2 = lv_label_create(row1_1);
    lv_obj_set_width(cui_AmsSlot1_2, lv_pct(100));
    lv_obj_set_height(cui_AmsSlot1_2, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsSlot1_2, 2);
    lv_obj_set_align(cui_AmsSlot1_2, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsSlot1_2, "Slot 2");
    lv_obj_add_flag(cui_AmsSlot1_2, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_AmsSlot1_2, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsSlot1_2, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsSlot1_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsSlot1_2, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsSlot1_2, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_AmsSlot1_2, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1_2, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsSlot1_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsSlot1_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsSlot1_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsSlot1_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsSlot1_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1_2, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1_2, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsSlot1_2, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsSlot1_3;
    cui_AmsSlot1_3 = lv_label_create(row1_1);
    lv_obj_set_width(cui_AmsSlot1_3, lv_pct(100));
    lv_obj_set_height(cui_AmsSlot1_3, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsSlot1_3, 2);
    lv_obj_set_align(cui_AmsSlot1_3, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsSlot1_3, "Slot 3");
    lv_obj_add_flag(cui_AmsSlot1_3, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_AmsSlot1_3, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsSlot1_3, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsSlot1_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsSlot1_3, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsSlot1_3, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_AmsSlot1_3, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1_3, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsSlot1_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsSlot1_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsSlot1_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsSlot1_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsSlot1_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1_3, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1_3, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsSlot1_3, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_AmsSlot1_4;
    cui_AmsSlot1_4 = lv_label_create(row1_1);
    lv_obj_set_width(cui_AmsSlot1_4, lv_pct(100));
    lv_obj_set_height(cui_AmsSlot1_4, lv_pct(100));
    lv_obj_set_flex_grow(cui_AmsSlot1_4, 2);
    lv_obj_set_align(cui_AmsSlot1_4, LV_ALIGN_CENTER);
    lv_label_set_text(cui_AmsSlot1_4, "Slot 4");
    lv_obj_add_flag(cui_AmsSlot1_4, LV_OBJ_FLAG_CLICKABLE);                                                                                                                                                                                                      /// Flags
    lv_obj_clear_flag(cui_AmsSlot1_4, LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN); /// Flags
    lv_obj_set_scrollbar_mode(cui_AmsSlot1_4, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_align(cui_AmsSlot1_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(cui_AmsSlot1_4, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_AmsSlot1_4, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(cui_AmsSlot1_4, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1_4, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1_4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_AmsSlot1_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_AmsSlot1_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_AmsSlot1_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_AmsSlot1_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_AmsSlot1_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_AmsSlot1_4, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(cui_AmsSlot1_4, 255, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(cui_AmsSlot1_4, AMS_BORDER, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 2段目: EDIT/LOAD ボタン群。残り領域の 2/3 */
    /* 2段目: EDIT/LOAD ボタン。全体の50% */
    lv_obj_t *row2_1 = lv_obj_create(cui_AmsControl1);
    lv_obj_set_width(row2_1, lv_pct(100));
    lv_obj_set_height(row2_1, 0);
    lv_obj_set_flex_grow(row2_1, 2);
    lv_obj_set_flex_flow(row2_1, LV_FLEX_FLOW_ROW);
    lv_obj_clear_flag(row2_1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_bottom(row1_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(row2_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(row2_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(row2_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(row2_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *sp = lv_obj_create(row2_1);
        lv_obj_set_width(sp, 0);
        lv_obj_set_height(sp, lv_pct(100));
        lv_obj_set_flex_grow(sp, 0);
        lv_obj_set_style_bg_opa(sp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(sp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(sp, LV_OBJ_FLAG_CLICKABLE);
        for (int i = 0; i < 4; i++)
        {
            lv_obj_t *btn_row = lv_obj_create(row2_1);
            lv_obj_set_width(btn_row, lv_pct(100));
            lv_obj_set_height(btn_row, lv_pct(100));
            lv_obj_set_flex_grow(btn_row, 2);
            lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_style_border_width(btn_row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(btn_row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(btn_row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(btn_row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(btn_row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_row(btn_row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_column(btn_row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(btn_row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_flag(btn_row, LV_OBJ_FLAG_SCROLLABLE);
            for (int b = 0; b < 2; b++)
            {
                lv_obj_t *btn = lv_btn_create(btn_row);
                lv_obj_set_width(btn, lv_pct(100));
                lv_obj_set_height(btn, lv_pct(50));
                lv_obj_set_flex_grow(btn, 1);
                lv_obj_set_style_radius(btn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_color(btn, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_width(btn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_color(btn, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_left(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_right(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_top(btn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_bottom(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_t *lbl = lv_label_create(btn);
                lv_label_set_text(lbl, b == 0 ? "EDIT" : "LOAD");
                lv_obj_center(lbl);
                lv_obj_set_style_text_font(lbl, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
                if (b == 0)
                    lv_obj_add_event_cb(btn, on_ams_btn_edit, LV_EVENT_CLICKED, NULL);
                else
                    lv_obj_add_event_cb(btn, onAmsSlotClickUnified, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
            }
        }
    }
    lv_obj_add_event_cb(row2_1, ui_amsViewComponent_onAmsLockSyncButtons, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, row2_1, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_STATE_UPDATE, row2_1, NULL);

    lv_obj_t **children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_AMSVIEWCOMPONENT_NUM);
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENT] = cui_amsViewComponent;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTCONTROL1] = cui_AmsControl1;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTCONTROL2] = cui_AmsControl1;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTCONTROL3] = cui_AmsControl1;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTCONTROL4] = cui_AmsControl1;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTHUMID1] = cui_AmsHumid1;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTHUMID2] = cui_AmsHumid1;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTHUMID3] = cui_AmsHumid1;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTHUMID4] = cui_AmsHumid1;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTSLOT1_1] = cui_AmsSlot1_1;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTSLOT1_2] = cui_AmsSlot1_2;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTSLOT1_3] = cui_AmsSlot1_3;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTSLOT1_4] = cui_AmsSlot1_4;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTSLOT2_1] = cui_AmsSlot1_1;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTSLOT2_2] = cui_AmsSlot1_2;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTSLOT2_3] = cui_AmsSlot1_3;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTSLOT2_4] = cui_AmsSlot1_4;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTSLOT3_1] = cui_AmsSlot1_1;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTSLOT3_2] = cui_AmsSlot1_2;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTSLOT3_3] = cui_AmsSlot1_3;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTSLOT3_4] = cui_AmsSlot1_4;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTSLOT4_1] = cui_AmsSlot1_1;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTSLOT4_2] = cui_AmsSlot1_2;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTSLOT4_3] = cui_AmsSlot1_3;
    children[UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTSLOT4_4] = cui_AmsSlot1_4;

    lv_obj_add_event_cb(cui_amsViewComponent, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
    lv_obj_add_event_cb(cui_amsViewComponent, del_component_child_event_cb, LV_EVENT_DELETE, children);
    lv_obj_add_event_cb(cui_amsViewComponent, ui_amsViewComponent_onAMSBitsSyncDropdown, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_BITS, cui_amsViewComponent, NULL);

    /* スロットクリック: 選択子＋slot_indexでLOAD */
    lv_obj_add_event_cb(cui_AmsSlot1_1, onAmsSlotClickUnified, LV_EVENT_CLICKED, (void *)0);
    lv_obj_add_event_cb(cui_AmsSlot1_2, onAmsSlotClickUnified, LV_EVENT_CLICKED, (void *)1);
    lv_obj_add_event_cb(cui_AmsSlot1_3, onAmsSlotClickUnified, LV_EVENT_CLICKED, (void *)2);
    lv_obj_add_event_cb(cui_AmsSlot1_4, onAmsSlotClickUnified, LV_EVENT_CLICKED, (void *)3);

    // Humidity: 1組のラベルを選択子に応じて更新
    lv_obj_add_event_cb(cui_AmsHumid1, ui_event_comp_amsViewComponent_onAmsHumidityUnified, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_HUMIDITY_UPDATE, cui_AmsHumid1, NULL);

    // Slot表示: 1組の4ラベルを選択子＋slot_indexで更新
    lv_obj_add_event_cb(cui_AmsSlot1_1, ui_event_comp_amsViewComponent_onAmsUpdateBySlotIndex, LV_EVENT_MSG_RECEIVED, (void *)0);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_AmsSlot1_1, (void *)0);

    lv_obj_add_event_cb(cui_AmsSlot1_2, ui_event_comp_amsViewComponent_onAmsUpdateBySlotIndex, LV_EVENT_MSG_RECEIVED, (void *)1);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_AmsSlot1_2, (void *)1);

    lv_obj_add_event_cb(cui_AmsSlot1_3, ui_event_comp_amsViewComponent_onAmsUpdateBySlotIndex, LV_EVENT_MSG_RECEIVED, (void *)2);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_AmsSlot1_3, (void *)2);

    lv_obj_add_event_cb(cui_AmsSlot1_4, ui_event_comp_amsViewComponent_onAmsUpdateBySlotIndex, LV_EVENT_MSG_RECEIVED, (void *)3);
    lv_msg_subsribe_obj(XTOUCH_ON_AMS_SLOT_UPDATE, cui_AmsSlot1_4, (void *)3);

    apply_selector_visibility(cui_amsViewComponent, s_ams_view_selector);

    ui_comp_amsViewComponent_create_hook(cui_amsViewComponent);

    struct XTOUCH_MESSAGE_DATA eventData;
    eventData.data = 0;
    lv_msg_send(XTOUCH_ON_AMS_BITS, &eventData);
    lv_msg_send(XTOUCH_ON_AMS_SLOT_UPDATE, &eventData);
    lv_msg_send(XTOUCH_ON_AMS_HUMIDITY_UPDATE, &eventData);

    return cui_amsViewComponent;
}