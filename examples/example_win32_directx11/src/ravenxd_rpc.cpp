#include "ravenxd_rpc.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <string>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <random>
#include <cstdio>

namespace RavenXD_RPC {
static HANDLE g_pipe = INVALID_HANDLE_VALUE;
static std::string g_clientId;
static long long g_start = 0;
static ULONGLONG g_lastCycle = 0;
static int g_cycleIndex = 0;
static const char* const g_cycleDetails[] = {
    "RavenXD Launcher",
    "RavenXD Launcher",
    "RavenXD Launcher",
    "RavenXD Launcher",
    "RavenXD Launcher"
};
static const char* const g_cycleStates[] = {
    "Minecraft 1.8.9",
    "Installing Forge 1.8.9",
    "Installing RavenXD Mod",
    "Managing Minecraft Accounts",
    "Ready to Launch"
};
static constexpr int g_cycleCount = static_cast<int>(sizeof(g_cycleStates) / sizeof(g_cycleStates[0]));
static constexpr ULONGLONG g_cycleIntervalMs = 6000;

static std::string Escape(const char* s) {
    std::string out;
    if (!s) return out;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p; ++p) {
        switch (*p) {
        case '\\': out += "\\\\"; break;
        case '"':  out += "\\\""; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default: out += static_cast<char>(*p); break;
        }
    }
    return out;
}

static bool WriteAll(const void* data, DWORD size) {
    if (g_pipe == INVALID_HANDLE_VALUE) return false;
    const char* p = static_cast<const char*>(data);
    DWORD left = size;
    while (left) {
        DWORD written = 0;
        if (!WriteFile(g_pipe, p, left, &written, nullptr) || written == 0) return false;
        p += written;
        left -= written;
    }
    return true;
}

static bool WritePacket(int opcode, const std::string& json) {
    DWORD header[2] = { static_cast<DWORD>(opcode), static_cast<DWORD>(json.size()) };
    return WriteAll(header, sizeof(header)) && WriteAll(json.data(), static_cast<DWORD>(json.size()));
}

static std::vector<std::string> ReadClientIdsFile() {
    char modulePath[MAX_PATH] = {};
    DWORD n = GetModuleFileNameA(nullptr, modulePath, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) return {};

    std::string path(modulePath, modulePath + n);
    const std::size_t slash = path.find_last_of("\\/");
    if (slash == std::string::npos) return {};
    path.resize(slash + 1);
    path += "discord_client_ids.txt";

    FILE* f = nullptr;
    if (fopen_s(&f, path.c_str(), "rb") != 0 || !f) return {};

    std::vector<std::string> ids;
    char line[256] = {};
    while (std::fgets(line, sizeof(line), f)) {
        std::string id(line);
        while (!id.empty() && (id.back() == '\r' || id.back() == '\n' || id.back() == ' ' || id.back() == '\t')) id.pop_back();
        std::size_t first = 0;
        while (first < id.size() && (id[first] == ' ' || id[first] == '\t')) ++first;
        if (first) id.erase(0, first);
        if (!id.empty() && id[0] != '#') ids.push_back(id);
    }
    std::fclose(f);
    return ids;
}

static std::string ChooseRandomClientId(const char* explicitId) {
    if (explicitId && *explicitId && std::string(explicitId) != "RANDOM") return explicitId;

    static std::vector<std::string> ids;
    ids = ReadClientIdsFile();
    if (ids.empty()) return {};

    static std::mt19937 rng(static_cast<unsigned int>(GetTickCount64() ^ GetCurrentProcessId()));
    std::uniform_int_distribution<std::size_t> dist(0, ids.size() - 1);
    return ids[dist(rng)];
}

static bool ConnectPipe() {
    for (int i = 0; i < 10; ++i) {
        char pipeName[64] = {};
        std::snprintf(pipeName, sizeof(pipeName), "\\\\.\\pipe\\discord-ipc-%d", i);
        g_pipe = CreateFileA(pipeName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (g_pipe != INVALID_HANDLE_VALUE) return true;
        Sleep(40);
    }
    return false;
}

bool Initialize(const char* clientId) {
    Shutdown();

    std::string fileId;
    static char envId[128] = {};
    if (!clientId || !*clientId) {
        DWORD n = GetEnvironmentVariableA("RAVENXD_DISCORD_CLIENT_ID", envId, sizeof(envId));
        if (n > 0 && n < sizeof(envId)) clientId = envId;
    }
    fileId = ChooseRandomClientId(clientId);
    if (fileId.empty()) return false;
    g_clientId = fileId;
    if (!ConnectPipe()) return false;

    const std::string handshake =
        std::string("{\"v\":1,\"client_id\":\"") + Escape(g_clientId.c_str()) + "\"}";

    if (!WritePacket(0, handshake)) {
        Shutdown();
        return false;
    }

    g_start = static_cast<long long>(std::time(nullptr));
    g_lastCycle = 0;
    g_cycleIndex = 0;
    return true;
}

void UpdatePresence(const char* details, const char* state) {
    if (g_pipe == INVALID_HANDLE_VALUE) return;
    if (g_start == 0) g_start = static_cast<long long>(std::time(nullptr));

    const char* safeDetails = details ? details : "RavenXD Launcher";
    const char* safeState = state ? state : "Minecraft 1.8.9";

    std::ostringstream json;
    json << "{\"cmd\":\"SET_ACTIVITY\",\"args\":{";
    json << "\"pid\":" << GetCurrentProcessId() << ",\"activity\":{";
    json << "\"type\":0";
    json << ",\"details\":\"" << Escape(safeDetails) << "\"";
    json << ",\"state\":\"" << Escape(safeState) << "\"";
    json << ",\"timestamps\":{\"start\":" << g_start << "}";
    json << ",\"assets\":{";
    json << "\"large_image\":\"ravenxd\",\"large_text\":\"RavenXD\"";
    json << "}},\"instance\":true},\"nonce\":\"ravenxd\"}";

    if (!WritePacket(1, json.str())) Shutdown();
}

void UpdateAutoCycle() {
    if (g_pipe == INVALID_HANDLE_VALUE) return;

    const ULONGLONG now = GetTickCount64();
    if (g_lastCycle != 0 && now - g_lastCycle < g_cycleIntervalMs) return;

    g_lastCycle = now;
    g_cycleIndex = (g_cycleIndex + 1) % g_cycleCount;

    // Pick another registered Discord application ID each cycle when multiple IDs are configured.
    // Discord requires every ID in discord_client_ids.txt to belong to a real application you control.
    const std::string nextId = ChooseRandomClientId("RANDOM");
    if (!nextId.empty() && nextId != g_clientId) {
        CloseHandle(g_pipe);
        g_pipe = INVALID_HANDLE_VALUE;
        g_clientId = nextId;
        if (!ConnectPipe()) return;
        std::string handshake = "{\"v\":1,\"client_id\":\"";
        handshake += Escape(g_clientId.c_str());
        handshake += "\"}";
        if (!WritePacket(0, handshake)) { Shutdown(); return; }
    }

    UpdatePresence(g_cycleDetails[g_cycleIndex], g_cycleStates[g_cycleIndex]);
}

void Shutdown() {
    if (g_pipe != INVALID_HANDLE_VALUE) {
        CloseHandle(g_pipe);
        g_pipe = INVALID_HANDLE_VALUE;
    }
    g_start = 0;
    g_lastCycle = 0;
    g_cycleIndex = 0;
    g_clientId.clear();
}

bool IsConnected() {
    return g_pipe != INVALID_HANDLE_VALUE;
}
}
