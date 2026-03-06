#ifndef _UI_COMP_PRINTERSCOMPONENT_H
#define _UI_COMP_PRINTERSCOMPONENT_H

#include "../ui.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // COMPONENT printersComponent
    enum PrintersComponents
    {
        UI_COMP_PRINTERSCOMPONENT_PRINTERSCOMPONENT,
        UI_COMP_PRINTERSCOMPONENT_LIST,
        _UI_COMP_PRINTERSCOMPONENT_NUM
    };

    lv_obj_t *ui_printersComponent_create(lv_obj_t *comp_parent);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

