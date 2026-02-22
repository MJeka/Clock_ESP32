# -*- coding: utf-8 -*-
"""
PlatformIO Extra Script: OTA Configuration & Auto-Export
Author: Gemini Collaborative Developer (Refined)
Date: 2026-02-22
"""

import json
import os
import shutil
import sys

# Доступ к глобальной среде сборки PlatformIO
Import("env")

def load_ota_configuration():
    """
    Парсит version.json и внедряет макросы C++ перед началом компиляции.
    """
    project_dir = env.get("PROJECT_DIR")
    version_file = os.path.join(project_dir, "version.json")

    if not os.path.exists(version_file):
        print(f"\033[91m[ERROR] Configuration file not found: {version_file}\033[0m")
        env.Append(CPPDEFINES=[("FIRMWARE_VERSION", '\\"0.0.0\\"'), ("JSON_URL", '\\"unknown\\"')])
        return

    try:
        with open(version_file, 'r', encoding='utf-8') as f:
            config = json.load(f)
            version = config.get('version', '0.0.0')
            ota_url = config.get('url', '')

            env.Append(CPPDEFINES=[
                ("FIRMWARE_VERSION", f'\\"{version}\\"'),
                ("JSON_URL", f'\\"{ota_url}\\"')
            ])
            print(f"\033[92m[SUCCESS] OTA AutoConfig: Version {version} injected\033[0m")
    except Exception as e:
        print(f"\033[91m[ERROR] Failed to parse version.json: {str(e)}\033[0m")
        sys.exit(1)

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
load_ota_configuration()

# 2. Регистрируем Post-Action: копирование бинарника после сборки
# $BUILD_DIR/${PROGNAME}.bin - это стандартный путь PlatformIO к итоговому файлу
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", copy_firmware_to_root)