<p align="center">
  <img src="program_info/svin-launcher.png" alt="Svin Launcher" width="96" height="96">
</p>

# Svin Launcher

Неофициальный форк [Prism Launcher](https://github.com/PrismLauncher/PrismLauncher)
для Windows с независимыми локальными профилями. Название **Svin Launcher**
одинаково во всех локализациях, включая русскую.

Проект не связан с командой Prism Launcher или Mojang/Microsoft и не одобрен ими.
Основа — develop Prism Launcher, коммит `899d2f3957215a09fe398a8a8cbb8a8a9c619968`.

## Что изменено

- Локальный профиль можно создать и использовать без Microsoft-аккаунта.
- Обычный запуск локального профиля скачивает необходимые файлы игры.
  Явный автономный запуск использует уже загруженный кеш.
- Исправлены ошибки загрузки библиотек и чтения метаданных при холодном старте.
- Неполные данные компонентов блокируют запуск: при отсутствии Fabric
  лаунчер не запускает vanilla вместо модовой сборки.
- Название — Svin Launcher, значок — оригинальная морда свиньи из Minecraft.
- Обновление на официальные сборки Prism отключено.
- Встроенные ключи Prism для Microsoft, CurseForge и Imgur удалены.

Для совместимости исполняемый файл называется `prismlauncher.exe`, а файл
настроек — `prismlauncher.cfg`. Профили, экземпляры и кеш остаются в прежнем формате.

## Сборка для Windows

Инструкции: [BUILDING.md](BUILDING.md).
В [Actions](https://github.com/bincews/svin-launcher/actions) предусмотрен
ручной сценарий **Build Svin Launcher for Windows**. Он собирает переносную
папку Windows x64 и сохраняет её в артефакт `Svin-Launcher-Windows-x64-portable`.
Первый запуск этого сценария на GitHub ещё не проверен.

Архив GitHub **Source code** содержит исходники. Для запуска нужна собранная
переносная папка целиком, вместе с DLL и подкаталогами.

## Ограничения и проверки

Для первоначального скачивания игры, Java и модов нужен Интернет.
Локальный профиль не авторизуется на серверах с проверкой лицензии.
Вход Microsoft и каталог CurseForge требуют собственных ключей приложения;
Modrinth не требует этих ключей.

Локально собран Windows x64 Release. Прошли 24 набора CTest и отдельно
12 обычных функций FileSystem. Проверены обычный и автономный запуск
Minecraft 1.21.10, Fabric 0.19.5, моды и Complementary Reimagined на копии мира.
Тесты символических ссылок с запросами администратора не запускались.
Forge, NeoForge, Quilt и установка сборок из каталогов отдельно не проверены.

Подробности изменений и границы проверки: [LOCAL-FORK.md](LOCAL-FORK.md).

## Авторы и лицензии

История и уведомления авторов Prism Launcher, PolyMC и MultiMC сохранены.
Лицензии исходников и зависимостей: [LICENSE](LICENSE), [COPYING.md](COPYING.md).
Происхождение изображения свиньи: [SVIN-BRANDING.md](program_info/SVIN-BRANDING.md).
Оригинальный README: [docs/upstream/README.md](docs/upstream/README.md).

В репозиторий не входят личные профили, модовая сборка, миры, кеши и файлы игры.
