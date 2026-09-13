#include "DiscordRPC.h"
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <random>
#include <algorithm>
#include <cctype>
#include <array>

namespace {
std::string Esc(const std::string& s){std::string o;for(char c:s){if(c=='"'||c=='\\')o+='\\';o+=c;}return o;}
std::wstring ExeDirectory(){wchar_t p[32768]{};DWORD n=GetModuleFileNameW(nullptr,p,32768);return n?std::filesystem::path(std::wstring(p,n)).parent_path().wstring():L".";}
std::string RandomClientId(){
    static const std::array<const char*,4> ids={
        "1547451186913878046","1548190308733423707","1548191970516668456","1548192094911471626"
    };
    static std::mt19937 rng([]{std::random_device rd;return rd();}());
    std::uniform_int_distribution<size_t> dist(0,ids.size()-1);
    return ids[dist(rng)];
}
}

namespace Launcher {

bool DiscordRPC::Start(const std::string& username){
    Stop(); m_Connected=false;
    std::string clientId=RandomClientId();
    // Optional override for testing; otherwise rotate the four supplied IDs.
    std::ifstream f(std::filesystem::path(ExeDirectory())/L"discord_client_id.txt");
    std::string overrideId; if(f) std::getline(f,overrideId);
    overrideId.erase(std::remove_if(overrideId.begin(),overrideId.end(),[](unsigned char c){return std::isspace(c);}),overrideId.end());
    if(overrideId.size()>=17 && std::all_of(overrideId.begin(),overrideId.end(),[](unsigned char c){return std::isdigit(c)!=0;})) clientId=overrideId;

    for(int i=0;i<10;i++){
        std::wstring pipe=L"\\\\.\\pipe\\discord-ipc-"+std::to_wstring(i);
        m_Pipe=CreateFileW(pipe.c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_EXISTING,0,nullptr);
        if(m_Pipe!=INVALID_HANDLE_VALUE)break;
        m_Pipe=nullptr;
    }
    if(!m_Pipe)return false;

    const std::string nonce=std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    const std::string hello="{\"v\":1,\"client_id\":\""+Esc(clientId)+"\",\"nonce\":\""+nonce+"\"}";
    if(!WriteFrame(0,hello)){Stop();return false;}

    // Discord sends READY after a successful handshake. Do not report connected before that.
    const DWORD start=GetTickCount();
    while(GetTickCount()-start<2500){
        DWORD avail=0;
        if(!PeekNamedPipe(m_Pipe,nullptr,0,nullptr,&avail,nullptr)){Stop();return false;}
        if(avail>=8){
            uint32_t header[2]{};DWORD got=0;
            if(!ReadFile(m_Pipe,header,sizeof(header),&got,nullptr)||got!=sizeof(header)){Stop();return false;}
            std::string payload(header[1],'\0');
            if(header[1] && (!ReadFile(m_Pipe,payload.data(),header[1],&got,nullptr)||got!=header[1])){Stop();return false;}
            if(payload.find("READY")!=std::string::npos){m_Connected=true;break;}
        } else Sleep(15);
    }
    if(!m_Connected){Stop();return false;}
    Update("RavenXD Launcher","Logged in as "+username);
    return true;
}

bool DiscordRPC::WriteFrame(int opcode,const std::string& json){
    if(!m_Pipe)return false;
    uint32_t header[2]={static_cast<uint32_t>(opcode),static_cast<uint32_t>(json.size())};DWORD written=0;
    if(!WriteFile(m_Pipe,header,sizeof(header),&written,nullptr)||written!=sizeof(header))return false;
    if(json.empty())return true;
    return WriteFile(m_Pipe,json.data(),static_cast<DWORD>(json.size()),&written,nullptr)&&written==json.size();
}

void DiscordRPC::Update(const std::string& state,const std::string& details){
    if(!m_Pipe||!m_Connected)return;
    const std::string nonce=std::to_string(m_Nonce++);
    const std::string json="{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":"+std::to_string(GetCurrentProcessId())+",\"activity\":{\"details\":\""+Esc(details)+"\",\"state\":\""+Esc(state)+"\",\"type\":0}},\"nonce\":\""+nonce+"\"}";
    if(!WriteFrame(1,json))Stop();
}

void DiscordRPC::Stop(){if(m_Pipe){CloseHandle(m_Pipe);m_Pipe=nullptr;}m_Connected=false;}
bool DiscordRPC::Connected()const{return m_Connected;}
}
