#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <RTClib.h>
extern RTC_DS3231 rtc;
// Уровни логирования
enum LogLevel {
  LOG_DEBUG = 0,
  LOG_INFO = 1,
  LOG_WARNING = 2,
  LOG_ERROR = 3
};

// Инициализация системы логирования
bool initLogger();
void flushLogs();
// Запись сообщения в лог
void log(LogLevel level, String message);

// Обработка логирования
void loggerLoop();

// Получение логов в формате JSON
String getLogsJson();

#endif // LOGGER_H
