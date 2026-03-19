#include "ui_weather.h"
#include "ui.h" // Объявления объектов графического интерфейса (экспорт из SquareLine Studio)

/**
 * @brief Карта соответствия кодов Open-Meteo (WMO) и иконок.
 * Используются раздельные указатели для дневного и ночного режимов.
 */
// Массив соответствия кодов погоды Open-Meteo ресурсам LVGL
const WeatherIconMap weather_icons[] = {
    {0,  &ui_img_00d_64_png, &ui_img_00n_64_png}, // Ясно: day - clear-day, night - clear-night
    {1,  &ui_img_01d_64_png, &ui_img_01n_64_png}, // Преимущественно ясно: day - partly-cloudy-day, night - partly-cloudy-night
    {2,  &ui_img_02_64_png, &ui_img_02_64_png}, // Переменная облачность: day - partly-cloudy-day, night - partly-cloudy-night
    {3,  &ui_img_03_64_png, &ui_img_03_64_png}, // Пасмурно: day - overcast-day, night - overcast-night
    {45, &ui_img_45_64_png, &ui_img_45_64_png},  // Туман: day - fog-day, night - fog-night
    {48, &ui_img_48_64_png, &ui_img_48_64_png},  // Оседающий иней: day - fog-day, night - fog-night
    {51, &ui_img_51_63d_64_png, &ui_img_51_63n_64_png, // Легкая морось: day - partly-cloudy-day-drizzle, night - partly-cloudy-night-drizzle
    {53, &ui_img_51_63d_64_png, &ui_img_51_63n_64_png}, // Умеренная морось: использование универсальной иконки drizzle
    {55, &ui_img_51_63d_64_png, &ui_img_51_63n_64_png}, // Плотная морось: использование универсальной иконки drizzle
    {61, &ui_img_51_63d_64_png, &ui_img_51_63n_64_png}, // Небольшой дождь: day - partly-cloudy-day-rain, night - partly-cloudy-night-rain
    {63, &ui_img_51_63d_64_png, &ui_img_51_63n_64_png},   // Умеренный дождь: использование универсальной иконки rain
    {65, &ui_img_65_64_png,  &ui_img_65_64_png},  // Сильный дождь: использование иконки extreme-rain
    {66, &ui_img_66_67_96_99_64_png, &ui_img_66_67_96_99_64_png},   // Легкий ледяной дождь: использование универсальной иконки sleet
    {67, &ui_img_66_67_96_99_64_png, &ui_img_66_67_96_99_64_png},   // Плотный ледяной дождь: использование универсальной иконки sleet
    {71, &ui_img_71d_64_png, &ui_img_71n_64_png},  // Небольшой снегопад: day - partly-cloudy-day-snow, night - partly-cloudy-night-snow
    {73, &ui_img_73_77_64_png, &ui_img_73_77_64_png},   // Умеренный снегопад: использование универсальной иконки snow
    {75, &ui_img_73_77_64_png, &ui_img_73_77_64_png}, // Сильный снегопад: использование иконки extreme-snow
    {77, &ui_img_73_77_64_png, &ui_img_73_77_64_png},   // Снежные зерна: использование универсальной иконки snow
    {80, &ui_img_80_82_64_png, &ui_img_80_82_64_png}, // Слабый ливневый дождь: day - partly-cloudy-day-rain, night - partly-cloudy-night-rain
    {81, &ui_img_80_82_64_png, &ui_img_80_82_64_png},   // Умеренный ливневый дождь: использование универсальной иконки rain
    {82, &ui_img_80_82_64_png, &ui_img_80_82_64_png},  // Сильный ливневый дождь: использование иконки extreme-rain
    {85, &ui_img_85_86_64_png, &ui_img_85_86_64_png},  // Небольшой снежный ливень: day - partly-cloudy-day-snow, night - partly-cloudy-night-snow
    {86, &ui_img_85_86_64_png, &ui_img_85_86_64_png}, // Сильный снежный ливень: использование иконки extreme-snow
    {95, &ui_img_95_64_png, &ui_img_95_64_png}, // Гроза: day - thunderstorms-day, night - thunderstorms-night
    {96, &ui_img_66_67_96_99_64_png, &ui_img_66_67_96_99_64_png},  // Гроза со слабым градом: day - thunderstorms-day-rain, night - thunderstorms-night-rain
    {99, &ui_img_66_67_96_99_64_png, &ui_img_66_67_96_99_64_png}   // Гроза с сильным градом: day - thunderstorms-extreme-day-rain, night - thunderstorms-extreme-night-rain
};

// Вычисляем размер массива автоматически, чтобы не хардкодить число иконок
const int weather_icons_count = sizeof(weather_icons) / sizeof(WeatherIconMap);
