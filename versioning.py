import subprocess
Import("env")

def get_git_version():
    try:
        # Пытаемся получить последний тег или хеш коммита
        return subprocess.check_output(["git", "describe", "--tags", "--always"]).decode().strip()
    except:
        return "v0.0.0"

# Передаем версию в макрос C++
version = get_git_version()
env.Append(CPPDEFINES=[("BUILD_VERSION", f'\\"{version}\\"')])