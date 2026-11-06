<p align="center">
  <img src="program_info/svin-launcher.png" alt="Svin Launcher" width="96" height="96">
</p>

# Svin Launcher

Лаунчер Minecraft для Windows на базе [Prism Launcher](https://github.com/PrismLauncher/PrismLauncher).
Можно играть с локальным профилем и держать несколько сборок отдельно:
у каждой свои моды, настройки и миры.

## Что есть

- Локальный профиль без входа в Microsoft.
- Загрузка файлов игры при обычном запуске и игра без интернета, когда всё уже скачано.
- Отдельные сборки с модами и шейдерами.
- Своя справка и ссылки на этот проект прямо в лаунчере.

Svin не обновляется поверх себя официальной версией Prism. Новые версии
Minecraft при этом появляются в общем каталоге, как и раньше.

## Запуск и сборка

Как собрать: [BUILDING.md](BUILDING.md). На GitHub для этого есть
[Actions → Build Svin Launcher for Windows](https://github.com/bincews/svin-launcher/actions).
Сценарий сборки на GitHub пока не проверен.

Готовую папку нужно распаковать целиком и запустить `prismlauncher.exe`.
Имена exe и `prismlauncher.cfg` оставлены прежними для совместимости.
Кнопка GitHub «Download ZIP» скачивает исходники, а не готовый лаунчер.

Как добавить профиль, выбрать Java и поставить моды: [справка](docs/HELP.md).

## Что нужно знать

Первое скачивание игры требует интернета. Локальный профиль не подходит
для серверов, которые проверяют лицензию Minecraft.

Для входа Microsoft и каталога CurseForge нужны свои ключи приложения.
Modrinth работает без них. Как задать ключи — в инструкции по сборке.

Проверены Minecraft 1.21.10, Fabric 0.19.5, моды и Complementary Reimagined.
Подробнее об изменениях и тестах — [LOCAL-FORK.md](LOCAL-FORK.md).

## Исходный проект

Svin — самостоятельный форк Prism Launcher. Лицензии и уведомления авторов
Prism, PolyMC и MultiMC сохранены в [LICENSE](LICENSE) и [COPYING.md](COPYING.md).
[Оригинальный README](docs/upstream/README.md) тоже оставлен в репозитории.

Иконка взята из текстуры свиньи Minecraft: [подробнее](program_info/SVIN-BRANDING.md).
