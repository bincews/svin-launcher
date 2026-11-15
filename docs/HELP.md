# Help

English · [Русский](HELP.ru.md)

<a id="Launcher-settings"></a>

## Profiles and instances

Add a local profile from the account menu in the upper-right corner.
Use **Add Instance** to create an instance, or its import tab to import a ZIP.
Language, appearance, and folder settings are under **Settings**.

A regular launch downloads missing files. An offline launch uses the cache.

<a id="Java-settings"></a>
<a id="java-wizard"></a>

## Java

Use **Settings → Java** for automatic Java selection or a custom Java path.
Minecraft 1.21.10 uses Java 21 x64. Set memory limits there, or per instance under **Edit → Settings → Java**.

<a id="Instance-Version"></a>

## Mods and shaders

Use **Edit → Version** to install Fabric and other mod loaders.
The **Mods** and **Shader Packs** tabs manage files for the selected instance.
Minecraft Java shaders require Iris or another mod with shader support.

## Horizon (Rust)

On Windows x64, open **Add Instance → Custom**, enable **Experiments**, and select **26.3 — Horizon (Rust)**.
The first launch downloads Horizon's client and prepares its resources. Each instance keeps its own game folder.
Horizon uses native mods. Fabric, Forge, and other Java mod loaders do not run in this client.

## Ely.by skins

Svin loads Ely.by skins by username when launching Minecraft Java with a local profile.
For Minecraft releases in the current catalog, Svin selects a patch matching the Authlib version or uses built-in skin loading for older clients.
Downloading the patch for the first time and fetching skins require an internet connection.
The setting is under **Settings → Minecraft → Advanced**.
Authlib patches are downloaded from the [PineconeMC repository](https://meta.pineconemc.ru/v1/by.ely.authlib/index.json).

Click **Load Ely.by avatar** in the account list to load an avatar by username.
After the first load, Svin checks it at startup and roughly once a minute while the launcher is open.

Snapshots support skins when a known patch is compatible.
For modded Minecraft Java instances, open CustomSkinLoader through **Edit → Mods → Ely.by skins in Minecraft**.

<a id="instance-copy"></a>

## Instance variants

The **Copy** dialog lets you choose which data to include in the new instance.

## Launch errors and updates

Open logs through **Help → View logs**. Report problems in [Issues](https://github.com/bincews/svin-launcher/issues).

**Help → Check for Svin updates** shows whether an update is available.
Choose **Update** to install it in the launcher's folder and restart.
