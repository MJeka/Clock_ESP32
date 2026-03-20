/**
 * @brief Версия прошивки
 */
#ifndef BUILD_VERSION
#define BUILD_VERSION "unknown"
#endif

#include "provider_om.h" // Добавляем новый заголовочный файл
#include "secrets.h" // Конфиденциальные данные и макроопределения (API-ключи, адреса NTP)
#include "ui.h" // Объявления объектов графического интерфейса (экспорт из SquareLine Studio)
#include "ui_weather.h" // Карта соответствия кодов погоды и иконок для UI
#include "wifi_logic.h" // Логика сетевых подключений и обработчиков веб-сервера
#include <ArduinoOTA.h> // Обязательно для работы метода ArduinoOTA.handle() в loop
#include <TFT_eSPI.h> // Графический драйвер нижнего уровня (инициализация и управление дисплеем)
#include <WiFi.h> // Сетевой стек 802.11 (управление радиомодулем, режимы STA и AP)
#include <lvgl.h> // Движок графического интерфейса пользователя (UI Engine)

// =============================================================================
// ПРОТОТИПЫ ФУНКЦИЙ
// =============================================================================
// void update_ui_elements(bool force = false); // Удалено, так как объявлено в
// wifi_logic.h
void update_weather_icon(int wmo_code, int is_day);

// =============================================================================
// ГЛОБАЛЬНЫЕ ОБЪЕКТЫ
// =============================================================================
TFT_eSPI tft = TFT_eSPI();
Preferences preferences;
WebServer server(80);
DNSServer dnsServer;

// Глобальный объект провайдера
ProviderOM weather_provider;

// =============================================================================
// ПЕРЕМЕННЫЕ ДАННЫХ
// =============================================================================
char ssid[32] = "";
char password[64] = "";
String apName;
char weather_city[64] = "";
lv_obj_t *load_label = nullptr;
static int last_wifi_status = -1; // Храним предыдущее состояние Wi-Fi для оптимизации обновлений UI
uint32_t lastReconnectAttempt = 0; // Время последней попытки подключения к Wi-Fi для управления интервалом повторных попыток

// =============================================================================
// ТАЙМЕРЫ
// =============================================================================
unsigned long lastUpdateTime = 0;
unsigned long lastWeatherCheck = 0;
const unsigned long weatherInterval = 15 * 60 * 1000; // 15 мин * 60 сек * 1000 мс = 900 000 мс
uint32_t configTimeout = 15 * 60 * 1000;  // Таймер автоперезагрузки в режиме точки доступа 15 мин * 60 сек * 1000 мс = 900 000 мс
bool isConfigMode = false;    // Флаг активного режима настройки
uint32_t configStartTime = 0; // Время запуска режима AP
uint32_t lastDisplayUpdate = 0; // Время последнего обновления экрана
const uint32_t reconnectInterval = 30 * 1000; // Интервал повторной попытки подключения к Wi-Fi (30 секунд)

/**
 * @section DISPLAY_BRIGHTNESS_SETTINGS
 * Конфигурация управления яркостью дисплея и параметры ночного режима.
 */
const uint8_t ledPin = 22; /**< Пин управления подсветкой (PWM) */
const uint32_t ledFreq = 5000; /**< Частота ШИМ (5 кГц достаточно для отсутствия мерцания) */
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
int current_wmo_code = -1;  /**< WMO-код состояния погоды для отображения в Web UI */
String last_weather_update = "---"; /**< Дата и время последнего успешного обновления погоды */
float weather_lat = 50.4501; // По умолчанию Киева
float weather_lon = 30.5234;

// Локализация
const char *days_ru[] = {"ВС", "ПН", "ВТ", "СР", "ЧТ", "ПТ", "СБ"};

static lv_disp_draw_buf_t draw_buf;
// reduce draw buffer height to save DRAM (was 30 rows)
static lv_color_t buf[320 * 20];

// =============================================================================
// ОБНОВЛЕНИЕ ПРОШИВКИ
// =============================================================================

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
 * @brief Обновляет иконку в интерфейсе на основе кода от Open-Meteo.
 * Если код не найден в справочнике, иконка скрывается.
 * @param wmo_code Код погоды (WMO)
 * @param is_day Флаг времени суток (1 - день, 0 - ночь)
 */
void update_weather_icon(int wmo_code, int is_day) {
  // Проверка на валидность указателей перед выполнением операций
  if (ui_uiLabelWeather == nullptr)
    return;

  // Изначально устанавливаем указатель в nullptr (вместо картинки по умолчанию)
  const lv_img_dsc_t *target_img = nullptr;

  // Приведение целочисленного флага к логическому значению для однозначности
  // сравнения
  bool day_mode = (is_day != 0);

  // Поиск соответствия в справочнике weather_icons
  for (int i = 0; i < weather_icons_count; i++) {
    if (wmo_code == weather_icons[i].code) {
      target_img = day_mode ? weather_icons[i].day_img : weather_icons[i].night_img;
      break;
    }
  }

  // Проверка: нашли ли мы подходящую иконку
  if (target_img != nullptr) {
    // Если иконка найдена, устанавливаем источник и делаем объект видимым
    lv_img_set_src(ui_uiLabelWeather, target_img);
    lv_obj_clear_flag(ui_uiLabelWeather, LV_OBJ_FLAG_HIDDEN);

    // Логирование успешной установки
    logInfo("UI_RENDER: Applied icon source for code: %d (Day: %d)\n", wmo_code,
            is_day);
  } else {
    // Если код не распознан, скрываем объект с экрана
    lv_obj_add_flag(ui_uiLabelWeather, LV_OBJ_FLAG_HIDDEN);

    // Логирование отсутствия данных
    logInfo("UI_RENDER: Weather code %d not found. Hiding object.\n", wmo_code);
  }

  // Тонирование иконки в более теплый цвет (закомментировано, так как требует
  // наличия картинки) if (target_img != nullptr) {
  //   lv_obj_set_style_img_recolor(ui_uiLabelWeather, lv_color_hex(0xFFA500),
  //   0); // Оранжевый lv_obj_set_style_img_recolor_opa(ui_uiLabelWeather, 120,
  //   0); // Легкое тонирование
  // }
}

/**
 * @brief Запрос данных о погоде через провайдер и обновление интерфейса.
 */
void fetch_weather() {
  // Проверка статуса сетевого соединения перед выполнением запроса
    if (WiFi.status() != WL_CONNECTED) return;

    logInfo("Weather update request for %s (%.4f, %.4f)", weather_city, weather_lat, weather_lon);

  // Запрос данных через провайдер
  weather_data data = weather_provider.fetch_current(weather_lat, weather_lon);

  if (data.is_valid) {
    // Обновление глобальных переменных состояния
    current_temp = data.temperature;
    current_humidity = data.humidity;
    current_pressure = data.pressure_mm;
    current_wmo_code = data.wmo_code; // Сохранение кода погоды для веба
    
    // Фиксация времени последнего обновления (Дата и время)
    struct tm ti;
    if (getLocalTime(&ti)) {
      char buf[32];
      snprintf(buf, sizeof(buf), "%02d.%02d.%d %02d:%02d:%02d", 
               ti.tm_mday, ti.tm_mon + 1, ti.tm_year + 1900,
               ti.tm_hour, ti.tm_min, ti.tm_sec);
      last_weather_update = String(buf);
    }

    // Обновление иконки и интерфейса
    update_weather_icon(data.wmo_code, data.is_day);

        logInfo("Weather updated: %.1f C, Hum: %d%%, Pres: %d mm, Code: %d, IsDay: %d",
                current_temp, current_humidity, current_pressure, data.wmo_code, data.is_day);
  } else {
    logInfo("Weather error: Provider failed to fetch data");
  }
}

// =============================================================================
// ОБНОВЛЕНИЕ ИНТЕРФЕЙСА
// =============================================================================

/**
 * @brief Обновление данных в элементах SquareLine UI.
 * @param force Если true, обновление выполняется немедленно, игнорируя проверку
 * минуты.
 */
void update_ui_elements(bool force) {
  struct tm timeinfo;
  static char buf_tmp[32];
  static int last_drawn_min = -1;
  static int last_drawn_day = -1;

  // Попытка получения локального времени из системного стека
  if (getLocalTime(&timeinfo)) {

    // Выполнение логики только при смене минуты для минимизации нагрузки на CPU
    if (force || timeinfo.tm_min != last_drawn_min) {

      // Инкапсуляция логики управления яркостью (Night Mode)
      check_brightness(&timeinfo);

      // Форматирование и вывод текущего времени
      strftime(buf_tmp, sizeof(buf_tmp), "%H:%M", &timeinfo);
      /**
       * lv_label_set_text выполняет внутреннюю проверку на идентичность строк.
       * Инвалидация объекта произойдет только при фактическом изменении текста.
       */
      lv_label_set_text(ui_uiLabelTime1, buf_tmp);

      // Логика обновления данных, зависящих от даты
      if (timeinfo.tm_mday != last_drawn_day) {
        logInfo("Date updated: %d.%02d", timeinfo.tm_mday, timeinfo.tm_mon + 1);

        // Форматирование полной даты
        strftime(buf_tmp, sizeof(buf_tmp), "%d.%m.%Y", &timeinfo);
        lv_label_set_text(ui_uiLabelDate1, buf_tmp);

        // Установка дня недели из локализованного массива
        lv_label_set_text(ui_uiLabelDay1, days_ru[timeinfo.tm_wday]);

        last_drawn_day = timeinfo.tm_mday;

        // Инвалидация всего экрана необходима только при глобальной смене даты
        lv_obj_invalidate(lv_scr_act());
      }

      logInfo("UI Updated for: %s", buf_tmp);

      // Обновление метеоданных: Температура, Влажность, Давление
      // Использование dtostrf для корректного преобразования float
      /**
       * Использование статического буфера buf_tmp здесь безопасно,
       * так как данные записываются в виджеты последовательно до выхода из
       * контекста функции.
       */
      dtostrf(current_temp, 4, 1, buf_tmp);
      lv_label_set_text_fmt(ui_uiLabelTemp1, "%s °C", buf_tmp);
      lv_label_set_text_fmt(ui_uiLabelHumidity1, "%d %%", current_humidity);
      // lv_label_set_text_fmt(ui_uiLabelPressure1, "%d mm", current_pressure);

      // Принудительный запуск цикла отрисовки LVGL для немедленного отображения
      // изменений
      /**
       * lv_refr_now гарантирует, что пользователь увидит обновление
       * времени и погоды одновременно, исключая разрыв кадров между обновлением
       * разных меток.
       */
      lv_refr_now(NULL);
      last_drawn_min = timeinfo.tm_min;
    }
  } else {
    // Регистрация сбоя получения времени (вероятная проблема синхронизации NTP)
    logInfo("Time error: getLocalTime failed");
  }
}

/**
 * @brief Проверяет текущее время и корректирует яркость подсветки дисплея.
 */
void check_brightness(struct tm *timeinfo) {
  struct tm timeinfo_local;
  if (timeinfo == nullptr) {
    if (!getLocalTime(&timeinfo_local))
      return;
    timeinfo = &timeinfo_local;
  }

  int currentHour = timeinfo->tm_hour;
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

    logInfo("[SYSTEM] Brightness updated to %d (Mode: %s)\n", targetBrightness,
            isNight ? "NIGHT" : "DAY");
  }
}

/**
 * @brief Проверка статуса Wi-Fi и управление индикатором.
 */
void handle_wifi_status() {
  int current_wifi_status = WiFi.status();
  if (current_wifi_status != last_wifi_status) {

    // Визуальное обновление статуса Wi-Fi в интерфейсе (зеленый для
    // подключения, красный для отключения)
    if (current_wifi_status == WL_CONNECTED) {
      lv_obj_set_style_bg_color(ui_WiFiStatus, lv_color_hex(0x02C112),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
      logInfo("WiFi Status: Connected. Indicator Green.");
    } else {
      lv_obj_set_style_bg_color(ui_WiFiStatus, lv_color_hex(0xFF0000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
      logInfo("WiFi Status: Disconnected. Indicator Red.");
    }

    /**
     * Принудительное обновление интерфейса при смене статуса сети,
     * чтобы индикатор изменился мгновенно, не дожидаясь начала новой минуты.
     */
    update_ui_elements();
    last_wifi_status = current_wifi_status;
  }
}

/**
 * @brief Обработка сетевых задач: погода и переподключение.
 */
void handle_network_tasks(uint32_t now, int current_wifi_status) {
  /**
   * Разграничение логики работы устройства в зависимости от статуса соединения.
   * Использование вложенных условий исключает избыточные проверки
   * и гарантирует атомарность выполнения операций в рамках одного состояния.
   */
  if (!isConfigMode) {
    if (current_wifi_status == WL_CONNECTED) {
      // Если Wi-Fi подключен — здесь может работать ваша основная логика

      // Данные: Запрос погоды по заданному интервалу
      if (now - lastWeatherCheck > weatherInterval) {
        fetch_weather();

        // Обновляем экран сразу после получения новых данных о погоде.
        update_ui_elements(true);
        lastWeatherCheck = now;
      }

      // Здесь можно вызвать syncTime() или другие сетевые службы
    } else {
      // Логика автоматического переподключения
      /**
       * Если соединение потеряно и мы не в режиме AP, инициируем попытку
       * переподключения по неблокирующему таймеру. Использование WiFi.begin()
       * без параметров заставляет ESP использовать последние сохраненные
       * учетные данные из Flash-памяти.
       */
      if (now - lastReconnectAttempt > reconnectInterval) {
        logInfo("Attempting to reconnect to WiFi...");
        WiFi.begin(); // Использует сохраненные SSID и пароль
        lastReconnectAttempt = now;
      }
    }
  }
}

/**
 * @brief Загрузка пользовательских настроек из энергонезависимой памяти.
 */
void load_system_preferences() {
  preferences.begin("wifi-config", true);
  strlcpy(ssid, preferences.getString("ssid", "").c_str(), sizeof(ssid));
  strlcpy(password, preferences.getString("pass", "").c_str(), sizeof(password));
  strlcpy(weather_city, preferences.getString("city", city).c_str(), sizeof(weather_city));

  weather_lat = preferences.getFloat("lat", weather_lat);
  weather_lon = preferences.getFloat("lon", weather_lon);

  dayBrightness = preferences.getInt("day_br", 255);
  nightBrightness = preferences.getInt("night_br", 20);
  nightStartHour = preferences.getInt("n_start", 22);
  nightEndHour = preferences.getInt("n_end", 7);

  preferences.end();
}

/**
 * @brief Базовая настройка дисплея и графической библиотеки LVGL.
 */
void init_display_subsystem() {
  tft.begin();
  tft.setRotation(3);
  tft.setSwapBytes(true);

  /**
   * Инициализация аппаратного ШИМ для управления подсветкой.
   */
  ledcAttach(ledPin, ledFreq, ledRes);
  ledcWrite(ledPin, dayBrightness); // Установка начальной яркости

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, 320 * 20);
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 320;
  disp_drv.ver_res = 240;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
}

/**
 * @brief Создание и настройка загрузочного экрана (Splash Screen).
 */
void create_boot_screen() {
  lv_obj_t *top_layer = lv_layer_top();
  lv_obj_set_style_bg_opa(top_layer, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(top_layer, lv_color_hex(0x000000), 0);

  load_label = lv_label_create(top_layer);
  lv_obj_set_style_text_color(load_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(load_label, &ui_font_roboto20, 0);
  lv_obj_set_style_text_align(load_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(load_label, LV_ALIGN_CENTER, 0, 0);
}

/**
 * @brief Цикл ожидания подключения к WiFi с визуализацией прогресса.
 */
void wait_for_wifi() {
  update_screen_status("Поиск сети...\n\n\nv. " BUILD_VERSION);
  int wait_retry = 0;
  int dot_count = 0;
  while (WiFi.status() != WL_CONNECTED && wait_retry < 30) {
    String dots = "";
    for (int i = 0; i < dot_count; i++) dots += ".";

    char msg[128];
    snprintf(msg, sizeof(msg), "Подключение к\n%s%s\n\n\nv. %s", ssid, dots.c_str(), BUILD_VERSION);
    update_screen_status(msg);

    dot_count = (dot_count + 1) % 4;
    wait_retry++;

    // Принудительная задержка для анимации точек
    for (int i = 0; i < 5; i++) {
      lv_timer_handler();
      delay(100);
      yield();
    }
  }
}

/**
 * @brief Синхронизация времени через NTP серверы.
 */
void sync_system_time(const char *ip_str) {
  char msg[128];
  snprintf(msg, sizeof(msg), "Синхронизация времени...\nIP: %s\n\n\nv. %s", ip_str, BUILD_VERSION);
  update_screen_status(msg);

  configTzTime(TZ_INFO, ntpServer, ntpServer2);

  int ntp_retry = 0;
  struct tm ti;
  while (!getLocalTime(&ti) && ntp_retry < 10) {
    ArduinoOTA.handle(); // Позволит прошить устройство, даже если NTP завис
    lv_timer_handler();
    delay(500);
    ntp_retry++;
  }
}

/**
 * @brief Финальная очистка загрузочного слоя и открытие основного интерфейса.
 */
void finalize_ui_startup(const char *ip_str) {
  char msg[128];
  snprintf(msg, sizeof(msg), "Система готова!\nIP: %s\n\n\nv. %s", ip_str, BUILD_VERSION);
  update_screen_status(msg);
  delay(2000);

  // --- КРИТИЧЕСКИЙ БЛОК ОЧИСТКИ ЗАСТАВКИ ---
  if (load_label != nullptr) {
    lv_obj_del(load_label);
    load_label = nullptr;
  }

  lv_obj_t *top_layer = lv_layer_top();
  lv_obj_set_style_bg_opa(top_layer, LV_OPA_TRANSP, 0);
  lv_obj_add_flag(top_layer, LV_OBJ_FLAG_HIDDEN);

  lv_obj_invalidate(lv_scr_act());
  lv_timer_handler();
  lv_refr_now(NULL);

  logInfo("System Ready and Interface Visible!");
}

// =============================================================================
// ИНИЦИАЛИЗАЦИЯ (SETUP)
// =============================================================================
void setup() {
  // Инициализация аппаратного Serial-порта для отладки
  Serial.begin(115200);
  delay(500); // Для стабилизации Serial
  logInfo("[SYSTEM] Firmware Version: " BUILD_VERSION);

  // Загрузка конфигурации из памяти
  load_system_preferences();

  // Инициализация аппаратной части и LVGL
  init_display_subsystem();

  // Создание черного слоя заставки
  create_boot_screen();

  update_screen_status("Инициализация...\n\n\nv. " BUILD_VERSION);
  ui_init(); // Загрузка интерфейса SquareLine под черным слоем
  delay(1000);

  // ПРОВЕРКА: Если настройки WiFi отсутствуют, сразу переходим в режим точки
  // доступа
  if (strlen(ssid) == 0) {
    logInfo("No WiFi settings found. Starting AP mode immediately.");
    update_screen_status( "Настройки не найдены\nЗапуск точки доступа...\n\n\nv. " BUILD_VERSION);
    delay(2000);

    generateAPName(); // Генерация уникального имени точки доступа на основе MAC-адреса
    setupWebHandlers(); // Настройка обработчиков веб-сервера для режима AP
    startConfigMode(); // Запуск режима точки доступа и веб-сервера для настройки
    return; // Прекращаем выполнение setup, так как мы ушли в режим настройки
  }

  // Попытка подключения WiFi (если SSID найден в памяти)
  if (strlen(ssid) > 0) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    logInfo("WiFi connect initiated for %s", ssid);
  }

  // Ожидание завершения подключения WiFi
  wait_for_wifi();

  // РАЗВИЛКА: Запуск или Режим точки доступа
  if (WiFi.status() == WL_CONNECTED) {
    IPAddress ip = WiFi.localIP();
    char ip_str[20];
    strncpy(ip_str, ip.toString().c_str(), sizeof(ip_str));

    char msg[128];
    snprintf(msg, sizeof(msg), "Сеть подключена!\nIP: %s\n\n\nv. %s", ip_str, BUILD_VERSION);
    logInfo("IP - %s", ip_str);
    update_screen_status(msg);
    delay(1000);

    // Инициализация сетевых служб
    setupOTA();
    server.begin();
    setupWebHandlers();

    // Синхронизация времени
    sync_system_time(ip_str);
    delay(1000);

    // Обновление погоды
    snprintf(msg, sizeof(msg), "Обновление погоды...\nIP: %s\n\n\nv. %s", ip_str, BUILD_VERSION);
    update_screen_status(msg);
    fetch_weather();
    delay(1000);

    // Заполняем интерфейс данными перед открытием
    update_ui_elements();

    // Завершение работы заставки и показ основного UI
    finalize_ui_startup(ip_str);

  } else {
    // Если WiFi не найден — уходим в режим настройки
    update_screen_status("Ошибка WiFi!\nРежим настройки...\n\nv. " BUILD_VERSION);
    WiFi.disconnect(true);
    delay(2000);

    generateAPName(); // Генерация уникального имени точки доступа на основе MAC-адреса
    setupWebHandlers(); // Настройка обработчиков веб-сервера для режима AP
    startConfigMode(); // Запуск режима точки доступа и веб-сервера для настройки
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

  // Если активен режим настройки — работает эта функция
  handleConfigMode();

  // Получаем текущее системное время в миллисекундах для неблокирующих таймеров
  uint32_t now = millis();

  // Проверка статуса Wi-Fi и управление индикатором
  handle_wifi_status();

  // Выполнение сетевых задач (погода, реконнект)
  /**
   * Передаем текущий статус и время в функцию задач, чтобы не вызывать
   * WiFi.status() повторно, экономя ресурсы.
   */
  handle_network_tasks(now, last_wifi_status);

  /**
   * Интерфейс: Опрос системного времени каждую секунду.
   * Мы сохраняем частоту опроса в 1с, чтобы гарантировать точность часов
   * до секунды, но сама функция update_ui_elements внутри себя выполнит
   * отрисовку (lv_label_set_text) только при фактической смене минуты.
   */
  if (now - lastUpdateTime > 1000) {
    update_ui_elements();
    lastUpdateTime = now;
  }

  // Графика: Вызов обработчика таймеров и отрисовки LVGL
  lv_timer_handler();

  // Система: Маленькая пауза для стабильности Wi-Fi стека и разгрузки процессора
  delay(5);
}