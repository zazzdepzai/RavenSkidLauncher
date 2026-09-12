if (dispPage == 1) {
    RavenLoadAccounts();

    Glass::BeginPanel("MINECRAFT ACCOUNTS");
    ImGui::TextDisabled("Launcher profiles");
    ImGui::Dummy(ImVec2(0, 8));

    for (int i = 0; i < static_cast<int>(ravenAccounts.size()); ++i) {
        ImGui::PushID(i);
        const bool selected = (ravenSelectedAccount == i);

        if (Glass::Button(selected ? "Selected" : "Use Account", selected, ImVec2(120, 38)))
            ravenSelectedAccount = i;

        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(ravenAccounts[i].c_str());

        if (i > 0) {
            ImGui::SameLine();
            if (Glass::Button("Remove", false, ImVec2(90, 38))) {
                ravenAccounts.erase(ravenAccounts.begin() + i);
                if (ravenAccounts.empty()) {
                    ravenAccounts.push_back("RavenXD");
                    ravenSelectedAccount = 0;
                } else if (ravenSelectedAccount >= static_cast<int>(ravenAccounts.size())) {
                    ravenSelectedAccount = static_cast<int>(ravenAccounts.size()) - 1;
                }
                RavenSaveAccounts();
                ImGui::PopID();
                break;
            }
        }

        ImGui::PopID();
    }

    ImGui::Dummy(ImVec2(0, 12));
    Glass::TextField("account_name", ravenAccountInput, static_cast<int>(sizeof(ravenAccountInput)),
                     "Minecraft username", Glass::Icon::Person);

    if (Glass::Button("Add Account", true, ImVec2(160, 42))) {
        std::string name(ravenAccountInput);
        if (!name.empty() && ravenAccounts.size() < 32) {
            bool exists = false;
            for (const auto& account : ravenAccounts) {
                if (_stricmp(account.c_str(), name.c_str()) == 0) {
                    exists = true;
                    break;
                }
            }

            if (!exists) {
                ravenAccounts.push_back(name);
                ravenSelectedAccount = static_cast<int>(ravenAccounts.size()) - 1;
                ravenAccountInput[0] = '\0';
                RavenSaveAccounts();
            }
        }
    }

    ImGui::SameLine();
    if (Glass::Button("Save", false, ImVec2(100, 42)))
        RavenSaveAccounts();

    ImGui::Dummy(ImVec2(0, 8));
    const char* selectedName = ravenAccounts.empty() ? "None" : ravenAccounts[ravenSelectedAccount].c_str();
    ImGui::TextDisabled("Selected: %s", selectedName);
    ImGui::TextDisabled("Offline profile manager. Microsoft credentials are never stored by RavenXD.");
    Glass::EndPanel();
}
