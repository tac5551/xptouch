#include <pgmspace.h>

// 最適化版を使用するため、元のファイルは最小限に
int hms_error_length = 0;
const char *hms_error_keys[] PROGMEM = {};
const char *hms_error_values[] PROGMEM = {};

int device_error_length = 0;
const char *device_error_keys[] PROGMEM = {};
const char *device_error_values[] PROGMEM = {};

// 削除済み - 使用されていないため 
// リンクエラー回避のためのダミー定義
int message_containing_retry_total = 0;
int message_containing_done_total = 0;
const char *message_containing_retry[] PROGMEM = {};
const char *message_containing_done[] PROGMEM = {}; 