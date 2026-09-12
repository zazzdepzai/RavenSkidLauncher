#pragma once

// RavenXD local/offline launcher profiles.
// No Microsoft passwords, access tokens, or session secrets are stored.
static std::vector<std::string> ravenAccounts;
static int ravenSelectedAccount = 0;
static char ravenAccountInput[64] = "";
static bool ravenAccountsLoaded = false;

static std::string RavenAccountFile()
{
    wchar_t buf[MAX_PATH] = {};
    DWORD n = GetEnvironmentVariableW(L"APPDATA", buf, MAX_PATH);
    if (!n || n >= MAX_PATH) return "ravenxd_accounts.txt";

    std::wstring p(buf, n);
    p += L"\\RavenXD";
    std::error_code ec;
    std::filesystem::create_directories(p, ec);
    p += L"\\accounts.txt";

    int len = WideCharToMultiByte(CP_UTF8, 0, p.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (!len) return "ravenxd_accounts.txt";

    std::string out(static_cast<size_t>(len - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, p.c_str(), -1, out.data(), len, nullptr, nullptr);
    return out;
}

static void RavenSaveAccounts()
{
    const std::string path = RavenAccountFile();
    std::ofstream f(path.c_str(), std::ios::trunc);
    if (!f) return;

    for (const auto& account : ravenAccounts) {
        if (!account.empty()) f << account << '\n';
    }
}

static void RavenLoadAccounts()
{
    if (ravenAccountsLoaded) return;
    ravenAccountsLoaded = true;

    const std::string path = RavenAccountFile();
    std::ifstream f(path.c_str());
    std::string line;

    while (std::getline(f, line)) {
        if (!line.empty() && ravenAccounts.size() < 32)
            ravenAccounts.push_back(line);
    }

    if (ravenAccounts.empty())
        ravenAccounts.push_back("RavenXD");

    if (ravenSelectedAccount < 0 || ravenSelectedAccount >= static_cast<int>(ravenAccounts.size()))
        ravenSelectedAccount = 0;
}
