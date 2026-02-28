#ifndef CITIES_DB_H
#define CITIES_DB_H

#include <Arduino.h>

/**
 * @brief Полностью локальная база городов (Flash-память).
 * Формат: "RU|UA|EN|КодСтраны"
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
    "Севастополь|Севастополь|Sevastopol|UA",
    "Мариуполь|Маріуполь|Mariupol|UA",
    "Луганск|Луганськ|Luhansk|UA",
    "Винница|Вінниця|Vinnytsia|UA",
    "Симферополь|Сімферополь|Simferopol|UA",
    "Херсон|Херсон|Kherson|UA",
    "Полтава|Полтава|Poltava|UA",
    "Чернигов|Чернігів|Chernihiv|UA",
    "Черкассы|Черкаси|Cherkasy|UA",
    "Житомир|Житомир|Zhytomyr|UA",
    "Сумы|Суми|Sumy|UA",
    "Хмельницкий|Хмельницький|Khmelnytskyi|UA",
    "Черновцы|Чернівці|Chernivtsi|UA",
    "Ровно|Рівне|Rivne|UA",
    "Ивано-Франковск|Івано-Франківськ|Ivano-Frankivsk|UA",
    "Кременчуг|Кременчук|Kremenchuk|UA",
    "Тернополь|Тернопіль|Ternopil|UA",
    "Луцк|Луцьк|Lutsk|UA",
    "Белая Церковь|Біла Церква|Bila Tserkva|UA",
    "Краматорск|Краматорськ|Kramatorsk|UA",
    "Мелитополь|Мелітополь|Melitopol|UA",
    "Керчь|Керч|Kerch|UA",
    "Ужгород|Ужгород|Uzhhorod|UA",
    "Бердянск|Бердянськ|Berdiansk|UA",
    "Никополь|Нікополь|Nikopol|UA",
    "Славянск|Слов'янськ|Sloviansk|UA",
    "Евпатория|Євпаторія|Yevpatoriia|UA",
    "Алчевск|Алчевськ|Alchevsk|UA",
    "Павлоград|Павлоград|Pavlohrad|UA",
    "Северодонецк|Сєвєродонецьк|Severodonetsk|UA",
    "Ялта|Ялта|Yalta|UA",
    "Лисичанск|Лисичанськ|Lysychansk|UA",
    "Феодосия|Феодосія|Feodosiia|UA",
    "Горловка|Горлівка|Horlivka|UA",
    "Макеевка|Макіївка|Makiivka|UA",
    "Бахчисарай|Бахчисарай|Bakhchysarai|UA",
    "Джанкой|Джанкой|Dzhankoi|UA"
};

const int CITIES_COUNT = sizeof(CITIES_DATABASE) / sizeof(CITIES_DATABASE[0]);

/**
 * @brief Сборка JSON-строки из PROGMEM для отправки клиенту.
 */
String getCitiesJson() {
    String json = "[";
    for (int i = 0; i < CITIES_COUNT; i++) {
        char buffer[128];
        strcpy_P(buffer, (char*)pgm_read_ptr(&(CITIES_DATABASE[i])));
        String s = String(buffer);
        
        int p1 = s.indexOf('|');
        int p2 = s.indexOf('|', p1 + 1);
        int p3 = s.lastIndexOf('|');

        json += "{\"ru\":\"" + s.substring(0, p1) + "\",";
        json += "\"ua\":\"" + s.substring(p1 + 1, p2) + "\",";
        json += "\"en\":\"" + s.substring(p2 + 1, p3) + "\",";
        json += "\"c\":\"" + s.substring(p3 + 1) + "\"}";
        if (i < CITIES_COUNT - 1) json += ",";
    }
    json += "]";
    return json;
}

#endif