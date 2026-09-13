#pragma once
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>
namespace UI {
struct FontSettingsData {
    int preset=0; std::string customPath; float size=16.0f;
    bool iceEffect=true, particles=true, glassSheen=true, blueGlow=true;
    bool shaderPack=true; int shaderIndex=1; float shaderIntensity=0.72f;
    int themePreset=0;
    float customAccent[4]{0.29f,0.62f,1.0f,1.0f};
    float customBackground[4]{0.02f,0.04f,0.08f,1.0f};
    float customCard[4]{0.06f,0.12f,0.22f,0.52f};
};
inline std::filesystem::path ConfigPath(){ wchar_t b[32768]{}; DWORD n=GetEnvironmentVariableW(L"APPDATA",b,32768); auto p=n?std::filesystem::path(b):std::filesystem::temp_directory_path(); p/=L"RavenXD"; std::error_code ec; std::filesystem::create_directories(p,ec); return p/L"ui.cfg"; }
inline FontSettingsData& FontSettings(){ static FontSettingsData d; return d; }
inline void ParseRGBA(const std::string& v,float out[4]){ std::stringstream ss(v); std::string x; int i=0; while(std::getline(ss,x,',')&&i<4){ try{out[i++]=std::stof(x);}catch(...){break;} } }
inline void LoadFontSettings(){ auto& d=FontSettings(); std::ifstream f(ConfigPath()); if(!f)return; std::string line,key,value; while(std::getline(f,line)){ auto pos=line.find('='); if(pos==std::string::npos)continue; key=line.substr(0,pos);value=line.substr(pos+1); try{ if(key=="font_preset")d.preset=std::stoi(value); else if(key=="font_path")d.customPath=value; else if(key=="font_size")d.size=std::stof(value); else if(key=="ice")d.iceEffect=value=="1"; else if(key=="particles")d.particles=value=="1"; else if(key=="sheen")d.glassSheen=value=="1"; else if(key=="glow")d.blueGlow=value=="1"; else if(key=="shader")d.shaderPack=value=="1"; else if(key=="shader_index")d.shaderIndex=std::stoi(value); else if(key=="shader_intensity")d.shaderIntensity=std::stof(value); else if(key=="theme_preset")d.themePreset=std::stoi(value); else if(key=="accent")ParseRGBA(value,d.customAccent); else if(key=="background")ParseRGBA(value,d.customBackground); else if(key=="card")ParseRGBA(value,d.customCard); }catch(...){ } } }
inline void SaveFontSettings(){ const auto& d=FontSettings(); std::ofstream f(ConfigPath(),std::ios::trunc); if(!f)return; f<<"font_preset="<<d.preset<<'\n'<<"font_path="<<d.customPath<<'\n'<<"font_size="<<d.size<<'\n'<<"ice="<<(d.iceEffect?1:0)<<'\n'<<"particles="<<(d.particles?1:0)<<'\n'<<"sheen="<<(d.glassSheen?1:0)<<'\n'<<"glow="<<(d.blueGlow?1:0)<<'\n'<<"shader="<<(d.shaderPack?1:0)<<'\n'<<"shader_index="<<d.shaderIndex<<'\n'<<"shader_intensity="<<d.shaderIntensity<<'\n'<<"theme_preset="<<d.themePreset<<'\n'<<"accent="<<d.customAccent[0]<<','<<d.customAccent[1]<<','<<d.customAccent[2]<<','<<d.customAccent[3]<<'\n'<<"background="<<d.customBackground[0]<<','<<d.customBackground[1]<<','<<d.customBackground[2]<<','<<d.customBackground[3]<<'\n'<<"card="<<d.customCard[0]<<','<<d.customCard[1]<<','<<d.customCard[2]<<','<<d.customCard[3]<<'\n'; }
}
