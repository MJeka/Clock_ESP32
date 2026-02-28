# -*- coding: utf-8 -*-
import os
import shutil
from SCons.Script import Import

# Доступ к глобальной среде сборки PlatformIO
Import("env")

# --- 1. АВТОМАТИЗАЦИЯ ПАРОЛЯ OTA ---
def update_ota_flags(source, target, env, *args, **kwargs):
    # Проверяем, какой протокол загрузки используется в данный момент
    # Если протокол НЕ 'espota', просто выходим из функции без добавления флагов
    if env.get("UPLOAD_PROTOCOL") != "espota":
        print("\033[93m[SKIP] USB mode detected, skipping OTA auth flags.\033[0m")
        return

    # Берем тот же пароль, что в secrets.h
    ota_pass = "GIG0wxJMHJIDxj" 
    print(f"\033[92m[OTA] Setting auth password: {ota_pass}\033[0m")
    
    # Добавляем флаг авторизации только для протокола OTA
    env.Append(UPLOADERFLAGS=[f"--auth={ota_pass}"])

# --- 2. КОПИРОВАНИЕ ПРОШИВКИ В КОРЕНЬ ---
def copy_firmware_to_root(source, target, env):
    # Получаем путь к директории проекта для копирования скомпилированного файла
    project_dir = env.get("PROJECT_DIR")
    destination = os.path.join(project_dir, "firmware.bin")
    bin_path = str(source[0])
    
    try:
        # Пытаемся скопировать файл прошивки в корень проекта
        shutil.copyfile(bin_path, destination)
        print(f"\033[94m[POST-BUILD] Firmware exported to root: firmware.bin\033[0m")
    except Exception as e:
        # Выводим сообщение об ошибке в случае неудачи при копировании
        print(f"\033[91m[ERROR] Failed to export firmware: {str(e)}\033[0m")

# --- РЕГИСТРАЦИЯ ДЕЙСТВИЙ ---

# Регистрируем функцию проверки флагов перед выполнением загрузки
env.AddPreAction("upload", update_ota_flags)

# Регистрируем функцию копирования после успешной сборки бинарного файла
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", copy_firmware_to_root)