#pragma once
#include <string>
#include <vector>

namespace Launcher {
struct Account {
    std::string username;
    std::string createdAt;
};

class AccountStore {
public:
    bool Register(const std::string& username, const std::string& password, const std::string& key, std::string& error);
    bool Login(const std::string& username, const std::string& password, std::string& error);
    bool Exists(const std::string& username) const;
    std::vector<Account> Accounts() const;
    std::string CurrentUser() const { return m_CurrentUser; }
    void Logout() { m_CurrentUser.clear(); }
private:
    struct Record { std::string username, salt, hash, createdAt; };
    std::vector<Record> Load() const;
    bool Save(const std::vector<Record>& records) const;
    static bool HashPassword(const std::string& password, const std::string& salt, std::string& out);
    std::string m_CurrentUser;
};
}
