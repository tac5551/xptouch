#ifndef _UI_COMP_CAMERACOMPONENT_H
#define _UI_COMP_CAMERACOMPONENT_H

#include "../ui.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum CameraComponents
{
    UI_COMP_CAMERACOMPONENT_CAMERACOMPONENT,
    UI_COMP_CAMERACOMPONENT_BACKBUTTON,
    UI_COMP_CAMERACOMPONENT_PREVIEWPANEL,
    UI_COMP_CAMERACOMPONENT_PREVIEWIMAGE,
    _UI_COMP_CAMERACOMPONENT_NUM
};

lv_obj_t *ui_cameraComponent_create(lv_obj_t *comp_parent);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
