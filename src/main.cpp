#include "secrets.h" // Конфиденциальные данные и макроопределения (API-ключи, адреса NTP)
#include "ui.h" // Объявления объектов графического интерфейса (экспорт из SquareLine Studio)
#include "wifi_logic.h" // Логика сетевых подключений и обработчиков веб-сервера
#include <ArduinoJson.h> // Десериализация JSON-структур (обработка ответов погодного API)
#include <ArduinoOTA.h> // Обязательно для работы метода ArduinoOTA.handle() в loop
#include <FS.h> // Абстрактный слой файловой системы (интерфейс доступа к Flash-памяти)
#include <HTTPClient.h> // Протоколы клиент-серверного взаимодействия (реализация HTTP-запросов)
#include <TFT_eSPI.h> // Графический драйвер нижнего уровня (инициализация и управление дисплеем)
#include <WiFi.h> // Сетевой стек 802.11 (управление радиомодулем, режимы STA и AP)
#include <lvgl.h> // Движок графического интерфейса пользователя (UI Engine)

// =============================================================================
// ПРОТОТИПЫ ФУНКЦИЙ
// =============================================================================
void check_brightness(struct tm &timeinfo);

// =============================================================================
// ГЛОБАЛЬНЫЕ ОБЪЕКТЫ
// =============================================================================
TFT_eSPI tft = TFT_eSPI();
Preferences preferences;
WebServer server(80);
DNSServer dnsServer;

// =============================================================================
// ПЕРЕМЕННЫЕ ДАННЫХ
// =============================================================================
char ssid[32] = "";
char password[64] = "";
String apName;
char weather_city[64] = "";
lv_obj_t *load_label = nullptr;

// =============================================================================
// ТАЙМЕРЫ
// =============================================================================
unsigned long lastUpdateTime = 0;
unsigned long lastWeatherCheck = 0;
const unsigned long weatherInterval = 30 * 60 * 1000;

/**
 * @section DISPLAY_BRIGHTNESS_SETTINGS
 * Конфигурация управления яркостью дисплея и параметры ночного режима.
 */
const uint8_t ledPin = 22; /**< Пин управления подсветкой (PWM) */
const uint32_t ledFreq =
    5000; /**< Частота ШИМ (5 кГц достаточно для отсутствия мерцания) */
const uint8_t ledRes = 8; /**< Разрядность ШИМ (8 бит: 0-255) */

// Параметры режима (будут загружаться из Preferences)
int dayBrightness = 255;  /**< Яркость в дневное время (0-255) */
int nightBrightness = 20; /**< Яркость в ночное время (0-255) */
int nightStartHour = 22;  /**< Час перехода в ночной режим (0-23) */
int nightEndHour = 7;     /**< Час возврата в дневной режим (0-23) */

// Состояние погоды
float current_temp = 0.0;
int current_humidity = 0;
int current_pressure = 0;

// Локализация
const char *days_ru[] = {"ВС", "ПН", "ВТ", "СР", "ЧТ", "ПТ", "СБ"};
const char *months_ru[] = {"января",   "февраля", "марта",  "апреля",
                           "мая",      "июня",    "июля",   "августа",
                           "сентября", "октября", "ноября", "декабря"};

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[320 * 20];

// =============================================================================
// ПОГОДА
// =============================================================================

/**
 * @brief Структура для сопоставления текстового кода API с ресурсом LVGL.
 */
struct WeatherIconMap {
  const char *code;        // ID иконки (например, "01d")
  const lv_img_dsc_t *img; // Указатель на структуру изображения из ui.h
};

/**
 * @brief Карта соответствия всех иконок OpenWeather и сгенерированных
 * SquareLine имен. Мы используем указатели (&), чтобы не копировать тяжелые
 * данные, а просто ссылаться на них.
 */
const WeatherIconMap weather_icons[] = {
    {"01d", &ui_img_1700430416}, // Ясно (день)
    {"01n", &ui_img_1232099950}, // Ясно (ночь)
    {"02d", &ui_img_1706357903}, // Малооблачно (день)
    {"02n", &ui_img_1238027437}, // Малооблачно (ночь)
    {"03d", &ui_img_1707406414}, // Облачно (день)
    {"03n", &ui_img_1239075948}, // Облачно (ночь)
    {"04d", &ui_img_1694638229}, // Пасмурно (день)
    {"04n", &ui_img_1226307763}, // Пасмурно (ночь)
    {"09d", &ui_img_1690960472}, // Ливень (день)
    {"09n", &ui_img_1222630006}, // Ливень (ночь)
    {"10d", &ui_img_1131223562}, // Дождь (день)
    {"10n", &ui_img_1599554028}, // Дождь (ночь)
    {"11d", &ui_img_1130175051}, // Гроза (день)
    {"11n", &ui_img_1598505517}, // Гроза (ночь)
    {"13d", &ui_img_1123199053}, // Снег (день)
    {"13n", &ui_img_1591529519}, // Снег (ночь)
    {"50d", &ui_img_1308932194}, // Туман (день)
    {"50n", &ui_img_840601728}   // Туман (ночь)
};

// Вычисляем размер массива автоматически, чтобы не хардкодить число иконок
const int weather_icons_count = sizeof(weather_icons) / sizeof(WeatherIconMap);

// =============================================================================
// ФУНКЦИИ ОТРИСОВКИ
// =============================================================================

/**
 * @brief Отрисовка буфера LVGL на дисплей
 */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area,
                   lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();
  lv_disp_flush_ready(disp);
}

// =============================================================================
// ПОЛУЧЕНИЕ ДАННЫХ
// =============================================================================

/**
 * @brief Обновляет иконку в интерфейсе на основе кода от OpenWeatherMap.
 * @param icon_id Строка с кодом, полученная из JSON (например, "04n")
 */
void update_weather_icon(const char *icon_id) {
  // Проверка на валидность указателей перед выполнением операций
  if (icon_id == nullptr || ui_uiLabelWeather == nullptr)
    return;

  // Резервная иконка (fallback), если пришедший код отсутствует в нашей таблице
  const lv_img_dsc_t *target_img = &ui_img_1238027437;

  // Поиск соответствия в справочнике weather_icons
  for (int i = 0; i < weather_icons_count; i++) {
    if (strcmp(icon_id, weather_icons[i].code) == 0) {
      target_img = weather_icons[i].img;
      break;
    }
  }

  // Установка нового источника изображения для объекта
  lv_img_set_src(ui_uiLabelWeather, target_img);

  // Логирование в Serial для верификации работы парсера
  Serial.printf("UI_RENDER: Applied icon source for code: %s\n", icon_id);
}

/**
 * @brief Запрос данных о погоде через OpenWeatherMap API и обновление
 * глобальных переменных
 */
void fetch_weather() {
  if (WiFi.status() != WL_CONNECTED)
    return;

  HTTPClient http;
  // Формирование URL запроса с учетом города, API-ключа и локализации
  String url = "http://api.openweathermap.org/data/2.5/weather?q=" +
               String(weather_city) + "&appid=" + String(weatherApiKey) +
               "&units=metric&lang=ru";

  logInfo("Weather update request for %s", weather_city);
  http.begin(url);

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, http.getString());

    if (!error) {
      if (doc.containsKey("main")) {
        // Извлечение основных метеоданных
        current_temp = doc["main"]["temp"];
        current_humidity = doc["main"]["humidity"];
        current_pressure = (int)doc["main"]["pressure"] *
                           0.750062; // Конвертация hPa в мм рт. ст.

        // Извлечение кода иконки (например, "01d", "02n")
        const char *icon_code = doc["weather"][0]["icon"];
        if (icon_code) {
          update_weather_icon(icon_code);
        }

        logInfo("Weather updated: %.1f C, Hum: %d%%, Pres: %d mm, Icon: %s",
                current_temp, current_humidity, current_pressure,
                icon_code ? icon_code : "N/A");
      } else {
        logInfo("Weather error: 'main' block missing in JSON");
      }
    } else {
      logInfo("Weather error: JSON parse failed (%s)", error.c_str());
    }
  } else {
    logInfo("Weather error: HTTP request failed, code: %d", httpCode);
  }
  http.end();
}

// =============================================================================
// ОБНОВЛЕНИЕ ИНТЕРФЕЙСА
// =============================================================================

/**
 * @brief Обновление данных в элементах SquareLine UI
 */
void update_ui_elements() {
  struct tm timeinfo;
  static char buf_tmp[32];
  static int last_drawn_min = -1;
  static int last_drawn_day = -1; // Храним день последней отрисовки

  if (getLocalTime(&timeinfo)) {
    // 1. Проверка смены минуты (для времени)
    if (timeinfo.tm_min != last_drawn_min) {

      // Сначала проверяем и устанавливаем яркость (Night Mode Logic)
      check_brightness(timeinfo);

      // Обновляем время
      strftime(buf_tmp, sizeof(buf_tmp), "%H:%M", &timeinfo);
      lv_label_set_text(ui_uiLabelTime1, buf_tmp);
      lv_obj_invalidate(ui_uiLabelTime1);

      // 2. Проверка смены дня (для даты и календаря)
      if (timeinfo.tm_mday != last_drawn_day) {
        // Лог на английском
        logInfo("Date updated: %d.%02d", timeinfo.tm_mday, timeinfo.tm_mon + 1);

        lv_label_set_text_fmt(ui_uiLabelDate1, "%d", timeinfo.tm_mday);
        // lv_label_set_text(ui_uiLabelMonth1, months_ru[timeinfo.tm_mon]);

        strftime(buf_tmp, sizeof(buf_tmp), "%d.%m.%Y", &timeinfo);
        lv_label_set_text(ui_uiLabelMonth1, buf_tmp);

        lv_label_set_text_fmt(ui_uiLabelYear1, "%d", 1900 + timeinfo.tm_year);
        lv_label_set_text(ui_uiLabelDay1, days_ru[timeinfo.tm_wday]);
        // lv_label_set_text_fmt(ui_uiLabelDay1, "%s |",
        // days_ru[timeinfo.tm_wday]);

        lv_obj_invalidate(lv_scr_act());

        last_drawn_day = timeinfo.tm_mday;
      }

      // Лог на английском
      logInfo("UI Updated for: %s", buf_tmp);

      // --- Погода ---
      // Возвращаем вывод температуры, влажности и давления в этот блок
      dtostrf(current_temp, 4, 1, buf_tmp);
      lv_label_set_text_fmt(ui_uiLabelTemp1, "%s °C", buf_tmp);
      lv_label_set_text_fmt(ui_uiLabelHumidity1, "%d %%", current_humidity);
      lv_label_set_text_fmt(ui_uiLabelPressure1, "%d mm", current_pressure);

      // Принудительная отрисовка изменений
      lv_refr_now(NULL);
      last_drawn_min = timeinfo.tm_min;
    }
  } else {
    logInfo("Time error: getLocalTime failed");
  }
}

/**
 * @brief Проверяет текущее время и корректирует яркость подсветки дисплея.
 */
void check_brightness(struct tm &timeinfo) {
  int currentHour = timeinfo.tm_hour;
  static int lastAppliedBrightness =
      -1; // Храним состояние, чтобы не дергать ШИМ зря
  int targetBrightness;

  // Определение состояния "Ночь" с учетом возможного перехода через 00:00
  bool isNight = false;
  if (nightStartHour > nightEndHour) {
    // Пример: Старт в 22:00, Конец в 07:00
    if (currentHour >= nightStartHour || currentHour < nightEndHour)
      isNight = true;
  } else {
    // Пример: Старт в 01:00, Конец в 05:00
    if (currentHour >= nightStartHour && currentHour < nightEndHour)
      isNight = true;
  }

  // Выбор целевого уровня яркости
  targetBrightness = isNight ? nightBrightness : dayBrightness;

  // Применяем изменения только если они отличаются от текущих
  if (targetBrightness != lastAppliedBrightness) {
    ledcWrite(ledPin, targetBrightness);
    lastAppliedBrightness = targetBrightness;

    Serial.printf("[SYSTEM] Brightness updated to %d (Mode: %s)\n",
                  targetBrightness, isNight ? "NIGHT" : "DAY");
  }
}

// =============================================================================
// ИНИЦИАЛИЗАЦИЯ (SETUP)
// =============================================================================

void setup() {
  Serial.begin(115200);

  // Загрузка конфигурации из памяти
  preferences.begin("wifi-config", true);
  strlcpy(password, preferences.getString("pass", "").c_str(),
          sizeof(password));
  strlcpy(weather_city, preferences.getString("city", city).c_str(),
          sizeof(weather_city));

  dayBrightness = preferences.getInt("day_br", 255);
  nightBrightness = preferences.getInt("night_br", 20);
  nightStartHour = preferences.getInt("n_start", 22);
  nightEndHour = preferences.getInt("n_end", 7);

  preferences.end();

  // Инициализация аппаратной части
  tft.begin();
  tft.setRotation(3);
  tft.setSwapBytes(true);

  /**
   * Инициализация аппаратного ШИМ для управления подсветкой.
   */
  ledcAttach(ledPin, ledFreq, ledRes);
  ledcWrite(ledPin, dayBrightness); // Установка начальной яркости

  // Инициализация LVGL
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, 320 * 20);
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 320;
  disp_drv.ver_res = 240;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Создание черного слоя заставки
  lv_obj_t *top_layer = lv_layer_top();
  lv_obj_set_style_bg_opa(top_layer, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(top_layer, lv_color_hex(0x000000), 0);

  load_label = lv_label_create(top_layer);
  lv_obj_set_style_text_color(load_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(load_label, &ui_font_roboto24, 0);
  lv_obj_set_style_text_align(load_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(load_label, LV_ALIGN_CENTER, 0, 0);

  update_screen_status("Инициализация...");
  ui_init(); // Загрузка интерфейса SquareLine под черным слоем
  delay(1000);

  // Попытка подключения WiFi
  if (strlen(ssid) > 0) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    logInfo("WiFi connect initiated for %s", ssid);
  }

  // Ожидание завершения подключения WiFi
  update_screen_status("Поиск сети...");
  int wait_retry = 0;
  int dot_count = 0;
  while (WiFi.status() != WL_CONNECTED && wait_retry < 30) {
    String dots = "";
    for (int i = 0; i < dot_count; i++)
      dots += ".";

    char msg[64];
    snprintf(msg, sizeof(msg), "Подключение к\n%s%s", ssid, dots.c_str());
    update_screen_status(msg);

    dot_count = (dot_count + 1) % 4;
    wait_retry++;

    for (int i = 0; i < 5; i++) {
      lv_timer_handler();
      delay(100);
      yield();
    }
  }

  // РАЗВИЛКА: Успех или Режим точки доступа
  if (WiFi.status() == WL_CONNECTED) {
    update_screen_status("Сеть подключена!");
    delay(500);

    // Инициализация службы обновления по воздуху
    setupOTA();

    server.begin();
    setupWebHandlers();

    update_screen_status("Синхронизация времени...");
    configTzTime(TZ_INFO, ntpServer, ntpServer2);

    int ntp_retry = 0;
    struct tm ti;
    while (!getLocalTime(&ti) && ntp_retry < 10) {
      lv_timer_handler();
      delay(500);
      ntp_retry++;
    }
    delay(500);

    update_screen_status("Обновление погоды...");
    fetch_weather();
    delay(500);

    // Заполняем интерфейс данными перед открытием
    update_ui_elements();

    update_screen_status("Система готова!");
    delay(1000);

    // --- КРИТИЧЕСКИЙ БЛОК ОЧИСТКИ ЗАСТАВКИ ---

    // Удаляем текст статуса
    if (load_label != nullptr) {
      lv_obj_del(load_label);
      load_label = nullptr;
    }

    // Делаем верхний слой полностью прозрачным и скрываем его
    lv_obj_set_style_bg_opa(top_layer, LV_OPA_TRANSP, 0);
    lv_obj_add_flag(top_layer, LV_OBJ_FLAG_HIDDEN);

    // Принудительная перерисовка активного экрана СЕЙЧАС
    lv_obj_invalidate(lv_scr_act());
    lv_timer_handler();
    lv_refr_now(NULL);

    logInfo("System Ready and Interface Visible!");
  } else {
    // Если WiFi не найден — уходим в режим настройки
    update_screen_status("Ошибка WiFi!\nРежим настройки...");
    WiFi.disconnect(true);
    delay(1500);

    generateAPName();
    setupWebHandlers();
    startConfigMode(); // Функция с бесконечным циклом внутри
  }
}

// =============================================================================
// ОСНОВНОЙ ЦИКЛ (LOOP)
// =============================================================================

void loop() {
  // Служба OTA: Проверка входящих пакетов прошивки (вызывать максимально часто)
  ArduinoOTA.handle();

  // Сервер: Обработка запросов веб-интерфейса настроек
  server.handleClient();

  // Графика: Вызов обработчика таймеров и отрисовки LVGL
  lv_timer_handler();

  // Интерфейс: Обновление времени/даты раз в секунду
  if (millis() - lastUpdateTime > 1000) {
    update_ui_elements();
    lastUpdateTime = millis();
  }

  // Данные: Запрос погоды по заданному интервалу
  if (millis() - lastWeatherCheck > weatherInterval) {
    fetch_weather();
    lastWeatherCheck = millis();
  }

  // Система: Маленькая пауза для стабильности Wi-Fi стека и разгрузки
  // процессора
  delay(5);
}