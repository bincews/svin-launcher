<p align="center">
  <img src="program_info/svin-launcher.png" alt="Svin Launcher" width="96" height="96">
</p>

# Svin Launcher

English · [Русский](README.ru.md)

Svin is a Minecraft launcher for Windows based on [Prism Launcher](https://github.com/PrismLauncher/PrismLauncher).
It supports local profiles, multiple game versions, and separate instances with their own mods, settings, and worlds.

**[Download for Windows x64](https://github.com/bincews/svin-launcher/releases/latest)**

## Features

- **Separate instances.** Each instance has its own Minecraft version, mod loader, mods, resource packs, shaders, and settings. Instances can be copied, imported, and exported.
- **Local profiles.** Choose a username in the launcher. Svin loads Ely.by skins by username in supported Minecraft versions.
- **Mod management.** Prism's tools for installing loaders, managing mods, and browsing Modrinth are included. CurseForge requires your own API key.
- **Java and memory.** Use automatic Java selection or a custom Java installation, with global and per-instance memory settings.
- **Game downloads.** A regular launch downloads missing libraries and assets. An offline launch uses files already downloaded.
- **Launch logs.** View game and launcher logs from the interface.

## Changes from Prism

Local profiles work independently, without another account in the account list or demo mode. Regular launches can download game files.

Svin also fixes library downloads and metadata loading after a restart. Incomplete Fabric metadata stops the launch with an error instead of starting vanilla Minecraft without mods.

Svin has its own name, pig icon, help pages, and repository links. Bundled service keys and updates from official Prism builds are disabled. See [Changes from Prism](LOCAL-FORK.md) for details.

## Profiles and services

Local profiles work in single-player and on servers that accept players without Microsoft account verification. Servers that require verification need an authenticated account.

Microsoft sign-in and CurseForge are not configured in the distributed build. They require your own application keys, listed in the [build instructions](BUILDING.md#service-keys). Modrinth works without these keys.

## Game versions and updates

The Minecraft version list comes from Prism's catalog and updates independently of Svin releases. Prepared instances can be launched offline.

On Windows x64, you can create a **26.3 — Horizon (Rust)** instance. Open **Add Instance → Custom** and enable **Experiments**. On first launch, Svin downloads the client from Horizon's release and prepares its resources. Horizon uses a native client, so Minecraft Java mod loaders and Java mods cannot run in it. Svin removes Horizon's bundled server entries.

**Help → Check for Svin updates** compares your installed version with releases on GitHub. Choose **Update** to replace the launcher files in the same folder and restart. When no update is available, Svin shows the installed version. Corrected builds can also be published within an existing release.

## Current build

**r9** is a portable Windows x64 build. Run `svinlauncher.exe`. Settings are stored in `svinlauncher.cfg`, and settings from earlier versions migrate automatically.

Regular and offline launches of Minecraft 1.21.10 with Fabric 0.19.5, nine mods, and Complementary Reimagined have been checked on Windows 11. Forge, NeoForge, and Quilt support is inherited from Prism and has not been tested separately. See [verification details](LOCAL-FORK.md#verification).

## Documentation

- [Settings, Java, mods, and logs](docs/HELP.md)
- [Building from source and GitHub Actions](BUILDING.md)
- [Report a bug or suggest a change](https://github.com/bincews/svin-launcher/issues)

## Credits and licenses

Svin is a fork of Prism Launcher, which is based on PolyMC and MultiMC. Licenses and author notices are in [LICENSE](LICENSE) and [COPYING.md](COPYING.md).

[Original Prism README](docs/upstream/README.md) · [Icon source](program_info/SVIN-BRANDING.md)
