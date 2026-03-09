#ifndef WEATHER_TYPES_H
#define WEATHER_TYPES_H

/**
 * @brief Структура для унифицированного обмена погодными данными.
 * Позволяет отвязать логику парсинга конкретного API от отображения UI.
 */
struct weather_data {
    float temperature;    // Текущая температура в градусах Цельсия
    int humidity;         // Относительная влажность в процентах
    int pressure_mm;      // Давление, приведенное к мм рт. ст.
    int wmo_code;         // Стандартизированный WMO-код состояния погоды
    bool is_day;          // Флаг времени суток (true - день, false - ночь)
    bool is_valid;        // Флаг успешности десериализации данных
};

#endif // WEATHER_TYPES_H