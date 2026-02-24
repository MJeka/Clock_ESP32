# -*- coding: utf-8 -*-
import os
import shutil
from SCons.Script import Import

# Доступ к глобальной среде сборки PlatformIO
Import("env")

# --- 1. АВТОМАТИЗАЦИЯ ПАРОЛЯ OTA ---
def update_ota_flags(source, target, env, *args, **kwargs):
    # Берем тот же пароль, что в secrets.h
    # Если нужно поменять, начала меняем в secrets.h, обновляемся 1 раз
    # Потом меняем тут и обновляемся второй раз для проверки
    ota_pass = "GIG0wxJMHJIDxj" 
    print(f"\033[92m[OTA] Setting auth password: {ota_pass}\033[0m")
    env.Append(UPLOADERFLAGS=[f"--auth={ota_pass}"])

# --- 2. КОПИРОВАНИЕ ПРОШИВКИ В КОРЕНЬ ---
def copy_firmware_to_root(source, target, env):
    project_dir = env.get("PROJECT_DIR")
    destination = os.path.join(project_dir, "firmware.bin")
    bin_path = str(source[0])
    
    try:
        shutil.copyfile(bin_path, destination)
        print(f"\033[94m[POST-BUILD] Firmware exported to root: firmware.bin\033[0m")
    except Exception as e:
        print(f"\033[91m[ERROR] Failed to export firmware: {str(e)}\033[0m")

# --- РЕГИСТРАЦИЯ ДЕЙСТВИЙ ---

# Запускаем пароль ПЕРЕД загрузкой
env.AddPreAction("upload", update_ota_flags)

# Запускаем копирование ПОСЛЕ сборки бинарника
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", copy_firmware_to_root)