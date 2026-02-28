#ifndef CITIES_DB_H
#define CITIES_DB_H

#include <Arduino.h>

/**
 * @brief Локальная база городов.
 * Использование модификаторов 'const char* const' совместно с макросом 'PROGMEM' 
 * гарантирует размещение массива указателей и самих строковых литералов в 
 * Flash-памяти программ. Это исключает нецелевой расход SRAM при хранении 
 * статических данных.
 * Формат записи: "RU|UA|EN|CountryCode"
 */
const char* const CITIES_DATABASE[] PROGMEM = {
    "Киев|Київ|Kyiv|UA",
    "Харьков|Харків|Kharkiv|UA",
    "Одесса|Одеса|Odesa|UA",
    "Днепр|Дніпро|Dnipro|UA",
    "Донецк|Донецьк|Donetsk|UA",
    "Запорожье|Запоріжжя|Zaporizhzhia|UA",
    "Львов|Львів|Lviv|UA",
    "Кривой Рог|Кривий Ріг|Kryvyi Rih|UA",
    "Николаев|Миколаїв|Mykolaiv|UA",
    "Мариуполь|Маріуполь|Mariupol|UA",
    "Луганск|Луганськ|Luhansk|UA",
    "Севастополь|Севастополь|Sevastopol|UA",
    "Винница|Вінниця|Vinnytsia|UA",
    "Симферополь|Сімферополь|Simferopol|UA",
    "Херсон|Херсон|Kherson|UA",
    "Полтава|Полтава|Poltava|UA",
    "Чернигов|Чернігів|Chernihiv|UA",
    "Черкассы|Черкаси|Cherkasy|UA",
    "Хмельницкий|Хмельницький|Khmelnytskyi|UA",
    "Житомир|Житомир|Zhytomyr|UA",
    "Черновцы|Чернівці|Chernivtsi|UA",
    "Сумы|Суми|Sumy|UA",
    "Ровно|Рівне|Rivne|UA",
    "Ивано-Франковск|Івано-Франківськ|Ivano-Frankivsk|UA",
    "Каменское|Кам'янське|Kamianske|UA",
    "Кропивницкий|Кропивницький|Kropyvnytskyi|UA",
    "Тернополь|Тернопіль|Ternopil|UA",
    "Кременчуг|Кременчук|Kremenchuk|UA",
    "Луцк|Луцьк|Lutsk|UA",
    "Белая Церковь|Біла Церква|Bila Tserkva|UA",
    "Краматорск|Краматорськ|Kramatorsk|UA",
    "Мелитополь|Мелітополь|Melitopol|UA",
    "Керчь|Керч|Kerch|UA",
    "Ужгород|Ужгород|Uzhhorod|UA",
    "Бердянск|Бердянськ|Berdiansk|UA",
    "Никополь|Нікополь|Nikopol|UA",
    "Славянск|Слов'янськ|Sloviansk|UA",
    "Евпатория|Євпаторія|Evpatoria|UA",
    "Алчевск|Алчевськ|Alchevsk|UA",
    "Павлоград|Павлоград|Pavlohrad|UA",
    "Северодонецк|Сєвєродонецьк|Sievierodonetsk|UA",
    "Лисичанск|Лисичанськ|Lysychansk|UA",
    "Мукачево|Мукачево|Mukachevo|UA",
    "Конотоп|Конотоп|Konotop|UA",
    "Умань|Умань|Uman|UA",
    "Ялта|Ялта|Yalta|UA",
    "Александрия|Олександрія|Oleksandriia|UA",
    "Енакиево|Єнакієве|Yenakiieve|UA",
    "Шостка|Шостка|Shostka|UA",
    "Бердичев|Бердичів|Berdychiv|UA",
    "Бахмут|Бахмут|Bakhmut|UA",
    "Каменец-Подольский|Кам'янець-Подільський|Kamianets-Podilskyi|UA",
    "Константиновка|Костянтинівка|Kostiantynivka|UA",
    "Ковель|Ковель|Kovel|UA",
    "Феодосия|Феодосія|Feodosia|UA",
    "Горловка|Горлівка|Horlivka|UA"
};

/**
 * @brief Генерирует JSON-строку на основе данных из Flash-памяти.
 * Функция выполняет итерацию по массиву в PROGMEM, десериализует упакованные 
 * строки и формирует массив объектов для фронтенд-части веб-интерфейса.
 * @return String Результирующий JSON-массив.
 */
String getCitiesJson() {
    String json = "[";
    
    // Определение количества элементов через размер структуры массива
    int count = sizeof(CITIES_DATABASE) / sizeof(CITIES_DATABASE[0]);
    
    for (int i = 0; i < count; i++) {
        // Извлечение указателя на строку из адресного пространства PROGMEM
        char* ptr = (char*)pgm_read_ptr(&(CITIES_DATABASE[i]));
        
        // Буферизация данных во временную переменную SRAM для обработки
        char buffer[128];
        strcpy_P(buffer, ptr);
        
        // Сегментация строки по разделителю пайп '|'
        char *ru = strtok(buffer, "|");
        char *ua = strtok(NULL, "|");
        char *en = strtok(NULL, "|");
        char *c  = strtok(NULL, "|");

        // Построение структуры JSON-объекта
        json += "{\"ru\":\"" + String(ru) + "\",";
        json += "\"ua\":\"" + String(ua) + "\",";
        json += "\"en\":\"" + String(en) + "\",";
        json += "\"c\":\"" + String(c) + "\"}";
        
        // Добавление разделителя элементов массива при наличии последующих записей
        if (i < count - 1) json += ",";
    }
    
    json += "]";
    return json;
}

#endif // CITIES_DB_H