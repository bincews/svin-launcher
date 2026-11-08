# Сборка Windows x64

Проверенное окружение: MSVC 19.44, Qt 6.11.2 MSVC x64, CMake 3.31.6,
Ninja 1.12.1, JDK 17 и Windows SDK. Модули Qt: NetworkAuth, SVG, ImageFormats.

## Локально

В Developer PowerShell for VS 2022 (x64):

```powershell
git clone --recurse-submodules https://github.com/bincews/svin-launcher.git
cd svin-launcher
```

`JAVA_HOME` — каталог JDK 17; `CMAKE_PREFIX_PATH` — каталог Qt MSVC x64.
CMake и Ninja должны быть в PATH.

```powershell
cmake --preset default -DVCPKG_HOST_TRIPLET=x64-windows -DVCPKG_TARGET_TRIPLET=x64-windows -DLauncher_BUILD_PLATFORM=svin-local-windows-x64
cmake --build --preset default --config Release
ctest --preset default --build-config Release --exclude-regex '^FileSystem$' --timeout 120
cmake --install build --config Release
cmake --install build --config Release --component portable
```

Для r8 добавь `-DLauncher_SVIN_RELEASE=8` к команде настройки CMake.
Если нужен отдельный номер ревизии, добавь `-DLauncher_SVIN_REVISION=1`.
Для нового релиза rN укажи номер релиза и ревизию 0.
Архив `Svin-Launcher-Windows-x64-rN.zip` или `Svin-Launcher-Windows-x64-rN.M.zip`
для ревизии M должен содержать папку
`Svin-Launcher-Windows-x64` с файлами установки, включая `svinlauncher_updater.exe`.
Архивы всех ревизий прикрепляются к одному релизу с тегом `svin-rN`.

Перед упаковкой релизной сборки запусти в Python 3.11 или новее
`python scripts/stamp-svin-package.py install` после добавления всех файлов в
папку установки. Скрипт запишет идентификатор сборки в `portable.txt` и выведет
метку для архива на GitHub. При замене архива с тем же номером вычисли метку
заново и укажи её в поле Label у загруженного ZIP. После установки через
обновлятор Svin запоминает также контрольную сумму скачанного архива.

Результат — `install/svinlauncher.exe` и зависимости рядом с ним.
Для переносной сборки нужны DLL из
`VC/Redist/MSVC/<версия>/x64/Microsoft.VC143.CRT` установленной Visual Studio.

Из общего прогона исключён FileSystem: тесты символических ссылок требуют
повышенных прав.

## GitHub Actions

**Actions → Build Svin Launcher for Windows → Run workflow**.
Артефакт: `Svin-Launcher-Windows-x64-portable`.
Этот способ сборки ещё не проверен на GitHub.

## Ключи сервисов

| Сервис | Локальный параметр CMake | Настройка GitHub |
| --- | --- | --- |
| CurseForge | `Launcher_CURSEFORGE_API_KEY` | Secret `LOCAL_CURSEFORGE_API_KEY` |
| Microsoft | `Launcher_MSA_CLIENT_ID` | Variable `LOCAL_MSA_CLIENT_ID` |

По умолчанию оба значения пустые.
