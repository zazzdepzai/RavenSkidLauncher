#pragma once
#include "UI/Screens/Screen.h"
#include "Launcher/AccountStore.h"
#include "Launcher/ModrinthClient.h"
#include "Launcher/DiscordRPC.h"
#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
namespace UI::Screens {
class RavenLauncherScreen: public Screen {
public:
    RavenLauncherScreen(); ~RavenLauncherScreen();
    void Initialize(ID3D11Device*) override; void Update(float) override; void Draw(ImVec2,ImVec2) override;
    void SetUser(const std::string&); void SetOnLogout(std::function<void()> cb){m_OnLogout=std::move(cb);}
private:
    enum class Tab{Play,Mods,Accounts,Settings,Logs};
    void Play(ImVec2,float); void Mods(ImVec2,float); void Accounts(ImVec2,float); void Settings(ImVec2,float); void Logs(ImVec2,float);
    void AddLog(const std::string&, const char* level="INFO"); void ApplyTheme();
    // Runs `work` on a background thread so long installs/launches (which used to call straight
    // into a blocking CreateProcess+WaitForSingleObject on the UI thread) no longer freeze the
    // whole window. Only one background task runs at a time; m_Busy gates re-entrant clicks.
    void RunAsync(const std::string& busyLabel, std::function<void()> work);
    void SetStatus(const std::string& s); std::string GetStatus();
    Launcher::AccountStore m_Accounts; Launcher::ModrinthClient m_Modrinth; Launcher::MinecraftLauncher m_Minecraft; Launcher::DiscordRPC m_Discord;
    std::vector<Launcher::ModInfo> m_Mods; std::vector<std::string> m_Logs;
    std::string m_User,m_Search,m_RavenUrl,m_JavaPath,m_GameDir,m_ForgePath; Tab m_Tab=Tab::Play; int m_Version=0;
    std::function<void()> m_OnLogout; float m_Time=0.0f;
    std::atomic<bool> m_Busy{false}; std::string m_BusyLabel; std::mutex m_StatusMutex; std::string m_Status; std::thread m_Worker; std::mutex m_LogMutex;
}; }
