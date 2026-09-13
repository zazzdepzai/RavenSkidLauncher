#pragma once
#include "imgui.h"
#include "LauncherTheme.h"

namespace RavenXD::UI {
    inline void DrawLauncherShell() {
        ApplyTheme();

        ImGui::Begin("RavenXD Minecraft Launcher", nullptr,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        ImGui::BeginChild("Sidebar", ImVec2(175, 0), true);
        ImGui::TextUnformatted("RAVENXD");
        ImGui::Separator();

        NavButton("⌂", "Home", Page::Home);
        NavButton("◆", "Misc", Page::Misc);
        NavButton("⚙", "Settings", Page::Settings);

        ImGui::Separator();
        bool light = LightTheme();
        if (ImGui::Checkbox("Light mode", &light)) {
            LightTheme() = light;
            ApplyTheme();
        }
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginChild("Content", ImVec2(0, 0), true);

        if (CurrentPage() == Page::Home) {
            ImGui::TextUnformatted("Minecraft");
            ImGui::Separator();
            ImGui::TextUnformatted("Minecraft 1.8.9");
            ImGui::Spacing();
            ImGui::Button("PLAY", ImVec2(180, 46));
            ImGui::Spacing();
            ImGui::TextUnformatted("Installation: RavenXD 1.8.9");
            ImGui::TextUnformatted("RavenXD v2");
        } else if (CurrentPage() == Page::Misc) {
            ImGui::TextUnformatted("Misc");
            ImGui::Separator();
            ImGui::Button("Modrinth Mods", ImVec2(180, 38));
            ImGui::Button("RavenXD v2", ImVec2(180, 38));
            ImGui::Button("Discord RPC", ImVec2(180, 38));
        } else {
            ImGui::TextUnformatted("Settings");
            ImGui::Separator();
            ImGui::TextUnformatted("RAM");
            static int ram = 2048;
            ImGui::SliderInt("##ram", &ram, 512, 16384, "%d MB");
            static char java[260] = "javaw.exe";
            ImGui::InputText("Java", java, sizeof(java));
            static char gameDir[260] = ".minecraft";
            ImGui::InputText("Game directory", gameDir, sizeof(gameDir));
        }

        ImGui::EndChild();
        ImGui::End();
    }
}
