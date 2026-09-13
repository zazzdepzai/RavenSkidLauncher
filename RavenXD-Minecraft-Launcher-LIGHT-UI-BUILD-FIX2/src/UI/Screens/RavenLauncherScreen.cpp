#include "RavenLauncherScreen.h"
#include "UI/Theme/Theme.h"
#include "imgui.h"
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <string>

namespace {
std::wstring FindJava(){
    wchar_t b[32768]{};
    DWORD n=SearchPathW(nullptr,L"javaw.exe",nullptr,32768,b,nullptr);
    return n?std::wstring(b,n):L"javaw.exe";
}
std::wstring W(const std::string&s){return std::wstring(s.begin(),s.end());}

void Card(ImDrawList* d, ImVec2 p, ImVec2 sz, ImU32 fill, ImU32 border, float r=18.0f){
    d->AddRectFilled(p,{p.x+sz.x,p.y+sz.y},fill,r);
    d->AddRect(p,{p.x+sz.x,p.y+sz.y},border,r,0,1.0f);
}

ImU32 Col(const ImVec4& c){return ImGui::ColorConvertFloat4ToU32(c);}
}

namespace UI::Screens {

RavenLauncherScreen::RavenLauncherScreen(){
    wchar_t b[32768]{};
    DWORD n=GetEnvironmentVariableW(L"APPDATA",b,32768);
    std::wstring app=n?std::wstring(b,n):L"";
    m_GameDir=std::filesystem::path(app+L"\\.minecraft").string();
    m_JavaPath=std::filesystem::path(FindJava()).string();
    m_ForgePath=m_GameDir+"\\forge-1.8.9.jar";
    m_RavenUrl="https://raw.githubusercontent.com/zazzdepzai/RavenLauncherXD/main/ravenXD-v2.jar";
}

RavenLauncherScreen::~RavenLauncherScreen(){m_Discord.Stop();}
void RavenLauncherScreen::Initialize(ID3D11Device*){}
void RavenLauncherScreen::SetUser(const std::string&u){
    m_User=u;
    if(m_Discord.Start(u)) m_Status="Discord RPC connected";
    else m_Status="Discord RPC unavailable — start Discord to enable presence";
}
void RavenLauncherScreen::Update(float){}

void RavenLauncherScreen::Draw(ImVec2 o,ImVec2 s){
    auto& c=Theme::Get().Colors();
    auto* d=ImGui::GetWindowDrawList();
    const ImU32 bg=Col(c.Background), surface=Col(c.CardSurface), border=Col(c.CardBorder);

    d->AddRectFilled(o,{o.x+s.x,o.y+s.y},bg,24.0f);

    // Sidebar
    const float sidebarW=190.0f;
    Card(d,{o.x+12,o.y+12},{sidebarW,s.y-24},surface,border,22.0f);
    ImGui::SetCursorScreenPos({o.x+32,o.y+28});
    ImGui::TextColored(c.TextPrimary,"RAVENXD");
    ImGui::SetCursorScreenPos({o.x+32,o.y+50});
    ImGui::TextDisabled("Minecraft Launcher");

    const char* icons[]={"H","M","A","S"};
    const char* labels[]={"Home","Mods","Accounts","Settings"};
    Tab pages[]={Tab::Play,Tab::Mods,Tab::Accounts,Tab::Settings};
    for(int i=0;i<4;i++){
        ImVec2 bp(o.x+24,o.y+92+i*48);
        const bool selected=m_Tab==pages[i];
        if(selected) d->AddRectFilled(bp,{bp.x+166,bp.y+40},Col(c.SidebarActive),12.0f);
        ImGui::SetCursorScreenPos({bp.x+12,bp.y+4});
        if(ImGui::Button((std::string(icons[i])+"##nav"+std::to_string(i)).c_str(),{32,32})) m_Tab=pages[i];
        ImGui::SameLine(bp.x+55);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY()+5);
        ImGui::TextColored(selected?c.TextPrimary:c.TextSecondary,"%s",labels[i]);
    }

    // Bottom status card
    Card(d,{o.x+24,o.y+s.y-112},{166,82},Col(c.OutlineSurface),border,16.0f);
    ImGui::SetCursorScreenPos({o.x+38,o.y+s.y-98});
    ImGui::TextColored(c.TextPrimary,"1.8.9");
    ImGui::SetCursorScreenPos({o.x+38,o.y+s.y-74});
    ImGui::TextDisabled("RavenXD v2");
    ImGui::SetCursorScreenPos({o.x+38,o.y+s.y-52});
    ImGui::TextColored(m_Discord.Connected()?c.TextPrimary:c.TextDisabled,
                       m_Discord.Connected()?"Discord RPC: ON":"Discord RPC: OFF");

    // Main area
    const float left=o.x+222.0f;
    const float top=o.y+20.0f;
    const float w=s.x-242.0f;
    ImGui::SetCursorScreenPos({left,top});
    ImGui::TextColored(c.TextPrimary,"RavenXD");
    ImGui::SameLine(); ImGui::TextDisabled("/ %s",labels[static_cast<int>(m_Tab==Tab::Play?0:m_Tab==Tab::Mods?1:m_Tab==Tab::Accounts?2:3)]);

    ImGui::SetCursorScreenPos({o.x+s.x-118,o.y+18});
    if(ImGui::Button("Log out",{92,32})&&m_OnLogout)m_OnLogout();

    ImGui::SetCursorScreenPos({left,top+52});
    if(m_Tab==Tab::Play) Play({left,top+52},w);
    else if(m_Tab==Tab::Mods) Mods({left,top+52},w);
    else if(m_Tab==Tab::Accounts) Accounts({left,top+52},w);
    else Settings({left,top+52},w);

    if(!m_Status.empty()){
        ImGui::SetCursorScreenPos({left, o.y+s.y-30});
        ImGui::TextDisabled("%s",m_Status.c_str());
    }
}

void RavenLauncherScreen::Play(ImVec2 p,float w){
    auto& c=Theme::Get().Colors(); auto* d=ImGui::GetWindowDrawList();
    const float heroH=154.0f;
    Card(d,p,{w,heroH},Col(c.GlassSurface),Col(c.GlassBorder),22.0f);
    ImGui::SetCursorScreenPos({p.x+24,p.y+22}); ImGui::TextColored(c.TextPrimary,"Minecraft 1.8.9");
    ImGui::SetCursorScreenPos({p.x+24,p.y+50}); ImGui::TextDisabled("Java Edition  •  RavenXD v2");
    ImGui::SetCursorScreenPos({p.x+24,p.y+84});
    ImGui::Text("Installation"); ImGui::SameLine();
    ImGui::RadioButton("Forge + RavenXD",&m_Version,0); ImGui::SameLine();
    ImGui::RadioButton("Vanilla",&m_Version,1);
    ImGui::SetCursorScreenPos({p.x+24,p.y+116});
    if(ImGui::Button("PLAY",{150,34})){
        std::string e; bool ok=false;
        if(m_Version==0) ok=m_Minecraft.LaunchForge(W(m_JavaPath),W(m_GameDir),W(m_ForgePath),m_User,e);
        else ok=m_Minecraft.LaunchVanilla(W(m_JavaPath),W(m_GameDir),W(m_GameDir+"\\versions\\1.8.9\\1.8.9.jar"),m_User,e);
        m_Status=ok?"Minecraft started.":e;
        if(ok)m_Discord.Update("Playing Minecraft 1.8.9","RavenXD v2");
    }

    const float y=p.y+heroH+18.0f;
    const float gap=14.0f;
    const float cw=(w-gap)/2.0f;
    Card(d,{p.x,y},{cw,124},Col(c.CardSurface),Col(c.CardBorder),18.0f);
    Card(d,{p.x+cw+gap,y},{cw,124},Col(c.CardSurface),Col(c.CardBorder),18.0f);

    ImGui::SetCursorScreenPos({p.x+18,y+18}); ImGui::TextColored(c.TextPrimary,"Minecraft files");
    ImGui::SetCursorScreenPos({p.x+18,y+48}); ImGui::TextDisabled("Download the official 1.8.9 client");
    ImGui::SetCursorScreenPos({p.x+18,y+78});
    if(ImGui::Button("DOWNLOAD 1.8.9",{150,34})){
        std::string e;
        if(m_Minecraft.InstallMinecraft18(W(m_GameDir),e)) m_Status="Minecraft 1.8.9 installed."; else m_Status=e;
    }

    ImGui::SetCursorScreenPos({p.x+cw+gap+18,y+18}); ImGui::TextColored(c.TextPrimary,"RavenXD v2");
    ImGui::SetCursorScreenPos({p.x+cw+gap+18,y+48}); ImGui::TextDisabled("Install the launcher mod into .minecraft/mods");
    ImGui::SetCursorScreenPos({p.x+cw+gap+18,y+78});
    if(ImGui::Button("INSTALL MOD",{150,34})){
        std::string e;
        if(m_Minecraft.InstallRaven(m_RavenUrl,W(m_GameDir+"\\mods"),e))m_Status="RavenXD v2 installed."; else m_Status=e;
    }

    ImGui::SetCursorScreenPos({p.x,y+heroH+160});
    ImGui::TextDisabled("Game directory: %s",m_GameDir.c_str());
}

void RavenLauncherScreen::Installations(ImVec2 p,float w){
    // Kept for compatibility with older callers.
    Play(p,w);
}

void RavenLauncherScreen::Mods(ImVec2 p,float w){
    auto& c=Theme::Get().Colors(); auto* d=ImGui::GetWindowDrawList();
    Card(d,p,{w,82},Col(c.CardSurface),Col(c.CardBorder),18.0f);
    ImGui::SetCursorScreenPos({p.x+18,p.y+15}); ImGui::TextColored(c.TextPrimary,"Modrinth");
    char b[256]{}; strncpy_s(b,m_Search.c_str(),sizeof(b)-1);
    ImGui::SetCursorScreenPos({p.x+18,p.y+42}); ImGui::SetNextItemWidth(w-130);
    if(ImGui::InputTextWithHint("##modsearch","Search mods for 1.8.9",b,sizeof(b)))m_Search=b;
    ImGui::SameLine(); if(ImGui::Button("Search",{92,28}))m_Mods=m_Modrinth.Search(m_Search.empty()?"1.8.9":m_Search);
    float y=p.y+96;
    for(size_t i=0;i<m_Mods.size()&&i<8;i++){
        Card(d,{p.x,y},{w,42},Col(c.OutlineSurface),Col(c.CardBorder),12.0f);
        ImGui::SetCursorScreenPos({p.x+14,y+11}); ImGui::Text("%s",m_Mods[i].title.c_str());
        ImGui::SetCursorScreenPos({p.x+w-104,y+7});
        if(ImGui::Button(("Install##"+std::to_string(i)).c_str(),{88,28})){
            std::string e; if(m_Modrinth.Install(m_Mods[i],m_GameDir+"\\mods",e))m_Status="Installed "+m_Mods[i].title;else m_Status=e;
        }
        y+=50;
    }
}

void RavenLauncherScreen::Accounts(ImVec2 p,float w){
    auto& c=Theme::Get().Colors(); auto* d=ImGui::GetWindowDrawList();
    Card(d,p,{w,150},Col(c.CardSurface),Col(c.CardBorder),18.0f);
    ImGui::SetCursorScreenPos({p.x+20,p.y+20}); ImGui::TextColored(c.TextPrimary,"Local account");
    ImGui::SetCursorScreenPos({p.x+20,p.y+56}); ImGui::Text("%s",m_User.empty()?"Player":m_User.c_str());
    ImGui::SetCursorScreenPos({p.x+20,p.y+88}); ImGui::TextDisabled("Microsoft authentication can be added separately; no credential bypass is used.");
}

void RavenLauncherScreen::Settings(ImVec2 p,float w){
    auto& c=Theme::Get().Colors(); auto* d=ImGui::GetWindowDrawList();
    Card(d,p,{w,190},Col(c.CardSurface),Col(c.CardBorder),18.0f);
    ImGui::SetCursorScreenPos({p.x+20,p.y+18}); ImGui::TextColored(c.TextPrimary,"Minecraft settings");
    ImGui::SetCursorScreenPos({p.x+20,p.y+52}); ImGui::Text("Java");
    char j[512]{}; strncpy_s(j,m_JavaPath.c_str(),sizeof(j)-1); ImGui::SetCursorScreenPos({p.x+110,p.y+48}); ImGui::SetNextItemWidth(w-130); if(ImGui::InputText("##java",j,sizeof(j)))m_JavaPath=j;
    ImGui::SetCursorScreenPos({p.x+20,p.y+94}); ImGui::Text("Game directory");
    char g[512]{}; strncpy_s(g,m_GameDir.c_str(),sizeof(g)-1); ImGui::SetCursorScreenPos({p.x+110,p.y+90}); ImGui::SetNextItemWidth(w-130); if(ImGui::InputText("##game",g,sizeof(g)))m_GameDir=g;
    ImGui::SetCursorScreenPos({p.x+20,p.y+138}); ImGui::TextDisabled("Discord RPC uses the configured application IDs automatically.");
}

} // namespace UI::Screens
