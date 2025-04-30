/*
 * ComeBack Device Clear
 * Система очистки камер видеонаблюдения
 * Версия: 1.0.0
 * Дата: 29.04.2025
 */

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <RTClib.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>

// Подключение заголовочных файлов
#include "config.h"
#include "network.h"
#include "sensors.h"
#include "scheduler.h"
#include "storage.h"
#include "logger.h"

// Глобальные переменные
RTC_DS3231 rtc;
AsyncWebServer server(80);
bool load1Status = false;
bool load2Status = false;
WaterLevelStatus waterLevel = WATER_LEVEL_ERROR;
float temperature = 0.0;

void setup() {
  // Инициализация последовательного порта
  Serial.begin(115200);
  if(!Serial) Serial.end();
  Serial.println(F("ComeBack Device Clear - Starting..."));


  // Инициализация пинов
  initPins();
  
  // Инициализация SPI
  SPI.begin();
  
// SD Card Mount
if(!SD.begin(PIN_CD_CS)){
  Serial.println("Card Mount Failed");
  // Здесь лучше не использовать бесконечный цикл, а установить флаг ошибки
  // while (1);
  return;
}
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    while (1);
  }
  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC) {
    Serial.println("MMC");
  }
  else if(cardType == CARD_SD) {
    Serial.println("SDSC");
  }
  else if(cardType == CARD_SDHC) {
    Serial.println("SDHC");
  }
  else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  listDir(SD, "/", 0);

  
  // Инициализация RTC
  if (!initRTC()) {
    Serial.println(F("RTC initialization failed!"));
  }
  
  // Инициализация датчиков
  initSensors();
  
  // Загрузка настроек
  loadSettings();
  
  // Инициализация сети
  initNetwork();
  
  // Инициализация веб-сервера
  initWebServer();
  
  // Инициализация планировщика
  initScheduler();
  
  // Инициализация системы логирования
  initLogger();
  
  Serial.println(F("ComeBack Device Clear - Ready!"));
}

void loop() {
  // Обработка сетевых событий
  networkLoop();
  
  // Обновление состояния датчиков
  updateSensors();
  
  // Обработка планировщика задач
  schedulerLoop();
  
  // Обработка кнопок
  handleButtons();
  
  // Обработка светодиодов
  updateLEDs();
  
  // Обработка логирования
  loggerLoop();
  
  // Небольшая задержка для стабильности
  delay(10);
}
