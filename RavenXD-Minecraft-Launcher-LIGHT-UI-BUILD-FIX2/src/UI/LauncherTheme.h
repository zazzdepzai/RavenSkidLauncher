#pragma once
#include "imgui.h"

namespace RavenXD::UI {
    enum class Page { Home, Misc, Settings };

    inline Page& CurrentPage() {
        static Page page = Page::Home;
        return page;
    }

    inline bool& LightTheme() {
        static bool light = false;
        return light;
    }

    inline void ApplyTheme() {
        ImGuiStyle& s = ImGui::GetStyle();
        if (LightTheme()) {
            ImGui::StyleColorsLight();
            s.WindowRounding = 14.0f;
            s.ChildRounding = 12.0f;
            s.FrameRounding = 10.0f;
            s.PopupRounding = 10.0f;
            s.GrabRounding = 10.0f;
        } else {
            ImGui::StyleColorsDark();
            s.WindowRounding = 14.0f;
            s.ChildRounding = 12.0f;
            s.FrameRounding = 10.0f;
            s.PopupRounding = 10.0f;
            s.GrabRounding = 10.0f;
        }
    }

    inline bool NavButton(const char* icon, const char* label, Page page) {
        const bool selected = CurrentPage() == page;
        if (selected) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        bool pressed = ImGui::Button((std::string(icon) + "  " + label).c_str(), ImVec2(145, 38));
        if (selected) ImGui::PopStyleColor();
        if (pressed) CurrentPage() = page;
        return pressed;
    }
}
