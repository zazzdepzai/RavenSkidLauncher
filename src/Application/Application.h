#pragma once
#include "Window/Win32Window.h"
#include "Renderer/D3D11Renderer.h"
#include "UI/Effects/ShaderPack.h"
#include "UI/Particles/SnowParticleSystem.h"
#include "UI/Authentication/AuthScreen.h"
#include "UI/Screens/RavenLauncherScreen.h"
#include "UI/Components/WindowChrome.h"
#include <string>
namespace Application { class Application { public: Application(); bool Initialize(); void Run(); void Shutdown(); private: enum class Screen { Auth, Launcher }; void SetScreen(Screen s,const std::string& user={}); void LoadFonts(); void Render(float dt); Window::Win32Window m_Window; Renderer::D3D11Renderer m_Renderer; UI::SnowParticleSystem m_Particles; UI::Effects::ShaderPack m_ShaderPack; UI::Authentication::AuthScreen m_Auth; UI::Screens::RavenLauncherScreen m_Launcher; UI::Components::WindowChrome m_Chrome; Screen m_Screen=Screen::Auth; std::string m_User; float m_Time=0.0f; }; }
