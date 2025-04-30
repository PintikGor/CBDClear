#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>
#include "config.h"
#include <RTClib.h>

// Режимы работы расписания
enum ScheduleMode {
  SCHEDULE_MODE_DAILY = 0,
  SCHEDULE_MODE_WEEKLY = 1,
  SCHEDULE_MODE_MONTHLY = 2
};

// Структура расписания
struct Schedule {
  bool enabled;
  int loadNumber;
  ScheduleMode mode;
  uint8_t days; // Битовая маска дней (для недельного режима)
  uint8_t day;  // День месяца (для месячного режима)
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  bool useDuration;
  uint8_t durationHour;
  uint8_t durationMinute;
  uint8_t durationSecond;
  uint8_t endHour;
  uint8_t endMinute;
  uint8_t endSecond;
};

// Инициализация планировщика
bool initScheduler();

// Обработка планировщика задач
void schedulerLoop();

// Получение расписаний в формате JSON
String getSchedulesJson();

// Обновление расписаний из JSON
bool updateSchedulesFromJson(String json);

// Получение времени следующего запланированного события
String getNextScheduleTime();

// Установка даты и времени
bool setDateTime(String dateStr, String timeStr);

// Инициализация RTC
bool initRTC();

#endif // SCHEDULER_H
