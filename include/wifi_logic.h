#ifndef WIFI_LOGIC_H
#define WIFI_LOGIC_H

#include <Arduino.h>
#include <stdarg.h>

#include "web_pages.h" // Шаблоны веб-интерфейса (HTML/CSS компоненты)
#include <ArduinoOTA.h> // Протокол беспроводной прошивки (обновление кода через Wi-Fi без USB)
#include <DNSServer.h> // DNS-сервер для Captive Portal (перенаправление на страницу настроек)
#include <FS.h> // Абстрактный слой файловой системы (интерфейс доступа к Flash-памяти)
#include <Preferences.h> // Работа с NVS-памятью (энергонезависимое хранение конфигурации)
#include <WebServer.h> // Реализация HTTP-сервера (обработка GET/POST запросов веб-интерфейса)
#include <WiFi.h> // Сетевой стек 802.11 (управление радиомодулем, режимы STA и AP)
#include <lvgl.h> // Движок графического интерфейса пользователя (UI Engine)

// =============================================================================
// ВНЕШНИЕ ССЫЛКИ (EXTERN)
// =============================================================================
extern WebServer server;
extern DNSServer dnsServer;
extern Preferences preferences;
extern char ssid[32];
extern char password[64];
extern char weather_city[64];
extern String apName;
extern lv_obj_t *load_label;

extern int dayBrightness;
extern int nightBrightness;
extern int nightStartHour;
extern int nightEndHour;

// =============================================================================
// ЛОГИРОВАНИЕ И ИНТЕРФЕЙС
// =============================================================================

/**
 * @brief Выводит форматированное сообщение в Serial с временной меткой.
 * @param format Строка формата (как в printf)
 */
void logInfo(const char *format, ...) {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStamp[20];
    strftime(timeStamp, sizeof(timeStamp), "%H:%M:%S", &timeinfo);
    Serial.printf("[%s] ", timeStamp);
  }
  char loc_res[128];
  va_list arg;
  va_start(arg, format);
  vsnprintf(loc_res, sizeof(loc_res), format, arg);
  va_end(arg);
  Serial.println(loc_res);
}

/**
 * @brief Обновляет текстовое сообщение на сервисном экране загрузки (LVGL).
 */
void update_screen_status(const char *txt) {
  if (load_label != nullptr) {
    lv_label_set_text(load_label, txt);
    lv_obj_invalidate(load_label);
    lv_timer_handler();
    lv_refr_now(NULL);
  }
}

// =============================================================================
// ОБНОВЛЕНИЕ ПО ВОЗДУХУ (OTA)
// =============================================================================

/**
 * @brief Настраивает параметры и обработчики событий для прошивки через Wi-Fi.
 */
void setupOTA() {
  // 1. Установка сетевого имени устройства (будет отображаться в списке портов
  // IDE)
  ArduinoOTA.setHostname(OTA_HOSTNAME);

  // 2. Установка пароля доступа (защита от несанкционированной прошивки)
  // ArduinoOTA.setPassword(OTA_PASSWORD);

  // 3. Обработчик события: Начало процесса обновления
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch"; // Обновление программы (кода)
    else
      type = "filesystem"; // Обновление файловой системы
    logInfo("OTA: Начало загрузки %s", type.c_str());
  });

  // 4. Обработчик события: Завершение прошивки
  ArduinoOTA.onEnd([]() { logInfo("OTA: Обновление успешно завершено"); });

  // 5. Обработчик события: Визуализация прогресса (вывод % в Serial)
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Прогресс: %u%%\r", (progress / (total / 100)));
  });

  // 6. Обработчик события: Возникновение критической ошибки
  ArduinoOTA.onError([](ota_error_t error) {
    logInfo("OTA Ошибка [%u]", error);
    if (error == OTA_AUTH_ERROR)
      logInfo("Ошибка: Отказ в авторизации");
    else if (error == OTA_BEGIN_ERROR)
      logInfo("Ошибка: Сбой инициализации");
    else if (error == OTA_CONNECT_ERROR)
      logInfo("Ошибка: Сбой соединения");
    else if (error == OTA_RECEIVE_ERROR)
      logInfo("Ошибка: Ошибка приема данных");
    else if (error == OTA_END_ERROR)
      logInfo("Ошибка: Сбой завершения");
  });

  // 7. Запуск фоновой службы прослушивания порта OTA
  ArduinoOTA.begin();
  logInfo("OTA: Служба запущена и готова к работе");
}

// =============================================================================
// СЕРВИСНЫЕ ФУНКЦИИ WIFI
// =============================================================================

/**
 * @brief Генерирует уникальное имя точки доступа на основе MAC-адреса чипа.
 */
void generateAPName() {
  uint64_t chipId = ESP.getEfuseMac();
  uint16_t shortId = (uint16_t)(chipId >> 32);
  char buf[25];
  sprintf(buf, "DIY-CLOCK-%04X", shortId);
  apName = String(buf);
  logInfo("Static AP Name: %s", apName.c_str());
}

/**
 * @brief Сканирует WiFi эфир и формирует список HTML-опций.
 */
String scanNetworks() {
  int n = WiFi.scanNetworks();
  String list = "";
  for (int i = 0; i < n; ++i) {
    list += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" +
            String(WiFi.RSSI(i)) + " dBm)</option>";
  }
  return list;
}

// =============================================================================
// ОБРАБОТЧИКИ WEB-СЕРВЕРА
// =============================================================================

void handleSaveSettings() {
  if (server.hasArg("d_br")) {
    preferences.begin("wifi-config", false);

    // Сохранение настроек яркости
    dayBrightness = server.arg("d_br").toInt();
    nightBrightness = server.arg("n_br").toInt();
    nightStartHour = server.arg("n_st").toInt();
    nightEndHour = server.arg("n_en").toInt();

    preferences.putInt("day_br", dayBrightness);
    preferences.putInt("night_br", nightBrightness);
    preferences.putInt("n_start", nightStartHour);
    preferences.putInt("n_end", nightEndHour);

    // Сохранение города
    if (server.hasArg("city")) {
      String newCity = server.arg("city");
      newCity.trim();
      if (newCity.length() > 0) {
        strlcpy(weather_city, newCity.c_str(), sizeof(weather_city));
        preferences.putString("city", newCity);
      }
    }

    preferences.end();
    logInfo("Device settings saved. City: %s", weather_city);
    server.sendHeader("Location", "/");
    server.send(303);
  }
}

void setupWebHandlers() {
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html",
                getIndexPage(scanNetworks(), ssid, dayBrightness,
                             nightBrightness, nightStartHour, nightEndHour,
                             String(weather_city)));
  });

  server.on("/save", HTTP_POST, []() {
    String s = server.arg("custom_ssid");
    if (s == "")
      s = server.arg("ssid_select");
    String p = server.arg("pass");
    if (s.length() > 0) {
      preferences.begin("wifi-config", false);
      preferences.putString("ssid", s);
      preferences.putString("pass", p);
      preferences.end();
      WiFi.disconnect(true, true);
      update_screen_status("Настройки сохранены\nПерезагрузка...");
      server.send(200, "text/html", "OK. Rebooting...");
      delay(1000);
      ESP.restart();
    }
  });

  server.on("/reset", HTTP_GET, []() {
    preferences.begin("wifi-config", false);
    preferences.clear();
    preferences.end();
    server.send(200, "text/plain", "Reset OK");
    delay(1000);
    ESP.restart();
  });

  server.on("/save_settings", HTTP_POST, handleSaveSettings);

  server.onNotFound([]() {
    server.sendHeader("Location", String("http://192.168.4.1"), true);
    server.send(302, "text/plain", "");
  });
}

// =============================================================================
// УПРАВЛЕНИЕ РЕЖИМАМИ
// =============================================================================

/**
 * @brief Запуск точки доступа и цикл обработки запросов (режим настройки).
 */
void startConfigMode() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apName.c_str());
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.begin();
  update_screen_status(
      ("НАСТРОЙКА\nСеть: " + apName + "\nIP: 192.168.4.1").c_str());
  while (true) {
    dnsServer.processNextRequest();
    server.handleClient();
    lv_timer_handler();
    delay(10);
    yield();
  }
}

#endif // WIFI_LOGIC_H