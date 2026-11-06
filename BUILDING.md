# Сборка Svin Launcher для Windows x64

Локальная сборка работает с таким набором: Visual Studio 2022 Build Tools (MSVC 19.44),
Qt 6.11.2 MSVC x64, CMake 3.31.6, Ninja 1.12.1, JDK 17.
Нужны модули Qt NetworkAuth, SVG и ImageFormats, а также установленный Windows SDK.
JDK 17 нужен для сборки самого лаунчера. Для игры Java выбирается отдельно.

## Исходники

```powershell
git clone --recurse-submodules https://github.com/bincews/svin-launcher.git
cd svin-launcher
```

Если репозиторий уже скачан, выполни `git submodule update --init --recursive`.
Версии зависимостей закреплены в репозитории.

## Локальная сборка

Открой Developer PowerShell for VS 2022 (x64). Укажи `JAVA_HOME` на JDK 17,
добавь Ninja и CMake в PATH, а `CMAKE_PREFIX_PATH` — на свой каталог Qt MSVC x64.
Первый запуск vcpkg скачивает и собирает зависимости через Интернет.

```powershell
cmake --preset default -DVCPKG_HOST_TRIPLET=x64-windows -DVCPKG_TARGET_TRIPLET=x64-windows -DLauncher_BUILD_PLATFORM=svin-local-windows-x64
cmake --build --preset default --config Release
ctest --preset default --build-config Release --exclude-regex '^FileSystem$' --timeout 120
cmake --install build --config Release
cmake --install build --config Release --component portable
```

Если команда завершилась с ошибкой, исправь её перед следующим шагом. Результат будет в `install`.
Запускай `prismlauncher.exe` из этой папки. DLL и подкаталоги должны остаться рядом.
Для компьютера без установленного Microsoft Visual C++ Runtime добавь
DLL из `VC/Redist/MSVC/<версия>/x64/Microsoft.VC143.CRT`
своей установки Visual Studio.

Тест FileSystem исключён из общей команды: его проверки символических ссылок
могут повторно запрашивать права администратора Windows. Другие 24 набора
выполняются без этих проверок.

## GitHub Actions

Открой **Actions → Build Svin Launcher for Windows → Run workflow**.
Когда сборка закончится, скачай артефакт `Svin-Launcher-Windows-x64-portable`.
Сценарий на GitHub ещё не запускался.

Если нужен вход Microsoft или CurseForge, задай в настройках репозитория:

- Secret `LOCAL_CURSEFORGE_API_KEY` — свой ключ CurseForge.
- Variable `LOCAL_MSA_CLIENT_ID` — свой идентификатор Microsoft-приложения.

Без ключей доступны локальные профили и Modrinth. Вход Microsoft и CurseForge работать не будут. При локальной настройке ключи задаются через
`Launcher_CURSEFORGE_API_KEY` и `Launcher_MSA_CLIENT_ID`. Ключ, встроенный в
распространяемый бинарный файл, можно извлечь из него.

Старые сценарии Prism оставлены в `docs/upstream/workflows` как справочные
файлы. GitHub их не запускает. Для Svin используется свой сценарий сборки.
