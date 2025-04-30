#include "scheduler.h"
#include "storage.h"
#include "sensors.h"
#include "logger.h"
#include <ArduinoJson.h>

// Глобальные переменные
extern RTC_DS3231 rtc;
Schedule schedules[MAX_SCHEDULES];
unsigned long lastScheduleCheckTime = 0;

// Инициализация RTC
bool initRTC() {
  // Инициализация RTC
  if (!rtc.begin()) {
    log(LOG_ERROR, "Couldn't find RTC");
    return false;
  }
  
  // Проверка потери питания RTC
  if (rtc.lostPower()) {
    log(LOG_WARNING, "RTC lost power, setting default time");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  log(LOG_INFO, "RTC initialized");
  return true;
}

// Инициализация планировщика
bool initScheduler() {
  // Загрузка расписаний
  loadSchedules(schedules, MAX_SCHEDULES);
  
  log(LOG_INFO, "Scheduler initialized");
  return true;
}

// Обработка планировщика задач
void schedulerLoop() {
  // Проверка расписаний каждую секунду
  if (millis() - lastScheduleCheckTime < 1000) {
    return;
  }
  
  lastScheduleCheckTime = millis();
  
  // Получение текущего времени
  DateTime now = rtc.now();
  
  // Проверка каждого расписания
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    if (!schedules[i].enabled) {
      continue;
    }
    
    // Проверка соответствия дня
    bool dayMatch = false;
    
    switch (schedules[i].mode) {
      case SCHEDULE_MODE_DAILY:
        dayMatch = true;
        break;
        
      case SCHEDULE_MODE_WEEKLY:
        // Проверка дня недели (0 = воскресенье, 6 = суббота)
        dayMatch = (schedules[i].days & (1 << now.dayOfTheWeek())) != 0;
        break;
        
      case SCHEDULE_MODE_MONTHLY:
        // Проверка дня месяца
        dayMatch = now.day() == schedules[i].day;
        break;
    }
    
    if (!dayMatch) {
      continue;
    }
    
    // Проверка времени включения
    if (now.hour() == schedules[i].hour && 
        now.minute() == schedules[i].minute && 
        now.second() == schedules[i].second) {
      
      // Включение нагрузки
      setLoadStatus(schedules[i].loadNumber, true);
      log(LOG_INFO, "Schedule " + String(i) + " triggered: turning ON load " + String(schedules[i].loadNumber));
    }
    
    // Проверка времени выключения
    if (schedules[i].useDuration) {
      // Вычисление времени выключения на основе длительности
      DateTime startTime(now.year(), now.month(), now.day(),
                         schedules[i].hour, schedules[i].minute, schedules[i].second);
      
      TimeSpan duration(0, 
                        schedules[i].durationHour, 
                        schedules[i].durationMinute, 
                        schedules[i].durationSecond);
      
      DateTime endTime = startTime + duration;
      
      if (now.hour() == endTime.hour() && 
          now.minute() == endTime.minute() && 
          now.second() == endTime.second()) {
        
        // Выключение нагрузки
                setLoadStatus(schedules[i].loadNumber, false);
        log(LOG_INFO, "Schedule " + String(i) + " duration ended: turning OFF load " + String(schedules[i].loadNumber));
      }
    } else {
      // Проверка конкретного времени выключения
      if (now.hour() == schedules[i].endHour && 
          now.minute() == schedules[i].endMinute && 
          now.second() == schedules[i].endSecond) {
        
        // Выключение нагрузки
        setLoadStatus(schedules[i].loadNumber, false);
        log(LOG_INFO, "Schedule " + String(i) + " end time reached: turning OFF load " + String(schedules[i].loadNumber));
      }
    }
  }
}

// Получение расписаний в формате JSON
String getSchedulesJson() {
  DynamicJsonDocument doc(4096);
  JsonArray schedulesArray = doc.createNestedArray("schedules");
  
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    JsonObject scheduleObj = schedulesArray.createNestedObject();
    
    scheduleObj["id"] = i;
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
  
  String result;
  serializeJson(doc, result);
  return result;
}

// Обновление расписаний из JSON
bool updateSchedulesFromJson(String json) {
  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    log(LOG_ERROR, "Failed to parse schedules JSON: " + String(error.c_str()));
    return false;
  }
  
  JsonArray schedulesArray = doc["schedules"];
  int i = 0;
  
  for (JsonObject scheduleObj : schedulesArray) {
    if (i >= MAX_SCHEDULES) break;
    
    int id = scheduleObj["id"] | i;
    if (id >= 0 && id < MAX_SCHEDULES) {
      schedules[id].enabled = scheduleObj["enabled"] | false;
      schedules[id].loadNumber = scheduleObj["loadNumber"] | 1;
      schedules[id].mode = (ScheduleMode)(scheduleObj["mode"] | SCHEDULE_MODE_DAILY);
      schedules[id].days = scheduleObj["days"] | 0x7F;
      schedules[id].day = scheduleObj["day"] | 1;
      schedules[id].hour = scheduleObj["hour"] | 12;
      schedules[id].minute = scheduleObj["minute"] | 0;
      schedules[id].second = scheduleObj["second"] | 0;
      schedules[id].useDuration = scheduleObj["useDuration"] | true;
      schedules[id].durationHour = scheduleObj["durationHour"] | 0;
      schedules[id].durationMinute = scheduleObj["durationMinute"] | 5;
      schedules[id].durationSecond = scheduleObj["durationSecond"] | 0;
      schedules[id].endHour = scheduleObj["endHour"] | 12;
      schedules[id].endMinute = scheduleObj["endMinute"] | 5;
      schedules[id].endSecond = scheduleObj["endSecond"] | 0;
    }
    
    i++;
  }
  
  // Сохранение расписаний в хранилище
  saveSchedules(schedules, MAX_SCHEDULES);
  
  log(LOG_INFO, "Schedules updated from JSON: " + String(i));
  return true;
}

// Получение времени следующего запланированного события
String getNextScheduleTime() {
  DateTime now = rtc.now();
  DateTime nextEvent(2099, 12, 31, 23, 59, 59); // Далекое будущее
  int nextScheduleId = -1;
  bool isStartEvent = false;
  
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    if (!schedules[i].enabled) {
      continue;
    }
    
    // Проверка соответствия дня
    int daysToAdd = 0;
    bool foundDay = false;
    
    switch (schedules[i].mode) {
      case SCHEDULE_MODE_DAILY:
        foundDay = true;
        break;
        
      case SCHEDULE_MODE_WEEKLY:
        // Поиск следующего подходящего дня недели
        for (int d = 0; d < 7; d++) {
          int dayToCheck = (now.dayOfTheWeek() + d) % 7;
          if (schedules[i].days & (1 << dayToCheck)) {
            daysToAdd = d;
            foundDay = true;
            break;
          }
        }
        break;
        
      case SCHEDULE_MODE_MONTHLY:
        // Если день месяца уже прошел, переходим к следующему месяцу
        if (now.day() > schedules[i].day) {
          daysToAdd = 32 - now.day() + schedules[i].day; // Примерно, не учитывает разное количество дней в месяцах
        } else if (now.day() < schedules[i].day) {
          daysToAdd = schedules[i].day - now.day();
        } else {
          foundDay = true;
        }
        break;
    }
    
    if (!foundDay) {
      continue;
    }
    
    // Создаем DateTime для времени включения
    DateTime startTime = now + TimeSpan(daysToAdd, 0, 0, 0);
    startTime = DateTime(startTime.year(), startTime.month(), startTime.day(),
                         schedules[i].hour, schedules[i].minute, schedules[i].second);
    
    // Если время уже прошло сегодня, переходим к следующему дню
    if (startTime < now && daysToAdd == 0) {
      startTime = startTime + TimeSpan(1, 0, 0, 0);
    }
    
    // Проверяем, является ли это ближайшим событием
    if (startTime < nextEvent) {
      nextEvent = startTime;
      nextScheduleId = i;
      isStartEvent = true;
    }
    
    // Создаем DateTime для времени выключения
    DateTime endTime;
    
    if (schedules[i].useDuration) {
      TimeSpan duration(0, 
                      schedules[i].durationHour, 
                      schedules[i].durationMinute, 
                      schedules[i].durationSecond);
      endTime = startTime + duration;
    } else {
      endTime = DateTime(startTime.year(), startTime.month(), startTime.day(),
                       schedules[i].endHour, schedules[i].endMinute, schedules[i].endSecond);
      
      // Если время выключения раньше времени включения, считаем, что это следующий день
      if (endTime < startTime) {
        endTime = endTime + TimeSpan(1, 0, 0, 0);
      }
    }
    
    // Если время уже прошло сегодня, переходим к следующему дню
    if (endTime < now && daysToAdd == 0) {
      endTime = endTime + TimeSpan(1, 0, 0, 0);
    }
    
    // Проверяем, является ли это ближайшим событием
    if (endTime < nextEvent && endTime > now) {
      nextEvent = endTime;
      nextScheduleId = i;
      isStartEvent = false;
    }
  }
  
  if (nextScheduleId == -1) {
    return "Нет запланированных событий";
  }
  
  char buffer[50];
  sprintf(buffer, "%02d:%02d:%02d - %s нагрузки %d", 
          nextEvent.hour(), 
          nextEvent.minute(), 
          nextEvent.second(),
          isStartEvent ? "Включение" : "Выключение",
          schedules[nextScheduleId].loadNumber);
  
  return String(buffer);
}

// Установка даты и времени
bool setDateTime(String dateStr, String timeStr) {
  // Формат даты: YYYY-MM-DD
  // Формат времени: HH:MM:SS
  
  int year = dateStr.substring(0, 4).toInt();
  int month = dateStr.substring(5, 7).toInt();
  int day = dateStr.substring(8, 10).toInt();
  
  int hour = timeStr.substring(0, 2).toInt();
  int minute = timeStr.substring(3, 5).toInt();
  int second = timeStr.substring(6, 8).toInt();
  
  if (year < 2000 || year > 2099 || month < 1 || month > 12 || day < 1 || day > 31 ||
      hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
    log(LOG_ERROR, "Invalid date or time format");
    return false;
  }
  
  DateTime newDateTime(year, month, day, hour, minute, second);
  rtc.adjust(newDateTime);
  
  log(LOG_INFO, "Date and time set to: " + dateStr + " " + timeStr);
  return true;
}
