#pragma once

#include <set>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "mbedtls/base64.h"
#include "types.h"
// #include "date.h"
#include "bbl-certs.h"

bool xtouch_cloud_pair_loop_exit = false;
#include <WiFiClientSecure.h>

class BambuCloud
{

private:
  String _region;
  String _auth_token;

public:
  String _email;
  bool loggedIn = false;

  bool getDeviceList(DynamicJsonDocument *&doc)
  {
    Serial.println(_region);
    Serial.println("Getting device list from Bambu Cloud");
    String url = _region == "China" ? "https://api.bambulab.cn/v1/iot-service/api/user/bind" : "https://api.bambulab.com/v1/iot-service/api/user/bind";
    String host = _region == "China" ? "api.bambulab.cn" : "api.bambulab.com";

    WiFiClientSecure client;
    client.setTimeout(500);
    client.setInsecure();
    String response = "";
    if (!client.connect(host.c_str(), 443))
      Serial1.println("Connection failed!");
    else
    {
      Serial1.println("Connected to server!");

      String request = "GET " + url + " HTTP/1.1\r\n";
      request += "Host: " + String(host) + "\r\n";
      request += "Authorization: Bearer " + _auth_token + "\r\n";
      request += "Connection: close\r\n";
      request += "\r\n";

      Serial.println("Sending request:");
      Serial.print(request);
      client.print(request);

      while (client.connected())
      {
        String line = client.readStringUntil('\n');
        if (line == "\r")
        {
          Serial1.println("headers received");
          break;
        }
      }

      while (client.available())
      {
        char c = client.read();
        // Serial1.write(c);
        response += c;
      }
      Serial1.println("\ndata received");

      client.stop();
    }
    Serial1.println("Connection closed");

    doc = new DynamicJsonDocument(2048);
    DeserializationError error = deserializeJson(*doc, response);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      return false;
    }
    return true;
  }

  JsonArray getPrivateFilaments()
  {
    return xtouch_filesystem_readJson(SD, xtouch_paths_private_filaments_flat, true).as<JsonArray>();
  }

  String getUsername() const
  {
    // cloud-username
    DynamicJsonDocument config = xtouch_filesystem_readJson(SD, xtouch_paths_provisioning, false, 2048);
    return config["cloud-username"].as<String>();
  }

  String getAuthToken() const
  {
    return _auth_token;
  }

  const char *getMqttCloudHost() const
  {
    return _region == "China" ? "cn.mqtt.bambulab.com" : "us.mqtt.bambulab.com";
  }

  String getRegion()
  {
    return _region;
  }

  void selectPrinter()
  {

    DynamicJsonDocument printers = xtouch_filesystem_readJson(SD, xtouch_paths_printers, false);
    DynamicJsonDocument *deviceListDocument = nullptr;
    JsonArray devices;
    if (getDeviceList(deviceListDocument))
    {
      if (deviceListDocument != nullptr)
      {
        devices = (*deviceListDocument)["devices"].as<JsonArray>();
      }
      else
      {
        return;
      }
    }
    else
    {
      return;
    }

    for (JsonVariant v : devices)
    {
      Serial.println("\n===========================================");
      serializeJsonPretty(v, Serial);
      if (!printers.containsKey(v["dev_id"].as<String>()))
      {
        printers[v["dev_id"].as<String>()] = v;
      }
    }

    serializeJsonPretty(printers, Serial);
    xtouch_filesystem_writeJson(SD, xtouch_paths_printers, printers);

    if (devices.size() == 0)
    {
      Serial.println("No devices found in Bambu Cloud");

      lv_label_set_text(introScreenCaption, LV_SYMBOL_CHARGE " No Cloud Registered Devices");
      lv_timer_handler();
      lv_task_handler();
      delay(3000);
      ESP.restart();
      return;
    }

    if (devices.size() == 1)
    {
      // auto select the only device
      setCurrentDevice(devices[0]["dev_id"].as<String>());
      setCurrentModel(devices[0]["dev_model_name"].as<String>());
      setPrinterName(devices[0]["name"].as<String>());

      JsonObject currentPrinterSettings = loadPrinters()[xTouchConfig.xTouchSerialNumber]["settings"];
      xTouchConfig.xTouchChamberSensorEnabled = currentPrinterSettings.containsKey("chamberTemp") ? currentPrinterSettings["chamberTemp"].as<bool>() : false;
      xTouchConfig.xTouchAuxFanEnabled = currentPrinterSettings.containsKey("auxFan") ? currentPrinterSettings["auxFan"].as<bool>() : false;
      xTouchConfig.xTouchChamberFanEnabled = currentPrinterSettings.containsKey("chamberFan") ? currentPrinterSettings["chamberFan"].as<bool>() : false;

      savePrinterPair(devices[0]["dev_id"].as<String>(), devices[0]["dev_model_name"].as<String>(), devices[0]["name"].as<String>());

      return;
    }

    loadScreen(5);

    String output = "";
    for (JsonVariant v : devices)
    {
      Serial.println("\n===========================================");
      serializeJsonPretty(v, Serial);
      output = output + LV_SYMBOL_CHARGE + " " + v["dev_id"].as<String>() + " (" + v["name"].as<String>() + ")\n";
    }

    if (!output.isEmpty())
    {
      output.remove(output.length() - 1);
      lv_roller_set_options(ui_printerPairScreenRoller, output.c_str(), LV_ROLLER_MODE_NORMAL);
      lv_obj_clear_flag(ui_printerPairScreenSubmitButton, LV_OBJ_FLAG_HIDDEN);

      while (!xtouch_cloud_pair_loop_exit)
      {
        lv_timer_handler();
        lv_task_handler();
      }
      uint16_t currentIndex = lv_roller_get_selected(ui_printerPairScreenRoller);
      setCurrentDevice(devices[currentIndex]["dev_id"].as<String>());
      setCurrentModel(devices[currentIndex]["dev_model_name"].as<String>());
      setPrinterName(devices[currentIndex]["name"].as<String>());

      JsonObject currentPrinterSettings = loadPrinters()[xTouchConfig.xTouchSerialNumber]["settings"];
      xTouchConfig.xTouchChamberSensorEnabled = currentPrinterSettings.containsKey("chamberTemp") ? currentPrinterSettings["chamberTemp"].as<bool>() : false;
      xTouchConfig.xTouchAuxFanEnabled = currentPrinterSettings.containsKey("auxFan") ? currentPrinterSettings["auxFan"].as<bool>() : false;
      xTouchConfig.xTouchChamberFanEnabled = currentPrinterSettings.containsKey("chamberFan") ? currentPrinterSettings["chamberFan"].as<bool>() : false;

      savePrinterPair(devices[currentIndex]["dev_id"].as<String>(), devices[currentIndex]["dev_model_name"].as<String>(), devices[currentIndex]["name"].as<String>());
    }
    delete deviceListDocument; // 使い終わったら必ず解放する
    deviceListDocument = nullptr;
    loadScreen(-1);
  }

  void savePrinterPair(String usn, String modelName, String printerName)
  {

    DynamicJsonDocument doc = xtouch_filesystem_readJson(SD, xtouch_paths_pair, false);

    doc["paired"] = usn.c_str();
    doc["model"] = modelName.c_str();
    doc["printerName"] = printerName.c_str();

    xtouch_filesystem_writeJson(SD, xtouch_paths_pair, doc);
  }

  bool isPaired()
  {
    if (!xtouch_filesystem_exist(SD, xtouch_paths_pair))
    {
      return false;
    }
    DynamicJsonDocument doc = xtouch_filesystem_readJson(SD, xtouch_paths_pair, false);
    return doc["paired"].as<String>() != "";
  }

  void loadPair()
  {
    DynamicJsonDocument doc = xtouch_filesystem_readJson(SD, xtouch_paths_pair, false);
    setCurrentDevice(doc["paired"].as<String>());
    setCurrentModel(doc["model"].as<String>());
    setPrinterName(doc["printerName"].as<String>());
  }

  void setCurrentDevice(String deviceId)
  {
    strcpy(xTouchConfig.xTouchSerialNumber, deviceId.c_str());
  }

  void setPrinterName(String printerName)
  {
    strcpy(xTouchConfig.xTouchPrinterName, printerName.c_str());
  }

  void setCurrentModel(String model)
  {
    strcpy(xTouchConfig.xTouchPrinterModel, model.c_str());
  }

  DynamicJsonDocument loadPrinters()
  {
    return xtouch_filesystem_readJson(SD, xtouch_paths_printers, false);
  }

  void clearDeviceList()
  {
    DynamicJsonDocument pairDoc(32);
    xtouch_filesystem_writeJson(SD, xtouch_paths_printers, pairDoc);
  }

  void clearPairList()
  {
    xtouch_filesystem_deleteFile(SD, xtouch_paths_pair);
  }
  void clearTokens()
  {
    DynamicJsonDocument config = xtouch_filesystem_readJson(SD, xtouch_paths_provisioning, false, 2048);
    config["cloud-authToken"] = "";
    xtouch_filesystem_writeJson(SD, xtouch_paths_provisioning, config);
  }

  void unpair()
  {
    ConsoleInfo.println("[xPTouch][SSDP] Unpairing device");
    DynamicJsonDocument pairFile = xtouch_filesystem_readJson(SD, xtouch_paths_pair, false);
    pairFile["paired"] = "";
    xtouch_filesystem_writeJson(SD, xtouch_paths_pair, pairFile);
    ESP.restart();
  }

  void saveAuthTokens()
  {
    DynamicJsonDocument config = xtouch_filesystem_readJson(SD, xtouch_paths_provisioning);
    config["cloud-authToken"] = _auth_token;
    xtouch_filesystem_writeJson(SD, xtouch_paths_provisioning, config, false, 2048);
    ESP.restart();
  }

  void loadAuthTokens()
  {
    ConsoleLog.println(ESP.getFreeHeap());
    DynamicJsonDocument config = xtouch_filesystem_readJson(SD, xtouch_paths_provisioning, false, 2048);
    _auth_token = config["cloud-authToken"].as<String>();
    DynamicJsonDocument wifiConfig = xtouch_filesystem_readJson(SD, xtouch_paths_provisioning);
    _region = wifiConfig["cloud-region"].as<const char *>();
    _email = wifiConfig["cloud-email"].as<String>();
    loggedIn = true;
  }

  bool hasAuthTokens()
  {
    if (!xtouch_filesystem_exist(SD, xtouch_paths_provisioning))
    {
      return false;
    }
    DynamicJsonDocument config = xtouch_filesystem_readJson(SD, xtouch_paths_provisioning, false, 2048);
    return config["cloud-authToken"].as<String>() != "";
  }
};
BambuCloud cloud;
