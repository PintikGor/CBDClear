#include "network.h"
#include "config.h"
#include "storage.h"
#include "logger.h"
#include <SPI.h>

// Глобальные переменные
EthernetServer ethernetServer(80);
NetworkSettings networkSettings;
bool ethernetConnected = false;
bool wifiConnected = false;

// Инициализация сети
bool initNetwork() {
  // Загрузка сетевых настроек
  networkSettings = loadNetworkSettings();
  
  // Инициализация Ethernet
  Ethernet.init(PIN_ETH_CS);
  delay(1500); // Задержка для инициализации W5500
  
  if (networkSettings.dhcpEnabled) {
    // Использование DHCP
    if (Ethernet.begin(networkSettings.mac) == 0) {
      log(LOG_ERROR, "Failed to configure Ethernet using DHCP");
      ethernetConnected = false;
    } else {
      log(LOG_INFO, "Ethernet connected via DHCP, IP: " + Ethernet.localIP().toString());
      ethernetConnected = true;
    }
  } else {
    // Использование статического IP
    Ethernet.begin(networkSettings.mac, networkSettings.ip, networkSettings.dns, networkSettings.gateway, networkSettings.subnet);
    log(LOG_INFO, "Ethernet connected with static IP: " + Ethernet.localIP().toString());
    ethernetConnected = true;
  }
  
  // Инициализация WiFi
  if (networkSettings.wifiEnabled) {
    if (networkSettings.apMode) {
      // Режим точки доступа
      WiFi.softAP(networkSettings.apSsid, networkSettings.apPassword);
      log(LOG_INFO, "WiFi AP started, SSID: " + String(networkSettings.apSsid));
      wifiConnected = true;
    } else {
      // Режим клиента
      WiFi.begin(networkSettings.ssid, networkSettings.password);
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        log(LOG_INFO, "WiFi connected, IP: " + WiFi.localIP().toString());
        wifiConnected = true;
      } else {
        log(LOG_ERROR, "Failed to connect to WiFi");
        wifiConnected = false;
      }
    }
  }
  
  // Синхронизация времени с NTP, если включено
  if (networkSettings.ntpEnabled && (ethernetConnected || wifiConnected)) {
    syncTimeWithNTP();
  }
  
  return ethernetConnected || wifiConnected;
}

// Обработка сетевых событий
void networkLoop() {
  // Проверка состояния Ethernet
  if (Ethernet.linkStatus() == LinkON && !ethernetConnected) {
    log(LOG_INFO, "Ethernet link restored");
    ethernetConnected = true;
  } else if (Ethernet.linkStatus() == LinkOFF && ethernetConnected) {
    log(LOG_WARNING, "Ethernet link lost");
    ethernetConnected = false;
  }
  
  // Проверка состояния WiFi
  if (networkSettings.wifiEnabled && !networkSettings.apMode) {
    if (WiFi.status() == WL_CONNECTED && !wifiConnected) {
      log(LOG_INFO, "WiFi connection restored");
      wifiConnected = true;
    } else if (WiFi.status() != WL_CONNECTED && wifiConnected) {
      log(LOG_WARNING, "WiFi connection lost");
      wifiConnected = false;
    }
  }
}

// Получение текущих сетевых настроек
NetworkSettings getNetworkSettings() {
  return networkSettings;
}

// Установка сетевых настроек
bool setNetworkSettings(NetworkSettings settings) {
  networkSettings = settings;
  saveNetworkSettings(settings);
  
  // Перезагрузка сетевых интерфейсов с новыми настройками
  return initNetwork();
}

// Сканирование WiFi сетей
String scanWiFiNetworks() {
  String result = "[";
  int n = WiFi.scanNetworks();
  
  for (int i = 0; i < n; ++i) {
    if (i > 0) result += ",";
    result += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + ",\"encryption\":" + String(WiFi.encryptionType(i)) + "}";
  }
  
  result += "]";
  return result;
}

// Проверка соединения
bool isNetworkConnected() {
  return ethernetConnected || wifiConnected;
}

// Получение текущего IP адреса
IPAddress getCurrentIP() {
  if (ethernetConnected) {
    return Ethernet.localIP();
  } else if (wifiConnected) {
    if (networkSettings.apMode) {
      return WiFi.softAPIP();
    } else {
      return WiFi.localIP();
    }
  }
  
  return IPAddress(0, 0, 0, 0);
}

// Синхронизация времени с NTP
bool syncTimeWithNTP() {
  // Здесь должен быть код для синхронизации с NTP сервером
  // Для ESP32 можно использовать встроенные функции
  configTime(networkSettings.gmtOffset, networkSettings.daylightOffset, networkSettings.ntpServer);
  
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    // Установка времени в RTC
    time_t now;
    time(&now);
    struct tm * timeinfo;
    timeinfo = localtime(&now);
    
    DateTime dt(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, 
                timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    rtc.adjust(dt);
    
    log(LOG_INFO, "Time synchronized with NTP");
    return true;
  }
  
  log(LOG_ERROR, "Failed to get time from NTP");
  return false;
}
