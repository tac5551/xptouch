#ifndef _UI_COMP_HISTORYCOMPONENT_H
#define _UI_COMP_HISTORYCOMPONENT_H

#include "../ui.h"

#ifdef __cplusplus
extern "C"
{
#endif

    enum HistoryComponents
    {
        UI_COMP_HISTORYCOMPONENT_HISTORYCOMPONENT,
        UI_COMP_HISTORYCOMPONENT_LIST,
        _UI_COMP_HISTORYCOMPONENT_NUM
    };

    lv_obj_t *ui_historyComponent_create(lv_obj_t *comp_parent);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
