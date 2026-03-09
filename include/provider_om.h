#ifndef PROVIDER_OM_H
#define PROVIDER_OM_H

#include "weather_provider.h"
#include <HTTPClient.h>

/**
 * @brief Класс адаптера для сервиса Open-Meteo.
 */
class ProviderOM : public WeatherProvider {
public:
    weather_data fetch_current(float lat, float lon) override;
};

#endif // PROVIDER_OM_H