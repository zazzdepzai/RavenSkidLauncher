#include "ModrinthClient.h"
#include <Windows.h>
#include <winhttp.h>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cstdio>

#pragma comment(lib, "winhttp.lib")

namespace {
std::wstring U8(const std::string& s) {
    if (s.empty()) return {};
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (n <= 1) return {};
    std::wstring w(static_cast<size_t>(n - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), n);
    return w;
}

std::string U8(const std::wstring& w) {
    if (w.empty()) return {};
    const int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (n <= 1) return {};
    std::string s(static_cast<size_t>(n - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, s.data(), n, nullptr, nullptr);
    return s;
}

std::string Enc(const std::string& s) {
    std::ostringstream o;
    o << std::hex << std::uppercase;
    for (unsigned char c : s) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') o << c;
        else o << '%' << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    }
    return o.str();
}

bool RunCore(const std::wstring& command, const std::vector<std::wstring>& args, std::string& e) {
    wchar_t module[MAX_PATH]{};
    const DWORD n = GetModuleFileNameW(nullptr, module, MAX_PATH);
    if (!n || n >= MAX_PATH) { e = "Cannot locate RavenXD executable."; return false; }
    std::filesystem::path root(module);
    root = root.parent_path();
    const std::filesystem::path bundledNode = root / L"core" / L"node.exe";
    const std::filesystem::path script = root / L"core" / L"bridge.mjs";
    std::wstring node = std::filesystem::exists(bundledNode) ? bundledNode.wstring() : L"node.exe";
    if (!std::filesystem::exists(script)) { e = "RavenXD core bridge is missing: core\\bridge.mjs"; return false; }

    auto quote = [](const std::wstring& v) {
        std::wstring q = L"\"";
        for (wchar_t c : v) {
            if (c == L'\"') q += L"\"";
            else q += c;
        }
        q += L"\"";
        return q;
    };
    std::wstring cmd = quote(node) + L" " + quote(script) + L" " + command;
    for (const auto& a : args) cmd += L" " + quote(a);

    STARTUPINFOW si{ sizeof(si) };
    PROCESS_INFORMATION pi{};
    std::vector<wchar_t> buffer(cmd.begin(), cmd.end());
    buffer.push_back(L'\0');
    if (!CreateProcessW(nullptr, buffer.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                        nullptr, root.wstring().c_str(), &si, &pi)) {
        e = "Could not start RavenXD Minecraft core. Install Node.js 18+ or use the packaged launcher.";
        return false;
    }
    DWORD code = STILL_ACTIVE;
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    if (code != 0) { e = "RavenXD Minecraft core failed with exit code " + std::to_string(code) + ". Check the launcher core log."; return false; }
    return true;
}

bool ExtractVersionUrl(const std::string& manifest, const std::string& wanted, std::string& out) {
    const std::regex entry(
        R"rx(\{[^\{\}]*"id"\s*:\s*")rx" + wanted +
        R"rx("[^\{\}]*"url"\s*:\s*"([^"]+)")rx");
    std::smatch m;
    if (!std::regex_search(manifest, m, entry) || m.size() < 2) return false;
    out = m[1].str();
    return true;
}

bool ExtractClientUrl(const std::string& versionJson, std::string& out) {
    const auto downloads = versionJson.find("\"downloads\"");
    if (downloads == std::string::npos) return false;
    const auto client = versionJson.find("\"client\"", downloads);
    if (client == std::string::npos) return false;
    const auto url = versionJson.find("\"url\"", client);
    if (url == std::string::npos) return false;
    const auto colon = versionJson.find(':', url);
    const auto first = versionJson.find('"', colon + 1);
    const auto last = versionJson.find('"', first + 1);
    if (colon == std::string::npos || first == std::string::npos || last == std::string::npos) return false;
    out = versionJson.substr(first + 1, last - first - 1);
    return !out.empty();
}
}

namespace Launcher {

std::string ModrinthClient::Get(const std::wstring& url, std::string& e) {
    URL_COMPONENTS c{};
    c.dwStructSize = sizeof(c);
    wchar_t host[512]{}, path[8192]{}, extra[4096]{};
    c.lpszHostName = host; c.dwHostNameLength = static_cast<DWORD>(std::size(host));
    c.lpszUrlPath = path; c.dwUrlPathLength = static_cast<DWORD>(std::size(path));
    c.lpszExtraInfo = extra; c.dwExtraInfoLength = static_cast<DWORD>(std::size(extra));

    if (!WinHttpCrackUrl(url.c_str(), 0, 0, &c)) {
        e = "Invalid URL";
        return {};
    }

    HINTERNET ses = WinHttpOpen(L"RavenXD-Launcher/1.1", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!ses) { e = "WinHTTP init failed"; return {}; }
    WinHttpSetTimeouts(ses, 15000, 15000, 30000, 30000);

    HINTERNET con = WinHttpConnect(ses, host, c.nPort, 0);
    if (!con) { WinHttpCloseHandle(ses); e = "HTTP connect failed"; return {}; }

    std::wstring requestPath = path;
    requestPath += extra;
    const DWORD flags = (c.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET req = WinHttpOpenRequest(con, L"GET", requestPath.c_str(), nullptr,
                                       WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!req) {
        WinHttpCloseHandle(con); WinHttpCloseHandle(ses);
        e = "HTTP request creation failed";
        return {};
    }

    std::string out;
    bool ok = WinHttpSendRequest(req, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, 0) != FALSE;
    ok = ok && WinHttpReceiveResponse(req, nullptr) != FALSE;

    DWORD status = 0, statusSize = sizeof(status);
    if (ok) WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize, WINHTTP_NO_HEADER_INDEX);

    if (!ok) {
        e = "HTTP request failed";
    } else if (status < 200 || status >= 300) {
        e = "HTTP error " + std::to_string(status);
        ok = false;
    } else {
        char buffer[64 * 1024];
        DWORD got = 0;
        while (WinHttpReadData(req, buffer, sizeof(buffer), &got) && got > 0) {
            out.append(buffer, got);
        }
        if (out.empty()) {
            e = "Server returned an empty response";
            ok = false;
        }
    }

    WinHttpCloseHandle(req);
    WinHttpCloseHandle(con);
    WinHttpCloseHandle(ses);
    return ok ? out : std::string{};
}

std::vector<ModInfo> ModrinthClient::Search(const std::string& q, const std::string& game, const std::string& loader) {
    std::string e;
    const std::string facet = "[\"project_type:mod\",\"versions:" + game + "\",\"categories:" + loader + "\"]";
    std::wstring url = L"https://api.modrinth.com/v2/search?limit=12&query=" + U8(q) + L"&facets=" + U8(facet);
    const std::string j = Get(url, e);
    std::vector<ModInfo> out;
    if (j.empty()) return out;

    const std::regex item(R"rx(\{[^{}]*"slug":"([^"]+)"[^{}]*"title":"([^"]+)")rx");
    for (auto it = std::sregex_iterator(j.begin(), j.end(), item); it != std::sregex_iterator() && out.size() < 12; ++it)
        out.push_back({ (*it)[2], (*it)[1], "", "", false });
    return out;
}

bool ModrinthClient::Download(const std::string& url, const std::wstring& path, std::string& e) {
    if (url.empty()) { e = "Download URL is empty."; return false; }
    const std::filesystem::path destination(path);
    std::error_code ec;
    std::filesystem::create_directories(destination.parent_path(), ec);
    if (ec) { e = "Cannot create destination directory: " + ec.message(); return false; }

    const std::string body = Get(U8(url), e);
    if (body.empty()) return false;

    const std::filesystem::path temp = destination.wstring() + L".part";
    {
        std::ofstream f(temp, std::ios::binary | std::ios::trunc);
        if (!f) { e = "Cannot create download file."; return false; }
        f.write(body.data(), static_cast<std::streamsize>(body.size()));
        if (!f.good()) { e = "Failed while writing download."; return false; }
    }
    std::filesystem::remove(destination, ec);
    std::filesystem::rename(temp, destination, ec);
    if (ec) { e = "Cannot finalize download: " + ec.message(); return false; }
    return true;
}

bool ModrinthClient::Install(const ModInfo& m, const std::string& modsDir, std::string& e) {
    const std::wstring u = L"https://api.modrinth.com/v2/project/" + U8(m.slug) +
        L"/version?game_versions=[%221.8.9%22]&loaders=[%22forge%22]";
    const std::string j = Get(u, e);
    if (j.empty()) return false;
    const std::regex file(R"rx("url":"(https:[^"]+)".*?"filename":"([^"]+)")rx");
    std::smatch x;
    if (!std::regex_search(j, x, file)) { e = "No Forge 1.8.9 file found."; return false; }
    std::filesystem::create_directories(modsDir);
    const std::wstring destination = (std::filesystem::path(U8(modsDir)) / U8(x[2].str())).wstring();
    return Download(x[1].str(), destination, e);
}

bool MinecraftLauncher::IsMinecraft18Installed(const std::wstring& gameDir) {
    namespace fs = std::filesystem;
    const auto dir = fs::path(gameDir) / L"versions" / L"1.8.9";
    return fs::exists(dir / L"1.8.9.json") && fs::exists(dir / L"1.8.9.jar");
}

bool MinecraftLauncher::IsForge18Installed(const std::wstring& gameDir) {
    namespace fs = std::filesystem;
    std::error_code ec;
    const auto versionsDir = fs::path(gameDir) / L"versions";
    if (!fs::exists(versionsDir, ec)) return false;
    for (const auto& entry : fs::directory_iterator(versionsDir, ec)) {
        if (!entry.is_directory()) continue;
        const std::wstring name = entry.path().filename().wstring();
        if (name.rfind(L"1.8.9-forge-", 0) == 0) {
            // A forge version directory exists only when its .json + .jar were fully written.
            if (fs::exists(entry.path() / (name + L".json")) && fs::exists(entry.path() / (name + L".jar")))
                return true;
        }
    }
    return false;
}

bool MinecraftLauncher::IsRavenModInstalled(const std::wstring& modsDir) {
    return std::filesystem::exists(std::filesystem::path(modsDir) / L"RavenXD-v2.jar");
}

bool MinecraftLauncher::InstallRaven(const std::string& url, const std::wstring& modsDir, std::string& e, bool force) {
    if (!force && IsRavenModInstalled(modsDir)) { e = "RavenXD v2 is already installed."; return true; }
    ModrinthClient c;
    std::filesystem::create_directories(modsDir);
    return c.Download(url, modsDir + L"\\RavenXD-v2.jar", e);
}

bool MinecraftLauncher::LaunchForge(const std::wstring& javaw, const std::wstring& gameDir,
                                    const std::wstring& forgeJar, const std::string& user, std::string& e) {
    (void)forgeJar;
    if (!std::filesystem::exists(javaw)) { e = "javaw.exe not found. Install Java 8 or select javaw.exe in Settings."; return false; }
    std::filesystem::create_directories(gameDir);
    // bridge.mjs's forge-launch already skips (re)installing Forge when a matching
    // versions/1.8.9-forge-* directory is present, and only re-verifies libraries/assets
    // once per install thanks to the .raven-verified marker it writes (see bridge.mjs).
    return RunCore(L"forge-launch", { gameDir, javaw, U8(user) }, e);
}

bool MinecraftLauncher::LaunchVanilla(const std::wstring& javaw, const std::wstring& gameDir,
                                      const std::wstring& jar, const std::string& user, std::string& e) {
    (void)jar;
    if (!std::filesystem::exists(javaw)) { e = "javaw.exe not found. Install Java 8 or select javaw.exe in Settings."; return false; }
    std::filesystem::create_directories(gameDir);
    if (!IsMinecraft18Installed(gameDir)) {
        if (!RunCore(L"install", { gameDir }, e)) return false;
    }
    return RunCore(L"launch", { gameDir, javaw, U8(user) }, e);
}

bool MinecraftLauncher::InstallMinecraft18(const std::wstring& gameDir, std::string& e, bool force) {
    std::filesystem::create_directories(gameDir);
    // Previously this always re-ran the full "install" command (which re-verifies every
    // library/asset in the whole .minecraft directory) even when 1.8.9 was already present.
    // Skip that work unless the user explicitly asked to repair/reinstall.
    if (!force && IsMinecraft18Installed(gameDir)) { e = "Minecraft 1.8.9 is already installed."; return true; }
    return RunCore(force ? L"repair" : L"install", { gameDir }, e);
}

} // namespace Launcher
