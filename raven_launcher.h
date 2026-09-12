#pragma once
#ifndef RAVENXD_LAUNCHER_H
#define RAVENXD_LAUNCHER_H

#include <windows.h>
#include <shellapi.h>
#include <urlmon.h>
#include <atomic>
#include <thread>
#include <string>
#include <filesystem>
#include <vector>

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "shell32.lib")

namespace RavenLauncher {

enum State : int {
    Idle = 0,
    Working = 1,
    Ready = 2,
    Failed = 3
};

inline constexpr const wchar_t* kForgeUrl =
    L"https://maven.minecraftforge.net/net/minecraftforge/forge/1.8.9-11.15.1.2318/forge-1.8.9-11.15.1.2318-installer.jar";
inline constexpr const wchar_t* kForgeFallbackUrl =
    L"https://files.minecraftforge.net/maven/net/minecraftforge/forge/1.8.9-11.15.1.2318/forge-1.8.9-11.15.1.2318-installer.jar";

inline constexpr const wchar_t* kModUrl =
    L"https://github.com/Raven-APlus/ravenxd-v2-old/releases/download/v2.2/ravenXD-v2.jar";

inline std::atomic<int> forgeState{Idle};
inline std::atomic<int> modState{Idle};
inline std::thread forgeWorker;
inline std::thread modWorker;
inline std::thread setupWorker;
inline std::atomic<int> setupState{Idle};

inline std::wstring AppDataPath() {
    wchar_t buf[MAX_PATH] = {};
    DWORD n = GetEnvironmentVariableW(L"APPDATA", buf, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) return L".";
    return std::wstring(buf);
}

inline std::wstring MinecraftDir() {
    return AppDataPath() + L"\\.minecraft";
}

inline std::wstring ForgeInstallerPath() {
    return MinecraftDir() + L"\\forge-1.8.9-11.15.1.2318-installer.jar";
}

inline std::wstring BundledForgeInstallerPath() {
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    std::filesystem::path p(exePath);
    return p.parent_path().wstring() + L"\\assets\\forge-1.8.9-11.15.1.2318-installer.jar";
}

inline std::wstring ModsDir() {
    return MinecraftDir() + L"\\mods";
}

inline std::wstring ModPath() {
    return ModsDir() + L"\\RavenXD-v2.jar";
}

inline void EnsureMinecraftDirs() {
    std::error_code ec;
    std::filesystem::create_directories(ModsDir(), ec);
}

inline void DownloadWorker(const wchar_t* url, const std::wstring& output, std::atomic<int>& state) {
    state.store(Working);
    EnsureMinecraftDirs();

    HRESULT hr = URLDownloadToFileW(
        nullptr, url, output.c_str(), 0, nullptr
    );

    // If the network download fails, use the installer bundled with the launcher.
    if (FAILED(hr) && url == kForgeUrl) {
        const std::wstring bundled = BundledForgeInstallerPath();
        if (GetFileAttributesW(bundled.c_str()) != INVALID_FILE_ATTRIBUTES) {
            std::error_code copyEc;
            std::filesystem::copy_file(
                bundled, output,
                std::filesystem::copy_options::overwrite_existing, copyEc);
            if (!copyEc && GetFileAttributesW(output.c_str()) != INVALID_FILE_ATTRIBUTES) {
                state.store(Ready);
                return;
            }
        }
    }

    // Forge mirrors can redirect differently. Retry the official Forge file mirror.
    if (FAILED(hr) && url == kForgeUrl) {
        DeleteFileW(output.c_str());
        hr = URLDownloadToFileW(
            nullptr, kForgeFallbackUrl, output.c_str(), 0, nullptr
        );
    }

    if (SUCCEEDED(hr) && GetFileAttributesW(output.c_str()) != INVALID_FILE_ATTRIBUTES)
        state.store(Ready);
    else {
        DeleteFileW(output.c_str());
        state.store(Failed);
    }
}

inline void StartForgeDownload() {
    if (forgeState.load() == Working) return;
    if (forgeWorker.joinable()) forgeWorker.join();

    forgeState.store(Working);
    forgeWorker = std::thread([] {
        DownloadWorker(kForgeUrl, ForgeInstallerPath(), forgeState);
    });
}

inline void StartModDownload() {
    if (modState.load() == Working) return;
    if (modWorker.joinable()) modWorker.join();
    if (setupWorker.joinable()) setupWorker.join();

    modState.store(Working);
    modWorker = std::thread([] {
        DownloadWorker(kModUrl, ModPath(), modState);
    });
}

inline bool RunForgeInstaller() {
    const std::wstring installer = ForgeInstallerPath();
    if (GetFileAttributesW(installer.c_str()) == INVALID_FILE_ATTRIBUTES)
        return false;

    // Forge 1.8.9 uses Java 8. "javaw.exe" is resolved through PATH.
    std::wstring cmd = L"javaw.exe -jar \"" + installer + L"\" --installClient";

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};

    std::vector<wchar_t> mutableCmd(cmd.begin(), cmd.end());
    mutableCmd.push_back(L'\0');

    BOOL ok = CreateProcessW(
        nullptr,
        mutableCmd.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        MinecraftDir().c_str(),
        &si,
        &pi
    );

    if (!ok) {
        // Fallback: show the installer with the default associated Java application.
        HINSTANCE r = ShellExecuteW(
            nullptr,
            L"open",
            installer.c_str(),
            nullptr,
            MinecraftDir().c_str(),
            SW_SHOWNORMAL
        );
        return reinterpret_cast<INT_PTR>(r) > 32;
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}


inline bool LaunchMinecraft();
inline bool ForgeInstalled();
inline bool ModInstalled();

inline void StartSetup() {
    if (setupState.load() == Working) return;
    if (setupWorker.joinable()) setupWorker.join();

    setupState.store(Working);
    setupWorker = std::thread([] {
        // Forge: download the installer if it is missing, then install the client.
        if (!ForgeInstalled()) {
            if (GetFileAttributesW(ForgeInstallerPath().c_str()) == INVALID_FILE_ATTRIBUTES) {
                DownloadWorker(kForgeUrl, ForgeInstallerPath(), forgeState);
            } else {
                forgeState.store(Ready);
            }

            if (forgeState.load() != Ready || !RunForgeInstaller()) {
                setupState.store(Failed);
                return;
            }

            // Give the installer a moment to finish writing the version directory.
            for (int i = 0; i < 120 && !ForgeInstalled(); ++i)
                Sleep(500);

            if (!ForgeInstalled()) {
                setupState.store(Failed);
                return;
            }
        } else {
            forgeState.store(Ready);
        }

        // RavenXD mod: download it automatically if it is missing.
        if (!ModInstalled()) {
            DownloadWorker(kModUrl, ModPath(), modState);
            if (modState.load() != Ready) {
                setupState.store(Failed);
                return;
            }
        } else {
            modState.store(Ready);
        }

        setupState.store(Ready);
        LaunchMinecraft();
    });
}

inline bool LaunchMinecraft() {
    // The minecraft: protocol opens the installed Minecraft Launcher.
    HINSTANCE r = ShellExecuteW(
        nullptr,
        L"open",
        L"minecraft:",
        nullptr,
        nullptr,
        SW_SHOWNORMAL
    );
    return reinterpret_cast<INT_PTR>(r) > 32;
}

inline bool ForgeInstalled() {
    const std::wstring versionDir =
        MinecraftDir() + L"\\versions\\1.8.9-forge1.8.9-11.15.1.2318";
    return GetFileAttributesW(versionDir.c_str()) != INVALID_FILE_ATTRIBUTES;
}

inline bool ModInstalled() {
    return GetFileAttributesW(ModPath().c_str()) != INVALID_FILE_ATTRIBUTES;
}

inline void Shutdown() {
    if (forgeWorker.joinable()) forgeWorker.join();
    if (modWorker.joinable()) modWorker.join();
    if (setupWorker.joinable()) setupWorker.join();
}

} // namespace RavenLauncher

#endif
