# Building for Windows x64

English · [Русский](BUILDING.ru.md)

The verified environment uses MSVC 19.44, Qt 6.11.2 MSVC x64, CMake 3.31.6,
Ninja 1.12.1, JDK 17, and the Windows SDK. Required Qt modules include NetworkAuth, SVG, and ImageFormats.

## Local build

Open Developer PowerShell for VS 2022 (x64) and clone the repository.

```powershell
git clone --recurse-submodules https://github.com/bincews/svin-launcher.git
cd svin-launcher
```

Set `JAVA_HOME` to the JDK 17 directory and `CMAKE_PREFIX_PATH` to the Qt MSVC x64 directory.
CMake and Ninja must be on `PATH`.

```powershell
cmake --preset default -DVCPKG_HOST_TRIPLET=x64-windows -DVCPKG_TARGET_TRIPLET=x64-windows -DLauncher_BUILD_PLATFORM=svin-local-windows-x64
cmake --build --preset default --config Release
ctest --preset default --build-config Release --exclude-regex '^FileSystem$' --timeout 120
cmake --install build --config Release
cmake --install build --config Release --component portable
```

For r9, add `-DLauncher_SVIN_RELEASE=9` to the CMake configure command.
To set a separate revision number, add `-DLauncher_SVIN_REVISION=1`.
For a new release rN, set its release number and revision 0.
The archive `Svin-Launcher-Windows-x64-rN.zip`, or `Svin-Launcher-Windows-x64-rN.M.zip` for revision M,
must contain a `Svin-Launcher-Windows-x64` folder holding the installation files, including `svinlauncher_updater.exe`.
Attach all revisions of a release to the same GitHub release tagged `svin-rN`.

Before archiving a release, use Python 3.11 or newer to run
`python scripts/stamp-svin-package.py install` after adding all files to the installation folder.
The script writes a build identifier to `portable.txt` and prints the label to use for the GitHub archive asset.
When replacing an archive with the same version number, regenerate the label and set the uploaded ZIP asset's **Label** field to it.
After installing through the updater, Svin also records the downloaded archive's checksum.

The output is `install/svinlauncher.exe` with its dependencies alongside it.
Portable packages also need the DLLs from the installed Visual Studio's
`VC/Redist/MSVC/<version>/x64/Microsoft.VC143.CRT` directory.

FileSystem is excluded from the general test command because its symbolic-link tests require elevated permissions.

## GitHub Actions

Open **Actions → Build Svin Launcher for Windows → Run workflow**.
The artifact is named `Svin-Launcher-Windows-x64-portable`.
This build method has not yet been verified on GitHub.

<a id="ключи-сервисов"></a>

## Service keys

| Service | Local CMake option | GitHub setting |
| --- | --- | --- |
| CurseForge | `Launcher_CURSEFORGE_API_KEY` | Secret `LOCAL_CURSEFORGE_API_KEY` |
| Microsoft | `Launcher_MSA_CLIENT_ID` | Variable `LOCAL_MSA_CLIENT_ID` |

Both values are empty by default.
