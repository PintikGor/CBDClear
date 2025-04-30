#include "storage.h"
#include "config.h"
#include "logger.h"
#include <SD.h>
#include <ArduinoJson.h>

// Пути к файлам настроек
#define NETWORK_SETTINGS_FILE "/settings/network.json"
#define SECURITY_SETTINGS_FILE "/settings/security.json"
#define SCHEDULES_FILE "/settings/schedules.json"
#define SENSORS_SETTINGS_FILE "/settings/sensors.json"

// Инициализация SD карты
bool initSDCard() {
  if (!SD.begin(PIN_CD_CS)) {
    Serial.println("Card Mount Failed");
    return false;
  }
  
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return false;
  }
  
  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  
  // Проверка и создание директорий
  if (!SD.exists("/settings")) {
    SD.mkdir("/settings");
  }
  
  if (!SD.exists("/logs")) {
    SD.mkdir("/logs");
  }
  
  log(LOG_INFO, "SD card initialized");
  return true;
}

// Загрузка настроек
bool loadSettings() {
  loadNetworkSettings();
  loadSecuritySettings();
  loadSensorsSettings();
  
  Schedule schedules[MAX_SCHEDULES];
  loadSchedules(schedules, MAX_SCHEDULES);
  
  return true;
}

// Загрузка сетевых настроек
NetworkSettings loadNetworkSettings() {
  NetworkSettings settings;
  
  // Значения по умолчанию
  byte defaultMac[] = { 0xCB, 0xA7, 0x91, 0x08, 0x64, 0xA6 };
  memcpy(settings.mac, defaultMac, 6);
  settings.dhcpEnabled = true;
  settings.ip = IPAddress(192, 168, 0, 34);
  settings.gateway = IPAddress(192, 168, 0, 1);
  settings.subnet = IPAddress(255, 255, 255, 0);
  settings.dns = IPAddress(8, 8, 8, 8);
  settings.wifiEnabled = false;
  strcpy(settings.ssid, "");
  strcpy(settings.password, "");
  settings.apMode = true;
  strcpy(settings.apSsid, DEFAULT_AP_SSID);
  strcpy(settings.apPassword, DEFAULT_AP_PASSWORD);
  settings.ntpEnabled = true;
  strcpy(settings.ntpServer, "pool.ntp.org");
  settings.gmtOffset = 3600 * 3; // GMT+3
  settings.daylightOffset = 0;
  
  // Проверка наличия файла настроек
  if (!SD.exists(NETWORK_SETTINGS_FILE)) {
    // Создаем файл с настройками по умолчанию
    saveNetworkSettings(settings);
    return settings;
  }
  
  // Открываем файл для чтения
  File file = SD.open(NETWORK_SETTINGS_FILE, FILE_READ);
  if (!file) {
    log(LOG_ERROR, "Failed to open network settings file");
    return settings;
  }
  
  // Чтение JSON из файла
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    log(LOG_ERROR, "Failed to parse network settings: " + String(error.c_str()));
    return settings;
  }
  
  // Заполнение структуры из JSON
  if (doc.containsKey("mac")) {
    JsonArray macArray = doc["mac"];
    for (int i = 0; i < 6; i++) {
      settings.mac[i] = macArray[i];
    }
  }
  
  settings.dhcpEnabled = doc["dhcpEnabled"] | true;
  
  if (doc.containsKey("ip")) {
    String ipStr = doc["ip"];
    settings.ip.fromString(ipStr);
  }
  
  if (doc.containsKey("gateway")) {
    String gwStr = doc["gateway"];
    settings.gateway.fromString(gwStr);
  }
  
  if (doc.containsKey("subnet")) {
    String snStr = doc["subnet"];
    settings.subnet.fromString(snStr);
  }
  
  if (doc.containsKey("dns")) {
    String dnsStr = doc["dns"];
    settings.dns.fromString(dnsStr);
  }
  
  settings.wifiEnabled = doc["wifiEnabled"] | false;
  
  if (doc.containsKey("ssid")) {
    strlcpy(settings.ssid, doc["ssid"], sizeof(settings.ssid));
  }
  
  if (doc.containsKey("password")) {
    strlcpy(settings.password, doc["password"], sizeof(settings.password));
  }
  
  settings.apMode = doc["apMode"] | true;
  
  if (doc.containsKey("apSsid")) {
    strlcpy(settings.apSsid, doc["apSsid"], sizeof(settings.apSsid));
  }
  
  if (doc.containsKey("apPassword")) {
    strlcpy(settings.apPassword, doc["apPassword"], sizeof(settings.apPassword));
  }
  
  settings.ntpEnabled = doc["ntpEnabled"] | true;
  
  if (doc.containsKey("ntpServer")) {
    strlcpy(settings.ntpServer, doc["ntpServer"], sizeof(settings.ntpServer));
  }
  
  settings.gmtOffset = doc["gmtOffset"] | 3600 * 3;
  settings.daylightOffset = doc["daylightOffset"] | 0;
  
  log(LOG_INFO, "Network settings loaded");
  return settings;
}

// Сохранение сетевых настроек
bool saveNetworkSettings(NetworkSettings settings) {
  // Создаем JSON документ
  DynamicJsonDocument doc(1024);
  
  // Заполняем JSON данными
  JsonArray macArray = doc.createNestedArray("mac");
  for (int i = 0; i < 6; i++) {
    macArray.add(settings.mac[i]);
  }
  
  doc["dhcpEnabled"] = settings.dhcpEnabled;
  doc["ip"] = settings.ip.toString();
  doc["gateway"] = settings.gateway.toString();
  doc["subnet"] = settings.subnet.toString();
  doc["dns"] = settings.dns.toString();
  doc["wifiEnabled"] = settings.wifiEnabled;
  doc["ssid"] = settings.ssid;
  doc["password"] = settings.password;
  doc["apMode"] = settings.apMode;
  doc["apSsid"] = settings.apSsid;
  doc["apPassword"] = settings.apPassword;
  doc["ntpEnabled"] = settings.ntpEnabled;
  doc["ntpServer"] = settings.ntpServer;
  doc["gmtOffset"] = settings.gmtOffset;
  doc["daylightOffset"] = settings.daylightOffset;
  
  // Открываем файл для записи
  File file = SD.open(NETWORK_SETTINGS_FILE, FILE_WRITE);
  if (!file) {
    log(LOG_ERROR, "Failed to open network settings file for writing");
    return false;
  }
  
  // Записываем JSON в файл
  if (serializeJson(doc, file) == 0) {
    log(LOG_ERROR, "Failed to write network settings to file");
    file.close();
    return false;
  }
  
  file.close();
  log(LOG_INFO, "Network settings saved");
  return true;
}

// Загрузка настроек безопасности
SecuritySettings loadSecuritySettings() {
  SecuritySettings settings;
  
  // Значения по умолчанию
  settings.authEnabled = true;
  strcpy(settings.username, "admin");
  strcpy(settings.password, "admin");
  
  // Проверка наличия файла настроек
  if (!SD.exists(SECURITY_SETTINGS_FILE)) {
    // Создаем файл с настройками по умолчанию
    saveSecuritySettings(settings);
    return settings;
  }
  
  // Открываем файл для чтения
  File file = SD.open(SECURITY_SETTINGS_FILE, FILE_READ);
  if (!file) {
    log(LOG_ERROR, "Failed to open security settings file");
    return settings;
  }
  
  // Чтение JSON из файла
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    log(LOG_ERROR, "Failed to parse security settings: " + String(error.c_str()));
    return settings;
  }
  
  // Заполнение структуры из JSON
  settings.authEnabled = doc["authEnabled"] | true;
  
  if (doc.containsKey("username")) {
    strlcpy(settings.username, doc["username"], sizeof(settings.username));
  }
  
  if (doc.containsKey("password")) {
    strlcpy(settings.password, doc["password"], sizeof(settings.password));
  }
  
  log(LOG_INFO, "Security settings loaded");
  return settings;
}

// Сохранение настроек безопасности
bool saveSecuritySettings(SecuritySettings settings) {
  // Создаем JSON документ
  DynamicJsonDocument doc(512);
  
  // Заполняем JSON данными
  doc["authEnabled"] = settings.authEnabled;
  doc["username"] = settings.username;
  doc["password"] = settings.password;
  
  // Открываем файл для записи
  File file = SD.open(SECURITY_SETTINGS_FILE, FILE_WRITE);
  if (!file) {
    log(LOG_ERROR, "Failed to open security settings file for writing");
    return false;
  }
  
  // Записываем JSON в файл
  if (serializeJson(doc, file) == 0) {
    log(LOG_ERROR, "Failed to write security settings to file");
    file.close();
    return false;
  }
  
  file.close();
  log(LOG_INFO, "Security settings saved");
  return true;
}

// Загрузка расписаний
bool loadSchedules(Schedule* schedules, int count) {
  // Инициализация расписаний по умолчанию
  for (int i = 0; i < count; i++) {
    schedules[i].enabled = false;
    schedules[i].loadNumber = 1;
    schedules[i].mode = SCHEDULE_MODE_DAILY;
    schedules[i].days = 0x7F; // Все дни недели
    schedules[i].day = 1;
    schedules[i].hour = 12;
    schedules[i].minute = 0;
    schedules[i].second = 0;
    schedules[i].useDuration = true;
    schedules[i].durationHour = 0;
    schedules[i].durationMinute = 5;
    schedules[i].durationSecond = 0;
    schedules[i].endHour = 12;
    schedules[i].endMinute = 5;
    schedules[i].endSecond = 0;
  }
  
  // Проверка наличия файла настроек
  if (!SD.exists(SCHEDULES_FILE)) {
    // Создаем файл с настройками по умолчанию
    saveSchedules(schedules, count);
    return true;
  }
  
  // Открываем файл для чтения
  File file = SD.open(SCHEDULES_FILE, FILE_READ);
  if (!file) {
    log(LOG_ERROR, "Failed to open schedules file");
    return false;
  }
  
  // Чтение JSON из файла
  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    log(LOG_ERROR, "Failed to parse schedules: " + String(error.c_str()));
    return false;
  }
  
  // Заполнение структуры из JSON
  JsonArray schedulesArray = doc["schedules"];
  int i = 0;
  
  for (JsonObject scheduleObj : schedulesArray) {
    if (i >= count) break;
    
    schedules[i].enabled = scheduleObj["enabled"] | false;
    schedules[i].loadNumber = scheduleObj["loadNumber"] | 1;
    schedules[i].mode = (ScheduleMode)(scheduleObj["mode"] | SCHEDULE_MODE_DAILY);
    schedules[i].days = scheduleObj["days"] | 0x7F;
    schedules[i].day = scheduleObj["day"] | 1;
    schedules[i].hour = scheduleObj["hour"] | 12;
    schedules[i].minute = scheduleObj["minute"] | 0;
    schedules[i].second = scheduleObj["second"] | 0;
    schedules[i].useDuration = scheduleObj["useDuration"] | true;
    schedules[i].durationHour = scheduleObj["durationHour"] | 0;
    schedules[i].durationMinute = scheduleObj["durationMinute"] | 5;
    schedules[i].durationSecond = scheduleObj["durationSecond"] | 0;
    schedules[i].endHour = scheduleObj["endHour"] | 12;
    schedules[i].endMinute = scheduleObj["endMinute"] | 5;
    schedules[i].endSecond = scheduleObj["endSecond"] | 0;
    
    i++;
  }
  
  log(LOG_INFO, "Schedules loaded: " + String(i));
  return true;
}

// Сохранение расписаний
bool saveSchedules(Schedule* schedules, int count) {
  // Создаем JSON документ
  DynamicJsonDocument doc(4096);
  JsonArray schedulesArray = doc.createNestedArray("schedules");
  
  // Заполняем JSON данными
  for (int i = 0; i < count; i++) {
    JsonObject scheduleObj = schedulesArray.createNestedObject();
    
    scheduleObj["enabled"] = schedules[i].enabled;
    scheduleObj["loadNumber"] = schedules[i].loadNumber;
    scheduleObj["mode"] = schedules[i].mode;
    scheduleObj["days"] = schedules[i].days;
    scheduleObj["day"] = schedules[i].day;
    scheduleObj["hour"] = schedules[i].hour;
    scheduleObj["minute"] = schedules[i].minute;
    scheduleObj["second"] = schedules[i].second;
    scheduleObj["useDuration"] = schedules[i].useDuration;
    scheduleObj["durationHour"] = schedules[i].durationHour;
    scheduleObj["durationMinute"] = schedules[i].durationMinute;
    scheduleObj["durationSecond"] = schedules[i].durationSecond;
    scheduleObj["endHour"] = schedules[i].endHour;
    scheduleObj["endMinute"] = schedules[i].endMinute;
    scheduleObj["endSecond"] = schedules[i].endSecond;
  }
  
  // Открываем файл для записи
  File file = SD.open(SCHEDULES_FILE, FILE_WRITE);
  if (!file) {
    log(LOG_ERROR, "Failed to open schedules file for writing");
    return false;
  }
  
  // Записываем JSON в файл
  if (serializeJson(doc, file) == 0) {
    log(LOG_ERROR, "Failed to write schedules to file");
    file.close();
    return false;
  }
  
  file.close();
  log(LOG_INFO, "Schedules saved");
  return true;
}

// Загрузка настроек датчиков
SensorsSettings loadSensorsSettings() {
  SensorsSettings settings;
  
  // Значения по умолчанию
  settings.tempCorrectionEnabled = false;
  settings.tempCorrectionValue = 0.0;
  settings.invertEmptySensor = false;
  settings.invertFullSensor = false;
  
  // Проверка наличия файла настроек
  if (!SD.exists(SENSORS_SETTINGS_FILE)) {
    // Создаем файл с настройками по умолчанию
    saveSensorsSettings(settings);
    return settings;
  }
  
  // Открываем файл для чтения
  File file = SD.open(SENSORS_SETTINGS_FILE, FILE_READ);
  if (!file) {
    log(LOG_ERROR, "Failed to open sensors settings file");
    return settings;
  }
  
  // Чтение JSON из файла
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    log(LOG_ERROR, "Failed to parse sensors settings: " + String(error.c_str()));
    return settings;
  }
  
  // Заполнение структуры из JSON
  settings.tempCorrectionEnabled = doc["tempCorrectionEnabled"] | false;
  settings.tempCorrectionValue = doc["tempCorrectionValue"] | 0.0;
  settings.invertEmptySensor = doc["invertEmptySensor"] | false;
  settings.invertFullSensor = doc["invertFullSensor"] | false;
  
  log(LOG_INFO, "Sensors settings loaded");
  return settings;
}

// Сохранение настроек датчиков
bool saveSensorsSettings(SensorsSettings settings) {
  // Создаем JSON документ
  DynamicJsonDocument doc(512);
  
  // Заполняем JSON данными
  doc["tempCorrectionEnabled"] = settings.tempCorrectionEnabled;
  doc["tempCorrectionValue"] = settings.tempCorrectionValue;
  doc["invertEmptySensor"] = settings.invertEmptySensor;
  doc["invertFullSensor"] = settings.invertFullSensor;
  
  // Открываем файл для записи
  File file = SD.open(SENSORS_SETTINGS_FILE, FILE_WRITE);
  if (!file) {
    log(LOG_ERROR, "Failed to open sensors settings file for writing");
    return false;
  }
  
  // Записываем JSON в файл
  if (serializeJson(doc, file) == 0) {
    log(LOG_ERROR, "Failed to write sensors settings to file");
    file.close();
    return false;
  }
  
  file.close();
  log(LOG_INFO, "Sensors settings saved");
  return true;
}

// Сброс настроек к заводским
bool resetSettings() {
  // Удаление файлов настроек
  SD.remove(NETWORK_SETTINGS_FILE);
  SD.remove(SECURITY_SETTINGS_FILE);
  SD.remove(SCHEDULES_FILE);
  SD.remove(SENSORS_SETTINGS_FILE);
  
  // Загрузка настроек по умолчанию
  loadSettings();
  
  log(LOG_INFO, "Settings reset to defaults");
  return true;
}
