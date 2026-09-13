#include "Application.h"
#include "UI/Theme/Theme.h"
#include "UI/Icons/IconFont.h"
#include "Utilities/EmbeddedAssets.h"
#include "Resources.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <chrono>
#include <thread>
namespace Application {
Application::Application() : m_Window(), m_Renderer(), m_Particles(), m_Auth(), m_Launcher(), m_Chrome(m_Window) {}
bool Application::Initialize(){ if(!m_Window.Create(L"RavenXD Minecraft Launcher",900,600)) return false; if(!m_Renderer.Initialize(m_Window.Handle())) return false; m_Window.SetOnResize([this](int w,int h){m_Renderer.ResizeSwapChain(w,h);m_Particles.Reset(ImVec2((float)w,(float)h));}); IMGUI_CHECKVERSION(); ImGui::CreateContext(); auto& io=ImGui::GetIO(); io.IniFilename=nullptr; io.LogFilename=nullptr; io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard; ImGui::GetStyle().AntiAliasedLines=true; ImGui::GetStyle().AntiAliasedFill=true; LoadFonts(); ImGui_ImplWin32_Init(m_Window.Handle()); ImGui_ImplDX11_Init(m_Renderer.Device(),m_Renderer.Context()); m_Chrome.Initialize(); m_Auth.Initialize(m_Renderer.Device()); m_Launcher.Initialize(m_Renderer.Device()); m_Auth.SetOnAuthenticated([this](const char* u){SetScreen(Screen::Launcher,u?u:"");}); m_Launcher.SetOnLogout([this]{SetScreen(Screen::Auth);}); m_Particles.Reset(ImVec2(900,600)); m_Window.Show(); return true; }
void Application::LoadFonts(){auto& io=ImGui::GetIO();auto& t=UI::Theme::Get().MutableTypography(); t.Heading=io.Fonts->AddFontDefault();t.Body=io.Fonts->AddFontDefault();t.BodyMedium=io.Fonts->AddFontDefault();t.Small=io.Fonts->AddFontDefault();t.IconSymbol=io.Fonts->AddFontDefault();t.IconAction=t.IconSymbol;t.IconControl=t.IconSymbol;t.IconBrand=t.IconSymbol;io.FontDefault=t.Body;io.Fonts->Build();}
void Application::SetScreen(Screen s,const std::string& user){m_Screen=s;m_User=user;if(s==Screen::Launcher){m_Launcher.SetUser(user);m_Window.ResizeCentered(900,600);m_Window.ClearRegion();}else{m_Window.ResizeCentered(420,520);m_Window.ClearRegion();m_Auth.Reset();}}
void Application::Render(float dt){m_Auth.Update(dt);m_Launcher.Update(dt);ImGui_ImplDX11_NewFrame();ImGui_ImplWin32_NewFrame();ImGui::NewFrame();float w=(float)m_Window.Width(),h=(float)m_Window.Height();ImGui::SetNextWindowPos({0,0});ImGui::SetNextWindowSize({w,h});ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{0,0});ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize,0);ImGui::Begin("##root",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoBackground);ImGui::PopStyleVar(2);if(m_Screen==Screen::Auth)m_Auth.Draw({0,0},{w,h});else m_Launcher.Draw({0,0},{w,h});ImGui::End();ImGui::Render();m_Renderer.BeginFrame(UI::Theme::Get().Colors().Background);ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());m_Renderer.EndFrame(true);}
void Application::Run(){auto prev=std::chrono::steady_clock::now();while(m_Window.PumpMessages()){auto now=std::chrono::steady_clock::now();float dt=std::chrono::duration<float>(now-prev).count();prev=now;if(!m_Window.IsMinimized())Render(dt);else std::this_thread::sleep_for(std::chrono::milliseconds(10));}}
void Application::Shutdown(){ImGui_ImplDX11_Shutdown();ImGui_ImplWin32_Shutdown();ImGui::DestroyContext();}
}
