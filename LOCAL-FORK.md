# Changes from Prism

English · [Русский](LOCAL-FORK.ru.md)

Based on Prism's `develop` branch at commit `899d2f3957215a09fe398a8a8cbb8a8a9c619968`.

## Launching

- Local profiles work without a Microsoft account.
- Regular launches download game files. Offline launches use the cache.
- Library downloads and metadata loading after a restart have been fixed.
- Incomplete Fabric metadata stops the launch instead of starting vanilla without mods.
- Experimental Horizon 26.3 (Rust) instances are available on Windows x64. Svin downloads the client and prepares its resources on first launch.
- Horizon's bundled server entries are removed. Other entries in the server list are preserved.

## Interface and services

- Svin Launcher branding, pig icon, and project help pages.
- Svin project links replace Prism community and news links.
- Updates from official Prism builds are disabled. Svin uses its own GitHub releases.
- Bundled Microsoft, CurseForge, and Imgur keys have been removed.
- The Minecraft version catalog and data format are retained.

## Verification

On Windows x64, the checks include 24 CTest suites and 12 FileSystem test functions
(25 cases including data-driven cases and setup/cleanup). Symbolic-link tests are excluded.

Regular and offline launches of Minecraft 1.21.10 with Fabric 0.19.5, nine mods, and Complementary Reimagined have been checked, along with file recovery and refusal to launch with incomplete component metadata.

Horizon has been launched in isolated test instances. Both fresh and existing server lists were checked, including preservation of custom entries.

Forge, NeoForge, Quilt, and installation of packs from catalogs have not been tested separately.
