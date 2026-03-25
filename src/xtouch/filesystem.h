#ifndef _XLCD_SD
#define _XLCD_SD

#include "FS.h"
#include "SD.h"
#include <ArduinoJson.h>
#include <Arduino.h>

bool xtouch_filesystem_exist(fs::FS &fs, const char *path)
{
    return fs.exists(path);
}

File xtouch_filesystem_open(fs::FS &fs, const char *path)
{
    return fs.open(path);
}

bool xtouch_filesystem_deleteFile(fs::FS &fs, const char *path)
{
    return fs.remove(path);
}

bool xtouch_filesystem_mkdir(fs::FS &fs, const char *path)
{
    if (!xtouch_filesystem_exist(SD, path))
    {
        return fs.mkdir(path);
    }
    return true;
}

bool xtouch_filesystem_rmdir(fs::FS &fs, const char *path)
{
    return fs.rmdir(path);
}

void xtouch_filesystem_writeJson(fs::FS &fs, const char *filename, DynamicJsonDocument json, bool defaultsToArray = false, int size = 1024)
{
    ConsoleDebug.printf("[xPTouch][I][FILESYSTEM] Writing JSON file: %s\n", filename);
    File configFile = fs.open(filename, FILE_WRITE);
    if (!configFile)
    {
        ConsoleError.printf("[xPTouch][E][FILESYSTEM] Failed to write json file: %s\n", filename);
        return;
    }

    serializeJson(json, configFile);
    configFile.close();
}

DynamicJsonDocument xtouch_filesystem_readJson(fs::FS &fs, const char *filename, bool defaultsToArray = false, int size = 1024)
{
    ConsoleDebug.printf("[xPTouch][I][FILESYSTEM] Reading JSON file: %s\n", filename);
    DynamicJsonDocument doc(size); // Adjust the size as needed

    if (!fs.exists(filename))
    {
        ConsoleError.printf("[xPTouch][E][FILESYSTEM] Error Reading JSON File: %s\n", filename);
        if (defaultsToArray)
        {
            return doc.createNestedArray();
        }
        else
        {
            return doc;
        }
    }

    File configFile = fs.open(filename);
    DeserializationError error = deserializeJson(doc, configFile);

    if (error)
    {
        ConsoleError.printf("[xPTouch][E][FILESYSTEM] Error Parsing JSON File: %s\n", filename);
    }

    configFile.close();
    return doc;
}


#endif