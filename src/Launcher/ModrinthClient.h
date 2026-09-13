#pragma once
#include <string>
#include <vector>
namespace Launcher {
struct ModInfo { std::string title, slug, url, filename; bool installed=false; };
class ModrinthClient {
public:
    std::vector<ModInfo> Search(const std::string&query,const std::string&game="1.8.9",const std::string&loader="forge");
    bool Install(const ModInfo&mod,const std::string&modsDir,std::string&error);
    bool Download(const std::string&url,const std::wstring&path,std::string&error);
    std::string Get(const std::wstring&url,std::string&error);
};
class MinecraftLauncher {
public:
    bool InstallRaven(const std::string&url,const std::wstring&modsDir,std::string&error,bool force=false);
    bool InstallMinecraft18(const std::wstring&gameDir,std::string&error,bool force=false);
    bool LaunchForge(const std::wstring&javaw,const std::wstring&gameDir,const std::wstring&forgeJar,
                     const std::string&username,std::string&error);
    bool LaunchVanilla(const std::wstring&javaw,const std::wstring&gameDir,const std::wstring&jar,
                       const std::string&username,std::string&error);
    // Cheap on-disk checks: no network/process spawn. Used to avoid re-triggering a full
    // Minecraft/Forge/mod install (which previously always re-verified/re-downloaded the
    // whole .minecraft directory) every time the corresponding button is pressed.
    static bool IsMinecraft18Installed(const std::wstring&gameDir);
    static bool IsForge18Installed(const std::wstring&gameDir);
    static bool IsRavenModInstalled(const std::wstring&modsDir);
};
}
