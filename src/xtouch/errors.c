#include "xtouch/bbl/bbl-errors.h"
#include <pgmspace.h>
#include <Arduino.h>

bool xtouch_errors_isKeyPresent(const char *key, const char *keys[], int length)
{
    for (int i = 0; i < length; i++)
    {
        // Retrieve a string from PROGMEM and compare it with the key
        if (strcmp_P(key, (PGM_P)pgm_read_ptr(&keys[i])) == 0)
        {
            return true; // Key found
        }
    }
    return false; // Key not found
}

bool xtouch_errors_deviceErrorHasDone(const char *key)
{
    return xtouch_errors_isKeyPresent(key, message_containing_done, message_containing_done_total);
}

bool xtouch_errors_deviceErrorHasRetry(const char *key)
{
    return xtouch_errors_isKeyPresent(key, message_containing_retry, message_containing_retry_total);
}

const char *xtouch_errors_getValueByKey(const char *key, const char *keys[], const char *values[], int numEntries)
{
    for (int i = 0; i < numEntries; i++)
    {
        if (strcmp_P(key, (PGM_P)pgm_read_ptr(&keys[i])) == 0)
        {
            return (const char *)pgm_read_ptr(&values[i]);
        }
    }
    return (const char *)0; // Key not found
}

const char *xtouch_errors_getHMSError(const char *key)
{
    return xtouch_errors_getValueByKey(key, hms_error_keys, hms_error_values, hms_error_length);
}

const char *xtouch_errors_getDeviceError(const char *key)
{
    return xtouch_errors_getValueByKey(key, device_error_keys, device_error_values, device_error_length);
}

const char *xtouch_errors_getFormattedErrorCode(const char *key)
{
    static char formattedCode[20]; // "0000-0000-0000-0000" + null terminator
    
    // エラーコードがNULLまたは空の場合はデフォルト値を返す
    if (key == NULL || strlen(key) == 0) {
        strcpy(formattedCode, "0000-0000-0000-0000");
        return formattedCode;
    }
    
    // エラーコードの長さをチェック（16文字の16進数コードを想定）
    if (strlen(key) != 16) {
        strcpy(formattedCode, "0000-0000-0000-0000");
        return formattedCode;
    }
    
    // エラーコードが有効な16進数文字列かチェック
    for (int i = 0; i < 16; i++) {
        char c = key[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            strcpy(formattedCode, "0000-0000-0000-0000");
            return formattedCode;
        }
    }
    
    // エラーコードが見つからない場合は、エラーコード自体をフォーマットして返す
    if (xtouch_errors_getHMSError(key) == NULL && xtouch_errors_getDeviceError(key) == NULL) {
        // 16文字のエラーコードを "0000-0000-0000-0000" 形式にフォーマット
        snprintf(formattedCode, sizeof(formattedCode), "%c%c%c%c-%c%c%c%c-%c%c%c%c-%c%c%c%c",
                 key[0], key[1], key[2], key[3],
                 key[4], key[5], key[6], key[7],
                 key[8], key[9], key[10], key[11],
                 key[12], key[13], key[14], key[15]);
        return formattedCode;
    }
    
    // 有効なエラーコードの場合はそのまま返す
    return key;
}

const char *xtouch_errors_formatErrorCode(const char *key)
{
    static char formattedCode[20]; // "0000-0000-0000" + null terminator
    
    // エラーコードがNULLまたは空の場合はデフォルト値を返す
    if (key == NULL || strlen(key) == 0) {
        strcpy(formattedCode, "0000-0000-0000");
        return formattedCode;
    }
    
    // エラーコードの長さをチェック（16文字の16進数コードを想定）
    if (strlen(key) != 16) {
        strcpy(formattedCode, "0000-0000-0000");
        return formattedCode;
    }
    
    // エラーコードが有効な16進数文字列かチェック
    for (int i = 0; i < 16; i++) {
        char c = key[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            strcpy(formattedCode, "0000-0000-0000");
            return formattedCode;
        }
    }
    
    // エラーコードが見つからない場合は、エラーコード自体をフォーマットして返す
    if (xtouch_errors_getHMSError(key) == NULL && xtouch_errors_getDeviceError(key) == NULL) {
        // 16文字のエラーコードを "0000-0000-0000-0000" 形式にフォーマット
        snprintf(formattedCode, sizeof(formattedCode), "%c%c%c%c-%c%c%c%c-%c%c%c%c-%c%c%c%c",
                 key[0], key[1], key[2], key[3],
                 key[4], key[5], key[6], key[7],
                 key[8], key[9], key[10], key[11],
                 key[12], key[13], key[14], key[15]);
        return formattedCode;
    }
    
    // 16文字のエラーコードを "0000-0000-0000" 形式にフォーマット
    // 例: "0500010000020002" -> "0500-0100-0002-0002"
    snprintf(formattedCode, sizeof(formattedCode), "%c%c%c%c-%c%c%c%c-%c%c%c%c-%c%c%c%c",
             key[0], key[1], key[2], key[3],
             key[4], key[5], key[6], key[7],
             key[8], key[9], key[10], key[11],
             key[12], key[13], key[14], key[15]);
    
    return formattedCode;
}

const char *xtouch_errors_getErrorMessage(const char *key)
{
    static char errorMessage[20]; // "0000-0000-0000-0000" + null terminator
    
    // エラーコードがNULLまたは空の場合はデフォルト値を返す
    if (key == NULL || strlen(key) == 0) {
        strcpy(errorMessage, "0000-0000-0000-0000");
        return errorMessage;
    }
    
    // エラーコードの長さをチェック（16文字の16進数コードを想定）
    if (strlen(key) != 16) {
        strcpy(errorMessage, "0000-0000-0000-0000");
        return errorMessage;
    }
    
    // エラーコードが有効な16進数文字列かチェック
    for (int i = 0; i < 16; i++) {
        char c = key[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            strcpy(errorMessage, "0000-0000-0000-0000");
            return errorMessage;
        }
    }
    
    // まずHMSエラーをチェック
    const char *hmsError = xtouch_errors_getHMSError(key);
    if (hmsError != NULL) {
        return hmsError;
    }
    
    // 次にDeviceエラーをチェック
    const char *deviceError = xtouch_errors_getDeviceError(key);
    if (deviceError != NULL) {
        return deviceError;
    }
    
    // エラーコードが見つからない場合は、エラーコード自体をフォーマットして返す
    snprintf(errorMessage, sizeof(errorMessage), "%c%c%c%c-%c%c%c%c-%c%c%c%c-%c%c%c%c",
             key[0], key[1], key[2], key[3],
             key[4], key[5], key[6], key[7],
             key[8], key[9], key[10], key[11],
             key[12], key[13], key[14], key[15]);
    return errorMessage;
}
