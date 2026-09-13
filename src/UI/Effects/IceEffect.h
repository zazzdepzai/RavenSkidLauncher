#pragma once
#include "imgui.h"
#include <cmath>
#include <algorithm>
namespace UI::Effects {
inline void DrawIceOverlay(ImDrawList* draw, ImVec2 min, ImVec2 max, float time, float intensity=1.0f){ if(!draw||intensity<=0.0f)return; const float w=max.x-min.x,h=max.y-min.y; if(w<=1||h<=1)return; const float sweep=std::fmod(time*85.0f,w+h+260.0f)-130.0f; const int a=(int)std::clamp(18.0f*intensity,0.0f,40.0f); draw->AddQuadFilled({min.x+sweep,min.y},{min.x+sweep+110,min.y},{min.x+sweep+110-h*.26f,max.y},{min.x+sweep-h*.26f,max.y},IM_COL32(105,190,255,a)); for(int i=0;i<28;i++){float fi=(float)i;float x=min.x+std::fmod(fi*137.0f+std::sin(time*.22f+fi)*55.0f,w);float y=min.y+std::fmod(fi*71.0f+std::cos(time*.18f+fi*1.7f)*45.0f,h);float r=1.0f+(float)(i%3);int al=(int)((16+(i%5)*6)*intensity);draw->AddLine({x-r*2,y},{x+r*2,y},IM_COL32(190,235,255,al),1);draw->AddLine({x,y-r*2},{x,y+r*2},IM_COL32(190,235,255,al),1);} draw->AddRect(min,max,IM_COL32(125,205,255,(int)(35*intensity)),22,0,1); }
inline void DrawBlueGlow(ImDrawList* draw,ImVec2 center,float radius,float intensity=1.0f){if(!draw||intensity<=0)return;for(int i=5;i>=1;i--){float r=radius*(1.0f+i*.28f);int a=(int)std::max(2.0f,12.0f*intensity/i);draw->AddCircleFilled(center,r,IM_COL32(40,125,255,a),48);}}
}
