
#ifndef _V3CONTROLCOMP_UI_LOADERS_H
#define _V3CONTROLCOMP_UI_LOADERS_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "ui.h"
#include "ui_msgs.h"

    void fillScreenData(int screen);
    void loadScreen(int screen);
    void initTopLayer();
    void ui_settings_mark_dirty(void);
    void ui_settings_clear_dirty(void);
    bool ui_settings_has_unsaved_changes(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif