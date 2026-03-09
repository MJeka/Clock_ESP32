/**
 * @file provider_om.cpp
 * @brief Провайдер данных для сервиса Open-Meteo (open-meteo.com).
 */
#include "provider_om.h"

/**
 * @brief Выполняет HTTP-запрос к API Open-Meteo и десериализует ответ.
 * @param lat Широта локации.
 * @param lon Долгота локации.
 * @return Структура weather_data с актуальными данными или флагом невалидности.
 */
weather_data ProviderOM::fetch_current(float lat, float lon) {
    weather_data data = {0.0f, 0, 0, 0, false, false};
    HTTPClient http;
    
    // Формирование строки запроса с параметрами Open-Meteo
    String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(lat, 4) +
                 "&longitude=" + String(lon, 4) +
                 "&current=temperature_2m,relative_humidity_2m,is_day,weather_code,surface_pressure&timezone=auto";

    http.begin(url);
    int http_code = http.GET();

    // Обработка успешного HTTP-ответа
    if (http_code == HTTP_CODE_OK) {
        JsonDocument doc;
        if (deserializeJson(doc, http.getString()) == DeserializationError::Ok) {
            // Маппинг полей JSON во внутреннюю структуру
            data.temperature = doc["current"]["temperature_2m"];
            data.humidity = doc["current"]["relative_humidity_2m"];
            data.pressure_mm = (int)((float)doc["current"]["surface_pressure"] * 0.750062f);
            data.wmo_code = doc["current"]["weather_code"];
            data.is_day = doc["current"]["is_day"];
            data.is_valid = true;
        }
    }
    
    http.end();
    return data;
}