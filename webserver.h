#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

// Инициализация веб-сервера
bool initWebServer();

// Обработка API запросов
void handleApiRequest(AsyncWebServerRequest *request);

// Обработка запросов на обновление прошивки
void handleFirmwareUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);

#endif // WEBSERVER_H
