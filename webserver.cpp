#include "webserver.h"
#include "config.h"
#include "network.h"
#include "sensors.h"
#include "scheduler.h"
#include "storage.h"
#include "logger.h"
#include <SD.h>
#include <ArduinoJson.h>
#include <Update.h>

// Глобальные переменные
extern AsyncWebServer server;
bool authEnabled = true;
String username = "admin";
String password = "admin";

// Проверка авторизации
bool checkAuth(AsyncWebServerRequest *request) {
  if (!authEnabled) return true;
  
  if (!request->authenticate(username.c_str(), password.c_str())) {
    request->requestAuthentication();
    return false;
  }
  
  return true;
}

// Инициализация веб-сервера
bool initWebServer() {
  // Загрузка настроек безопасности
  SecuritySettings securitySettings = loadSecuritySettings();
  authEnabled = securitySettings.authEnabled;
  username = securitySettings.username;
  password = securitySettings.password;
  
  // Обработчик для статических файлов с SD карты
  server.serveStatic("/", SD, "/").setDefaultFile("index.html");
  
  // API эндпоинты
  
  // Получение общей информации о состоянии устройства
  server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    DynamicJsonDocument doc(1024);
    doc["version"] = FIRMWARE_VERSION;
    doc["uptime"] = millis() / 1000;
    doc["temperature"] = temperature;
    doc["waterLevel"] = waterLevel;
    doc["load1"] = load1Status;
    doc["load2"] = load2Status;
    doc["nextSchedule"] = getNextScheduleTime();
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // Включение/выключение нагрузок
  server.on("/api/toggle", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    if (!request->hasParam("load", true)) {
      request->send(400, "application/json", "{\"error\":\"Missing load parameter\"}");
      return;
    }
    
    int load = request->getParam("load", true)->value().toInt();
    bool state = false;
    
    if (request->hasParam("state", true)) {
      state = request->getParam("state", true)->value() == "1";
    } else {
      // Если состояние не указано, переключаем текущее
      state = !getLoadStatus(load);
    }
    
    bool success = setLoadStatus(load, state);
    
    if (success) {
      request->send(200, "application/json", "{\"success\":true,\"load\":" + String(load) + ",\"state\":" + String(state) + "}");
    } else {
      request->send(400, "application/json", "{\"error\":\"Failed to set load state\"}");
    }
  });
  
  // Получение/сохранение настроек расписания
  server.on("/api/schedule", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    String schedules = getSchedulesJson();
    request->send(200, "application/json", schedules);
  });
  
  server.on("/api/schedule", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    if (!request->hasParam("data", true)) {
      request->send(400, "application/json", "{\"error\":\"Missing data parameter\"}");
      return;
    }
    
    String data = request->getParam("data", true)->value();
    bool success = updateSchedulesFromJson(data);
    
    if (success) {
      request->send(200, "application/json", "{\"success\":true}");
    } else {
      request->send(400, "application/json", "{\"error\":\"Failed to update schedules\"}");
    }
  });
  
  // Получение/сохранение сетевых настроек
  server.on("/api/network", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    NetworkSettings settings = getNetworkSettings();
    
    DynamicJsonDocument doc(1024);
    doc["dhcpEnabled"] = settings.dhcpEnabled;
    doc["ip"] = settings.ip.toString();
    doc["gateway"] = settings.gateway.toString();
    doc["subnet"] = settings.subnet.toString();
    doc["dns"] = settings.dns.toString();
    doc["wifiEnabled"] = settings.wifiEnabled;
    doc["ssid"] = settings.ssid;
    doc["apMode"] = settings.apMode;
    doc["apSsid"] = settings.apSsid;
    doc["ntpEnabled"] = settings.ntpEnabled;
    doc["ntpServer"] = settings.ntpServer;
    doc["gmtOffset"] = settings.gmtOffset;
    doc["daylightOffset"] = settings.daylightOffset;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  server.on("/api/network", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    if (!request->hasParam("data", true)) {
      request->send(400, "application/json", "{\"error\":\"Missing data parameter\"}");
      return;
    }
    
    String data = request->getParam("data", true)->value();
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, data);
    
    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    NetworkSettings settings = getNetworkSettings();
    
    if (doc.containsKey("dhcpEnabled")) settings.dhcpEnabled = doc["dhcpEnabled"].as<bool>();
    if (doc.containsKey("ip")) settings.ip.fromString(doc["ip"].as<String>());
    if (doc.containsKey("gateway")) settings.gateway.fromString(doc["gateway"].as<String>());
    if (doc.containsKey("subnet")) settings.subnet.fromString(doc["subnet"].as<String>());
    if (doc.containsKey("dns")) settings.dns.fromString(doc["dns"].as<String>());
    if (doc.containsKey("wifiEnabled")) settings.wifiEnabled = doc["wifiEnabled"].as<bool>();
    if (doc.containsKey("ssid")) strlcpy(settings.ssid, doc["ssid"].as<String>().c_str(), sizeof(settings.ssid));
    if (doc.containsKey("password")) strlcpy(settings.password, doc["password"].as<String>().c_str(), sizeof(settings.password));
    if (doc.containsKey("apMode")) settings.apMode = doc["apMode"].as<bool>();
    if (doc.containsKey("apSsid")) strlcpy(settings.apSsid, doc["apSsid"].as<String>().c_str(), sizeof(settings.apSsid));
    if (doc.containsKey("apPassword")) strlcpy(settings.apPassword, doc["apPassword"].as<String>().c_str(), sizeof(settings.apPassword));
    if (doc.containsKey("ntpEnabled")) settings.ntpEnabled = doc["ntpEnabled"].as<bool>();
    if (doc.containsKey("ntpServer")) strlcpy(settings.ntpServer, doc["ntpServer"].as<String>().c_str(), sizeof(settings.ntpServer));
    if (doc.containsKey("gmtOffset")) settings.gmtOffset = doc["gmtOffset"].as<long>();
    if (doc.containsKey("daylightOffset")) settings.daylightOffset = doc["daylightOffset"].as<int>();
    
    bool success = setNetworkSettings(settings);
    
    if (success) {
      request->send(200, "application/json", "{\"success\":true}");
    } else {
      request->send(400, "application/json", "{\"error\":\"Failed to update network settings\"}");
    }
  });
  
  // Сканирование WiFi сетей
  server.on("/api/wifi-scan", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    String networks = scanWiFiNetworks();
    request->send(200, "application/json", networks);
  });
  
  // Получение системной информации
  server.on("/api/system", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    DynamicJsonDocument doc(1024);
    doc["version"] = FIRMWARE_VERSION;
    doc["uptime"] = millis() / 1000;
    doc["sdCardSize"] = SD.cardSize() / (1024 * 1024);
    doc["sdCardUsed"] = (SD.cardSize() - SD.usedBytes()) / (1024 * 1024);
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["cpuFreq"] = ESP.getCpuFreqMHz();
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // Настройки времени
  server.on("/api/time", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    DateTime now = rtc.now();
    
    DynamicJsonDocument doc(256);
    doc["date"] = now.timestamp(DateTime::TIMESTAMP_DATE);
    doc["time"] = now.timestamp(DateTime::TIMESTAMP_TIME);
    doc["temperature"] = rtc.getTemperature();
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  server.on("/api/time", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    if (!request->hasParam("date", true) || !request->hasParam("time", true)) {
      request->send(400, "application/json", "{\"error\":\"Missing date or time parameter\"}");
      return;
    }
    
    String dateStr = request->getParam("date", true)->value();
    String timeStr = request->getParam("time", true)->value();
    
    bool success = setDateTime(dateStr, timeStr);
    
    if (success) {
      request->send(200, "application/json", "{\"success\":true}");
    } else {
      request->send(400, "application/json", "{\"error\":\"Failed to set date and time\"}");
    }
  });
  
  // Настройки безопасности
  server.on("/api/security", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    SecuritySettings settings = loadSecuritySettings();
    
    DynamicJsonDocument doc(256);
    doc["authEnabled"] = settings.authEnabled;
    doc["username"] = settings.username;
    // Не отправляем пароль в ответе
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  server.on("/api/security", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    if (!request->hasParam("data", true)) {
      request->send(400, "application/json", "{\"error\":\"Missing data parameter\"}");
      return;
    }
    
    String data = request->getParam("data", true)->value();
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, data);
    
    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    SecuritySettings settings = loadSecuritySettings();
    
    if (doc.containsKey("authEnabled")) settings.authEnabled = doc["authEnabled"].as<bool>();
    if (doc.containsKey("username")) strlcpy(settings.username, doc["username"].as<String>().c_str(), sizeof(settings.username));
    if (doc.containsKey("password")) strlcpy(settings.password, doc["password"].as<String>().c_str(), sizeof(settings.password));
    
    bool success = saveSecuritySettings(settings);
    
    if (success) {
      // Обновляем текущие настройки
      authEnabled = settings.authEnabled;
      username = settings.username;
      password = settings.password;
      
      request->send(200, "application/json", "{\"success\":true}");
    } else {
      request->send(400, "application/json", "{\"error\":\"Failed to update security settings\"}");
    }
  });
  
  // Настройки датчиков
  server.on("/api/sensors", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    SensorsSettings settings = loadSensorsSettings();
    
    DynamicJsonDocument doc(256);
    doc["tempCorrectionEnabled"] = settings.tempCorrectionEnabled;
    doc["tempCorrectionValue"] = settings.tempCorrectionValue;
    doc["invertEmptySensor"] = settings.invertEmptySensor;
    doc["invertFullSensor"] = settings.invertFullSensor;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  server.on("/api/sensors", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    if (!request->hasParam("data", true)) {
      request->send(400, "application/json", "{\"error\":\"Missing data parameter\"}");
      return;
    }
    
       String data = request->getParam("data", true)->value();
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, data);
    
    if (error) {
      request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    SensorsSettings settings = loadSensorsSettings();
    
    if (doc.containsKey("tempCorrectionEnabled")) settings.tempCorrectionEnabled = doc["tempCorrectionEnabled"].as<bool>();
    if (doc.containsKey("tempCorrectionValue")) settings.tempCorrectionValue = doc["tempCorrectionValue"].as<float>();
    if (doc.containsKey("invertEmptySensor")) settings.invertEmptySensor = doc["invertEmptySensor"].as<bool>();
    if (doc.containsKey("invertFullSensor")) settings.invertFullSensor = doc["invertFullSensor"].as<bool>();
    
    bool success = saveSensorsSettings(settings);
    
    if (success) {
      request->send(200, "application/json", "{\"success\":true}");
    } else {
      request->send(400, "application/json", "{\"error\":\"Failed to update sensors settings\"}");
    }
  });
  
  // Получение журнала событий
  server.on("/api/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    String logs = getLogsJson();
    request->send(200, "application/json", logs);
  });
  
  // Сброс настроек к заводским
  server.on("/api/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    bool success = resetSettings();
    
    if (success) {
      request->send(200, "application/json", "{\"success\":true}");
    } else {
      request->send(400, "application/json", "{\"error\":\"Failed to reset settings\"}");
    }
  });
  
  // Перезагрузка устройства
  server.on("/api/reboot", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    request->send(200, "application/json", "{\"success\":true}");
    delay(500);
    ESP.restart();
  });
  
  // Обработчик обновления прошивки
  server.on("/api/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) return;
    
    bool success = Update.hasError() ? false : true;
    
    if (success) {
      request->send(200, "application/json", "{\"success\":true}");
      delay(500);
      ESP.restart();
    } else {
      request->send(400, "application/json", "{\"error\":\"Update failed\"}");
    }
  }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!checkAuth(request)) return;
    
    if (index == 0) {
      log(LOG_INFO, "Firmware update started: " + filename);
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        log(LOG_ERROR, "Failed to start firmware update");
        Update.printError(Serial);
      }
    }
    
    if (Update.write(data, len) != len) {
      log(LOG_ERROR, "Failed to write firmware data");
      Update.printError(Serial);
    }
    
    if (final) {
      if (Update.end(true)) {
        log(LOG_INFO, "Firmware update successful");
      } else {
        log(LOG_ERROR, "Firmware update failed");
        Update.printError(Serial);
      }
    }
  });
  
  // Обработчик для неизвестных запросов
  server.onNotFound([](AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      if (request->url().startsWith("/api")) {
        request->send(404, "application/json", "{\"error\":\"Not found\"}");
      } else {
        request->redirect("/");
      }
    }
  });
  
  // Запуск сервера
  server.begin();
  log(LOG_INFO, "Web server started");
  
  return true;
}
