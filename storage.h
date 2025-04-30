#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include "network.h"
#include "scheduler.h"
#include "sensors.h"

// Структура настроек безопасности
struct SecuritySettings {
  bool authEnabled;
  char username[32];
  char password[64];
};

void listDir(fs::FS &fs, const char *dirname, uint8_t levels);

// Инициализация SD карты
bool initSDCard();

// Загрузка настроек
bool loadSettings();

// Загрузка сетевых настроек
NetworkSettings loadNetworkSettings();

// Сохранение сетевых настроек
bool saveNetworkSettings(NetworkSettings settings);

// Загрузка настроек безопасности
SecuritySettings loadSecuritySettings();

// Сохранение настроек безопасности
bool saveSecuritySettings(SecuritySettings settings);

// Загрузка расписаний
bool loadSchedules(Schedule* schedules, int count);

// Сохранение расписаний
bool saveSchedules(Schedule* schedules, int count);

// Сброс настроек к заводским
bool resetSettings();

#endif // STORAGE_H
