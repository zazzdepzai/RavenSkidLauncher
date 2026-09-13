# RavenXD Launcher UI

Added a clean Minecraft-launcher navigation layer:

- Home (`⌂`)
- Misc (`◆`)
- Settings (`⚙`)
- Light/Dark mode
- Home: Minecraft 1.8.9 / Play
- Misc: Modrinth, RavenXD v2, Discord RPC
- Settings: RAM, Java path, game directory

`src/UI/LauncherTheme.h` contains theme/navigation state.
`src/UI/LauncherShell.h` contains the standalone ImGui launcher shell.

Call `RavenXD::UI::DrawLauncherShell()` from the main launcher render loop to display it.
