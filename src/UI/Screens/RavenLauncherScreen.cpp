#include "RavenLauncherScreen.h"
#include "UI/Theme/Theme.h"
#include "UI/Theme/FontSettings.h"
#include "UI/Effects/IceEffect.h"
#include "UI/Effects/ShaderPack.h"
#include "imgui.h"
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
#include <shellapi.h>
#include <chrono>
#include <ctime>

namespace {
std::wstring FindJava(){ wchar_t b[32768]{}; DWORD n=SearchPathW(nullptr,L"javaw.exe",nullptr,32768,b,nullptr); return n?std::wstring(b,n):L"javaw.exe"; }
std::wstring W(const std::string&s){ return std::wstring(s.begin(),s.end()); }
ImU32 Col(const ImVec4& c){ return ImGui::ColorConvertFloat4ToU32(c); }

void Glass(ImDrawList* d, ImVec2 p, ImVec2 sz, ImU32 fill, ImU32 border, float r=18.0f){
    d->AddRectFilled(p,{p.x+sz.x,p.y+sz.y},fill,r);
    d->AddRect(p,{p.x+sz.x,p.y+sz.y},border,r,0,1.0f);
}

void Icon(ImDrawList* d, ImVec2 c, int type, ImU32 col, bool active){
    const ImU32 glow=IM_COL32(75,165,255,active?70:25);
    if(active) d->AddCircleFilled(c,17,glow,24);
    d->AddCircle(c,9.5f,col,24,1.7f);
    if(type==0){ // home
        d->AddLine({c.x-5,c.y+1},{c.x,c.y-4},col,1.9f); d->AddLine({c.x,c.y-4},{c.x+5,c.y+1},col,1.9f);
        d->AddLine({c.x-4,c.y+1},{c.x-4,c.y+5},col,1.9f); d->AddLine({c.x+4,c.y+1},{c.x+4,c.y+5},col,1.9f);
        d->AddLine({c.x-4,c.y+5},{c.x+4,c.y+5},col,1.9f);
    } else if(type==1){ // mods
        d->AddLine({c.x-5,c.y-5},{c.x+5,c.y+5},col,1.7f); d->AddLine({c.x+5,c.y-5},{c.x-5,c.y+5},col,1.7f);
        d->AddCircleFilled({c.x-5,c.y-5},2.0f,col,12); d->AddCircleFilled({c.x+5,c.y+5},2.0f,col,12);
    } else if(type==2){ // account
        d->AddCircle({c.x,c.y-3},3.0f,col,16,1.8f);
        // Older Dear ImGui versions do not expose AddArc; draw the lower arc manually.
        ImVec2 pts[17]{};
        for(int i=0;i<17;i++){ float a=3.35f + (6.08f-3.35f)*(float(i)/16.0f); pts[i]={c.x+6.0f*cosf(a),c.y+4.0f+6.0f*sinf(a)}; }
        d->AddPolyline(pts,17,col,ImDrawFlags_None,1.8f);
    } else if(type==3){ // settings
        d->AddCircle(c,3.0f,col,16,1.8f); for(int i=0;i<8;i++){float a=i*0.785398f; ImVec2 a1{c.x+6*cosf(a),c.y+6*sinf(a)}; ImVec2 a2{c.x+9*cosf(a),c.y+9*sinf(a)}; d->AddLine(a1,a2,col,1.7f);} 
    } else { // logs
        d->AddRect(ImVec2{c.x-6,c.y-8}, ImVec2{c.x+6,c.y+8}, col, 2.0f, ImDrawFlags_None, 1.7f); d->AddLine({c.x-3,c.y-3},{c.x+3,c.y-3},col,1.5f); d->AddLine({c.x-3,c.y+1},{c.x+4,c.y+1},col,1.5f); d->AddLine({c.x-3,c.y+5},{c.x+2,c.y+5},col,1.5f);
    }
}

bool Toggle(const char* id, bool& value, ImVec2 p, ImDrawList* d){
    const ImVec2 sz(46,27); ImGui::SetCursorScreenPos(p); bool pressed=ImGui::InvisibleButton(id,sz); if(pressed)value=!value;
    const bool hover=ImGui::IsItemHovered();
    const ImU32 on=hover?IM_COL32(75,170,255,255):IM_COL32(45,135,245,240), off=hover?IM_COL32(75,92,116,205):IM_COL32(55,70,92,175);
    d->AddRectFilled(p,{p.x+46,p.y+27},value?on:off,14); if(value)d->AddRect(p,ImVec2{p.x+46,p.y+27},IM_COL32(140,215,255,100),14.0f,ImDrawFlags_None,1.0f);
    d->AddCircleFilled({p.x+(value?32:14),p.y+13.5f},9.2f,IM_COL32(246,250,255,255),20); return pressed;
}

bool BlueButton(const char* label, ImVec2 p, ImVec2 size, ImDrawList* d, bool primary=true){
    ImGui::SetCursorScreenPos(p); bool pressed=ImGui::InvisibleButton(label,size); const bool hovered=ImGui::IsItemHovered(), held=ImGui::IsItemActive();
    ImVec2 q{p.x+size.x,p.y+size.y};
    ImU32 top=primary?IM_COL32(76,164,255,255):IM_COL32(28,63,105,235);
    ImU32 bottom=primary?IM_COL32(25,92,220,250):IM_COL32(16,35,65,225);
    if(hovered){top=primary?IM_COL32(108,190,255,255):IM_COL32(42,83,130,245);bottom=primary?IM_COL32(36,112,238,255):IM_COL32(22,48,82,240);}
    if(held){top=primary?IM_COL32(55,135,245,255):IM_COL32(30,65,100,245);}
    if(hovered && primary)d->AddRectFilled({p.x-4,p.y-4},{q.x+4,q.y+4},IM_COL32(55,145,255,28),16);
    d->AddRectFilled(p,q,top,14); d->AddRectFilled({p.x,p.y+size.y*.50f},q,bottom,14);
    d->AddRect(p,q,hovered?IM_COL32(155,225,255,155):IM_COL32(120,195,255,95),14,0,1.0f);
    ImVec2 ts=ImGui::CalcTextSize(label); ImGui::SetCursorScreenPos({p.x+(size.x-ts.x)*.5f,p.y+(size.y-ts.y)*.5f}); ImGui::TextColored(ImVec4(.97f,.99f,1,1),"%s",label); return pressed;
}

void SectionTitle(const UI::ThemeTypography& t, const char* title, const char* sub, ImVec2 p){
    if(t.Heading) ImGui::PushFont(t.Heading); ImGui::SetCursorScreenPos(p); ImGui::TextUnformatted(title); if(t.Heading) ImGui::PopFont();
    ImGui::SetCursorScreenPos({p.x,p.y+27}); ImGui::TextDisabled("%s",sub);
}

std::string Now(){ auto tp=std::chrono::system_clock::now(); std::time_t tt=std::chrono::system_clock::to_time_t(tp); tm tmv{}; localtime_s(&tmv,&tt); char b[32]{}; sprintf_s(b,"%02d:%02d:%02d",tmv.tm_hour,tmv.tm_min,tmv.tm_sec); return b; }
}

namespace UI::Screens {
RavenLauncherScreen::RavenLauncherScreen(){
    wchar_t b[32768]{}; DWORD n=GetEnvironmentVariableW(L"APPDATA",b,32768); std::wstring app=n?std::wstring(b,n):L"";
    m_GameDir=std::filesystem::path(app+L"\\.minecraft").string(); m_JavaPath=std::filesystem::path(FindJava()).string(); m_ForgePath=m_GameDir+"\\forge-1.8.9.jar";
    m_RavenUrl="https://raw.githubusercontent.com/zazzdepzai/RavenLauncherXD/main/ravenXD-v2.jar";
    AddLog("RavenXD launcher initialized"); AddLog("UI system loaded"); AddLog("Custom Shader Pack: 39 effects available");
}
RavenLauncherScreen::~RavenLauncherScreen(){m_Discord.Stop(); if(m_Worker.joinable())m_Worker.join();}
void RavenLauncherScreen::Initialize(ID3D11Device*){ ApplyTheme(); }
void RavenLauncherScreen::SetUser(const std::string&u){m_User=u; AddLog("Authenticated as "+(u.empty()?std::string("Player"):u)); if(m_Discord.Start(u)){SetStatus("Discord RPC connected");AddLog("Discord RPC connected");}else{SetStatus("Discord RPC unavailable");AddLog("Discord RPC unavailable","WARN");}}
void RavenLauncherScreen::Update(float dt){m_Time+=dt; ApplyTheme();}

void RavenLauncherScreen::AddLog(const std::string& msg,const char* level){
    std::string line="["+Now()+"] ["+level+"] "+msg;
    { std::lock_guard<std::mutex> lock(m_LogMutex); m_Logs.push_back(line); if(m_Logs.size()>160)m_Logs.erase(m_Logs.begin(),m_Logs.begin()+40); }
    wchar_t b[32768]{}; DWORD n=GetEnvironmentVariableW(L"APPDATA",b,32768); std::filesystem::path dir=n?std::filesystem::path(b):std::filesystem::temp_directory_path(); dir/=L"RavenXD"; std::error_code ec; std::filesystem::create_directories(dir,ec); std::ofstream f(dir/"launcher.log",std::ios::app); if(f)f<<line<<'\n';
}

void RavenLauncherScreen::SetStatus(const std::string& s){ std::lock_guard<std::mutex> lock(m_StatusMutex); m_Status=s; }
std::string RavenLauncherScreen::GetStatus(){ std::lock_guard<std::mutex> lock(m_StatusMutex); return m_Status; }

void RavenLauncherScreen::RunAsync(const std::string& busyLabel, std::function<void()> work){
    if(m_Busy.load()) return; // one background task at a time; buttons are also hidden/disabled while busy.
    if(m_Worker.joinable()) m_Worker.join(); // previous task already finished (guarded by m_Busy) - safe to reap.
    m_BusyLabel=busyLabel; m_Busy=true;
    m_Worker=std::thread([this,work]{
        work();
        m_Busy=false;
    });
}

void RavenLauncherScreen::ApplyTheme(){
    auto& f=UI::FontSettings(); auto& c=Theme::Get().MutableColors();
    c.TextPrimary=ImVec4(.95f,.96f,.99f,1); c.TextSecondary=ImVec4(.48f,.52f,.60f,1); c.TextDisabled=ImVec4(.32f,.37f,.46f,1);
    if(f.themePreset==0){ // Raven Blue
        c.Background=ImVec4(.02f,.04f,.08f,1); c.Accent=ImVec4(.29f,.62f,1,1); c.AccentHover=ImVec4(.45f,.75f,1,1); c.AccentPressed=ImVec4(.16f,.46f,.92f,1); c.CardSurface=ImVec4(.08f,.20f,.36f,.42f); c.GlassSurface=ImVec4(.08f,.22f,.40f,.46f); c.CardBorder=ImVec4(.35f,.67f,1,.20f); c.GlassBorder=ImVec4(.45f,.78f,1,.25f); c.SidebarActive=ImVec4(.16f,.48f,.92f,.30f);
    } else if(f.themePreset==1){ // Midnight
        c.Background=ImVec4(.015f,.018f,.028f,1); c.Accent=ImVec4(.70f,.75f,.85f,1); c.AccentHover=ImVec4(.88f,.92f,1,1); c.AccentPressed=ImVec4(.48f,.55f,.68f,1); c.CardSurface=ImVec4(.09f,.10f,.14f,.55f); c.GlassSurface=ImVec4(.10f,.11f,.16f,.56f); c.CardBorder=ImVec4(.65f,.72f,.86f,.18f); c.GlassBorder=ImVec4(.75f,.82f,1,.22f); c.SidebarActive=ImVec4(.40f,.46f,.58f,.28f);
    } else if(f.themePreset==2){ // Purple
        c.Background=ImVec4(.045f,.018f,.08f,1); c.Accent=ImVec4(.67f,.38f,1,1); c.AccentHover=ImVec4(.80f,.58f,1,1); c.AccentPressed=ImVec4(.50f,.25f,.85f,1); c.CardSurface=ImVec4(.22f,.08f,.38f,.42f); c.GlassSurface=ImVec4(.27f,.10f,.46f,.48f); c.CardBorder=ImVec4(.76f,.50f,1,.22f); c.GlassBorder=ImVec4(.82f,.62f,1,.26f); c.SidebarActive=ImVec4(.56f,.28f,.90f,.30f);
    } else if(f.themePreset==3){ // White glass
        c.Background=ImVec4(.91f,.94f,.98f,1); c.Accent=ImVec4(.10f,.42f,.85f,1); c.AccentHover=ImVec4(.18f,.52f,.95f,1); c.AccentPressed=ImVec4(.08f,.32f,.72f,1); c.CardSurface=ImVec4(.97f,.99f,1,.70f); c.GlassSurface=ImVec4(1,1,1,.74f); c.CardBorder=ImVec4(.20f,.38f,.62f,.22f); c.GlassBorder=ImVec4(.25f,.45f,.75f,.28f); c.SidebarActive=ImVec4(.30f,.58f,.95f,.24f); c.TextPrimary=ImVec4(.07f,.10f,.16f,1); c.TextSecondary=ImVec4(.28f,.33f,.42f,1); c.TextDisabled=ImVec4(.42f,.46f,.53f,1);
    } else { // Custom
        c.Background=ImVec4(f.customBackground[0],f.customBackground[1],f.customBackground[2],f.customBackground[3]); c.Accent=ImVec4(f.customAccent[0],f.customAccent[1],f.customAccent[2],f.customAccent[3]); c.AccentHover=ImVec4(std::min(1.f,f.customAccent[0]+.12f),std::min(1.f,f.customAccent[1]+.12f),std::min(1.f,f.customAccent[2]+.12f),1); c.AccentPressed=ImVec4(f.customAccent[0]*.72f,f.customAccent[1]*.72f,f.customAccent[2]*.72f,1); c.CardSurface=ImVec4(f.customCard[0],f.customCard[1],f.customCard[2],f.customCard[3]); c.GlassSurface=c.CardSurface; c.CardBorder=ImVec4(f.customAccent[0],f.customAccent[1],f.customAccent[2],.20f); c.GlassBorder=ImVec4(f.customAccent[0],f.customAccent[1],f.customAccent[2],.28f); c.SidebarActive=ImVec4(f.customAccent[0],f.customAccent[1],f.customAccent[2],.25f);
    }
}

void RavenLauncherScreen::Draw(ImVec2 o,ImVec2 s){
    ApplyTheme(); auto& c=Theme::Get().Colors(); auto& t=Theme::Get().Typography(); auto* d=ImGui::GetWindowDrawList();
    d->AddRectFilled(o,{o.x+s.x,o.y+s.y},Col(c.Background),24);
    if(UI::FontSettings().blueGlow){Effects::DrawBlueGlow(d,{o.x+225,o.y+140},90,.65f);Effects::DrawBlueGlow(d,{o.x+s.x-260,o.y+100},100,.55f);}
    const float sidebar=202.0f; Glass(d,{o.x+12,o.y+12},{sidebar,s.y-24},IM_COL32(7,19,38,215),Col(c.GlassBorder),22);
    if(t.Heading)ImGui::PushFont(t.Heading); ImGui::SetCursorScreenPos({o.x+34,o.y+28}); ImGui::TextColored(c.TextPrimary,"RavenXD"); if(t.Heading)ImGui::PopFont();
    ImGui::SetCursorScreenPos({o.x+34,o.y+51}); ImGui::TextColored(c.Accent,"Minecraft Launcher"); d->AddLine({o.x+30,o.y+78},{o.x+sidebar-20,o.y+78},Col(c.Divider),1);

    const char* labels[]={"Home","Mods","Accounts","Settings","Logs"}; const char* subs[]={"Main page","Manage mods","Your accounts","Launcher settings","Activity & debug"}; int icons[]={0,1,2,3,4}; Tab pages[]={Tab::Play,Tab::Mods,Tab::Accounts,Tab::Settings,Tab::Logs};
    for(int i=0;i<5;i++){
        ImVec2 bp{o.x+24,o.y+94+i*58}; bool selected=m_Tab==pages[i]; ImGui::SetCursorScreenPos(bp); bool hit=ImGui::InvisibleButton((std::string("nav##")+std::to_string(i)).c_str(),{178,48}); if(hit)m_Tab=pages[i]; bool hover=ImGui::IsItemHovered();
        if(selected||hover){d->AddRectFilled(bp,{bp.x+178,bp.y+48},selected?IM_COL32(40,125,245,72):IM_COL32(45,90,145,42),14); if(selected)d->AddRect({bp.x,bp.y},{bp.x+178,bp.y+48},IM_COL32(95,190,255,100),14,0,1);}
        Icon(d,{bp.x+25,bp.y+24},icons[i],selected?IM_COL32(175,225,255,255):IM_COL32(145,165,195,220),selected);
        ImGui::SetCursorScreenPos({bp.x+52,bp.y+8}); ImGui::TextColored(selected?c.TextPrimary:c.TextSecondary,"%s",labels[i]); ImGui::SetCursorScreenPos({bp.x+52,bp.y+27}); ImGui::TextColored(c.TextDisabled,"%s",subs[i]);
    }
    Glass(d,{o.x+24,o.y+s.y-108},{178,80},IM_COL32(8,25,48,190),Col(c.CardBorder),16); ImGui::SetCursorScreenPos({o.x+40,o.y+s.y-94}); ImGui::TextColored(c.TextPrimary,"RavenXD v2"); ImGui::SetCursorScreenPos({o.x+40,o.y+s.y-70}); ImGui::TextDisabled("Minecraft 1.8.9"); ImGui::SetCursorScreenPos({o.x+40,o.y+s.y-48}); ImGui::TextColored(m_Discord.Connected()?ImVec4(.31f,.86f,.60f,1):c.TextDisabled,m_Discord.Connected()?"● Discord connected":"● Discord offline");

    const float left=o.x+228, top=o.y+22, w=s.x-248; int idx=static_cast<int>(m_Tab==Tab::Play?0:m_Tab==Tab::Mods?1:m_Tab==Tab::Accounts?2:m_Tab==Tab::Settings?3:4);
    ImGui::SetCursorScreenPos({left,top}); if(t.Heading)ImGui::PushFont(t.Heading); ImGui::TextColored(c.TextPrimary,"RavenXD"); if(t.Heading)ImGui::PopFont(); ImGui::SameLine(); ImGui::TextDisabled(" / %s",labels[idx]);
    ImGui::SetCursorScreenPos({o.x+s.x-110,o.y+17}); if(ImGui::SmallButton("Log out")){AddLog("User logged out");if(m_OnLogout)m_OnLogout();}
    ImGui::SetCursorScreenPos({left,top+42}); if(m_Tab==Tab::Play)Play({left,top+42},w); else if(m_Tab==Tab::Mods)Mods({left,top+42},w); else if(m_Tab==Tab::Accounts)Accounts({left,top+42},w); else if(m_Tab==Tab::Settings)Settings({left,top+42},w); else Logs({left,top+42},w);
    const std::string status=GetStatus(); if(!status.empty()){ImGui::SetCursorScreenPos({left,o.y+s.y-24});ImGui::TextColored(c.TextDisabled,"%s",status.c_str());}
}

void RavenLauncherScreen::Play(ImVec2 p,float w){
    auto& c=Theme::Get().Colors(); auto& t=Theme::Get().Typography(); auto* d=ImGui::GetWindowDrawList(); const float hero=170;
    const bool busy=m_Busy.load();
    const bool mcReady=Launcher::MinecraftLauncher::IsMinecraft18Installed(W(m_GameDir));
    const bool forgeReady=Launcher::MinecraftLauncher::IsForge18Installed(W(m_GameDir));
    const bool ravenReady=Launcher::MinecraftLauncher::IsRavenModInstalled(W(m_GameDir+"\\mods"));
    Glass(d,p,{w,hero},Col(c.GlassSurface),Col(c.GlassBorder),22); if(UI::FontSettings().glassSheen)Effects::DrawIceOverlay(d,p,{p.x+w,p.y+hero},m_Time,.65f);
    SectionTitle(t,"Welcome back, RavenXD","Ready to play Minecraft 1.8.9? Choose an installation and launch.",{p.x+24,p.y+20}); d->AddCircleFilled({p.x+w-72,p.y+48},7,IM_COL32(55,225,145,255),24); ImGui::SetCursorScreenPos({p.x+w-180,p.y+40}); ImGui::TextColored(c.TextSecondary,"Online");
    ImGui::SetCursorScreenPos({p.x+24,p.y+84}); ImGui::TextColored(c.TextSecondary,"Installation"); ImGui::SameLine(); ImGui::RadioButton("Forge + RavenXD",&m_Version,0); ImGui::SameLine(); ImGui::RadioButton("Vanilla",&m_Version,1);
    if(busy){
        // Small rotating spinner + label so a long install/launch reads as "working", not "frozen".
        ImVec2 sc{p.x+24+95,p.y+137}; float a=m_Time*6.0f;
        for(int i=0;i<8;i++){float ang=a+i*0.785398f; float fade=0.25f+0.75f*(float)i/7.0f; d->AddCircleFilled({sc.x+cosf(ang)*11,sc.y+sinf(ang)*11},2.4f,IM_COL32(150,205,255,(int)(fade*255)),8);}
        ImGui::SetCursorScreenPos({p.x+24+42,p.y+128}); ImGui::TextColored(c.TextSecondary,"%s",m_BusyLabel.c_str());
    } else if(!BlueButton("PLAY MINECRAFT",{p.x+24,p.y+116},{190,42},d)){
        // not pressed, nothing to do
    } else {
        AddLog(m_Version==0?"Launching Forge 1.8.9":"Launching Minecraft 1.8.9");
        std::string javaw=m_JavaPath,gameDir=m_GameDir,forgeJar=m_ForgePath,user=m_User; int ver=m_Version;
        RunAsync("Launching Minecraft...",[this,javaw,gameDir,forgeJar,user,ver]{
            std::string e; bool ok=ver==0?m_Minecraft.LaunchForge(W(javaw),W(gameDir),W(forgeJar),user,e):m_Minecraft.LaunchVanilla(W(javaw),W(gameDir),W(gameDir+"\\versions\\1.8.9\\1.8.9.jar"),user,e);
            SetStatus(ok?"Minecraft launched successfully.":e); AddLog(ok?"Minecraft process started":"Launch failed: "+e,ok?"SUCCESS":"ERROR");
            if(ok)m_Discord.Update("Playing Minecraft 1.8.9",ver==0?"Forge + RavenXD":"Vanilla");
        });
    }
    float y=p.y+hero+16,gap=14,cw=(w-gap)/2; Glass(d,{p.x,y},{cw,132},Col(c.CardSurface),Col(c.CardBorder),18); Glass(d,{p.x+cw+gap,y},{cw,132},Col(c.CardSurface),Col(c.CardBorder),18);
    SectionTitle(t,"Minecraft 1.8.9","Client + libraries + assets",{p.x+18,y+15});
    ImGui::SetCursorScreenPos({p.x+18,y+60}); ImGui::TextColored(mcReady?ImVec4(.35f,.92f,.62f,1):c.TextDisabled,mcReady?"Installed":"Not installed");
    if(!busy && BlueButton(mcReady?"REINSTALL":"DOWNLOAD 1.8.9",{p.x+18,y+86},{160,34},d)){
        std::string gameDir=m_GameDir; bool force=mcReady;
        AddLog(force?"Repairing Minecraft 1.8.9 install":"Installing Minecraft 1.8.9");
        RunAsync(force?"Repairing Minecraft 1.8.9...":"Downloading Minecraft 1.8.9...",[this,gameDir,force]{
            std::string e; bool ok=m_Minecraft.InstallMinecraft18(W(gameDir),e,force);
            SetStatus(ok?"Minecraft 1.8.9 installed.":e); AddLog(ok?"Minecraft installation complete":"Minecraft installation failed: "+e,ok?"SUCCESS":"ERROR");
        });
    }
    SectionTitle(t,"RavenXD v2","Download the current mod build",{p.x+cw+gap+18,y+15});
    ImGui::SetCursorScreenPos({p.x+cw+gap+18,y+60}); ImGui::TextColored(ravenReady?ImVec4(.35f,.92f,.62f,1):c.TextDisabled,ravenReady?"Installed":"Not installed");
    if(!busy && BlueButton(ravenReady?"REINSTALL MOD":"INSTALL MOD",{p.x+cw+gap+18,y+86},{140,34},d)){
        std::string url=m_RavenUrl,modsDir=m_GameDir+"\\mods"; bool force=ravenReady;
        AddLog("Downloading RavenXD v2");
        RunAsync("Downloading RavenXD v2...",[this,url,modsDir,force]{
            std::string e; bool ok=m_Minecraft.InstallRaven(url,W(modsDir),e,force);
            SetStatus(ok?"RavenXD v2 installed.":e); AddLog(ok?"RavenXD v2 installed":"RavenXD installation failed: "+e,ok?"SUCCESS":"ERROR");
        });
    }
    ImGui::SetCursorScreenPos({p.x,y+148}); ImGui::TextDisabled("Game directory: %s  |  Forge: %s",m_GameDir.c_str(),forgeReady?"installed":"not installed");
}

void RavenLauncherScreen::Mods(ImVec2 p,float w){auto& c=Theme::Get().Colors();auto* d=ImGui::GetWindowDrawList();Glass(d,p,{w,76},Col(c.CardSurface),Col(c.CardBorder),18);ImGui::SetCursorScreenPos({p.x+18,p.y+14});ImGui::TextColored(c.TextPrimary,"Modrinth");char b[256]{};strncpy_s(b,m_Search.c_str(),sizeof(b)-1);ImGui::SetCursorScreenPos({p.x+18,p.y+40});ImGui::SetNextItemWidth(w-126);if(ImGui::InputTextWithHint("##modsearch","Search 1.8.9 mods",b,sizeof(b)))m_Search=b;ImGui::SameLine();if(ImGui::Button("Search",{90,28})){AddLog("Searching Modrinth: "+(m_Search.empty()?std::string("1.8.9"):m_Search));m_Mods=m_Modrinth.Search(m_Search.empty()?"1.8.9":m_Search);}float y=p.y+90;for(size_t i=0;i<m_Mods.size()&&i<8;i++){Glass(d,{p.x,y},{w,44},Col(c.OutlineSurface),Col(c.CardBorder),12);ImGui::SetCursorScreenPos({p.x+14,y+12});ImGui::TextColored(c.TextPrimary,"%s",m_Mods[i].title.c_str());ImGui::SetCursorScreenPos({p.x+w-104,y+8});if(!m_Busy.load() && ImGui::Button(("Install##"+std::to_string(i)).c_str(),{88,28})){Launcher::ModInfo mod=m_Mods[i];std::string modsDir=m_GameDir+"\\mods";AddLog("Installing mod: "+mod.title);RunAsync("Installing "+mod.title+"...",[this,mod,modsDir]{std::string e;bool ok=m_Modrinth.Install(mod,modsDir,e);SetStatus(ok?"Installed "+mod.title:e);AddLog(ok?"Installed mod: "+mod.title:"Mod install failed: "+e,ok?"SUCCESS":"ERROR");});}y+=52;}}

void RavenLauncherScreen::Accounts(ImVec2 p,float w){auto& c=Theme::Get().Colors();auto* d=ImGui::GetWindowDrawList();Glass(d,p,{w,148},Col(c.CardSurface),Col(c.CardBorder),18);ImGui::SetCursorScreenPos({p.x+20,p.y+20});ImGui::TextColored(c.TextPrimary,"Local account");ImGui::SetCursorScreenPos({p.x+20,p.y+56});ImGui::TextColored(c.Accent,"● %s",m_User.empty()?"Player":m_User.c_str());ImGui::SetCursorScreenPos({p.x+20,p.y+90});ImGui::TextDisabled("Offline/local profile. Official Microsoft authentication can be added separately.");}

void RavenLauncherScreen::Settings(ImVec2 p,float w){
    auto& c=Theme::Get().Colors(); auto& t=Theme::Get().Typography(); auto& f=UI::FontSettings(); auto* d=ImGui::GetWindowDrawList();
    Glass(d,p,{w,570},Col(c.CardSurface),Col(c.CardBorder),20); SectionTitle(t,"Settings","Appearance, custom theme, shader pack and runtime",{p.x+22,p.y+18});
    ImGui::SetCursorScreenPos({p.x+22,p.y+72}); ImGui::TextColored(c.TextPrimary,"Theme"); const char* themes[]={"Raven Blue","Midnight","Purple","White Glass","Custom"}; ImGui::SetCursorScreenPos({p.x+190,p.y+68}); ImGui::SetNextItemWidth(210); if(ImGui::Combo("##theme",&f.themePreset,themes,IM_ARRAYSIZE(themes))){UI::SaveFontSettings();ApplyTheme();AddLog(std::string("Theme changed to ")+themes[f.themePreset]);}
    if(f.themePreset==4){
        ImGui::SetCursorScreenPos({p.x+22,p.y+110}); ImGui::TextColored(c.TextSecondary,"Accent"); ImGui::SetCursorScreenPos({p.x+190,p.y+106}); if(ImGui::ColorEdit4("##accent",f.customAccent,ImGuiColorEditFlags_NoInputs|ImGuiColorEditFlags_AlphaBar)){UI::SaveFontSettings();ApplyTheme();}
        ImGui::SetCursorScreenPos({p.x+22,p.y+148}); ImGui::TextColored(c.TextSecondary,"Background"); ImGui::SetCursorScreenPos({p.x+190,p.y+144}); if(ImGui::ColorEdit4("##background",f.customBackground,ImGuiColorEditFlags_NoInputs|ImGuiColorEditFlags_AlphaBar)){UI::SaveFontSettings();ApplyTheme();}
        ImGui::SetCursorScreenPos({p.x+22,p.y+186}); ImGui::TextColored(c.TextSecondary,"Glass/card"); ImGui::SetCursorScreenPos({p.x+190,p.y+182}); if(ImGui::ColorEdit4("##card",f.customCard,ImGuiColorEditFlags_NoInputs|ImGuiColorEditFlags_AlphaBar)){UI::SaveFontSettings();ApplyTheme();}
    }
    float base=f.themePreset==4?224.0f:110.0f;
    ImGui::SetCursorScreenPos({p.x+22,p.y+base}); ImGui::TextColored(c.TextPrimary,"Font"); ImGui::SetCursorScreenPos({p.x+190,p.y+base-4}); ImGui::SetNextItemWidth(210); const char* presets[]={"System / Segoe UI","SF Pro Bold","Custom font"}; if(ImGui::Combo("##font",&f.preset,presets,IM_ARRAYSIZE(presets)))UI::SaveFontSettings();
    ImGui::SetCursorScreenPos({p.x+22,p.y+base+40}); ImGui::TextColored(c.TextPrimary,"Font size"); ImGui::SetCursorScreenPos({p.x+190,p.y+base+36}); ImGui::SetNextItemWidth(210); if(ImGui::SliderFloat("##fontsize",&f.size,12,24,"%.0f px"))UI::SaveFontSettings();
    ImGui::SetCursorScreenPos({p.x+22,p.y+base+80}); ImGui::TextColored(c.TextSecondary,"Shader Pack"); if(Toggle("##shaderpack",f.shaderPack,{p.x+w-70,p.y+base+75},d))UI::SaveFontSettings();
    ImGui::SetCursorScreenPos({p.x+22,p.y+base+118}); ImGui::TextColored(c.TextSecondary,"Shader look"); ImGui::SetCursorScreenPos({p.x+190,p.y+base+114}); ImGui::SetNextItemWidth(w-212); const char* shaderNames[UI::Effects::ShaderPack::Count()]; for(int i=0;i<UI::Effects::ShaderPack::Count();++i)shaderNames[i]=UI::Effects::ShaderPack::Name(i); if(ImGui::Combo("##shaderlook",&f.shaderIndex,shaderNames,UI::Effects::ShaderPack::Count()))UI::SaveFontSettings();
    ImGui::SetCursorScreenPos({p.x+22,p.y+158+base}); ImGui::TextColored(c.TextSecondary,"Intensity"); ImGui::SetCursorScreenPos({p.x+190,p.y+154+base}); ImGui::SetNextItemWidth(w-212); if(ImGui::SliderFloat("##shaderintensity",&f.shaderIntensity,0,1.5f,"%.2f"))UI::SaveFontSettings();
    ImGui::SetCursorScreenPos({p.x+22,p.y+198+base}); ImGui::TextColored(c.TextPrimary,"Visual effects"); const char* names[]={"Ice shader / sheen","Snow particles","Glass highlights","Blue glow"}; bool* vals[]={&f.iceEffect,&f.particles,&f.glassSheen,&f.blueGlow}; for(int i=0;i<4;i++){float yy=p.y+226+base+i*25;ImGui::SetCursorScreenPos({p.x+22,yy});ImGui::TextColored(c.TextSecondary,"%s",names[i]);if(Toggle((std::string("##fx")+std::to_string(i)).c_str(),*vals[i],{p.x+w-70,yy-5},d))UI::SaveFontSettings();}
    ImGui::SetCursorScreenPos({p.x+22,p.y+346+base}); ImGui::TextColored(c.TextPrimary,"Minecraft Java"); char j[512]{};strncpy_s(j,m_JavaPath.c_str(),sizeof(j)-1);ImGui::SetCursorScreenPos({p.x+190,p.y+342+base});ImGui::SetNextItemWidth(w-212);if(ImGui::InputText("##java",j,sizeof(j)))m_JavaPath=j;
}

void RavenLauncherScreen::Logs(ImVec2 p,float w){
    auto& c=Theme::Get().Colors(); auto* d=ImGui::GetWindowDrawList();
    Glass(d,p,{w,570},Col(c.CardSurface),Col(c.CardBorder),20); ImGui::SetCursorScreenPos({p.x+22,p.y+18}); ImGui::TextColored(c.TextPrimary,"Activity & debug logs"); ImGui::SetCursorScreenPos({p.x+22,p.y+44}); ImGui::TextDisabled("Live launcher events are also saved to %APPDATA%\\RavenXD\\launcher.log");
    ImGui::SetCursorScreenPos({p.x+w-210,p.y+18}); if(BlueButton("CLEAR LOG",{p.x+w-210,p.y+14},{92,30},d,false)){{std::lock_guard<std::mutex> lock(m_LogMutex);m_Logs.clear();}AddLog("Log buffer cleared");}
    if(BlueButton("OPEN FOLDER",{p.x+w-110,p.y+14},{92,30},d,false)){wchar_t b[32768]{};DWORD n=GetEnvironmentVariableW(L"APPDATA",b,32768);std::wstring dir=n?std::wstring(b,n):L"";dir+=L"\\RavenXD";ShellExecuteW(nullptr,L"open",dir.c_str(),nullptr,nullptr,SW_SHOWNORMAL);AddLog("Opened RavenXD data folder");}
    ImGui::SetCursorScreenPos({p.x+16,p.y+62}); ImGui::BeginChild("##logview",{w-32,490},true,ImGuiWindowFlags_HorizontalScrollbar);
    std::vector<std::string> logsCopy; { std::lock_guard<std::mutex> lock(m_LogMutex); logsCopy=m_Logs; } // copy under lock: AddLog() may run on a background install/launch thread.
    for(const auto& line:logsCopy){ImVec4 cc=c.TextSecondary;if(line.find("[ERROR]")!=std::string::npos)cc=ImVec4(.98f,.32f,.30f,1);else if(line.find("[WARN]")!=std::string::npos)cc=ImVec4(1,.72f,.25f,1);else if(line.find("[SUCCESS]")!=std::string::npos)cc=ImVec4(.28f,.92f,.60f,1);ImGui::TextColored(cc,"%s",line.c_str());}
    if(ImGui::GetScrollY()>=ImGui::GetScrollMaxY()-4)ImGui::SetScrollHereY(1.0f); ImGui::EndChild();
}
}
