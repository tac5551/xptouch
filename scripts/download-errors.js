const fs = require('fs');
const https = require('https');

const url = 'https://e.bambulab.com/query.php?lang=en';
const hmsDataFileC = 'src/xtouch/bbl/bbl-errors.c';
const hmsDataFileH = 'src/xtouch/bbl/bbl-errors.h';
const hmsDataFileOptimizedC = 'src/xtouch/bbl/bbl-errors-optimized.c';
const hmsDataFileOptimizedH = 'src/xtouch/bbl/bbl-errors-optimized.h';

// 削除済み - 使用されていないため


const getFlatRecordsKeysC = (root, varName) => {
    // フィルターを外して全てのエラーを取得
    const filteredRoot = root;

    const ecodeArray = filteredRoot.map(e => String(e.ecode));
    const arrayContent = JSON.stringify(ecodeArray, null, 2).slice(1, -1);

    return `const char *${varName}[] PROGMEM = {${arrayContent}};`;
}


const getFlatRecordsValuesC = (root, varName = 0) => { // varNameのデフォルト値は元のコードに従います
    // フィルターを外して全てのエラーを取得
    const filteredRoot = root;

    const introArray = filteredRoot.map(e => String(e.intro));

    const arrayContent = JSON.stringify(introArray, null, 2).slice(1, -1);

    return `const char *${varName}[] PROGMEM = {${arrayContent}};`;
}

const getKeyValueLengthC = (root, varName) => {
    // フィルターを外して全てのエラーを取得
    const filteredRoot = root;

    const introArray = filteredRoot.map(e => String(e.intro));

    const arrayContent = JSON.stringify(introArray, null, 2).slice(1, -1);
    return `int ${varName} = ${filteredRoot.length};`;
}

const getFlatRecordsKeysH = (root, varName) => {
    return `extern const char *${varName}[] PROGMEM;`;
}

const getFlatRecordsValuesH = (root, varName = 0) => {
    return `extern const char *${varName}[] PROGMEM;`;
}

const getKeyValueLengthH = (root, varName) => {
    return `extern int ${varName};`;
}

// 正規化処理関数
const normalizeErrorMessages = (errors) => {
    const patterns = new Map();
    const normalizedErrors = [];
    
    errors.forEach(error => {
        let message = error.intro;
        let pattern = message;
        let placeholders = [];
        
        // AMS/AMS-HT デバイス名の正規化
        const amsDeviceMatch = message.match(/(AMS(?:-HT)?)\s+([A-H])/g);
        if (amsDeviceMatch) {
            amsDeviceMatch.forEach(match => {
                const deviceType = match.includes('-HT') ? 'AMS-HT' : 'AMS';
                const deviceLetter = match.match(/[A-H]/)[0];
                const placeholder = `AMS_DEVICE_${deviceLetter}`;
                pattern = pattern.replace(match, placeholder);
                placeholders.push({ type: 'AMS_DEVICE', value: deviceLetter, original: match });
            });
        }
        
        // Slot番号の正規化
        const slotMatch = message.match(/Slot\s+(\d+)/g);
        if (slotMatch) {
            slotMatch.forEach(match => {
                const slotNum = match.match(/\d+/)[0];
                const placeholder = `SLOT_${slotNum}`;
                pattern = pattern.replace(match, placeholder);
                placeholders.push({ type: 'SLOT', value: slotNum, original: match });
            });
        }
        
        // Heater番号の正規化
        const heaterMatch = message.match(/heater\s+(\d+)/gi);
        if (heaterMatch) {
            heaterMatch.forEach(match => {
                const heaterNum = match.match(/\d+/)[0];
                const placeholder = `HEATER_${heaterNum}`;
                pattern = pattern.replace(new RegExp(match, 'gi'), placeholder);
                placeholders.push({ type: 'HEATER', value: heaterNum, original: match });
            });
        }
        
        // Valve番号の正規化
        const valveMatch = message.match(/valve\s+(\d+)/gi);
        if (valveMatch) {
            valveMatch.forEach(match => {
                const valveNum = match.match(/\d+/)[0];
                const placeholder = `VALVE_${valveNum}`;
                pattern = pattern.replace(new RegExp(match, 'gi'), placeholder);
                placeholders.push({ type: 'VALVE', value: valveNum, original: match });
            });
        }
        
        // 共通フレーズの正規化
        const commonPhrases = [
            { pattern: /motor is stalled, cannot rotate the spool/gi, placeholder: 'MOTOR_STALLED' },
            { pattern: /assist motor has slipped/gi, placeholder: 'ASSIST_MOTOR_SLIPPED' },
            { pattern: /filament odometer has no signal/gi, placeholder: 'ODOMETER_NO_SIGNAL' },
            { pattern: /filament status is abnormal/gi, placeholder: 'FILAMENT_STATUS_ABNORMAL' },
            { pattern: /feeder unit motor has no signal/gi, placeholder: 'FEEDER_MOTOR_NO_SIGNAL' },
            { pattern: /power adapter voltage is too high/gi, placeholder: 'POWER_ADAPTER_HIGH' },
            { pattern: /power adapter voltage is too low/gi, placeholder: 'POWER_ADAPTER_LOW' },
            { pattern: /Please update it on the "Firmware" page/gi, placeholder: 'PAGE_NAME' },
            { pattern: /Please pull out the filament, cut off the worn part, and then try again/gi, placeholder: 'FILAMENT_CUT_INSTRUCTIONS' }
        ];
        
        commonPhrases.forEach(phrase => {
            if (phrase.pattern.test(pattern)) {
                pattern = pattern.replace(phrase.pattern, phrase.placeholder);
                placeholders.push({ type: 'COMMON_PHRASE', value: phrase.placeholder, original: phrase.pattern.source });
            }
        });
        
        // 引用符のエスケープ
        pattern = pattern.replace(/"/g, '\\"');
        
        // パターンの正規化（大文字小文字、スペースの統一）
        const normalizedPattern = pattern.toLowerCase().replace(/\s+/g, ' ').trim();
        
        if (!patterns.has(normalizedPattern)) {
            patterns.set(normalizedPattern, {
                pattern: pattern,
                placeholders: placeholders,
                errors: []
            });
        }
        
        patterns.get(normalizedPattern).errors.push({
            ecode: error.ecode,
            original: message,
            placeholders: placeholders
        });
    });
    
    return Array.from(patterns.values());
};

const generateOptimizedC = (normalizedErrors) => {
    const errorPatterns = normalizedErrors.map((group, index) => {
        const placeholderDefs = group.placeholders.map(p => `"${p.type}_${p.value}"`).join(', ');
        
        return `    {"${group.errors[0].ecode}", "${group.pattern}", {${placeholderDefs}}, ${group.placeholders.length}}`;
    }).join(',\n');
    
    return `#include <pgmspace.h>

// Error pattern structure for normalized error handling
typedef struct {
    const char* ecode;
    const char* pattern;
    const char* placeholders[8];  // Max 8 placeholders for more complex patterns
    int placeholder_count;
} error_pattern_t;

// Normalized HMS error patterns
int hms_error_patterns_length = ${normalizedErrors.length};

const error_pattern_t hms_error_patterns[] PROGMEM = {
${errorPatterns}
};`;
};


https.get(url, (response) => {
    if (response.statusCode === 200) {
        let data = '';

        response.on('data', (chunk) => {
            data += chunk;
        });

        response.on('end', () => {

            // data = data.replace(/ /g);
            const dataObject = JSON.parse(data);
            const device_hms = dataObject.data.device_hms.en;
            const device_error = dataObject.data.device_error.en;

            // 正規化処理
            const normalizedHms = normalizeErrorMessages(device_hms);
            const normalizedDevice = normalizeErrorMessages(device_error);

            // 最適化版のみを生成（元のメッセージは削除）
            const optimizedOutputC = generateOptimizedC(normalizedHms);
            fs.writeFileSync(hmsDataFileOptimizedC, optimizedOutputC);

            // 最適化版のヘッダーファイル生成
            const optimizedOutputH = `#ifndef _XLCD_BBL_ERRORS_OPTIMIZED
#define _XLCD_BBL_ERRORS_OPTIMIZED

#ifdef __cplusplus
extern "C"
{
#endif

#include <pgmspace.h>

typedef struct {
    const char* ecode;
    const char* pattern;
    const char* placeholders[8];
    int placeholder_count;
} error_pattern_t;

extern int hms_error_patterns_length;
extern const error_pattern_t hms_error_patterns[] PROGMEM;

#ifdef __cplusplus
}
#endif

#endif`;
            fs.writeFileSync(hmsDataFileOptimizedH, optimizedOutputH);

            console.log('[xtouch error downloader] Done!!');
            console.log(`[xtouch error downloader] Optimized errors: ${normalizedHms.length} (${Math.round((1 - normalizedHms.length/device_hms.length) * 100)}% reduction from ${device_hms.length} original errors)`);
        });
    } else {
        console.error('[xtouch error downloader] Error al descargar el archivo. Código de estado:', response.statusCode);
    }
}).on('error', (error) => {
    console.error('[xtouch error downloader] Error al realizar la solicitud:', error);
});

