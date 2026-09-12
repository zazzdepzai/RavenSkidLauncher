# RavenXD Launcher

Windows x64 Minecraft 1.8.9 launcher built with C++ / Dear ImGui / DirectX 11 / Liquid Glass.

Features:
- Minecraft 1.8.9 Forge installer
- RavenXD mod download
- Minecraft Launcher open
- Local Minecraft account/profile manager
- Discord RPC hook point
- RavenXD watermark and branding
- Runtime font/resource packaging
- CMake + Visual Studio 2022
- GitHub Actions Windows x64 build

Build locally:

```cmd
build.bat
```

The GitHub Actions artifact contains `RavenXD-Launcher.exe` and the required `data` directory.

Account Manager stores launcher profile names locally. It does not store Microsoft passwords or session tokens.

## RavenXD automatic setup

The launcher can automatically prepare Minecraft 1.8.9 Forge and the RavenXD mod.
Use **Install Forge + RavenXD** in the Minecraft page. Forge 1.8.9-11.15.1.2318 is downloaded from the Forge mirror; a bundled installer is also included as a fallback. The RavenXD mod is downloaded from the Raven-APlus release URL into `%APPDATA%\.minecraft\mods\RavenXD-v2.jar`.
