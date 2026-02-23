# -*- coding: utf-8 -*-
import json
import os
import shutil
import sys

# Доступ к глобальной среде сборки PlatformIO
Import("env")

def copy_firmware_to_root(source, target, env):
    """
    Функция обратного вызова: выполняется СРАЗУ ПОСЛЕ успешной сборки .bin файла.
    source - это путь к файлу в .pio/build/...
    target - конечный путь
    """
    project_dir = env.get("PROJECT_DIR")
    # Имя файла в корне проекта
    destination = os.path.join(project_dir, "firmware.bin")
    
    # Путь к только что собранному файлу
    bin_path = str(source[0])
    
    try:
        shutil.copyfile(bin_path, destination)
        print(f"\033[94m[POST-BUILD] Firmware successfully exported to: {destination}\033[0m")
    except Exception as e:
        print(f"\033[91m[ERROR] Failed to export firmware: {str(e)}\033[0m")

# --- ЛОГИКА ЗАПУСКА ---

# 1. Сначала подготавливаем конфигурацию (выполняется при запуске скрипта)
# load_ota_configuration()

# 2. Регистрируем Post-Action: копирование бинарника после сборки
# $BUILD_DIR/${PROGNAME}.bin - это стандартный путь PlatformIO к итоговому файлу
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", copy_firmware_to_root)