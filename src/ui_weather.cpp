#include "ui_weather.h"
#include "ui.h" // Объявления объектов графического интерфейса (экспорт из SquareLine Studio)

/**
 * @brief Карта соответствия кодов Open-Meteo (WMO) и иконок.
 * Используются раздельные указатели для дневного и ночного режимов.
 */
// Массив соответствия кодов погоды Open-Meteo ресурсам LVGL
const WeatherIconMap weather_icons[] = {
    {0,  &ui_img_1142085373, &ui_img_1774994173}, // Ясно: day - clear-day, night - clear-night
    {1,  &ui_img_860055119,  &ui_img_1722154033}, // Преимущественно ясно: day - partly-cloudy-day, night - partly-cloudy-night
    {2,  &ui_img_860055119,  &ui_img_1722154033}, // Переменная облачность: day - partly-cloudy-day, night - partly-cloudy-night
    {3,  &ui_img_1190589243, &ui_img_1009007355}, // Пасмурно: day - overcast-day, night - overcast-night
    {45, &ui_img_459294810,  &ui_img_834737478},  // Туман: day - fog-day, night - fog-night
    {48, &ui_img_459294810,  &ui_img_834737478},  // Оседающий иней: day - fog-day, night - fog-night
    {51, &ui_img_1937960972, &ui_img_1618810380}, // Легкая морось: day - partly-cloudy-day-drizzle, night - partly-cloudy-night-drizzle
    {53, &ui_img_drizzle_png, &ui_img_drizzle_png}, // Умеренная морось: использование универсальной иконки drizzle
    {55, &ui_img_drizzle_png, &ui_img_drizzle_png}, // Плотная морось: использование универсальной иконки drizzle
    {61, &ui_img_1181745046, &ui_img_2139431338}, // Небольшой дождь: day - partly-cloudy-day-rain, night - partly-cloudy-night-rain
    {63, &ui_img_rain_png,   &ui_img_rain_png},   // Умеренный дождь: использование универсальной иконки rain
    {65, &ui_img_102872400,  &ui_img_102872400},  // Сильный дождь: использование иконки extreme-rain
    {66, &ui_img_sleet_png, &ui_img_sleet_png},   // Легкий ледяной дождь: использование универсальной иконки sleet
    {67, &ui_img_sleet_png, &ui_img_sleet_png},   // Плотный ледяной дождь: использование универсальной иконки sleet
    {71, &ui_img_454646321, &ui_img_544304527},  // Небольшой снегопад: day - partly-cloudy-day-snow, night - partly-cloudy-night-snow
    {73, &ui_img_snow_png,  &ui_img_snow_png},   // Умеренный снегопад: использование универсальной иконки snow
    {75, &ui_img_1533765271, &ui_img_1533765271}, // Сильный снегопад: использование иконки extreme-snow
    {77, &ui_img_snow_png,  &ui_img_snow_png},   // Снежные зерна: использование универсальной иконки snow
    {80, &ui_img_1181745046, &ui_img_2139431338}, // Слабый ливневый дождь: day - partly-cloudy-day-rain, night - partly-cloudy-night-rain
    {81, &ui_img_rain_png,   &ui_img_rain_png},   // Умеренный ливневый дождь: использование универсальной иконки rain
    {82, &ui_img_102872400,  &ui_img_102872400},  // Сильный ливневый дождь: использование иконки extreme-rain
    {85, &ui_img_454646321, &ui_img_544304527},  // Небольшой снежный ливень: day - partly-cloudy-day-snow, night - partly-cloudy-night-snow
    {86, &ui_img_1533765271, &ui_img_1533765271}, // Сильный снежный ливень: использование иконки extreme-snow
    {95, &ui_img_1041458778, &ui_img_1963032070}, // Гроза: day - thunderstorms-day, night - thunderstorms-night
    {96, &ui_img_980277765,  &ui_img_235723173},  // Гроза со слабым градом: day - thunderstorms-day-rain, night - thunderstorms-night-rain
    {99, &ui_img_1589909526, &ui_img_683190538}   // Гроза с сильным градом: day - thunderstorms-extreme-day-rain, night - thunderstorms-extreme-night-rain
};

// Вычисляем размер массива автоматически, чтобы не хардкодить число иконок
const int weather_icons_count = sizeof(weather_icons) / sizeof(WeatherIconMap);
