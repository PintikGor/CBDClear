#ifndef CONFIG_H
#define CONFIG_H

// Версия прошивки
#define FIRMWARE_VERSION "1.0.0"

// Пины устройства
#define PIN_CD_CS 4         // CS карты памяти
#define PIN_ETH_CS 32       // CS для ethernet
#define PIN_TEMP_SENSOR 16  // Датчик температуры DS18B20

// Входы
#define PIN_BUTTON1 34      // кнопка нагрузки 1
#define PIN_BUTTON2 35      // кнопка нагрузки 2
#define PIN_SENSOR_EMPTY 36 // поплавок пустого бака
#define PIN_SENSOR_FULL 39  // поплавок полного бака

// Выходы
#define PIN_LED_EMPTY 25    // светодиод пустого бака
#define PIN_LED_FULL 33     // светодиод полного бака
#define PIN_LOAD1 27        // реле нагрузки 1
#define PIN_LOAD2 26        // реле нагрузки 2

// Настройки датчиков
#define SENSOR_EMPTY_STATE_ON 1  // состояние, при котором поплавок пустого бака сработал
#define SENSOR_FULL_STATE_ON 1   // состояние, при котором поплавок полного бака сработал
#define SENSOR_EMPTY_STATE_OFF !SENSOR_EMPTY_STATE_ON
#define SENSOR_FULL_STATE_OFF !SENSOR_FULL_STATE_ON

// Настройки сети
#define DEFAULT_HOSTNAME "comeback-device"
#define DEFAULT_AP_SSID "ComeBackDevice"
#define DEFAULT_AP_PASSWORD "password123"

// Настройки планировщика
#define MAX_SCHEDULES 10

// Настройки логирования
#define LOG_FILE_PATH "/logs/system.log"
#define MAX_LOG_SIZE 1024 * 1024 // 1 МБ

// Статусы уровня воды
enum WaterLevelStatus {
  WATER_LEVEL_EMPTY = 0,
  WATER_LEVEL_NORMAL = 1,
  WATER_LEVEL_FULL = 2,
  WATER_LEVEL_ERROR = 3
};

// Инициализация пинов
void initPins() {
  // Настройка выходов
  pinMode(PIN_LOAD1, OUTPUT);
  pinMode(PIN_LOAD2, OUTPUT);
  pinMode(PIN_LED_EMPTY, OUTPUT);
  pinMode(PIN_LED_FULL, OUTPUT);
  
  // Настройка входов
  pinMode(PIN_SENSOR_EMPTY, INPUT_PULLUP);
  pinMode(PIN_SENSOR_FULL, INPUT_PULLUP);
  pinMode(PIN_BUTTON1, INPUT_PULLUP);
  pinMode(PIN_BUTTON2, INPUT_PULLUP);
  
  // Установка начальных состояний
  digitalWrite(PIN_LOAD1, LOW);    // реле 1 по умолчанию выключено
  digitalWrite(PIN_LOAD2, LOW);    // реле 2 по умолчанию выключено
  digitalWrite(PIN_LED_EMPTY, LOW);
  digitalWrite(PIN_LED_FULL, LOW);
}

#endif // CONFIG_H
