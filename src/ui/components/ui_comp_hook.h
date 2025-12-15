#ifndef _V3CONTROLCOMP_UI_COMP_HOOK_H
#define _V3CONTROLCOMP_UI_COMP_HOOK_H

#ifdef __cplusplus
extern "C"
{
#endif

    void ui_comp_sidebarComponent_create_hook(lv_obj_t *comp);
    void ui_comp_homeComponent_create_hook(lv_obj_t *comp);
    void ui_comp_temperatureComponent_create_hook(lv_obj_t *comp);
    void ui_comp_LightingPanel_create_hook(lv_obj_t *comp);
    void ui_comp_controlComponent_create_hook(lv_obj_t *comp);
    void ui_comp_settingsComponent_create_hook(lv_obj_t *comp);
    void ui_comp_optionalComponent_create_hook(lv_obj_t *comp);
    void ui_comp_confirmPanel_create_hook(lv_obj_t *comp);
    void ui_comp_hmsPanel_create_hook(lv_obj_t *comp);
    void ui_comp_mainScreenStatus_create_hook(lv_obj_t *comp);
    void ui_comp_filamentComponent_create_hook(lv_obj_t *comp);
    void ui_comp_amsViewComponent_create_hook(lv_obj_t *comp);
    void ui_comp_characterComponent_create_hook(lv_obj_t *comp);
    void ui_comp_utilNozzleChagneComponent_create_hook(lv_obj_t *comp);
    void ui_comp_utilComponent_create_hook(lv_obj_t *comp);
    void ui_comp_utilCalibrationComponent_create_hook(lv_obj_t *comp);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
