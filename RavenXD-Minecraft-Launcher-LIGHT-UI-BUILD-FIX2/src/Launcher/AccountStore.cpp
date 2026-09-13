#include "AccountStore.h"
#include <Windows.h>
#include <bcrypt.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#pragma comment(lib, "bcrypt.lib")

namespace {
std::filesystem::path DbPath() {
    wchar_t buf[32768]{};
    DWORD n=GetEnvironmentVariableW(L"APPDATA",buf,32768);
    std::filesystem::path p = n ? std::filesystem::path(buf) : std::filesystem::current_path();
    p /= L"RavenXD"; std::filesystem::create_directories(p);
    return p / L"accounts.db";
}
std::string Hex(const std::vector<unsigned char>& b) {
    std::ostringstream s; s<<std::hex<<std::setfill('0');
    for(auto c:b) s<<std::setw(2)<<(int)c;
    return s.str();
}
std::string RandomSalt() {
    std::random_device rd; std::vector<unsigned char> b(16);
    for(auto& x:b) x=(unsigned char)rd();
    return Hex(b);
}
std::string Now() {
    SYSTEMTIME t{}; GetLocalTime(&t); char b[32];
    sprintf_s(b,"%04u-%02u-%02u %02u:%02u",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute);
    return b;
}
}
namespace Launcher {
std::vector<AccountStore::Record> AccountStore::Load() const {
    std::vector<Record> out; std::ifstream f(DbPath(),std::ios::binary); std::string line;
    while(std::getline(f,line)) {
        std::stringstream ss(line); Record r;
        if(std::getline(ss,r.username,'|')&&std::getline(ss,r.salt,'|')&&std::getline(ss,r.hash,'|')&&std::getline(ss,r.createdAt))
            out.push_back(std::move(r));
    } return out;
}
bool AccountStore::Save(const std::vector<Record>& rs) const {
    std::ofstream f(DbPath(),std::ios::binary|std::ios::trunc);
    if(!f) return false;
    for(auto&r:rs) f<<r.username<<'|'<<r.salt<<'|'<<r.hash<<'|'<<r.createdAt<<"\n";
    return true;
}
bool AccountStore::HashPassword(const std::string& password,const std::string& salt,std::string& out) {
    std::vector<unsigned char> data(password.begin(),password.end());
    data.insert(data.end(),salt.begin(),salt.end());
    BCRYPT_ALG_HANDLE alg=nullptr;
    if(BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)!=0) return false;
    std::vector<unsigned char> hash(32); bool ok=BCryptHash(alg,nullptr,0,data.data(),(ULONG)data.size(),hash.data(),(ULONG)hash.size())==0;
    BCryptCloseAlgorithmProvider(alg,0); if(ok) out=Hex(hash); return ok;
}
bool AccountStore::Exists(const std::string& u) const {
    auto rs=Load(); return std::any_of(rs.begin(),rs.end(),[&](auto&r){return _stricmp(r.username.c_str(),u.c_str())==0;});
}
bool AccountStore::Register(const std::string& u,const std::string&p,const std::string&key,std::string&e) {
    if(u.size()<3){e="Username must be at least 3 characters.";return false;}
    if(p.size()<4){e="Password must be at least 4 characters.";return false;}
    if(key.empty()){e="Enter an invite/license key.";return false;}
    if(Exists(u)){e="Account already exists.";return false;}
    Record r; r.username=u; r.salt=RandomSalt(); r.createdAt=Now();
    if(!HashPassword(p,r.salt,r.hash)){e="Password hashing failed.";return false;}
    auto rs=Load(); rs.push_back(r); if(!Save(rs)){e="Could not save account.";return false;}
    m_CurrentUser=u; return true;
}
bool AccountStore::Login(const std::string&u,const std::string&p,std::string&e) {
    for(auto&r:Load()) if(_stricmp(r.username.c_str(),u.c_str())==0) {
        std::string h; if(!HashPassword(p,r.salt,h)||h!=r.hash){e="Invalid username or password.";return false;}
        m_CurrentUser=r.username; return true;
    }
    e="Account does not exist."; return false;
}
std::vector<Account> AccountStore::Accounts() const {
    std::vector<Account> out; for(auto&r:Load()) out.push_back({r.username,r.createdAt}); return out;
}
}
