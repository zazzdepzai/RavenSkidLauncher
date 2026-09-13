#pragma once
#include "UI/Screens/Screen.h"
#include "Launcher/AccountStore.h"
#include "Launcher/ModrinthClient.h"
#include "Launcher/DiscordRPC.h"
#include <string>
#include <vector>
#include <functional>
namespace UI::Screens { class RavenLauncherScreen: public Screen { public: RavenLauncherScreen(); ~RavenLauncherScreen(); void Initialize(ID3D11Device*) override; void Update(float) override; void Draw(ImVec2,ImVec2) override; void SetUser(const std::string&); void SetOnLogout(std::function<void()> cb){m_OnLogout=std::move(cb);} private: enum class Tab{Play,Installations,Mods,Accounts,Settings}; void Play(ImVec2,float);void Installations(ImVec2,float);void Mods(ImVec2,float);void Accounts(ImVec2,float);void Settings(ImVec2,float); Launcher::AccountStore m_Accounts;Launcher::ModrinthClient m_Modrinth;Launcher::MinecraftLauncher m_Minecraft;Launcher::DiscordRPC m_Discord;std::vector<Launcher::ModInfo> m_Mods;std::string m_User,m_Status,m_Search,m_RavenUrl,m_JavaPath,m_GameDir,m_ForgePath;Tab m_Tab=Tab::Play;int m_Version=0;std::function<void()> m_OnLogout;};}
