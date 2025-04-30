#include "sensors.h"
#include "logger.h"
#include "storage.h"

// Глобальные переменные
OneWire oneWire(PIN_TEMP_SENSOR);
DallasTemperature sensors(&oneWire);
SensorsSettings sensorsSettings;
unsigned long lastTempReadTime = 0;
unsigned long lastButtonCheckTime = 0;
unsigned long lastLedBlinkTime = 0;
bool ledBlinkState = false;

// Инициализация датчиков
bool initSensors() {
  // Загрузка настроек датчиков
  sensorsSettings = loadSensorsSettings();
  
  // Инициализация датчика температуры
  sensors.begin();
  
  // Первое чтение температуры
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);
  
  // Если включена коррекция температуры
  if (sensorsSettings.tempCorrectionEnabled) {
    temperature += sensorsSettings.tempCorrectionValue;
  }
  
  // Первое чтение уровня воды
  updateWaterLevel();
  
  log(LOG_INFO, "Sensors initialized, temperature: " + String(temperature) + "°C, water level: " + String(waterLevel));
  
  return true;
}

// Обновление уровня воды
void updateWaterLevel() {
  bool emptySensor = digitalRead(PIN_SENSOR_EMPTY);
  bool fullSensor = digitalRead(PIN_SENSOR_FULL);
  
  // Инвертирование показаний датчиков, если настроено
  if (sensorsSettings.invertEmptySensor) {
    emptySensor = !emptySensor;
  }
  
  if (sensorsSettings.invertFullSensor) {
    fullSensor = !fullSensor;
  }
  
  // Определение статуса уровня воды
  if (emptySensor == SENSOR_EMPTY_STATE_ON && fullSensor == SENSOR_FULL_STATE_OFF) {
    waterLevel = WATER_LEVEL_EMPTY;
  } else if (emptySensor == SENSOR_EMPTY_STATE_OFF && fullSensor == SENSOR_FULL_STATE_OFF) {
    waterLevel = WATER_LEVEL_NORMAL;
  } else if (emptySensor == SENSOR_EMPTY_STATE_OFF && fullSensor == SENSOR_FULL_STATE_ON) {
    waterLevel = WATER_LEVEL_FULL;
  } else {
    waterLevel = WATER_LEVEL_ERROR;
  }
}

// Обновление состояния датчиков
void updateSensors() {
  // Обновление температуры каждые 30 секунд
  if (millis() - lastTempReadTime > 30000) {
    sensors.requestTemperatures();
    temperature = sensors.getTempCByIndex(0);
    
    // Если включена коррекция температуры
    if (sensorsSettings.tempCorrectionEnabled) {
      temperature += sensorsSettings.tempCorrectionValue;
    }
    
    lastTempReadTime = millis();
  }
  
  // Обновление уровня воды
  updateWaterLevel();
}

// Обработка кнопок
void handleButtons() {
  // Проверка кнопок каждые 100 мс
  if (millis() - lastButtonCheckTime < 100) {
    return;
  }
  
  lastButtonCheckTime = millis();
  
  // Кнопка 1
  if (digitalRead(PIN_BUTTON1) == LOW) {
    // Инвертируем состояние нагрузки 1
    setLoadStatus(1, !load1Status);
    delay(200); // Защита от дребезга контактов
  }
  
  // Кнопка 2
  if (digitalRead(PIN_BUTTON2) == LOW) {
    // Инвертируем состояние нагрузки 2, если уровень воды не пустой
    if (waterLevel != WATER_LEVEL_EMPTY) {
      setLoadStatus(2, !load2Status);
    }
    delay(200); // Защита от дребезга контактов
  }
}

// Обновление светодиодов
void updateLEDs() {
  // Обновление светодиодов в зависимости от уровня воды
  switch (waterLevel) {
    case WATER_LEVEL_EMPTY:
      // Пустой бак - красный светодиод
      digitalWrite(PIN_LED_EMPTY, HIGH);
      digitalWrite(PIN_LED_FULL, LOW);
      break;
      
    case WATER_LEVEL_NORMAL:
      // Нормальный уровень - мигающий зеленый светодиод
      digitalWrite(PIN_LED_EMPTY, LOW);
      
      // Мигание зеленого светодиода
      if (millis() - lastLedBlinkTime > 500) {
        ledBlinkState = !ledBlinkState;
        digitalWrite(PIN_LED_FULL, ledBlinkState ? HIGH : LOW);
        lastLedBlinkTime = millis();
      }
      break;
      
    case WATER_LEVEL_FULL:
      // Полный бак - зеленый светодиод
      digitalWrite(PIN_LED_EMPTY, LOW);
      digitalWrite(PIN_LED_FULL, HIGH);
      break;
      
    case WATER_LEVEL_ERROR:
      // Ошибка датчиков - мигание обоих светодиодов
      if (millis() - lastLedBlinkTime > 300) {
        ledBlinkState = !ledBlinkState;
        digitalWrite(PIN_LED_EMPTY, ledBlinkState ? HIGH : LOW);
        digitalWrite(PIN_LED_FULL, ledBlinkState ? LOW : HIGH);
        lastLedBlinkTime = millis();
      }
      break;
  }
}

// Получение статуса уровня воды
WaterLevelStatus getWaterLevelStatus() {
  return waterLevel;
}

// Получение состояния нагрузки
bool getLoadStatus(int loadNumber) {
  if (loadNumber == 1) {
    return load1Status;
  } else if (loadNumber == 2) {
    return load2Status;
  }
  
  return false;
}

// Установка состояния нагрузки
bool setLoadStatus(int loadNumber, bool state) {
  if (loadNumber == 1) {
    load1Status = state;
    digitalWrite(PIN_LOAD1, state ? HIGH : LOW);
    log(LOG_INFO, "Load 1 set to " + String(state ? "ON" : "OFF"));
    return true;
  } else if (loadNumber == 2) {
    // Проверка уровня воды для нагрузки 2
    if (state && waterLevel == WATER_LEVEL_EMPTY) {
      log(LOG_WARNING, "Cannot turn on Load 2: water level is empty");
      return false;
    }
    
    load2Status = state;
    digitalWrite(PIN_LOAD2, state ? HIGH : LOW);
    log(LOG_INFO, "Load 2 set to " + String(state ? "ON" : "OFF"));
    return true;
  }
  
  return false;
}
