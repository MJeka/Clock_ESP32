#ifndef WIFI_LOGIC_H
#define WIFI_LOGIC_H

#include "web_pages.h" // Шаблоны веб-интерфейса (HTML/CSS компоненты)
#include "cities_db.h" // Локальная база городов Украины и мира (PROGMEM)
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
extern void check_brightness(struct tm *timeinfo = nullptr);
extern void fetch_weather();
extern uint32_t configTimeout;
extern bool isConfigMode;
extern uint32_t configStartTime;
extern uint32_t lastDisplayUpdate;

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
 * @brief Активирует верхний графический слой ("занавес") для отображения системных процессов.
 * Используется при загрузке и OTA-обновлении, чтобы перекрыть основной интерфейс.
 */
void show_ota_layer() {
  lv_obj_t *top_layer = lv_layer_top();
  
  // Делаем слой видимым и непрозрачным
  lv_obj_clear_flag(top_layer, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_bg_opa(top_layer, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(top_layer, lv_color_hex(0x000000), 0);
  
  // Воссоздаем текстовую метку, если она была удалена после инициализации
  if (load_label == nullptr) {
    load_label = lv_label_create(top_layer);
    lv_obj_set_style_text_color(load_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(load_label, &ui_font_roboto24, 0);
    lv_obj_set_style_text_align(load_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(load_label, LV_ALIGN_CENTER, 0, 0);
  }
}

/**
 * @brief Обновляет текстовую информацию о прогрессе на верхнем слое.
 * @param message Основное сообщение (статус)
 * @param percent Процент выполнения (0-100)
 */
void update_ota_status(const char *message, int percent) {
  if (load_label != nullptr) {
    lv_label_set_text_fmt(load_label, "%s\n%d%%", message, percent);
    lv_obj_invalidate(load_label); // Помечаем объект как требующий перерисовки
  }
  // Принудительный вызов обработчика для немедленного обновления экрана
  lv_timer_handler();
  lv_refr_now(NULL);  // Принудительно отрисовываем экран ПРЯМО СЕЙЧАС
}

/**
 * @brief Настраивает параметры и обработчики событий для прошивки через Wi-Fi.
 * Обеспечивает визуализацию процесса на дисплее через верхний графический слой.
 */
void setupOTA() {
  // 1. Установка сетевого имени устройства (будет отображаться в списке портов IDE)
  ArduinoOTA.setHostname(OTA_HOSTNAME);

  // 2. Установка пароля доступа (защита от несанкционированной прошивки)
  ArduinoOTA.setPassword(OTA_PASSWORD);

  // 3. Обработчик события: Начало процесса обновления
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch"; // Обновление программы (кода)
    else
      type = "filesystem"; // Обновление файловой системы (SPIFFS/LittleFS)
    
    show_ota_layer(); // Активация черного экрана заставки
    update_ota_status("Обновление...", 0);
    logInfo("OTA: Начало загрузки %s", type.c_str());
  });

  // 4. Обработчик события: Завершение прошивки
  ArduinoOTA.onEnd([]() { 
    update_ota_status("Готово!\nПерезагрузка", 100);
    logInfo("OTA: Обновление успешно завершено"); 
  });

  // 5. Обработчик события: Визуализация прогресса (вывод % на дисплей и в Serial)
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int percent = (progress / (total / 100));
    update_ota_status("Обновление прошивки...", percent);
    Serial.printf("OTA Прогресс: %u%%\r", percent);
  });

  // 6. Обработчик события: Возникновение критической ошибки
  ArduinoOTA.onError([](ota_error_t error) {
    char err_buf[64];
    const char* err_desc = "Ошибка";

    if (error == OTA_AUTH_ERROR) err_desc = "Отказ в авторизации";
    else if (error == OTA_BEGIN_ERROR) err_desc = "Сбой инициализации";
    else if (error == OTA_CONNECT_ERROR) err_desc = "Сбой соединения";
    else if (error == OTA_RECEIVE_ERROR) err_desc = "Ошибка приема данных";
    else if (error == OTA_END_ERROR) err_desc = "Сбой завершения";

    snprintf(err_buf, sizeof(err_buf), "OTA %s\n[%u]", err_desc, error);
    update_ota_status(err_buf, 0);
    logInfo("OTA: %s [%u]", err_desc, error);

    // Пауза перед скрытием слоя ошибки, чтобы пользователь успел прочитать текст
    delay(3000); 
    lv_obj_add_flag(lv_layer_top(), LV_OBJ_FLAG_HIDDEN);
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
 * @brief Вспомогательная функция для сборки HTML-списка из результатов сканирования.
 * @param n Количество найденных сетей.
 */
String buildNetworkList(int n) {
  if (n <= 0) return "<option value=''>Сети не найдены/</option>";
  
  String list = "";
  for (int i = 0; i < n; ++i) {
    /**
     * Формируем строку выбора: SSID и уровень сигнала в дБм.
     * Используем локальные переменные для ускорения сборки строки.
     */
    list += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" +
            String(WiFi.RSSI(i)) + " dBm)</option>";
  }
  return list;
}

/**
 * @brief Формирует список HTML-опций на основе последнего сканирования (асинхронно).
 * Исключает блокировку основного цикла (loop).
 */
String getCachedNetworks() {
  // Проверяем текущий статус сканера
  int n = WiFi.scanComplete(); 

  if (n == -2) {
    /**
     * Сканирование еще не инициировано. Запускаем в фоновом режиме (async = true).
     * Это не остановит выполнение кода и часов.
     */
    WiFi.scanNetworks(true); 
    return "<option>Сканирование начато...</option>";
  }
  
  if (n == -1) {
    // Сканирование в процессе выполнения
    return "<option>Поиск сетей (подождите)...</option>";
  }

  // Если n >= 0, значит данные в кэше готовы. Мы их отдаем, но НЕ удаляем,
  // чтобы список был доступен до ручного запроса на обновление.
  return buildNetworkList(n);
}

/**
 * @brief Старая версия для совместимости (если нужна блокирующая работа).
 */
String scanNetworks() {
  // Выполняем синхронное сканирование (блокирует loop до завершения)
  int n = WiFi.scanNetworks();
  String list = buildNetworkList(n);
  WiFi.scanDelete();
  return list;
}

// =============================================================================
// ОБРАБОТЧИКИ WEB-СЕРВЕРА
// =============================================================================

/**
 * @brief Обработчик сохранения настроек через веб-интерфейс.
 * Выполняет сохранение параметров в NVS и инициирует обновление состояния системы.
 */
void handleSaveSettings() {
  if (server.hasArg("d_br")) {
    String oldCity = String(weather_city);
    preferences.begin("wifi-config", false);

    // Настройки яркости и времени
    dayBrightness = server.arg("d_br").toInt();
    nightBrightness = server.arg("n_br").toInt();
    nightStartHour = server.arg("n_st").toInt();
    nightEndHour = server.arg("n_en").toInt();

    preferences.putInt("day_br", dayBrightness);
    preferences.putInt("night_br", nightBrightness);
    preferences.putInt("n_start", nightStartHour);
    preferences.putInt("n_end", nightEndHour);

    /**
     * @section CITY_SAVE_LOGIC
     * Сохранение выбранного города. Теперь данные приходят из локальной базы,
     * поэтому дополнительная сложная валидация (проверка запятых) не требуется.
     */
    if (server.hasArg("city")) {
      String newCity = server.arg("city");
      newCity.trim();
      if (newCity.length() > 0) {
        strlcpy(weather_city, newCity.c_str(), sizeof(weather_city));
        preferences.putString("city", newCity);
      }
    }

    preferences.end();
    check_brightness(); // Обновление яркости

    // Если город изменился — немедленный запрос новых метеоданных
    if (oldCity != String(weather_city)) {
      logInfo("City changed from %s to %s. Updating weather...",
              oldCity.c_str(), weather_city);
      fetch_weather();
    }

    logInfo("Device settings saved. City: %s", weather_city);
    server.sendHeader("Location", "/");
    server.send(303);
  }
}

/**
 * @brief Инициализация веб-обработчиков.
 * Настраивает маршруты для главной страницы, сохранения Wi-Fi и сброса настроек.
 */
void setupWebHandlers() {
  /**
   * @brief Главная страница настроек.
   * Передает локальный JSON городов из PROGMEM для работы автодополнения.
   */
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html",
                getIndexPage(getCachedNetworks(), ssid, dayBrightness,
                             nightBrightness, nightStartHour, nightEndHour,
                             String(weather_city), getCitiesJson()));
  });

  /**
   * @brief Инициирует принудительное пересканирование сетей.
   * Очищает кэш и запускает новый фоновый поиск.
   */
  server.on("/scan_trigger", HTTP_GET, []() {
    WiFi.scanDelete();        // Удаление старого результата
    WiFi.scanNetworks(true);  // Запуск нового асинхронного поиска
    server.send(200, "text/plain", "OK");
  });

  /**
   * @brief Сохранение учетных данных Wi-Fi.
   */
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

  /**
   * @brief Сброс настроек WiFi.
   */
  server.on("/reset", HTTP_GET, []() {
    preferences.begin("wifi-config", false);

    // Удаляем только ключи, отвечающие за Wi-Fi
    preferences.remove("ssid");
    preferences.remove("pass");

    // preferences.clear();
    preferences.end();
    server.send(200, "text/plain", "Reset WiFi OK");
    delay(1000);
    ESP.restart();
  });

  /**
   * @brief Полный сброс настроек устройства.
   */
  server.on("/full_reset", HTTP_GET, []() {
    preferences.begin("wifi-config", false);

    preferences.clear();
    preferences.end();
    server.send(200, "text/plain", "Full Reset OK");
    delay(1000);
    ESP.restart();
  });

  /**
   * @brief Обработка настроек яркости и выбора города.
   */
  server.on("/save_settings", HTTP_POST, handleSaveSettings);

  /**
   * @brief Возвращает статус сканирования для JS-скрипта.
   * -1: в процессе, -2: не начиналось, >=0: количество найденных сетей.
   */
  server.on("/scan_status", HTTP_GET, []() {
    server.send(200, "text/plain", String(WiFi.scanComplete()));
  });

  /**
   * @brief Принудительная перезагрузка контроллера.
   */
  server.on("/reboot", HTTP_GET, []() {
    server.send(200, "text/plain", "Rebooting...");
    delay(1000);
    ESP.restart();
  });

  /**
   * @brief Перенаправление для Captive Portal.
   */
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
    
    // Сразу инициируем первое сканирование, чтобы к открытию страницы был кэш
    WiFi.scanNetworks(true);

    dnsServer.start(53, "*", WiFi.softAPIP());
    server.begin();

    configStartTime = millis(); // Засекаем время старта
    isConfigMode = true;        // Активируем флаг режима настройки
    
    logInfo("Config mode initialized via AP: %s", apName.c_str());
}

/**
 * @brief Обработка логики конфигурирования и таймера перезагрузки.
 */
void handleConfigMode() {
    if (!isConfigMode) return; // Если мы не в режиме настройки — выходим

    uint32_t currentMillis = millis();
    uint32_t elapsed = currentMillis - configStartTime;

    // Проверка таймаута
    if (elapsed >= configTimeout) {
        logInfo("Timeout. Restarting...");
        update_screen_status("Время вышло!\nПерезагрузка...");
        delay(2000);
        ESP.restart();
    }

    // Обновление экрана раз в секунду
    if (currentMillis - lastDisplayUpdate >= 1000) {
        lastDisplayUpdate = currentMillis;

        uint32_t remaining = (configTimeout - elapsed) / 1000;
        uint32_t m = remaining / 60;
        uint32_t s = remaining % 60;

        char msg[128];
        snprintf(msg, sizeof(msg), 
                 "НАСТРОЙКА\nСеть: %s\nIP: 192.168.4.1\n\nПерезагрузка через\n%u:%02u", 
                 apName.c_str(), m, s);
        update_screen_status(msg);
    }

    // 3. Обслуживание сетевых сервисов
    dnsServer.processNextRequest();
    server.handleClient();
}

#endif // WIFI_LOGIC_H