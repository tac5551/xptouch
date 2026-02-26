/**
 * AMS Edit: フィラメント選択時に非同期で取得した slicer setting の温度を保持。
 * 編集対象スロット（ams_id / tray_id）も保持。AMS View で Edit 押下時に set、Save 時に参照。
 */
#ifndef _XTOUCH_AMS_EDIT_TEMP_H
#define _XTOUCH_AMS_EDIT_TEMP_H

#ifdef __cplusplus
extern "C" {
#endif

extern char ams_edit_fetched_setting_id[16];
extern int ams_edit_fetched_min;
extern int ams_edit_fetched_max;
/** Cloud getSlicerSetting から取得した filament_id（例: GFB00）。MQTT の tray_info_idx / filament_id に使う。 */
extern char ams_edit_fetched_filament_id[16];

/** 編集対象スロット（AMS View で Edit 押下時に設定）。ams_id=0..3, tray_id=1..4 */
extern int ams_edit_current_ams_id;
extern int ams_edit_current_tray_id;

/** 編集中のトレイ色（RRGGBBAA。デフォルト "00000000" は透明）。色パレットで設定。 */
extern char ams_edit_current_tray_color[12];

/** 編集中の Brand/Type ドロップダウン選択（色画面から戻ったときに復元）。-1 のときはトレイ種別からロード。 */
extern int ams_edit_current_brand_index;
extern int ams_edit_current_type_index;

void ams_edit_set_fetched_temps(const char *id, int min_val, int max_val, const char *filament_id);
void ams_edit_set_editing_slot(int ams_id, int tray_id);
/** デバッグ: AMS Edit Save 時の温度ログ（C から呼ぶ。実体は C++） */
void xtouch_debug_log_ams_save(const char *id_buf, const char *fetched_id, int id_match, int fetched_min, int fetched_max, int payload_min, int payload_max);
/** 色パレットで選択した色を保存。hex8 は "RRGGBBAA" または "RRGGBB"（6桁のときは末尾に "FF" を付与）。 */
void ams_edit_set_tray_color(const char *hex8);

#ifdef __cplusplus
}
#endif

#endif
