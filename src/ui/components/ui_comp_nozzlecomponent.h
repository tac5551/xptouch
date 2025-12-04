#ifndef _UI_COMP_NOZZLECOMPONENT_H
#define _UI_COMP_NOZZLECOMPONENT_H

#include "../ui.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // COMPONENT nozzleComponent

    enum NozzleComponents
    {
        UI_COMP_NOZZLECOMPONENT_NOZZLECOMPONENT,

        UI_COMP_NOZZLECOMPONENT_NOZZLETYPE,
        UI_COMP_NOZZLECOMPONENT_NOZZLETYPE_DROPDOWN,
        UI_COMP_NOZZLECOMPONENT_NOZZLEDEMILITER_DROPDOWN,
        UI_COMP_NOZZLECOMPONENT_NOZZLE_SAVE,
        _UI_COMP_NOZZLECOMPONENT_NUM
    };
    lv_obj_t *ui_nozzleComponent_create(lv_obj_t *comp_parent);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif