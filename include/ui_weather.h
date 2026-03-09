#ifndef UI_WEATHER_H
#define UI_WEATHER_H

#include <lvgl.h>

/**
 * @brief Структура для сопоставления WMO-кода с иконками LVGL.
 */
struct WeatherIconMap {
    int code;                      // WMO Weather Code
    const lv_img_dsc_t *day_img;   // Указатель на дневную иконку
    const lv_img_dsc_t *night_img; // Указатель на ночную иконку
};

// Объявления для внешнего доступа к массиву и его размеру
extern const WeatherIconMap weather_icons[];
extern const int weather_icons_count;

#endif // UI_WEATHER_H