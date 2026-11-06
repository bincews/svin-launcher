# Prism Local — неофициальный личный форк

Дата изменений: 2026-11-06.
Оригинал: https://github.com/PrismLauncher/PrismLauncher
База: ветка develop, коммит 899d2f3957215a09fe398a8a8cbb8a8a9c619968.
Это снимок разрабатываемой версии, а не проверенный стабильный релиз.
Форк не связан с командой Prism Launcher и не одобрен ею.
Сторонний код OffPrism или PollyMC не добавлялся.

## Изменения

- Разрешено добавление офлайн-профиля без Microsoft-аккаунта.
- Офлайн-профиль считается пригодным для выбора и скачивания файлов игры.
- Выбранный офлайн-профиль запускается через существующий Offline-режим,
  без поиска другого лицензированного аккаунта и без перехода в Demo.
- Явный запуск Demo и проверка прав Microsoft-аккаунтов оставлены прежними.
- Обновлены подсказки об аккаунтах. Новые строки пока на английском.
- Название в интерфейсе: Prism Local (unofficial). Имя exe остаётся prismlauncher.exe.
- Автообновление на официальные сборки отключено, чтобы оно не стёрло изменения.
- Очищены встроенные API-ключи Prism для Microsoft, CurseForge и Imgur.
  Собственные ключи можно задать штатными переменными CMake.
- Код Fabric, Forge, NeoForge, Quilt, Modrinth и CurseForge не менялся.
  Это не подтверждает их работу в готовой сборке: сборка ещё не запускалась.

## Ограничения

Без собственного Launcher_CURSEFORGE_API_KEY каталог CurseForge недоступен.
Без Launcher_MSA_CLIENT_ID недоступен вход Microsoft; офлайн-профилям он не нужен.
Modrinth не требует этих ключей. Поддержка каталогов не удалена.
Офлайн-профиль не проходит авторизацию на серверах с проверкой лицензии.
Для первоначального скачивания игры, Java и модов требуется Интернет.
Изменение исходников не доказывает отсутствие уязвимостей или вирусов.

## Что проверено

- Изучены создание, выбор, загрузка аккаунтов и выбор режима запуска.
- git diff --check: успешно.
- Добавлены Qt-тесты OfflineAccount: работа без Microsoft-аккаунта,
  сохранение и восстановление профиля, неизменность проверки прав Microsoft.
- Эти Qt-тесты НЕ выполнялись: в облачной среде отсутствуют CMake и Qt.
- cmake --preset default завершился ошибкой: cmake: command not found.
- Загрузка submodules не завершилась из-за ограничений сети.
  Их исходные gitlink-коммиты сохранены; зависимости нужно загрузить при продолжении.
- Windows exe НЕ собран; запуск Minecraft и антивирусная проверка НЕ выполнялись.
- Репозиторий в аккаунте пользователя на GitHub НЕ создан: подключения нет.

## Продолжить на Windows 11

Установить Git, Visual Studio 2022 с Desktop development with C++, CMake с
поддержкой preset schema 8, Ninja, JDK 17 и Qt 6.11.2 MSVC x64 с NetworkAuth
и imageformats (версия Qt взята из CI выбранного снимка).
Открыть Developer PowerShell for VS 2022. Указать путь Qt через CMAKE_PREFIX_PATH
или Qt6_DIR. Если приложение требует более новый компилятор для C++23,
обновить инструменты C++.

Архив содержит полный рабочий checkout с Git-историей глубиной один исходный
коммит и коммитом локальных изменений. Папки submodules пока пусты.
Открыть папку PrismLauncher, затем выполнить:

```powershell
git submodule update --init --recursive
cmake --preset default -DVCPKG_HOST_TRIPLET=x64-windows -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build --preset default --config Release
ctest --preset default --build-config Release
cmake --install build --config Release
cmake --install build --config Release --component portable
```

Проверять код выхода каждой команды. При ошибке остановиться и исправить её.
При успешной установке результат находится в install: запускать prismlauncher.exe
вместе со всеми DLL и каталогами из этой папки. portable.txt сохраняет данные
рядом с приложением. Не переносить только один exe.

Для собственного ключа CurseForge добавить к configure аргумент
"-DLauncher_CURSEFORGE_API_KEY=$env:LOCAL_CURSEFORGE_API_KEY".
Не публиковать личные ключи в исходниках или логах.

## Сборка в GitHub Actions

Создать собственный репозиторий/форк, перенести ветку offline-local, включить Actions.
Выбрать Build Prism Local for Windows → Run workflow.
Сценарий .github/workflows/local-windows.yml использует рецепт зависимостей
из исходного Prism, собирает Release, выполняет тесты и выкладывает portable-папку
как артефакт Prism-Local-Windows-x64-portable. Сценарий пока НЕ запускался.
Необязательный secret: LOCAL_CURSEFORGE_API_KEY; variable: LOCAL_MSA_CLIENT_ID.
Ключ CurseForge попадёт в бинарный файл, как в исходном проекте.

## Проверки после сборки

1. Распаковать portable-папку отдельно от установленного Prism.
2. Добавить Add Offline при пустом списке аккаунтов.
3. Перезапустить лаунчер: профиль должен сохраниться и выбираться.
4. Скачать vanilla-инстанс, запустить и убедиться, что это не Demo.
5. Проверить явный Demo, выбор профиля без default и отмену диалогов.
6. Проверить Fabric, Forge, Modrinth; CurseForge — с собственным ключом.
7. Проверить сборку актуальным антивирусом; результаты не считать гарантией безопасности.

Лицензии и уведомления авторов сохранены в LICENSE и COPYING.md.
