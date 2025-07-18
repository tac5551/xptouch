const fs = require('fs');
const https = require('https');

const url = 'https://e.bambulab.com/query.php?lang=en';
const hmsDataFileC = 'src/xtouch/bbl/bbl-errors.c';
const hmsDataFileH = 'src/xtouch/bbl/bbl-errors.h';
const hmsDataFileOptimizedC = 'src/xtouch/bbl/bbl-errors-optimized.c';
const hmsDataFileOptimizedH = 'src/xtouch/bbl/bbl-errors-optimized.h';
const hmsDataFileOldC = 'src/xtouch/bbl/bbl-errors_old.c';
const hmsDataFileOldH = 'src/xtouch/bbl/bbl-errors_old.h';

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
    const nonOptimizedErrors = [];
    const identicalMessageGroups = new Map(); // 同一メッセージのグループ化
    const seenErrorCodes = new Set(); // 重複エラーコードを追跡
    
    // まず、エラーコードレベルでの重複を除去
    const uniqueErrors = [];
    errors.forEach(error => {
        if (!seenErrorCodes.has(error.ecode)) {
            seenErrorCodes.add(error.ecode);
            uniqueErrors.push(error);
        }
    });
    
    console.log(`[xtouch error downloader] Original errors: ${errors.length}, Unique error codes: ${uniqueErrors.length}`);
    
    // 次に、完全に同一のメッセージをグループ化
    uniqueErrors.forEach(error => {
        let message = error.intro;
        
        // 空のメッセージをスキップ
        if (!message || message.trim() === '' || message === '""') {
            return;
        }
        
        // 同一メッセージのグループ化
        const normalizedMessage = message.toLowerCase().trim();
        if (!identicalMessageGroups.has(normalizedMessage)) {
            identicalMessageGroups.set(normalizedMessage, {
                message: message,
                errors: []
            });
        }
        identicalMessageGroups.get(normalizedMessage).errors.push(error);
    });
    
    // 同一メッセージグループごとに処理
    identicalMessageGroups.forEach((group, normalizedMessage) => {
        // 同一メッセージが複数ある場合は、最初のエラーコードを代表として使用
        const representativeError = group.errors[0];
        let message = representativeError.intro;
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
            { pattern: /Please pull out the filament, cut off the worn part, and then try again/gi, placeholder: 'FILAMENT_CUT_INSTRUCTIONS' },
            // --- Filament transport error patterns ---
            { pattern: /Unable to feed filament.*extruder/gi, placeholder: 'FEED_FILAMENT_FAILED' },
            { pattern: /Failed to pull out the filament.*extruder/gi, placeholder: 'PULL_OUT_FILAMENT_FAILED' },
            { pattern: /AMS failed to pull back filament/gi, placeholder: 'AMS_PULL_BACK_FAILED' },
            { pattern: /The AMS failed to send out filament/gi, placeholder: 'AMS_SEND_OUT_FAILED' },
            // --- RFID error patterns ---
            { pattern: /RFID cannot be read because of a hardware/gi, placeholder: 'RFID_READ_HARDWARE_ERROR' },
            { pattern: /RFID cannot be read because of an encryption chip failure/gi, placeholder: 'RFID_READ_ENCRYPTION_ERROR' },
            { pattern: /Failed to read the filament information.*RFID tag may be damaged/gi, placeholder: 'RFID_TAG_DAMAGED' },
            { pattern: /Failed to read the filament information.*RFID tag verification failed/gi, placeholder: 'RFID_TAG_VERIFICATION_FAILED' },
            { pattern: /The RFID-tag on .* cannot be identified/gi, placeholder: 'RFID_TAG_CANNOT_IDENTIFY' },
            { pattern: /Failed to read the filament information.*RFID tag cannot rotate/gi, placeholder: 'RFID_TAG_CANNOT_ROTATE' },
            { pattern: /Failed to read the filament information.*AMS main board may be malfunctioning/gi, placeholder: 'RFID_AMS_BOARD_ERROR' },
            { pattern: /Failed to read the filament information.*positioned at the edge of the RFID detection device/gi, placeholder: 'RFID_TAG_POSITION_ERROR' },
            { pattern: /Failed to read the filament information.*non-official RFID tag was detected/gi, placeholder: 'RFID_TAG_NON_OFFICIAL' },
            { pattern: /The RFID-tag on AMS_DEVICE_A Slot\d+ is damaged/gi, placeholder: 'RFID_TAG_DAMAGED' },
            { pattern: /RFID coil is broken or the RF hardware circuit has an error/gi, placeholder: 'RFID_COIL_OR_CIRCUIT_ERROR' },
            // --- Heatbed force sensor error patterns ---
            { pattern: /Heatbed force sensor (\d+) is too sensitive\. It may be stuck between the strain arm and heatbed support, or the adjusting screw may be too tight\./gi, placeholder: 'HEATBED_FORCE_SENSOR_TOO_SENSITIVE' },
            { pattern: /The signal of heatbed force sensor (\d+) is weak\. The force sensor may be broken or have poor electric connection\./gi, placeholder: 'HEATBED_FORCE_SENSOR_WEAK' },
            { pattern: /The signal of heatbed force sensor (\d+) is too weak\. The electronic connection to the sensor may be broken\./gi, placeholder: 'HEATBED_FORCE_SENSOR_TOO_WEAK' },
            // --- Motor error patterns ---
            { pattern: /The resistance of Motor-([A-Z]) is abnormal; the motor may have failed\./gi, placeholder: 'MOTOR_RESISTANCE_ABNORMAL' },
            { pattern: /Motor-([A-Z]) has a short-circuit\. It may have failed\./gi, placeholder: 'MOTOR_SHORT_CIRCUIT' },
            { pattern: /Motor-([A-Z]) has an open-circuit\. The connection may be loose, or the motor may have failed\./gi, placeholder: 'MOTOR_OPEN_CIRCUIT' },
            { pattern: /The current sensor of Motor-([A-Z]) is abnormal\. This may be caused by a failure of the hardware sampling circuit\./gi, placeholder: 'MOTOR_CURRENT_SENSOR_ABNORMAL' },
            // --- Temperature sensor error patterns ---
            { pattern: /The (nozzle|heatbed|chamber) temperature is abnormal; the (heater|sensor) may have an open circuit\./gi, placeholder: 'TEMP_SENSOR_OPEN_CIRCUIT' },
            { pattern: /The (nozzle|heatbed|chamber) temperature is abnormal; the (heater|sensor) may have a short circuit\./gi, placeholder: 'TEMP_SENSOR_SHORT_CIRCUIT' },
            { pattern: /The (nozzle|heatbed|chamber) temperature is abnormal; the (heater|sensor) is over temperature\./gi, placeholder: 'TEMP_SENSOR_OVER_TEMP' },
            // --- Laser error patterns ---
            { pattern: /The (horizontal|vertical) laser is not lit\. Please check if it's covered or hardware connection has a problem\./gi, placeholder: 'LASER_NOT_LIT' },
            { pattern: /The (horizontal|vertical) laser is not bright enough at homing position\. Please clean or replace the heatbed if this message appears repeatedly\./gi, placeholder: 'LASER_NOT_BRIGHT_ENOUGH' },
            { pattern: /The (horizontal|vertical) laser line is too wide\. Please check if the heatbed is dirty\./gi, placeholder: 'LASER_LINE_TOO_WIDE' },
            // --- Force sensor error patterns ---
            { pattern: /Force sensor (\d+) detected unexpected continuous force\. The heatbed may be stuck, or the analog front end may be broken\./gi, placeholder: 'FORCE_SENSOR_CONTINUOUS_FORCE' },
            { pattern: /An external disturbance was detected on force sensor (\d+)\. The heatbed plate may have touched something outside the heatbed\./gi, placeholder: 'FORCE_SENSOR_EXTERNAL_DISTURBANCE' },
            // --- Chamber temperature error patterns ---
            { pattern: /Chamber temperature is abnormal\. The chamber heater's temperature sensor (?:at the air outlet )?may have an open circuit\./gi, placeholder: 'CHAMBER_TEMP_SENSOR_OPEN' },
            { pattern: /Chamber temperature is abnormal\. The chamber heater's temperature sensor (?:at the air outlet )?may have a short circuit\./gi, placeholder: 'CHAMBER_TEMP_SENSOR_SHORT' },
            { pattern: /Chamber temperature is abnormal\. The temperature sensor at the power supply may have an open circuit\./gi, placeholder: 'CHAMBER_TEMP_SENSOR_POWER_OPEN' },
            { pattern: /Chamber temperature is abnormal\. The temperature sensor at the power supply may have a short circuit\./gi, placeholder: 'CHAMBER_TEMP_SENSOR_POWER_SHORT' },
            // --- MicroSD card error patterns ---
            { pattern: /MicroSD Card (?:performance degradation has been detected|read\/write exception|capacity is insufficient|is in Read-Only mode|is write-protected|sector data is damaged)\./gi, placeholder: 'MICROSD_CARD_ERROR' },
            { pattern: /Unformatted MicroSD Card: please format it\./gi, placeholder: 'MICROSD_CARD_UNFORMATTED' },
            { pattern: /Not enough space on MicroSD Card; please clear some space\./gi, placeholder: 'MICROSD_CARD_NO_SPACE' },
            // --- Network connection error patterns ---
            { pattern: /Failed to connect to the (?:internet|Bambu Cloud|router)\. Please check your network connection\./gi, placeholder: 'NETWORK_CONNECTION_FAILED' },
            { pattern: /Router connection failed due to incorrect password\. Please check the password and try again\./gi, placeholder: 'ROUTER_PASSWORD_ERROR' },
            { pattern: /Failed to obtain IP address, which may be caused by wireless interference resulting in data transmission failure or the DHCP address pool of the router being full\./gi, placeholder: 'IP_ADDRESS_FAILED' },
            // --- Homing error patterns ---
            { pattern: /([A-Z]) axis homing abnormal: (?:please check if the toolhead is stuck or the (?:Y carriage has too much resistance|carbon rod resistance is too high)|the timing belt may be loose)\./gi, placeholder: 'AXIS_HOMING_ABNORMAL' },
            { pattern: /([A-Z]) axis homing failed\./gi, placeholder: 'AXIS_HOMING_FAILED' },
            // --- Resonance frequency error patterns ---
            { pattern: /The resonance frequency of the ([A-Z]) axis is low\. The timing belt may be loose\./gi, placeholder: 'RESONANCE_FREQUENCY_LOW' },
            { pattern: /The resonance frequency of the ([A-Z])-axis differs (?:greatly|significantly) from the last calibration\./gi, placeholder: 'RESONANCE_FREQUENCY_DIFFERS' },
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
        
        // 最適化の条件をチェック
        const canOptimize = placeholders.length > 0 || 
                           normalizedPattern.includes('ams_device_') ||
                           normalizedPattern.includes('slot_') ||
                           normalizedPattern.includes('heater_') ||
                           normalizedPattern.includes('valve_') ||
                           normalizedPattern.includes('motor_stalled') ||
                           normalizedPattern.includes('assist_motor_slipped') ||
                           normalizedPattern.includes('odometer_no_signal') ||
                           normalizedPattern.includes('filament_status_abnormal') ||
                           normalizedPattern.includes('feeder_motor_no_signal') ||
                           normalizedPattern.includes('power_adapter_') ||
                           normalizedPattern.includes('page_name') ||
                           normalizedPattern.includes('filament_cut_instructions') ||
                           normalizedPattern.includes('feed_filament_failed') ||
                           normalizedPattern.includes('pull_out_filament_failed') ||
                           normalizedPattern.includes('ams_pull_back_failed') ||
                           normalizedPattern.includes('ams_send_out_failed') ||
                           normalizedPattern.includes('rfid_') ||
                           normalizedPattern.includes('heatbed_force_sensor_') ||
                           normalizedPattern.includes('motor_') ||
                           normalizedPattern.includes('temp_sensor_') ||
                           normalizedPattern.includes('laser_') ||
                           normalizedPattern.includes('force_sensor_') ||
                           normalizedPattern.includes('chamber_temp_sensor_') ||
                           normalizedPattern.includes('microsd_card_') ||
                           normalizedPattern.includes('network_') ||
                           normalizedPattern.includes('axis_homing_') ||
                           normalizedPattern.includes('resonance_frequency_') ||
                           group.errors.length > 1; // 同一メッセージが複数ある場合も最適化対象
        
        if (canOptimize && !patterns.has(normalizedPattern)) {
            patterns.set(normalizedPattern, {
                pattern: pattern,
                placeholders: placeholders,
                errors: group.errors // 同一メッセージの全エラーを含める
            });
        } else if (canOptimize && patterns.has(normalizedPattern)) {
            // 既存のパターンに同一メッセージのエラーを追加（重複チェック付き）
            const existingPattern = patterns.get(normalizedPattern);
            const existingCodes = new Set(existingPattern.errors.map(e => e.ecode));
            const newErrors = group.errors.filter(error => !existingCodes.has(error.ecode));
            existingPattern.errors.push(...newErrors);
        } else {
            // 最適化できないエラーは別途保存（重複チェック付き）
            const existingCodes = new Set(nonOptimizedErrors.map(e => e.ecode));
            const newErrors = group.errors.filter(error => !existingCodes.has(error.ecode));
            if (newErrors.length > 0) {
                nonOptimizedErrors.push(newErrors[0]); // 最初の1つだけを保存
            }
        }
    });
    
    return {
        normalized: Array.from(patterns.values()),
        nonOptimized: nonOptimizedErrors
    };
};

// リトライメッセージとDoneメッセージを抽出する関数
const extractRetryAndDoneMessages = (hmsErrors, deviceErrors) => {
    const retryMessages = [];
    const doneMessages = [];
    
    // リトライメッセージのパターン
    const retryPatterns = [
        /retry/i,
        /try again/i,
        /resume/i,
        /continue/i,
        /push.*forward/i,
        /pull.*out.*try/i
    ];
    
    // Doneメッセージのパターン
    const donePatterns = [
        /select.*done/i,
        /select.*continue/i,
        /done.*extruded/i
    ];
    
    const allErrors = [...hmsErrors, ...deviceErrors];
    
    allErrors.forEach(error => {
        const message = error.intro.toLowerCase();
        
        // リトライメッセージのチェック
        if (retryPatterns.some(pattern => pattern.test(message))) {
            retryMessages.push(error.ecode);
        }
        
        // Doneメッセージのチェック
        if (donePatterns.some(pattern => pattern.test(message))) {
            doneMessages.push(error.ecode);
        }
    });
    
    // 重複を除去
    return {
        retry: [...new Set(retryMessages)],
        done: [...new Set(doneMessages)]
    };
};

const generateOptimizedC = (normalizedHms, normalizedDevice, retryDoneMessages) => {
    const hmsErrorPatterns = normalizedHms.map((group, index) => {
        const placeholderDefs = group.placeholders.map(p => `"${p.type}_${p.value}"`).join(', ');
        
        return `    {"${group.errors[0].ecode}", "${group.pattern}", {${placeholderDefs}}, ${group.placeholders.length}}`;
    }).join(',\n');
    
    const deviceErrorPatterns = normalizedDevice.map((group, index) => {
        const placeholderDefs = group.placeholders.map(p => `"${p.type}_${p.value}"`).join(', ');
        
        return `    {"${group.errors[0].ecode}", "${group.pattern}", {${placeholderDefs}}, ${group.placeholders.length}}`;
    }).join(',\n');
    
    const retryMessagesArray = retryDoneMessages.retry.map(code => `  "${code}"`).join(',\n');
    const doneMessagesArray = retryDoneMessages.done.map(code => `  "${code}"`).join(',\n');
    
    return `#include <pgmspace.h>

// Error pattern structure for normalized error handling
typedef struct {
    const char* ecode;
    const char* pattern;
    const char* placeholders[8];  // Max 8 placeholders for more complex patterns
    int placeholder_count;
} error_pattern_t;

// Normalized HMS error patterns
int hms_error_patterns_length = ${normalizedHms.length};

const error_pattern_t hms_error_patterns[] PROGMEM = {
${hmsErrorPatterns}
};

// Normalized Device error patterns
int device_error_patterns_length = ${normalizedDevice.length};

const error_pattern_t device_error_patterns[] PROGMEM = {
${deviceErrorPatterns}
};

// Retry and Done message arrays (for backward compatibility)
int message_containing_retry_total = ${retryDoneMessages.retry.length};

const char *message_containing_retry[] PROGMEM = {
${retryMessagesArray}
};

int message_containing_done_total = ${retryDoneMessages.done.length};

const char *message_containing_done[] PROGMEM = {
${doneMessagesArray}
};`;
};

const generateLegacyC = (nonOptimizedHms, nonOptimizedDevice) => {
    const hmsKeys = nonOptimizedHms.map(e => `  "${e.ecode}"`).join(',\n');
    const hmsValues = nonOptimizedHms.map(e => `  "${e.intro.replace(/"/g, '\\"')}"`).join(',\n');
    const deviceKeys = nonOptimizedDevice.map(e => `  "${e.ecode}"`).join(',\n');
    const deviceValues = nonOptimizedDevice.map(e => `  "${e.intro.replace(/"/g, '\\"')}"`).join(',\n');
    
    return `#include <pgmspace.h>

// Non-optimized HMS errors (legacy format)
int hms_error_length = ${nonOptimizedHms.length};

const char *hms_error_keys[] PROGMEM = {
${hmsKeys}
};

const char *hms_error_values[] PROGMEM = {
${hmsValues}
};

// Non-optimized Device errors (legacy format)
int device_error_length = ${nonOptimizedDevice.length};

const char *device_error_keys[] PROGMEM = {
${deviceKeys}
};

const char *device_error_values[] PROGMEM = {
${deviceValues}
};
`;
};

const generateAllC = (hmsErrors, deviceErrors, retryDoneMessages) => {
    const hmsKeys = hmsErrors.map(e => `  "${e.ecode}"`).join(',\n');
    const hmsValues = hmsErrors.map(e => `  "${e.intro.replace(/"/g, '\\"')}"`).join(',\n');
    const deviceKeys = deviceErrors.map(e => `  "${e.ecode}"`).join(',\n');
    const deviceValues = deviceErrors.map(e => `  "${e.intro.replace(/"/g, '\\"')}"`).join(',\n');
    
    return `#include <pgmspace.h>

// All HMS errors (no optimization)
int hms_error_length = ${hmsErrors.length};

const char *hms_error_keys[] PROGMEM = {
${hmsKeys}
};

const char *hms_error_values[] PROGMEM = {
${hmsValues}
};

// All Device errors (no optimization)
int device_error_length = ${deviceErrors.length};

const char *device_error_keys[] PROGMEM = {
${deviceKeys}
};

const char *device_error_values[] PROGMEM = {
${deviceValues}
};

// Retry and Done message arrays
int message_containing_retry_total = ${retryDoneMessages.retry.length};

const char *message_containing_retry[] PROGMEM = {
${retryDoneMessages.retry.map(code => `  "${code}"`).join(',\n')}
};

int message_containing_done_total = ${retryDoneMessages.done.length};

const char *message_containing_done[] PROGMEM = {
${retryDoneMessages.done.map(code => `  "${code}"`).join(',\n')}
};`;
};

https.get(url, (response) => {
    if (response.statusCode === 200) {
        let data = '';

        response.on('data', (chunk) => {
            data += chunk;
        });

        response.on('end', () => {

            // data = data.replace(/ /g);
            const dataObject = JSON.parse(data);
            const device_hms = dataObject.data.device_hms.en;
            const device_error = dataObject.data.device_error.en;

            // 重複除去（エラーコードレベル）
            const uniqueHmsErrors = [];
            const seenHmsCodes = new Set();
            device_hms.forEach(error => {
                if (!seenHmsCodes.has(error.ecode)) {
                    seenHmsCodes.add(error.ecode);
                    uniqueHmsErrors.push(error);
                }
            });
            
            const uniqueDeviceErrors = [];
            const seenDeviceCodes = new Set();
            device_error.forEach(error => {
                if (!seenDeviceCodes.has(error.ecode)) {
                    seenDeviceCodes.add(error.ecode);
                    uniqueDeviceErrors.push(error);
                }
            });
            
            console.log(`[xtouch error downloader] After deduplication: HMS ${uniqueHmsErrors.length}, Device ${uniqueDeviceErrors.length}`);
            
            // 正規化処理
            const normalizedHms = normalizeErrorMessages(uniqueHmsErrors);
            const normalizedDevice = normalizeErrorMessages(uniqueDeviceErrors);

            // リトライとDoneメッセージの抽出
            const retryDoneMessages = extractRetryAndDoneMessages(uniqueHmsErrors, uniqueDeviceErrors);

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

// Optimized error patterns
extern int hms_error_patterns_length;
extern const error_pattern_t hms_error_patterns[] PROGMEM;

extern int device_error_patterns_length;
extern const error_pattern_t device_error_patterns[] PROGMEM;

// Retry and Done messages
extern int message_containing_retry_total;
extern const char *message_containing_retry[] PROGMEM;

extern int message_containing_done_total;
extern const char *message_containing_done[] PROGMEM;

#ifdef __cplusplus
}
#endif

#endif`;
            fs.writeFileSync(hmsDataFileOptimizedH, optimizedOutputH);

            // レガシー版のヘッダーファイル生成
            const legacyOutputH = `#ifndef _XLCD_BBL_ERRORS
#define _XLCD_BBL_ERRORS

#ifdef __cplusplus
extern "C"
{
#endif

#include <pgmspace.h>

// Legacy HMS error arrays
extern int hms_error_length;
extern const char *hms_error_keys[] PROGMEM;
extern const char *hms_error_values[] PROGMEM;

// Legacy Device error arrays
extern int device_error_length;
extern const char *device_error_keys[] PROGMEM;
extern const char *device_error_values[] PROGMEM;

// Retry and Done messages
extern int message_containing_retry_total;
extern const char *message_containing_retry[] PROGMEM;

extern int message_containing_done_total;
extern const char *message_containing_done[] PROGMEM;

#ifdef __cplusplus
}
#endif

#endif`;
            fs.writeFileSync(hmsDataFileH, legacyOutputH);

            // 従来版のヘッダーファイル生成
            const oldOutputH = `#ifndef _XLCD_BBL_ERRORS_OLD
#define _XLCD_BBL_ERRORS_OLD

#ifdef __cplusplus
extern "C"
{
#endif

#include <pgmspace.h>

// Old HMS error arrays (for comparison)
extern int hms_error_length;
extern const char *hms_error_keys[] PROGMEM;
extern const char *hms_error_values[] PROGMEM;

// Old Device error arrays (for comparison)
extern int device_error_length;
extern const char *device_error_keys[] PROGMEM;
extern const char *device_error_values[] PROGMEM;

// Retry and Done messages
extern int message_containing_retry_total;
extern const char *message_containing_retry[] PROGMEM;

extern int message_containing_done_total;
extern const char *message_containing_done[] PROGMEM;

#ifdef __cplusplus
}
#endif

#endif`;
            fs.writeFileSync(hmsDataFileOldH, oldOutputH);

            console.log('[xtouch error downloader] Done!!');
            console.log(`[xtouch error downloader] Optimized HMS errors: ${normalizedHms.normalized.length} (${Math.round((1 - normalizedHms.normalized.length/device_hms.length) * 100)}% reduction from ${device_hms.length} original HMS errors)`);
            console.log(`[xtouch error downloader] Non-optimized HMS errors: ${normalizedHms.nonOptimized.length} (legacy format)`);
            console.log(`[xtouch error downloader] Optimized Device errors: ${normalizedDevice.normalized.length} (${Math.round((1 - normalizedDevice.normalized.length/device_error.length) * 100)}% reduction from ${device_error.length} original Device errors)`);
            console.log(`[xtouch error downloader] Non-optimized Device errors: ${normalizedDevice.nonOptimized.length} (legacy format)`);
            console.log(`[xtouch error downloader] Retry messages: ${retryDoneMessages.retry.length}, Done messages: ${retryDoneMessages.done.length}`);
            console.log(`[xtouch error downloader] Total optimization: ${Math.round((1 - (normalizedHms.normalized.length + normalizedDevice.normalized.length)/(device_hms.length + device_error.length)) * 100)}% reduction`);
            
            // Oldファイルとの比較チェック
            console.log('\n[xtouch error downloader] Coverage check against old file:');
            console.log(`[xtouch error downloader] Old HMS errors: 1822, Current total: ${normalizedHms.normalized.length + normalizedHms.nonOptimized.length} (${Math.round((normalizedHms.normalized.length + normalizedHms.nonOptimized.length)/1822*100)}% coverage)`);
            console.log(`[xtouch error downloader] Old Device errors: 178, Current total: ${normalizedDevice.normalized.length + normalizedDevice.nonOptimized.length} (${Math.round((normalizedDevice.normalized.length + normalizedDevice.nonOptimized.length)/178*100)}% coverage)`);
            console.log(`[xtouch error downloader] Old Retry messages: 19, Current: ${retryDoneMessages.retry.length} (${Math.round(retryDoneMessages.retry.length/19*100)}% coverage)`);
            console.log(`[xtouch error downloader] Old Done messages: 2, Current: ${retryDoneMessages.done.length} (${Math.round(retryDoneMessages.done.length/2*100)}% coverage)`);
            
            // 重複チェックと最終的な重複除去
            const allHmsCodes = new Set();
            const finalHmsNormalized = [];
            const finalHmsNonOptimized = [];
            
            // 最適化されたエラーの重複除去
            normalizedHms.normalized.forEach(group => {
                const uniqueErrors = [];
                group.errors.forEach(error => {
                    if (!allHmsCodes.has(error.ecode)) {
                        allHmsCodes.add(error.ecode);
                        uniqueErrors.push(error);
                    }
                });
                if (uniqueErrors.length > 0) {
                    finalHmsNormalized.push({
                        ...group,
                        errors: uniqueErrors
                    });
                }
            });
            
            // 非最適化エラーの重複除去（最適化済みエラーコードを除外）
            normalizedHms.nonOptimized.forEach(error => {
                if (!allHmsCodes.has(error.ecode)) {
                    allHmsCodes.add(error.ecode);
                    finalHmsNonOptimized.push(error);
                }
            });
            
            const allDeviceCodes = new Set();
            const finalDeviceNormalized = [];
            const finalDeviceNonOptimized = [];
            
            // 最適化されたDeviceエラーの重複除去
            normalizedDevice.normalized.forEach(group => {
                const uniqueErrors = [];
                group.errors.forEach(error => {
                    if (!allDeviceCodes.has(error.ecode)) {
                        allDeviceCodes.add(error.ecode);
                        uniqueErrors.push(error);
                    }
                });
                if (uniqueErrors.length > 0) {
                    finalDeviceNormalized.push({
                        ...group,
                        errors: uniqueErrors
                    });
                }
            });
            
            // 非最適化Deviceエラーの重複除去（最適化済みエラーコードを除外）
            normalizedDevice.nonOptimized.forEach(error => {
                if (!allDeviceCodes.has(error.ecode)) {
                    allDeviceCodes.add(error.ecode);
                    finalDeviceNonOptimized.push(error);
                }
            });
            
            console.log(`[xtouch error downloader] Final unique HMS error codes: ${allHmsCodes.size}`);
            console.log(`[xtouch error downloader] Final unique Device error codes: ${allDeviceCodes.size}`);
            
            if (allHmsCodes.size !== device_hms.length) {
                console.log(`[xtouch error downloader] INFO: HMS error count after deduplication: ${allHmsCodes.size} (${device_hms.length - allHmsCodes.size} duplicates removed)`);
            }
            if (allDeviceCodes.size !== device_error.length) {
                console.log(`[xtouch error downloader] INFO: Device error count after deduplication: ${allDeviceCodes.size} (${device_error.length - allDeviceCodes.size} duplicates removed)`);
            }
            
            // 重複除去後の結果を表示
            console.log(`[xtouch error downloader] Final optimized HMS errors: ${finalHmsNormalized.length}`);
            console.log(`[xtouch error downloader] Final non-optimized HMS errors: ${finalHmsNonOptimized.length}`);
            console.log(`[xtouch error downloader] Final optimized Device errors: ${finalDeviceNormalized.length}`);
            console.log(`[xtouch error downloader] Final non-optimized Device errors: ${finalDeviceNonOptimized.length}`);
            console.log(`[xtouch error downloader] Old errors (for comparison): HMS ${uniqueHmsErrors.length}, Device ${uniqueDeviceErrors.length}`);
            
            // 重複除去後のファイルを生成
            const optimizedOutputC = generateOptimizedC(finalHmsNormalized, finalDeviceNormalized, retryDoneMessages);
            fs.writeFileSync(hmsDataFileOptimizedC, optimizedOutputC);

            // レガシー版を生成（最適化できなかった項目）
            const legacyOutputC = generateLegacyC(finalHmsNonOptimized, finalDeviceNonOptimized);
            fs.writeFileSync(hmsDataFileC, legacyOutputC);

            // 従来版を生成（比較用）
            const oldOutputC = generateAllC(uniqueHmsErrors, uniqueDeviceErrors, retryDoneMessages);
            fs.writeFileSync(hmsDataFileOldC, oldOutputC);
        });
    } else {
        console.error('[xtouch error downloader] Error al descargar el archivo. Código de estado:', response.statusCode);
    }
}).on('error', (error) => {
    console.error('[xtouch error downloader] Error al realizar la solicitud:', error);
});

