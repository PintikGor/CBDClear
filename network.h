#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <Ethernet.h>
#include <WiFi.h>

// Структура сетевых настроек
struct NetworkSettings {
  // Ethernet
  byte mac[6];
  bool dhcpEnabled;
  IPAddress ip;
  IPAddress gateway;
  IPAddress subnet;
  IPAddress dns;
  
  // WiFi
  bool wifiEnabled;
  char ssid[32];
  char password[64];
  bool apMode;
  char apSsid[32];
  char apPassword[64];
  
  // NTP
  bool ntpEnabled;
  char ntpServer[64];
  long gmtOffset;
  int daylightOffset;
};

// Инициализация сети
bool initNetwork();

// Обработка сетевых событий
void networkLoop();

// Получение текущих сетевых настроек
NetworkSettings getNetworkSettings();

// Установка сетевых настроек
bool setNetworkSettings(NetworkSettings settings);

// Сканирование WiFi сетей
String scanWiFiNetworks();

// Проверка соединения
bool isNetworkConnected();

// Получение текущего IP адреса
IPAddress getCurrentIP();

// Синхронизация времени с NTP
bool syncTimeWithNTP();

#endif // NETWORK_H
