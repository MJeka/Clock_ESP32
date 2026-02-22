# TFT Arduino PlatformIO Project

Проект перенесен из Arduino IDE в PlatformIO. Основной код (`main.cpp`) полностью оригинальный и поддерживает функции Arduino Core 3.0 (например, `ledcAttach`).

## Требования
- **Python 3.11** (уже установлен в системе через brew).
- **PlatformIO Core**.

## Основные команды (CLI)

Выполняйте эти команды, находясь в корневой папке проекта:

### 1. Компиляция (Сборка)
```bash
pio run
```

### 2. Загрузка на плату (через USB)
```bash
pio run --target upload
```

### 3. Мониторинг порта (просмотр логов)
```bash
pio device monitor
```
*Для выхода из монитора нажмите `Ctrl + C` или `Ctrl + ]`.*

### 4. Сборка и загрузка одной командой
```bash
pio run -t upload
```

## Работа в VSCode (с расширением PlatformIO)
1. **Собрать**: Нажмите иконку ![](https://raw.githubusercontent.com/platformio/platformio-vscode-ide/develop/assets/images/build-icon.png) (галочка) в нижней панели.
2. **Загрузить**: Нажмите иконку ![](https://raw.githubusercontent.com/platformio/platformio-vscode-ide/develop/assets/images/upload-icon.png) (стрелочка вправо) в нижней панели.
3. **Монитор**: Нажмите иконку ![](https://raw.githubusercontent.com/platformio/platformio-vscode-ide/develop/assets/images/monitor-icon.png) (вилка/розетка) для просмотра `Serial.print`.

## Обновление по воздуху (OTA)

Проект поддерживает беспроводную прошивку через Wi-Fi.

### 1. Подготовка
Убедитесь, что ваш компьютер и ESP32 находятся в одной Wi-Fi сети.

### 2. Загрузка через терминал
Вы можете запустить загрузку, указав IP-адрес устройства или его сетевое имя (из `secrets.h`):
```bash
pio run -t upload --upload-port Clock-ESP32.local
```
Если запросит пароль (хотя в коде он может быть закомментирован), используйте тот, что указан в `secrets.h`.

### 3. Настройка в platformio.ini (постоянная)
Чтобы загрузка по нажатию кнопки в VSCode всегда шла через OTA, добавьте эти строки в `platformio.ini`:
```ini
upload_protocol = espota
upload_port = Clock-ESP32.local
; upload_flags = --auth=jeka  ; Раскомментируйте, если пароль включен в коде
```

## Настройка дисплея и железа
Все параметры дисплея (ST7789, пины подключения, частота) вынесены в `platformio.ini` в разделе `build_flags`. Если нужно изменить пины, делайте это там, чтобы не менять код.
