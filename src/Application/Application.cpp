#include "Application.h"
#include "UI/Theme/Theme.h"
#include "UI/Theme/FontSettings.h"
#include "UI/Effects/IceEffect.h"
#include "UI/Icons/IconFont.h"
#include "Utilities/EmbeddedAssets.h"
#include "Resources.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <chrono>
#include <algorithm>
#include <thread>
namespace {
// Dear ImGui's DX11 backend calls D3DCompile() at runtime (see
// ImGui_ImplDX11_CreateDeviceObjects), which dynamically resolves d3dcompiler_47.dll. On some
// Windows installs that DLL is missing/outdated, which used to crash the launcher immediately
// with an unhelpful "module not found" error before a single frame was drawn.
// A copy of this DLL is bundled next to RavenXD-Launcher.exe by the build/packaging step, and
// Windows always searches the application's own directory before anything else, so in the
// normal case this silently succeeds. If it's still missing (e.g. someone copied only the .exe
// out of the package), show a clear message and exit cleanly instead of crashing.
bool EnsureD3DCompilerAvailable() {
    HMODULE h = LoadLibraryW(L"d3dcompiler_47.dll");
    if (h) { FreeLibrary(h); return true; }
    MessageBoxW(nullptr,
        L"RavenXD could not find d3dcompiler_47.dll, which is required to render the launcher UI.\n\n"
        L"This file should ship next to RavenXD-Launcher.exe. Please re-download/re-extract the full "
        L"launcher package (don't move only the .exe on its own), or install the \"Microsoft Visual C++ "
        L"Redistributable (x64)\", which also provides this file.",
        L"RavenXD - Missing component", MB_OK | MB_ICONERROR);
    return false;
}
}
namespace Application {
Application::Application() : m_Window(), m_Renderer(), m_Particles(), m_Auth(), m_Launcher(), m_Chrome(m_Window) {}
bool Application::Initialize(){ if(!EnsureD3DCompilerAvailable()) return false; if(!m_Window.Create(L"RavenXD Minecraft Launcher",900,680)) return false; if(!m_Renderer.Initialize(m_Window.Handle())) return false; m_Window.SetOnResize([this](int w,int h){m_Renderer.ResizeSwapChain(w,h);m_Particles.Reset(ImVec2((float)w,(float)h));}); IMGUI_CHECKVERSION(); ImGui::CreateContext(); auto& io=ImGui::GetIO(); io.IniFilename=nullptr; io.LogFilename=nullptr; io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard; ImGui::GetStyle().AntiAliasedLines=true; ImGui::GetStyle().AntiAliasedFill=true; LoadFonts(); ImGui_ImplWin32_Init(m_Window.Handle()); ImGui_ImplDX11_Init(m_Renderer.Device(),m_Renderer.Context()); m_Chrome.Initialize(); m_ShaderPack.Initialize(m_Renderer.Device()); m_Auth.Initialize(m_Renderer.Device()); m_Launcher.Initialize(m_Renderer.Device()); m_Auth.SetOnAuthenticated([this](const char* u){SetScreen(Screen::Launcher,u?u:"");}); m_Launcher.SetOnLogout([this]{SetScreen(Screen::Auth);}); m_Particles.Reset(ImVec2(900,680)); m_Window.Show(); return true; }
void Application::LoadFonts(){
    UI::LoadFontSettings();
    auto& io=ImGui::GetIO();
    auto& t=UI::Theme::Get().MutableTypography();
    const auto& cfg=UI::FontSettings();
    std::string regular="C:\\Windows\\Fonts\\segoeui.ttf";
    std::string bold="C:\\Windows\\Fonts\\seguisb.ttf";
    if(cfg.preset==1){
        const char* candidates[]={
            "C:\\Windows\\Fonts\\SF-Pro-Display-Bold.otf",
            "C:\\Windows\\Fonts\\SFProDisplay-Bold.otf",
            "C:\\Windows\\Fonts\\SF-Pro-Text-Bold.otf",
            "C:\\Windows\\Fonts\\SFProText-Bold.otf"};
        for(const char* f:candidates){ if(GetFileAttributesA(f)!=INVALID_FILE_ATTRIBUTES){regular=f;bold=f;break;} }
    } else if(cfg.preset==2 && !cfg.customPath.empty() && GetFileAttributesA(cfg.customPath.c_str())!=INVALID_FILE_ATTRIBUTES){ regular=cfg.customPath; bold=cfg.customPath; }
    const float size=std::clamp(cfg.size,12.0f,24.0f);
    ImFont* body=io.Fonts->AddFontFromFileTTF(regular.c_str(),size);
    ImFont* heading=io.Fonts->AddFontFromFileTTF(bold.c_str(),size+1.0f);
    if(!body) body=io.Fonts->AddFontDefault();
    if(!heading) heading=body;
    t.Heading=heading; t.Body=body; t.BodyMedium=heading; t.Small=io.Fonts->AddFontFromFileTTF(regular.c_str(),size-2.0f); if(!t.Small)t.Small=body;
    t.IconSymbol=body; t.IconAction=body; t.IconControl=body; t.IconBrand=heading; t.IconAwesome=body;
    io.FontDefault=body; io.Fonts->Build();
}
void Application::SetScreen(Screen s,const std::string& user){m_Screen=s;m_User=user;if(s==Screen::Launcher){m_Launcher.SetUser(user);m_Window.ResizeCentered(900,680);m_Window.ClearRegion();}else{m_Window.ResizeCentered(420,520);m_Window.ClearRegion();m_Auth.Reset();}}
void Application::Render(float dt){m_Time+=dt; m_Auth.Update(dt);m_Launcher.Update(dt);m_Particles.Update(dt);ImGui_ImplDX11_NewFrame();ImGui_ImplWin32_NewFrame();ImGui::NewFrame();float w=(float)m_Window.Width(),h=(float)m_Window.Height();ImGui::SetNextWindowPos({0,0});ImGui::SetNextWindowSize({w,h});ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{0,0});ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize,0);ImGui::Begin("##root",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoBackground);ImGui::PopStyleVar(2); auto* bg=ImGui::GetWindowDrawList(); if(!UI::FontSettings().shaderPack) bg->AddRectFilled({0,0},{w,h},IM_COL32(4,9,18,255)); if(UI::FontSettings().blueGlow){UI::Effects::DrawBlueGlow(bg,{w*0.16f,h*0.22f},110.0f,1.0f); UI::Effects::DrawBlueGlow(bg,{w*0.78f,h*0.34f},160.0f,0.85f); UI::Effects::DrawBlueGlow(bg,{w*0.56f,h*0.90f},130.0f,0.65f);} if(UI::FontSettings().particles)m_Particles.RenderInRoundedRect(bg,{0,0},{w,h},24.0f); if(UI::FontSettings().iceEffect)UI::Effects::DrawIceOverlay(bg,{0,0},{w,h},m_Time,0.85f); if(m_Screen==Screen::Auth)m_Auth.Draw({0,0},{w,h});else m_Launcher.Draw({0,0},{w,h});ImGui::End();ImGui::Render();m_Renderer.BeginFrame(UI::Theme::Get().Colors().Background); if(UI::FontSettings().shaderPack && m_ShaderPack.Ready()) m_ShaderPack.Render(m_Renderer.Context(),m_Renderer.RenderTarget(),w,h,m_Time,UI::FontSettings().shaderIndex,UI::FontSettings().shaderIntensity); ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());m_Renderer.EndFrame(true);}
void Application::Run(){auto prev=std::chrono::steady_clock::now();while(m_Window.PumpMessages()){auto now=std::chrono::steady_clock::now();float dt=std::chrono::duration<float>(now-prev).count();prev=now;if(!m_Window.IsMinimized())Render(dt);else std::this_thread::sleep_for(std::chrono::milliseconds(10));}}
void Application::Shutdown(){m_ShaderPack.Shutdown();ImGui_ImplDX11_Shutdown();ImGui_ImplWin32_Shutdown();ImGui::DestroyContext();}
}
