# -*- coding: utf-8 -*-
"""
PlatformIO Extra Script: OTA Configuration Generator
Author: Gemini Collaborative Developer
Date: 2026-02-22
"""

import json
import os
import sys

# Доступ к глобальной среде сборки PlatformIO
Import("env")

def load_ota_configuration():
    """
    Выполняет парсинг конфигурационного файла и внедряет макросы в процесс компиляции.
    """
    # Определение абсолютного пути к файлу конфигурации
    project_dir = env.get("PROJECT_DIR")
    version_file = os.path.join(project_dir, "version.json")

    # Валидация наличия файла
    if not os.path.exists(version_file):
        print(f"\033[91m[ERROR] Configuration file not found: {version_file}\033[0m")
        # Внедряем заглушки, чтобы компиляция не прервалась
        env.Append(CPPDEFINES=[
            ("FIRMWARE_VERSION", '\\"0.0.0\\"'),
            ("JSON_URL", '\\"unknown\\"')
        ])
        return

    try:
        with open(version_file, 'r', encoding='utf-8') as f:
            config = json.load(f)
            
            # Извлечение данных с валидацией по умолчанию (Graceful degradation)
            version = config.get('version', '0.0.0')
            ota_url = config.get('url', '')

            # Внедрение макросов в препроцессор C++
            # Используется экранирование кавычек для корректной передачи строк
            env.Append(CPPDEFINES=[
                ("FIRMWARE_VERSION", f'\\"{version}\\"'),
                ("JSON_URL", f'\\"{ota_url}\\"')
            ])

            print(f"\033[92m[SUCCESS] OTA AutoConfig: Version {version} injected\033[0m")

    except Exception as e:
        print(f"\033[91m[ERROR] Failed to parse version.json: {str(e)}\033[0m")
        sys.exit(1)

# Запуск функции при вызове скрипта
load_ota_configuration()