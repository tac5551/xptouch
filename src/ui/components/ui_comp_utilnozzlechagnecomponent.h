#ifndef _UI_COMP_UTILNOZZLECHANGECOMPONENT_H
#define _UI_COMP_UTILNOZZLECHANGECOMPONENT_H

#include "../ui.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // COMPONENT utilNozzleChagneComponent

    enum NozzleChagneComponents
    {
        UI_COMP_UTILNOZZLECHANGECOMPONENT_UTILNOZZLECHANGECOMPONENT,

        UI_COMP_UTILNOZZLECHANGECOMPONENT_NOZZLETYPE,
        UI_COMP_UTILNOZZLECHANGECOMPONENT_NOZZLETYPE_DROPDOWN,
        UI_COMP_UTILNOZZLECHANGECOMPONENT_NOZZLEDEMILITER_DROPDOWN,
        UI_COMP_UTILNOZZLECHANGECOMPONENT_NOZZLE_SAVE,
        _UI_COMP_UTILNOZZLECHANGECOMPONENT_NUM
    };
    lv_obj_t *ui_utilNozzleChagneComponent_create(lv_obj_t *comp_parent);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif