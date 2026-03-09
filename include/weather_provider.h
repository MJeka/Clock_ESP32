#ifndef WEATHER_PROVIDER_H
#define WEATHER_PROVIDER_H

#include "weather_types.h"
#include <ArduinoJson.h> // Десериализация JSON-структур (обработка ответов погодного API)

/**
 * @brief Абстрактный базовый класс (интерфейс) для погодных провайдеров.
 */
class WeatherProvider {
public:
    virtual ~WeatherProvider() {}
    
    // Метод получения актуальных данных о погоде
    virtual weather_data fetch_current(float lat, float lon) = 0;
};

#endif // WEATHER_PROVIDER_H
