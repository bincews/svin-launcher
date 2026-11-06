# Сборка Svin Launcher для Windows x64

Проверенное локальное окружение: Visual Studio 2022 Build Tools (MSVC 19.44),
Qt 6.11.2 MSVC x64, CMake 3.31.6, Ninja 1.12.1, JDK 17.
Нужны модули Qt NetworkAuth, SVG и ImageFormats, а также установленный Windows SDK.
JDK 17 используется для сборки лаунчера; версия Java для игры выбирается отдельно.

## Получение исходников

```powershell
git clone --recurse-submodules https://github.com/bincews/svin-launcher.git
cd svin-launcher
```

Для уже клонированной копии выполните `git submodule update --init --recursive`.
Версии зависимостей закреплены в репозитории.

## Локальная сборка

Откройте Developer PowerShell for VS 2022 (x64). Установите `JAVA_HOME` на JDK 17,
добавьте Ninja и CMake в PATH, а `CMAKE_PREFIX_PATH` — на свой каталог Qt MSVC x64.
Первый запуск vcpkg скачивает и собирает зависимости через Интернет.

```powershell
cmake --preset default -DVCPKG_HOST_TRIPLET=x64-windows -DVCPKG_TARGET_TRIPLET=x64-windows -DLauncher_BUILD_PLATFORM=svin-local-windows-x64
cmake --build --preset default --config Release
ctest --preset default --build-config Release --exclude-regex '^FileSystem$' --timeout 120
cmake --install build --config Release
cmake --install build --config Release --component portable
```

Проверяйте результат каждой команды перед следующей. Готовая папка — `install`.
Запускайте `prismlauncher.exe` вместе со всеми DLL и подкаталогами.
Для компьютера без установленного Microsoft Visual C++ Runtime добавьте
официальные DLL из `VC/Redist/MSVC/<версия>/x64/Microsoft.VC143.CRT`
вашей установки Visual Studio.

Тест FileSystem исключён из общей команды: его проверки символических ссылок
могут повторно запрашивать права администратора Windows. Другие 24 набора
выполняются без этих проверок.

## GitHub Actions

Откройте **Actions → Build Svin Launcher for Windows → Run workflow**.
После успешной сборки скачайте артефакт `Svin-Launcher-Windows-x64-portable`.
Первый запуск сценария на GitHub пока не проверен.

Необязательные настройки репозитория:

- Secret `LOCAL_CURSEFORGE_API_KEY` — свой ключ CurseForge.
- Variable `LOCAL_MSA_CLIENT_ID` — свой идентификатор Microsoft-приложения.

Без этих настроек сборка работает с локальными профилями; соответствующие
внешние сервисы недоступны. При локальной настройке ключи задаются через
`Launcher_CURSEFORGE_API_KEY` и `Launcher_MSA_CLIENT_ID`. Ключ, встроенный в
распространяемый бинарный файл, можно извлечь из него.

Исходные автоматизации Prism сохранены в `docs/upstream/workflows` как справочные
файлы. GitHub их не запускает; активен только сценарий сборки Svin Launcher.
