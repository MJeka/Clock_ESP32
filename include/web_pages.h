#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include <Arduino.h> // Прототипы стандартных функций (setup/loop), работа с GPIO (digitalWrite) и системными типами данных

/**
 * @brief CSS-стили для веб-интерфейса.
 * Содержат настройки адаптивной верстки (Flexbox), темную тему и стилизацию
 * карточек.
 */
const char WEB_STYLE[] PROGMEM = R"=====(
<style>
  /* Основные параметры страницы */
  body { 
    font-family: sans-serif; 
    background: #1a1a1a; 
    color: #eee; 
    margin: 0; 
    padding: 20px 10px; 
    display: flex; 
    flex-direction: column; 
    align-items: center; 
  }

  /* Контейнер для центрирования карточек и ограничения максимальной ширины */
  .container { 
    width: 100%; 
    max-width: 360px; 
    display: flex; 
    flex-direction: column; 
    gap: 15px; 
  }

  /* Стили карточек разделов */
  .card { 
    background: #2a2a2a; 
    padding: 20px; 
    border-radius: 12px; 
    box-sizing: border-box; 
    border: 1px solid #444; 
    width: 100%; 
    text-align: center;
  }

  h2 { margin-top: 0; font-size: 22px; color: #fff; }
  
  /* Элементы управления: поля ввода, выпадающие списки */
  input, select, button { 
    width: 100%; 
    padding: 14px; 
    margin: 10px 0; 
    border-radius: 8px; 
    border: none; 
    font-size: 16px; 
    box-sizing: border-box; 
  }
  
  input, select { background: #333; color: #fff; }
  
  /* Подписи к полям ввода */
  label { 
    display: block; 
    text-align: left; 
    font-size: 14px; 
    color: #aaa; 
    margin-top: 5px; 
    padding-left: 2px;
  }
  
  /* Стилизация кнопок */
  button { background: #008cff; color: white; font-weight: bold; cursor: pointer; }
  .btn-save { background: #28a745; }
  
  /* Ссылка для полного сброса настроек */
  .reset-link { 
    color: #ff4444; 
    text-decoration: none; 
    font-size: 14px; 
    display: block; 
    margin-top: 15px; 
    font-weight: bold;
  }
  
  /* Стили для поля пароля с кнопкой переключения видимости */
  .pass-wrapper { position: relative; width: 100%; }
  .pass-wrapper input { padding-right: 45px; }
  
  .eye-btn {
    position: absolute; right: 10px; top: 50%; transform: translateY(-50%);
    width: 30px; height: 30px; cursor: pointer; z-index: 10;
    background-size: 24px; background-repeat: no-repeat; background-position: center;
    filter: brightness(0) invert(0.8);
    opacity: 0.7;
    transition: opacity 0.2s;
  }
  .eye-btn:hover { opacity: 1; }
  
  /* Иконки глаза (Base64) */
  .eye-open{ background-image: url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAQAAADZc7J/AAAACXBIWXMAAA53AAAOdwHGnZK7AAACGUlEQVRIx+3Uy2/MURwF8E9H00kTqh4h6hGNWKjSkiBYEBUSr4Wy6Ia/QFig7crW0iORSkiwFSuCv2BKaqEeLeqV6EyxoEEi03T6tdCOX5tRj8TOuav7+N57vuecXP7jn6NM2a8OlELKHMvVqzXTFF9k9XjgtaHfeTNtozMe+yoSY1i/q/aZPjmDlDUO2ala+OilV94pqLLIUjXK5WWccvtnTGY74a2Ql3HUatOKOxVqtbhmUPjsnIWlyhvcVjCiywEzkNKg3RWXtWuQQlqTG4aEjPUTy7frFQadNA9UajVQVCDnuEpQ5YgB4YVdyfJm/UKfZuWjyrTKK+jWoUO3grzjo3qV2eqhkLN3rHyPnNBlbfHCVXLCBTWgxkUhqzHRbqeQtQOaPBfuqE8wahfum1+cL9AttCVO1OkUntnME6HXynHWXhY6xml0Xrg0bqXRU6E3ZcRvBLYEQnzP0SbPhE7L/qiFFe4Kz22BHbJCJtHGr0Rc556Qs3tsYa+c8EBT0aqf21iuWZ/Q/8NG2OWFMOCwqkmDVOOkQaHHtomirJcRhly3RRopjaNRbhuN8kwH3TOi4JaGUroudM5nYdA1LWpVFHemWe2YTnnhrRNmJT1PIm27IzZIG5bV541Pppir1hLVynx001ldRibzt9p+V/UbHvehfPXIaRsTrEoyGEOFxVaqM99UBR+88lCP95O/XBp/k9H/+FN8A2QS1V8nEhPbAAAAAElFTkSuQmCC'); }
  .eye-closed{ background-image: url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAQAAADZc7J/AAAACXBIWXMAAA7EAAAOxAGVKw4bAAAChUlEQVRIx+XVzWsUVhQF8N9kFDMZP8DYkCAiGGOGVOtKiphuFCIKklVog6S7rApas3HRBnfFP6C40Gxs3fmB0IVuTKLRYnVtUXGs1sEhCbMwYGgSZ56LmUzejKMuXBXv8r5z3r33vPPe4zONDTZ8Cj3jukvWfwzWJK3NFlu0SWuq5rvdElxb3mBVA+oaGfvssd0XUpg364l77nhom3O+MeVHc43rpvW7bFYQLCrIySlYFAQzLvtLcEv3CiFR0/TXTuqTknfHlAfyXiOtw0699tuIKcMeNard4oS8IOuUHqtBsw4dmsFXHlSqN9lpXT291RmL5o3JVLpqN2JCVta44/a5WW1+wAtntMb0TS4IXhq2ppLZZVLJkmnTliwpRLN/b0bJ7zYt09cZEzx1qLphuwnBXd/p0mnUYo10CUc8E5wrD5I0quhffVFHI4K7OkGXiXrlcUjOGz9LMqDglcEaOceVKpmybcrS7ZaKUEfNKRjgseC0ZLS0Wda0HTV0Bt22OUIlnRY8buTE5eiquG7YI51+URIawQYUzDlaM8INRaPGo9mHBDe0RKhBr8ojJP3kjZzD0eJxRQsRfYf7gpEI0eeFYllE1joreK6/auze6rknbDPkvmBCe3QGTwVjK35s9ZuSaT9IvSPdM0Ulk3ZVsM2G5QUXVowEG/1qwX/OOxyZdrfb/jFupFI9IWPMvIV6K5elOyan5HU0e7MO7ZXLtFqPU7KCvBM1ckYxVKUn9DroS1tt1eOAYy56KZh31d7ofaqJbpPV6t+atVT3oMy6ol+6lrQqoq/YJqnoD53apDDjb0/c86eHFt7nuvWu1V2ZhJaGj+p7N7jkuszHYB+KT/ws/sfxFt2N6XbIrTXBAAAAAElFTkSuQmCC'); }
</style>
)=====";

/**
 * @brief Генерирует HTML-код главной страницы настроек.
 */
String getIndexPage(String networks, String savedSSID, int dBr, int nBr,
                    int nStart, int nEnd, String weatherCity, String citiesJson) {
  String s = "<html><head><meta charset='UTF-8'>";
  s += "<link rel='icon' href='data:image/svg+xml,<svg xmlns=%22http://www.w3.org/2000/svg%22 viewBox=%220 0 100 100%22><text y=%22.9em%22 font-size=%2290%22>🕔</text></svg>'>";
  s += "<meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>";
  s += String(WEB_STYLE);
  s += "</head><body>";

  s += "<div class='container'>";

  // --- БЛОК НАСТРОЙКИ WIFI ---
  s += "<div class='card'><h2>Настройка WiFi</h2><form action='/save' method='POST'>";
  s += "<label>Доступные сети</label>";
  s += "<select name='ssid_select' id='ssid_select' onchange='document.getElementById(\"custom_ssid\").value=this.value'>";
  s += "<option value=''>-- Выберите сеть --</option>" + networks + "</select>";
  s += "<input type='text' name='custom_ssid' id='custom_ssid' placeholder='Имя сети (SSID)' value='" + savedSSID + "'>";
  s += "<div class='pass-wrapper'><input type='password' name='pass' id='pass' placeholder='Пароль'>";
  s += "<div id='eye_icon' class='eye-btn eye-closed' onclick='togglePass()'></div></div>";
  s += "<button type='submit'>СОХРАНИТЬ WIFI</button></form></div>";

  // --- БЛОК НАСТРОЕК ЭКРАНА И ПОГОДЫ ---
  s += "<div class='card'><h2>Настройки устройства</h2><form action='/save_settings' method='POST'>";
  s += "<label>Город (Поиск по локальной базе)</label>";
  s += "<input type='text' name='city' id='cityInput' list='citySuggestions' placeholder='Напр: Київ' autocomplete='off' oninput='searchCity(this.value)' value='" + weatherCity + "'>";
  s += "<datalist id='citySuggestions'></datalist>";

  s += "<label>Яркость День (0-255)</label>";
  s += "<input type='number' name='d_br' min='0' max='255' value='" + String(dBr) + "'>";
  s += "<label>Яркость Ночь (0-255)</label>";
  s += "<input type='number' name='n_br' min='0' max='255' value='" + String(nBr) + "'>";
  s += "<label>Начало ночи (час, 0-23)</label>";
  s += "<input type='number' name='n_st' min='0' max='23' value='" + String(nStart) + "'>";
  s += "<label>Конец ночи (час, 0-23)</label>";
  s += "<input type='number' name='n_en' min='0' max='23' value='" + String(nEnd) + "'>";

  s += "<button type='submit' class='btn-save'>ОБНОВИТЬ НАСТРОЙКИ</button></form>";
  s += "<a href='/reset' class='reset-link' onclick='return confirm(\"Сбросить все настройки?\")'>СБРОСИТЬ ВСЁ</a></div>";

  s += "</div>";

// --- JAVASCRIPT ---
  s += "<script>";
  /* Загрузка локальной базы городов, переданной с ESP */
  s += "const localCities = " + citiesJson + ";";

  s += "function togglePass(){";
  s += "  var x=document.getElementById('pass'); var icon=document.getElementById('eye_icon');";
  s += "  if(x.type==='password'){ x.type='text'; icon.classList.remove('eye-closed'); icon.classList.add('eye-open'); }";
  s += "  else { x.type='password'; icon.classList.remove('eye-open'); icon.classList.add('eye-closed'); }";
  s += "}";

  /** * @section CITY_AUTOCOMPLETE_LOCAL
   * Исправленная логика поиска: проверяет совпадения по всем языкам (RU, UA, EN) одновременно.
   */
  s += "function searchCity(query) {";
  s += "  const list = document.getElementById('citySuggestions');";
  s += "  list.innerHTML = '';";
  s += "  if (query.length < 2) return;";
  s += "  const q = query.toLowerCase();";
  
  s += "  localCities.forEach(item => {";
  /* Проверка вхождения строки во все языковые поля */
  s += "    const matchRu = item.ru.toLowerCase().includes(q);";
  s += "    const matchUa = item.ua.toLowerCase().includes(q);";
  s += "    const matchEn = item.en.toLowerCase().includes(q);";
  
  s += "    if (matchRu || matchUa || matchEn) {";
  s += "      const option = document.createElement('option');";
  
  /* Выбор отображаемого имени: если введена латиница - берем EN, если кириллица - приоритет UA/RU */
  s += "      let displayName = '';";
  s += "      if (/[a-z]/i.test(q)) { displayName = item.en; }";
  s += "      else if (matchUa) { displayName = item.ua; }";
  s += "      else { displayName = item.ru; }";
  
  /* Сохранение значения в формате 'Имя,Страна' и отображение в списке */
  s += "      option.value = displayName + ',' + item.c;";
  s += "      option.textContent = displayName + ' (UA)';";
  s += "      list.appendChild(option);";
  s += "    }";
  s += "  });";
  s += "}";
  
  s += "</script></body></html>";

  return s;
}

#endif // WEB_PAGES_H