if (dispPage == 0) {
    RavenLauncher::EnsureMinecraftDirs();
    RavenLoadAccounts();

    const int setup = RavenLauncher::setupState.load();
    const bool forgeInstalled = RavenLauncher::ForgeInstalled();
    const bool modInstalled = RavenLauncher::ModInstalled();
    const bool setupBusy = setup == RavenLauncher::Working;
    const bool setupFailed = setup == RavenLauncher::Failed;
    const bool setupReady = setup == RavenLauncher::Ready && forgeInstalled && modInstalled;

    Glass::BeginPanel("RAVENXD LAUNCHER");
    ImGui::TextUnformatted("Minecraft 1.8.9");
    ImGui::SameLine();
    if (setupReady)
        Glass::StatusPill("Ready", IM_COL32(52,209,88,255));
    else if (setupBusy)
        Glass::StatusPill("Setting up...", IM_COL32(10,132,255,255));
    else if (setupFailed)
        Glass::StatusPill("Setup failed", IM_COL32(255,69,58,255));
    else
        Glass::StatusPill("Not installed", IM_COL32(255,159,10,255));

    ImGui::Dummy(ImVec2(0, 8));
    ImGui::TextDisabled("Account");
    ImGui::SameLine(150.0f);
    ImGui::TextUnformatted(
        ravenAccounts.empty() ? "None" : ravenAccounts[ravenSelectedAccount].c_str()
    );

    ImGui::Dummy(ImVec2(0, 12));

    // One-click setup: Forge installer + Forge client + RavenXD mod + Minecraft Launcher.
    if (Glass::Button(
        setupBusy ? "Installing Forge / RavenXD..." :
        setupReady ? "Launch Minecraft" : "Install Forge + RavenXD",
        !setupBusy,
        ImVec2(330, 50))) {
        if (setupReady) {
            if (!RavenLauncher::LaunchMinecraft()) {
                Glass::Notify(Glass::Icon::Bolt, IM_COL32(255,69,58,255),
                              "RavenXD", "Minecraft Launcher not found",
                              "Install the official Minecraft Launcher first.");
            }
        } else {
            RavenLauncher::StartSetup();
            Glass::Notify(Glass::Icon::Download, IM_COL32(10,132,255,255),
                          "RavenXD", "Automatic setup started",
                          "Downloading Forge, installing Forge and downloading RavenXD mod.");
        }
    }

    ImGui::Dummy(ImVec2(0, 8));

    if (Glass::Button("Launch Minecraft", !setupBusy, ImVec2(210, 44))) {
        if (!forgeInstalled || !modInstalled) {
            Glass::Notify(Glass::Icon::Bolt, IM_COL32(255,159,10,255),
                          "RavenXD", "Setup is incomplete",
                          "Use Install Forge + RavenXD first.");
        } else if (!RavenLauncher::LaunchMinecraft()) {
            Glass::Notify(Glass::Icon::Bolt, IM_COL32(255,69,58,255),
                          "RavenXD", "Minecraft Launcher not found",
                          "Install the official Minecraft Launcher first.");
        }
    }

    ImGui::SameLine(0, 10);

    if (Glass::Button("Open .minecraft", true, ImVec2(210, 44))) {
        ShellExecuteW(nullptr, L"open", RavenLauncher::MinecraftDir().c_str(),
                      nullptr, nullptr, SW_SHOWNORMAL);
    }

    ImGui::Dummy(ImVec2(0, 12));

    const int fs = RavenLauncher::forgeState.load();
    const int ms = RavenLauncher::modState.load();

    ImGui::TextDisabled("Forge: %s",
        fs == RavenLauncher::Working ? "Downloading..." :
        fs == RavenLauncher::Ready ? "Ready" :
        fs == RavenLauncher::Failed ? "Download failed" :
        forgeInstalled ? "Installed" : "Not installed");

    ImGui::TextDisabled("RavenXD mod: %s",
        ms == RavenLauncher::Working ? "Downloading..." :
        ms == RavenLauncher::Ready ? "Ready" :
        ms == RavenLauncher::Failed ? "Download failed" :
        modInstalled ? "Installed" : "Not installed");

    ImGui::TextDisabled("Forge version: 1.8.9 • 11.15.1.2318");
    ImGui::TextDisabled("Mod: RavenXD-v2.jar");

    if (setupBusy) {
        ImGui::Dummy(ImVec2(0, 8));
        ImGui::TextDisabled("Please wait: Forge installer and RavenXD mod are being prepared automatically.");
    } else if (setupFailed) {
        ImGui::Dummy(ImVec2(0, 8));
        ImGui::TextDisabled("Automatic setup failed. Press Install Forge + RavenXD to retry.");
    }

    Glass::EndPanel();

    Glass::BeginPanel("MINECRAFT FILES");
    ImGui::TextDisabled("Game directory");
    ImGui::SameLine(180);
    ImGui::TextUnformatted("%APPDATA%\\.minecraft");
    ImGui::TextDisabled("Forge installer");
    ImGui::SameLine(180);
    ImGui::TextUnformatted("forge-1.8.9-11.15.1.2318-installer.jar");
    ImGui::TextDisabled("RavenXD mod");
    ImGui::SameLine(180);
    ImGui::TextUnformatted("mods\\RavenXD-v2.jar");
    Glass::EndPanel();

    const char* wm = "RavenXD • Minecraft 1.8.9 • Liquid Glass";
    ImVec2 ws = ImGui::CalcTextSize(wm);
    ImVec2 wp = ImGui::GetCursorScreenPos();
    float watermarkW = ImGui::GetContentRegionAvail().x;
    ImGui::GetWindowDrawList()->AddText(
        ImVec2(wp.x + ((watermarkW > ws.x) ? (watermarkW - ws.x) : 0.0f), wp.y + 8.0f),
        IM_COL32(255,255,255,100), wm);
    ImGui::Dummy(ImVec2(watermarkW, 34));
}
