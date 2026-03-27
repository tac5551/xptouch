#include <stdio.h>
#include <string.h>
#include "../ui.h"
#include "../ui_msgs.h"
#include "../ui_helpers.h"

#ifdef __XTOUCH_SCREEN_50__

static const char *history_status_str(int s)
{
    switch (s)
    {
    case 2: return "Finished";
    case 3: return "Failed";
    default: return "-";
    }
}

/* 日付 "2026-03-13T07:57:33Z" を "03-13 07:57" 風に短くする */
static void format_time_short(const char *src, char *out, size_t out_size)
{
    if (!src || !out || out_size < 12)
    {
        if (out && out_size)
            out[0] = '\0';
        return;
    }
    /* 0-3: 年, 5-6: 月, 8-9: 日, 11-12: 時, 14-15: 分 */
    int mo = 0, d = 0, h = 0, mi = 0;
    if (strlen(src) >= 16)
    {
        sscanf(src, "%*4d-%2d-%2dT%2d:%2d", &mo, &d, &h, &mi);
        snprintf(out, out_size, "%02d-%02d %02d:%02d", mo, d, h, mi);
    }
    else
        strncpy(out, src, out_size - 1), out[out_size - 1] = '\0';
}

static void update_one_row(int idx, lv_obj_t *row)
{
    if (row == NULL)
        return;
    lv_obj_t *leftBox = lv_obj_get_child(row, 0);
    lv_obj_t *rightCol = lv_obj_get_child(row, 1);
    lv_obj_t *reprintBtn = lv_obj_get_child(row, 2);
    if (!rightCol || !reprintBtn)
        return;
    lv_obj_t *titleLabel = lv_obj_get_child(rightCol, 0);
    lv_obj_t *printerLabel = lv_obj_get_child(rightCol, 1);
    lv_obj_t *dateLabel = lv_obj_get_child(rightCol, 2);
    lv_obj_t *statusLabel = lv_obj_get_child(rightCol, 3);
    if (!titleLabel || !printerLabel || !dateLabel || !statusLabel)
        return;

    /* 行別: cover サムネイルを表示（xtouch_history_cover_dsc[idx] が設定されていれば） */
    if (leftBox)
    {
        lv_obj_t *coverImg = lv_obj_get_child(leftBox, 0);
        lv_obj_t *placeLabel = lv_obj_get_child(leftBox, 1);
        if (coverImg && placeLabel)
        {
            if (xTouchConfig.xTouchHideAllThumbnails)
            {
                if (xtouch_thumbnail_slot_dsc[0] != NULL)
                {
                    lv_img_set_src(coverImg, (const lv_img_dsc_t *)xtouch_thumbnail_slot_dsc[0]);
                    lv_obj_clear_flag(coverImg, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(placeLabel, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_invalidate(coverImg);
                }
                else
                {
                    lv_obj_add_flag(coverImg, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(placeLabel, LV_OBJ_FLAG_HIDDEN);
                }
            }
            else if (idx < XTOUCH_HISTORY_COVER_SLOTS && xtouch_history_cover_dsc[idx] != NULL)
            {
                lv_img_set_src(coverImg, (const void *)xtouch_history_cover_dsc[idx]);
                lv_obj_clear_flag(coverImg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(placeLabel, LV_OBJ_FLAG_HIDDEN);
            }
            else
            {
                lv_obj_add_flag(coverImg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(placeLabel, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }

    if (idx >= xtouch_history_count || !xtouch_history_tasks[idx].valid)
    {
        lv_label_set_text(titleLabel, "-");
        lv_label_set_text(printerLabel, "");
        lv_label_set_text(dateLabel, "");
        lv_label_set_text(statusLabel, "");
        lv_obj_add_state(reprintBtn, LV_STATE_DISABLED);
        return;
    }

    const xtouch_history_task_t *t = &xtouch_history_tasks[idx];
    lv_label_set_text(titleLabel, t->title[0] ? t->title : "-");
    {
        char pbuf[XTOUCH_HISTORY_DEVICE_NAME_LEN + XTOUCH_HISTORY_DEVICE_MODEL_LEN + 8];
        if (t->device_name[0] && t->device_model[0] && strcmp(t->device_name, t->device_model) != 0)
            snprintf(pbuf, sizeof(pbuf), "%s (%s)", t->device_name, t->device_model);
        else if (t->device_name[0])
            snprintf(pbuf, sizeof(pbuf), "%s", t->device_name);
        else if (t->device_model[0])
            snprintf(pbuf, sizeof(pbuf), "%s", t->device_model);
        else
            pbuf[0] = '\0';
        lv_label_set_text(printerLabel, pbuf[0] ? pbuf : "-");
    }
    char dateBuf[24];
    format_time_short(t->end_time[0] ? t->end_time : t->start_time, dateBuf, sizeof(dateBuf));
    lv_label_set_text(dateLabel, dateBuf);
    lv_label_set_text(statusLabel, history_status_str(t->status));
    if (t->is_printable)
        lv_obj_clear_state(reprintBtn, LV_STATE_DISABLED);
    else
        lv_obj_add_state(reprintBtn, LV_STATE_DISABLED);
}

void ui_history_on_list_refresh(lv_msg_t *m, void *user_data)
{
    (void)m;
    (void)user_data;
    if (ui_historyListContainer == NULL || xTouchConfig.currentScreenIndex != 15)
        return;
    for (int i = 0; i < XTOUCH_HISTORY_TASKS_MAX; i++)
    {
        lv_obj_t *row = lv_obj_get_child(ui_historyListContainer, i);
        if (row == NULL)
            continue;
        if (i < xtouch_history_count)
        {
            lv_obj_clear_flag(row, LV_OBJ_FLAG_HIDDEN);
            update_one_row(i, row);
        }
        else
            lv_obj_add_flag(row, LV_OBJ_FLAG_HIDDEN);
    }
}

static void ui_event_history_on_list_refresh(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_MSG_RECEIVED)
        return;
    ui_history_on_list_refresh(lv_event_get_msg(e), NULL);
}

void ui_historyScreen_screen_init(void)
{
    ui_historyScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_historyScreen, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_set_scrollbar_mode(ui_historyScreen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(ui_historyScreen, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_historyScreen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(ui_historyScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_historyScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_historyScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_historyScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(ui_historyScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(ui_historyScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_sidebarComponent = ui_sidebarComponent_create(ui_historyScreen);
    lv_obj_set_x(ui_sidebarComponent, 387);
    lv_obj_set_y(ui_sidebarComponent, 178);

    ui_historyComponent_create(ui_historyScreen);
    lv_msg_subsribe_obj(XTOUCH_HISTORY_LIST_REFRESH, ui_historyListContainer, NULL);
    lv_obj_add_event_cb(ui_historyListContainer, ui_event_history_on_list_refresh, LV_EVENT_MSG_RECEIVED, NULL);
    {
        ui_msg_send(XTOUCH_HISTORY_LIST_REFRESH, 0, 0);
    }
}

#else

void ui_historyScreen_screen_init(void)
{
    (void)0;
}

#endif
