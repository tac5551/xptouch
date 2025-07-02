#ifndef _UI_COMP_AMSVIEWCOMPONENT_H
#define _UI_COMP_AMSVIEWCOMPONENT_H

#include "../ui.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENT 0
#define UI_COMP_AMSVIEWCOMPONENT_AMSVIEWCOMPONENTCONTAINER 1
#define _UI_COMP_AMSVIEWCOMPONENT_NUM 2

    lv_obj_t *ui_amsViewComponent_create(lv_obj_t *comp_parent);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif