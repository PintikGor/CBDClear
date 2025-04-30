#include "logger.h"
#include "config.h"
#include <SD.h>
#include <ArduinoJson.h>

// Глобальные переменные
extern RTC_DS3231 rtc;
#define MAX_LOG_ENTRIES 100
String logEntries[MAX_LOG_ENTRIES];
int logIndex = 0;
int logCount = 0;
unsigned long lastLogFlushTime = 0;

// Инициализация системы логирования
bool initLogger() {
  // Проверка и создание директории для логов
  if (!SD.exists("/logs")) {
    SD.mkdir("/logs");
  }
  
  // Проверка размера лог-файла
  File logFile = SD.open(LOG_FILE_PATH, FILE_READ);
  if (logFile) {
    size_t fileSize = logFile.size();
    logFile.close();
    
    // Если файл слишком большой, создаем новый
    if (fileSize > MAX_LOG_SIZE) {
      // Создаем архивный файл
      String archiveFileName = "/logs/system_" + String(rtc.now().timestamp(DateTime::TIMESTAMP_DATE)) + ".log";
      SD.rename(LOG_FILE_PATH, archiveFileName);
      
      log(LOG_INFO, "Log file archived to " + archiveFileName);
    }
  }
  
  log(LOG_INFO, "Logger initialized");
  return true;
}

// Запись сообщения в лог
void log(LogLevel level, String message) {
  // Получение текущего времени
  DateTime now = rtc.now();
  char buffer[25];
sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
        now.year(), now.month(), now.day(),
        now.hour(), now.minute(), now.second());
String timestamp = String(buffer);
  
  // Формирование строки лога
  String levelStr;
  switch (level) {
    case LOG_DEBUG:
      levelStr = "DEBUG";
      break;
    case LOG_INFO:
      levelStr = "INFO";
      break;
    case LOG_WARNING:
      levelStr = "WARNING";
      break;
    case LOG_ERROR:
      levelStr = "ERROR";
      break;
  }
  
  String logEntry = timestamp + " [" + levelStr + "] " + message;
  
  // Вывод в консоль
  Serial.println(logEntry);
  
  // Добавление в буфер логов
  logEntries[logIndex] = logEntry;
  logIndex = (logIndex + 1) % MAX_LOG_ENTRIES;
  if (logCount < MAX_LOG_ENTRIES) {
    logCount++;
  }
}

// Обработка логирования
void loggerLoop() {
  // Запись логов на SD-карту каждые 60 секунд
  if (millis() - lastLogFlushTime > 60000) {
    flushLogs();
    lastLogFlushTime = millis();
  }
}

// Запись логов на SD-карту
void flushLogs() {
  if (logCount == 0) {
    return;
  }
  
  // Открываем файл для добавления
  File logFile = SD.open(LOG_FILE_PATH, FILE_APPEND);
  if (!logFile) {
    Serial.println("Failed to open log file for writing");
    return;
  }
  
  // Определяем начальный индекс для записи
  int startIndex = (logCount < MAX_LOG_ENTRIES) ? 0 : logIndex;
  
  // Записываем логи в файл
  for (int i = 0; i < logCount; i++) {
    int index = (startIndex + i) % MAX_LOG_ENTRIES;
    logFile.println(logEntries[index]);
  }
  
  logFile.close();
  
  // Сбрасываем счетчики
  logCount = 0;
  logIndex = 0;
}

// Получение логов в формате JSON
String getLogsJson() {
  DynamicJsonDocument doc(10240);
  JsonArray logsArray = doc.createNestedArray("logs");
  
  // Определяем начальный индекс для чтения
  int startIndex = (logCount < MAX_LOG_ENTRIES) ? 0 : logIndex;
  
  // Добавляем логи из буфера
  for (int i = 0; i < logCount; i++) {
    int index = (startIndex + i) % MAX_LOG_ENTRIES;
    logsArray.add(logEntries[index]);
  }
  
  // Добавляем логи из файла (последние 100 строк)
  File logFile = SD.open(LOG_FILE_PATH, FILE_READ);
  if (logFile) {
    // Определяем размер файла
    size_t fileSize = logFile.size();
    
    // Если файл слишком большой, читаем только последние 10KB
    if (fileSize > 10240) {
      logFile.seek(fileSize - 10240);
      // Пропускаем первую неполную строку
      while (logFile.available() && logFile.read() != '\n');
    }
    
    // Читаем строки из файла
    String line = "";
    while (logFile.available()) {
      char c = logFile.read();
      if (c == '\n') {
        logsArray.add(line);
        line = "";
      } else {
        line += c;
      }
    }
    
    // Добавляем последнюю строку, если она не пустая
    if (line.length() > 0) {
      logsArray.add(line);
    }
    
    logFile.close();
  }
  
  String result;
  serializeJson(doc, result);
  return result;
}
