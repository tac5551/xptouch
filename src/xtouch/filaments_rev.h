/**
 * filaments_rev.json の読み込み: GFL99 等 → b/t 逆引き。
 * JSON 形式: { "filaments_by_id": { "GFL99": { "b": "Bambu Lab", "t": "ABS" }, ... } }
 * 実装はこの .h のみ。main.cpp で include して使い回す。
 */
#ifndef _XTOUCH_FILAMENTS_REV_H
#define _XTOUCH_FILAMENTS_REV_H

#ifdef __cplusplus
#include <ArduinoJson.h>
#include <SD.h>
#include <string.h>

extern "C" {

int xtouch_public_filaments_rev_lookup(const char *filament_id, char *out_brand, size_t out_brand_len, char *out_type, size_t out_type_len);

} /* extern "C" */

#endif /* __cplusplus */
#endif /* _XTOUCH_FILAMENTS_REV_H */

/* 実装: この .h を include した TU で1回だけコンパイル（main.cpp で include して使い回す） */
#ifndef _XTOUCH_FILAMENTS_REV_IMPL_GUARD
#define _XTOUCH_FILAMENTS_REV_IMPL_GUARD
#if defined(__cplusplus)
/* xtouch_paths_filaments_rev は main.cpp が paths.h で取得済みの前提 */
extern "C" {

int xtouch_public_filaments_rev_lookup(const char *filament_id, char *out_brand, size_t out_brand_len, char *out_type, size_t out_type_len)
{
    if (!filament_id || filament_id[0] == '\0' || !out_brand || out_brand_len == 0 || !out_type || out_type_len == 0)
        return 0;
    out_brand[0] = '\0';
    out_type[0] = '\0';

    if (!SD.exists(xtouch_paths_filaments_rev))
        return 0;

    File f = SD.open(xtouch_paths_filaments_rev, FILE_READ);
    if (!f || !f.available())
    {
        if (f)
            f.close();
        return 0;
    }

    const size_t doc_size = 1536;
    DynamicJsonDocument doc(doc_size);
    DeserializationError err = deserializeJson(doc, f);
    f.close();
    if (err || doc.isNull())
        return 0;

    JsonObject by_id = doc["filaments_by_id"];
    if (by_id.isNull())
        return 0;

    JsonObject entry = by_id[filament_id];
    if (entry.isNull())
        return 0;

    const char *b = entry["b"].as<const char *>();
    const char *t = entry["t"].as<const char *>();
    if (!b || !t)
        return 0;

    size_t bl = strlen(b);
    if (bl >= out_brand_len)
        bl = out_brand_len - 1;
    memcpy(out_brand, b, bl);
    out_brand[bl] = '\0';

    size_t tl = strlen(t);
    if (tl >= out_type_len)
        tl = out_type_len - 1;
    memcpy(out_type, t, tl);
    out_type[tl] = '\0';

    return 1;
}

} /* extern "C" */
#endif /* __cplusplus */
#endif /* _XTOUCH_FILAMENTS_REV_IMPL_GUARD */
