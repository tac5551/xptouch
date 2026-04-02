#include "../ui.h"
#include "../ui_msgs.h"
#include "ui_comp_systemcomponent.h"

lv_obj_t *ui_systemComponent_create(lv_obj_t *comp_parent)
{
    lv_obj_t *cui_systemComponent = lv_obj_create(comp_parent);
    lv_obj_set_height(cui_systemComponent, lv_pct(100));
    lv_obj_set_flex_grow(cui_systemComponent, 1);
    lv_obj_set_flex_flow(cui_systemComponent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cui_systemComponent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_radius(cui_systemComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_systemComponent, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_systemComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_systemComponent, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_systemComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_systemComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(cui_systemComponent, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(cui_systemComponent, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_systemComponent, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(cui_systemComponent, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(cui_systemComponent, LV_SCROLLBAR_MODE_ACTIVE);

    lv_obj_t *cui_pairingTitle = lv_label_create(cui_systemComponent);
    lv_obj_set_width(cui_pairingTitle, lv_pct(100));
    lv_obj_set_height(cui_pairingTitle, LV_SIZE_CONTENT);
    lv_label_set_text(cui_pairingTitle, LV_SYMBOL_LIST " CONNECTED PRINTERS");
    lv_obj_set_scrollbar_mode(cui_pairingTitle, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_pairingTitle, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_pairingTitle, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_pairingTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_pairingTitle, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_pairingTitle, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_pairingTitle, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_pairingTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_pairingTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_pairingTitle, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_pairingTitle, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_unpairButton = lv_label_create(cui_systemComponent);
    lv_obj_set_width(cui_unpairButton, lv_pct(100));
    lv_obj_set_height(cui_unpairButton, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(cui_unpairButton, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(cui_unpairButton, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(cui_unpairButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_unpairButton, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_unpairButton, lv_color_hex(0xFF682A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_unpairButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_unpairButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_unpairButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_unpairButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_unpairButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_unpairButton, lv_color_hex(0x000), LV_PART_MAIN | LV_STATE_DEFAULT);
    if (xTouchConfig.xTouchLanOnlyMode)
    {
        /* LAN モード: プリンタ名のみ表示、Unlink 禁止（ボタン無効）、その下に接続先 IP を表示 */
        lv_label_set_text_fmt(cui_unpairButton, "[ %s ]", xTouchConfig.xTouchPrinterName[0] ? xTouchConfig.xTouchPrinterName : xTouchConfig.xTouchSerialNumber);
        lv_obj_clear_flag(cui_unpairButton, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_state(cui_unpairButton, LV_STATE_DISABLED);

        lv_obj_t *cui_lanIpLabel = lv_label_create(cui_systemComponent);
        lv_obj_set_width(cui_lanIpLabel, lv_pct(100));
        lv_obj_set_height(cui_lanIpLabel, LV_SIZE_CONTENT);
        lv_label_set_text_fmt(cui_lanIpLabel, LV_SYMBOL_WIFI " %s", xTouchConfig.xTouchHost[0] ? xTouchConfig.xTouchHost : "-");
        lv_obj_set_style_text_font(cui_lanIpLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_left(cui_lanIpLabel, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(cui_lanIpLabel, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(cui_lanIpLabel, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(cui_lanIpLabel, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(cui_lanIpLabel, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(cui_lanIpLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(cui_lanIpLabel, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(cui_lanIpLabel, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else
    {
        lv_label_set_text_fmt(cui_unpairButton, LV_SYMBOL_SHUFFLE " Unlink [ %s ]", xTouchConfig.xTouchPrinterName);
    }

    lv_obj_t *cui_deviceTitle = lv_label_create(cui_systemComponent);
    lv_obj_set_width(cui_deviceTitle, lv_pct(100));
    lv_obj_set_height(cui_deviceTitle, LV_SIZE_CONTENT);
    lv_label_set_text_fmt(cui_deviceTitle, LV_SYMBOL_LIST " XTOUCH v%s", XTOUCH_FIRMWARE_VERSION);
    lv_obj_set_scrollbar_mode(cui_deviceTitle, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(cui_deviceTitle, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_deviceTitle, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_deviceTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_deviceTitle, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_deviceTitle, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_deviceTitle, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_deviceTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_deviceTitle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cui_deviceTitle, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_deviceTitle, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    /* AUX FAN / CHAMBER FAN は General タブに配置 */

    lv_obj_t *cui_settings_ota = lv_obj_create(cui_systemComponent);
    lv_obj_set_width(cui_settings_ota, lv_pct(100));
    lv_obj_set_height(cui_settings_ota, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cui_settings_ota, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cui_settings_ota, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(cui_settings_ota, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cui_settings_ota, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_settings_ota, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cui_settings_ota, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_settings_ota, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_settings_ota, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_settings_ota, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_settings_ota, 16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_settings_otaLabel = lv_label_create(cui_settings_ota);
    lv_obj_set_width(cui_settings_otaLabel, LV_SIZE_CONTENT);
    lv_obj_set_height(cui_settings_otaLabel, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(cui_settings_otaLabel, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_settings_otaLabel, LV_SYMBOL_DOWNLOAD "OTA Update");
    lv_obj_set_scrollbar_mode(cui_settings_otaLabel, LV_SCROLLBAR_MODE_OFF);

    ui_settings_otaSwitch = lv_switch_create(cui_settings_ota);
    lv_obj_set_width(ui_settings_otaSwitch, 50);
    lv_obj_set_height(ui_settings_otaSwitch, 25);
    lv_obj_set_style_bg_color(ui_settings_otaSwitch, lv_color_hex(0x2AFF00), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_settings_otaSwitch, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui_settings_otaSwitch, lv_color_hex(0x2AFF00), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_settings_otaSwitch, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_settings_otaSwitch, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(ui_settings_otaSwitch, 255, LV_PART_KNOB | LV_STATE_CHECKED);
    if (xTouchConfig.xTouchOTAEnabled)
        lv_obj_add_state(ui_settings_otaSwitch, LV_STATE_CHECKED);

    lv_obj_t *cui_otaNowButton = lv_label_create(cui_systemComponent);
    lv_obj_set_width(cui_otaNowButton, lv_pct(100));
    lv_obj_set_height(cui_otaNowButton, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(cui_otaNowButton, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_otaNowButton, LV_SYMBOL_DOWNLOAD " Update Now");
    lv_obj_add_flag(cui_otaNowButton, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(cui_otaNowButton, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(cui_otaNowButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_otaNowButton, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_otaNowButton, lv_color_hex(0x68FF2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_otaNowButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_otaNowButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_otaNowButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_otaNowButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_otaNowButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_otaNowButton, lv_color_hex(0x000), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cui_reseDeviceButton = lv_label_create(cui_systemComponent);
    lv_obj_set_width(cui_reseDeviceButton, lv_pct(100));
    lv_obj_set_height(cui_reseDeviceButton, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(cui_reseDeviceButton, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_reseDeviceButton, LV_SYMBOL_POWER " Reboot Device");
    lv_obj_add_flag(cui_reseDeviceButton, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(cui_reseDeviceButton, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(cui_reseDeviceButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_reseDeviceButton, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_reseDeviceButton, lv_color_hex(0xFF682A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_reseDeviceButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_reseDeviceButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_reseDeviceButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_reseDeviceButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_reseDeviceButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_reseDeviceButton, lv_color_hex(0x000), LV_PART_MAIN | LV_STATE_DEFAULT);

#ifdef __XTOUCH_PLATFORM_S3__
    lv_obj_t *cui_clearCacheButton = lv_label_create(cui_systemComponent);
    lv_obj_set_width(cui_clearCacheButton, lv_pct(100));
    lv_obj_set_height(cui_clearCacheButton, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(cui_clearCacheButton, lv_font_small, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(cui_clearCacheButton, LV_SYMBOL_REFRESH " Clear Cache");
    lv_obj_add_flag(cui_clearCacheButton, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(cui_clearCacheButton, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(cui_clearCacheButton, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(cui_clearCacheButton, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cui_clearCacheButton, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cui_clearCacheButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cui_clearCacheButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cui_clearCacheButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cui_clearCacheButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cui_clearCacheButton, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cui_clearCacheButton, lv_color_hex(0xEEEEEE), LV_PART_MAIN | LV_STATE_DEFAULT);
#endif

    lv_obj_add_event_cb(cui_unpairButton, ui_event_comp_settingsComponent_unpairButton, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(cui_otaNowButton, ui_event_comp_settingsComponent_OtaUpdateNowButton, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(cui_reseDeviceButton, ui_event_comp_settingsComponent_resetDeviceButton, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_settings_otaSwitch, ui_event_comp_settingsComponent_onOTA, LV_EVENT_VALUE_CHANGED, NULL);
#ifdef __XTOUCH_PLATFORM_S3__
    lv_obj_add_event_cb(cui_clearCacheButton, ui_event_comp_settingsComponent_clearCacheButton, LV_EVENT_CLICKED, NULL);
#endif

    return cui_systemComponent;
}
