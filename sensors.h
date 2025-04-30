#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "config.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// Структура настроек датчиков
struct SensorsSettings {
  bool tempCorrectionEnabled;
  float tempCorrectionValue;
  bool invertEmptySensor;
  bool invertFullSensor;
};

// Глобальные переменные
extern float temperature;
extern WaterLevelStatus waterLevel;
extern bool load1Status;
extern bool load2Status;
bool updateWaterLevel();
// Инициализация датчиков
bool initSensors();

// Обновление состояния датчиков
void updateSensors();

// Обработка кнопок
void handleButtons();

// Обновление светодиодов
void updateLEDs();

// Получение статуса уровня воды
WaterLevelStatus getWaterLevelStatus();

// Получение состояния нагрузки
bool getLoadStatus(int loadNumber);

// Установка состояния нагрузки
bool setLoadStatus(int loadNumber, bool state);

// Загрузка настроек датчиков
SensorsSettings loadSensorsSettings();

// Сохранение настроек датчиков
bool saveSensorsSettings(SensorsSettings settings);

#endif // SENSORS_H
