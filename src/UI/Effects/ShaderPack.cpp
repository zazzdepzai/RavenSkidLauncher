#include "ShaderPack.h"
#include <d3dcompiler.h>
#include <algorithm>
#include <cstring>

#pragma comment(lib, "d3dcompiler.lib")

namespace UI::Effects {
namespace {
struct Constants {
    float resolution[2];
    float time;
    float intensity;
    int mode;
    float pad[3];
};

static const char* kNames[] = {
    "Color Bends", "Galaxy", "Lightning", "Light Rays", "Line Waves",
    "Liquid Chrome", "Liquid Ether", "Silk", "Particles", "Pixel Snow",
    "Rain", "Plasma", "Prismatic Burst", "Aurora", "Caustics", "Ember",
    "Fog", "Iridescence", "Smoke", "Marble", "Ripple", "Magnet", "Tunnel",
    "Warp", "Flame", "Kaleido", "Cells", "Water", "Matrix", "Lava",
    "Horror Spiral", "Creation", "Star Nest", "Julia", "Electric", "Hexagons",
    "Grain", "Scanlines", "Flaring"
};
static const char* kIds[] = {
    "bends","galaxy","lightning","rays","waves","chrome","ether","silk",
    "particles","snow","rain","plasma","burst","aurora","caustics","ember",
    "fog","iridescence","smoke","marble","ripple","magnet","tunnel","warp",
    "flame","kaleido","cells","water","matrix","lava","horror","creation",
    "starnest","julia","electric","hexagons","grain","scanlines","flaring"
};

static const char* kShader = R"HLSL(
cbuffer RavenFX : register(b0) {
    float2 Resolution;
    float Time;
    float Intensity;
    int Mode;
    float3 Padding;
};
struct VSOut { float4 Pos : SV_POSITION; float2 UV : TEXCOORD0; };
VSOut VS(uint id : SV_VertexID) {
    VSOut o;
    float2 p = float2((id == 2) ? 3.0 : -1.0, (id == 1) ? 3.0 : -1.0);
    o.Pos = float4(p, 0, 1);
    o.UV = p * 0.5 + 0.5;
    o.UV.y = 1.0 - o.UV.y;
    return o;
}
float hash21(float2 p) { p = frac(p * float2(123.34, 456.21)); p += dot(p, p + 45.32); return frac(p.x * p.y); }
float noise(float2 p) {
    float2 i = floor(p), f = frac(p); f = f*f*(3.0-2.0*f);
    float a=hash21(i), b=hash21(i+float2(1,0)), c=hash21(i+float2(0,1)), d=hash21(i+float2(1,1));
    return lerp(lerp(a,b,f.x),lerp(c,d,f.x),f.y);
}
float fbm(float2 p) { float v=0,a=0.5; [unroll] for(int i=0;i<4;i++){v+=noise(p)*a;p=p*2.03+7.1;a*=0.5;} return v; }
float3 pal(float t) { return 0.5+0.5*cos(6.28318*(float3(0.0,0.17,0.34)+t)); }
float2 rot(float2 p,float a){float c=cos(a),s=sin(a);return float2(c*p.x-s*p.y,s*p.x+c*p.y);}
float star(float2 p,float seed){float d=length(p);float core=0.008/max(d,0.0005);float cross=pow(saturate(1-abs(p.x*p.y)*1800),4);return (core+cross*0.018)*(0.5+0.5*sin(Time*(1+seed*3)+seed*6.28));}
float3 effect(float2 uv) {
    float2 p=(uv-0.5)*float2(Resolution.x/max(Resolution.y,1),1);
    float t=Time;
    float3 col=float3(0.012,0.018,0.032);
    float2 q=p;
    if(Mode==0){ q=rot(q,t*.15); float n=fbm(q*2.2+t*.08); col=lerp(float3(.03,.06,.12),float3(.16,.32,.72),n); col+=pal(n+t*.04)*.18; }
    else if(Mode==1){ q=rot(q,t*.07); col=float3(.006,.01,.025); [loop] for(int l=0;l<4;l++){float s=7.0+l*5.0;float2 g=q*s+float2(l*17.3,l*29.1);float2 c=floor(g);float2 f=frac(g)-.5;for(int y=-1;y<=1;y++)for(int x=-1;x<=1;x++){float2 id=c+float2(x,y);float h=hash21(id);float2 d=f-float2(x,y)-(float2(frac(h*13.1),frac(h*27.7))-.5)*.7;col+=float3(.55,.72,1)*star(d,h)*(.2+.2*h);}}}
    else if(Mode==2){float n=fbm(float2(p.x*2+t*.2,p.y*3));float bolts=0;for(int i=0;i<4;i++){float x=(hash21(float2(i,3.1))-.5)*1.6;float d=abs(p.x-x-(n-.5)*.22*sin(p.y*5+i));bolts+=pow(.018/max(d,.001),1.2)*smoothstep(.9,-.2,p.y)*step(.55,hash21(float2(floor(t*.8)+i,7)));}col+=float3(.35,.65,1)*bolts*.035;}
    else if(Mode==3){float2 o=float2(0,1.0),v=p-o;float d=length(v),a=dot(normalize(v),normalize(float2(.1,-1)));float rays=pow(saturate(a),5)*(1-d*.35);col+=float3(.18,.38,.75)*rays*(.8+.2*sin(t*2));}
    else if(Mode==4){float w=sin(p.y*8+t)+.45*sin(p.y*16-t*1.3)+.2*sin(p.x*7+t*.7);float line=pow(saturate(.5+.5*cos((p.x+w*.15)*13)),7);col+=pal(w*.12+t*.03)*line*.35;}
    else if(Mode==5){q=p;for(int i=1;i<7;i++){q+=.5/i*float2(cos(i*1.2*q.y+t),cos(i*1.35*q.x-t));}float s=abs(sin(t*.7-q.x-q.y));col=float3(.3,.38,.46)/max(s,.12)*.35;}
    else if(Mode==6){float n=fbm(q*3+t*.05);float f=pow(saturate(n),2);col=lerp(float3(.015,.03,.05),float3(.05,.45,.5),n);col+=float3(.55,.9,.85)*f*.35;col+=float3(.25,.06,.25)*pow(1-n,3)*.25;}
    else if(Mode==7){float w=sin(p.x*2+p.y*.8+t*.5)+.6*sin(p.x*4-p.y+t);float r=.5+.5*sin(w*.7);col=lerp(float3(.04,.04,.07),pal(r+t*.02),r*.7);col+=pow(saturate(.5+.5*sin(p.x*7+w*2+t)),8)*.12;}
    else if(Mode==8){for(int i=0;i<35;i++){float h=hash21(float2(i,2));float2 s=float2(frac(h*17)-.5,frac(h*31)-.5);s.y=frac(s.y-t*(.05+.1*h)+1)-.5;float d=length(p-s);col+=float3(.35,.62,1)*smoothstep(.018,.001,d)*.35;}}
    else if(Mode==9){for(int i=0;i<28;i++){float h=hash21(float2(i,9));float2 s=float2(frac(h*19)-.5,frac(h*37)-.5);s.y=frac(s.y-t*(.025+.05*h)+1)-.5;float d=length(p-s);col+=float3(.72,.86,1)*smoothstep(.035,.006,d)*.22;}}
    else if(Mode==10){float rain=0;for(int i=0;i<18;i++){float h=hash21(float2(i,11));float x=frac(h*31)-.5;float y=frac(h*17-t*(.35+h*.5)+1)-.5;rain+=smoothstep(.012,.002,abs(p.x-x+.08*(p.y-y)))*smoothstep(.35,-.2,p.y-y);}col+=float3(.3,.55,.8)*rain*.22;}
    else if(Mode==11){float n=sin(p.x*7+t)+sin(p.y*8-t)+sin((p.x+p.y)*6+t*.5);col=pal(n*.12+t*.05)*(.25+.35*(.5+.5*sin(n)));}
    else if(Mode==12){float a=atan2(p.y,p.x),r=length(p);float burst=pow(saturate(cos(a*16+t*.4)),22)*(1-r);col+=pal(a*.1+t*.03)*burst*.55;}
    else if(Mode==13){float n=fbm(float2(p.x*2,p.y*1.5-t*.04));float curtain=pow(saturate(.5+.5*sin(p.x*5+n*5+t)),5);col+=float3(.1,.75,.6)*curtain*.3+float3(.35,.1,.7)*pow(curtain,2)*.2;}
    else if(Mode==14){float n=fbm(p*6+t*.12);float ca=pow(saturate(.5+.5*sin((p.x+n)*13)*sin((p.y-n)*15)),7);col+=float3(.12,.38,.62)*ca*.5;}
    else if(Mode==15){for(int i=0;i<24;i++){float h=hash21(float2(i,15));float2 s=float2(frac(h*23)-.5,frac(h*17+t*(.02+.03*h))- .5);s.y-=t*.06; s.y=frac(s.y+.5)-.5;float d=length(p-s);col+=float3(1,.35,.08)*smoothstep(.018,.002,d)*.25;}}
    else if(Mode==16){float n=fbm(p*2+t*.03);col=lerp(float3(.015,.02,.035),float3(.15,.2,.3),n);col*=.8+.2*sin(p.y*3+t);}
    else if(Mode==17){float n=fbm(p*4+t*.05);float sheen=pow(saturate(.5+.5*sin(n*9+p.x*6+t)),6);col=lerp(float3(.04,.06,.08),pal(n*.4),.35)+float3(.4,.55,.8)*sheen*.25;}
    else if(Mode==18){float n=fbm(p*3+float2(0,-t*.05));col=lerp(float3(.025,.03,.04),float3(.22,.28,.34),n);col+=float3(.2,.25,.3)*pow(n,4)*.2;}
    else if(Mode==19){float n=fbm(rot(p,t*.03)*3);float veins=pow(1-abs(sin(n*8+p.x*2)),9);col=lerp(float3(.6,.62,.66),float3(.05,.07,.1),veins*.7);col+=float3(.7,.8,1)*pow(n,8)*.12;}
    else if(Mode==20){float d=length(p);float ring=exp(-abs(sin(d*28-t*6))*4)*exp(-d*1.2);col+=float3(.35,.7,1)*ring*.45;}
    else if(Mode==21){float d=length(p);float pull=exp(-d*d*7);float n=fbm((p*(1+pull*.4))+t*.08);col+=float3(.5,.3,.65)*n*.4+float3(1,.7,.35)*pull*.25;}
    else if(Mode==22){float d=max(length(p),.03);float z=.2/d+t*.2;float r=abs(sin(z*7));float rib=abs(sin(atan2(p.y,p.x)*8+z));col=lerp(float3(.02,.03,.05),float3(.5,.6,.8),pow(r*.7+rib*.3,2)*.8);}
    else if(Mode==23){float d=length(p);float streak=pow(saturate(1-d*1.2),3)*pow(saturate(sin(atan2(p.y,p.x)*12-t*2)),18);col+=float3(.25,.55,1)*streak*.6;}
    else if(Mode==24){float n=fbm(float2(p.x*3,p.y*3-t*.15));float flame=saturate(n+(.5-p.y)*.8);col=lerp(float3(.15,.01,.005),float3(1,.35,.02),pow(flame,2));}
    else if(Mode==25){float2 k=rot(p,t*.15);float a=atan2(k.y,k.x);float r=length(k);a=abs(fmod(a+3.14159/6,3.14159/3)-3.14159/6);float n=sin(r*18-a*5-t);col=pal(n*.15)*(.25+.5*pow(.5+.5*sin(n),4));}
    else if(Mode==26){float2 g=p*5;float2 id=floor(g);float2 f=frac(g)-.5;float best=2;for(int y=-1;y<=1;y++)for(int x=-1;x<=1;x++){float h=hash21(id+float2(x,y));float2 site=float2(x,y)+(float2(frac(h*13),frac(h*29))-.5);best=min(best,length(f-site));}col=lerp(float3(.02,.04,.07),pal(best*2+t*.02),1-best*1.5);}
    else if(Mode==27){float n=fbm(p*2+t*.04);float swell=sin(p.x*4+n*3+t)+sin(p.y*5-n*2-t);col=lerp(float3(.02,.08,.12),float3(.08,.45,.62),.5+.5*sin(swell));}
    else if(Mode==28){float x=frac(p.x*9);float y=frac(p.y*7+t*.35);float digit=step(.55,frac(sin(floor(p.x*9)*12.3+floor(p.y*7)*4.7)*43758.5));col+=float3(.1,.85,.38)*digit*step(.8,x)*.35;col+=float3(.03,.18,.08)*y;}
    else if(Mode==29){float n=fbm(p*3+t*.03);float cracks=pow(1-abs(sin(n*12+p.x*3)),10);col=lerp(float3(.03,.012,.008),float3(1,.16,.02),cracks*.7);}
    else if(Mode==30){float a=atan2(p.y,p.x);float r=length(p);float s=sin(log(r+.02)*8-a*5+t);col=pal(s*.2)*pow(.5+.5*s,3)*.5;}
    else if(Mode==31){float r=length(p);float rings=abs(sin(r*16-t*2));float grid=abs(sin(p.x*11))*abs(sin(p.y*11));col+=pal(r*.5+t*.03)*(rings*.55+grid*.2);}
    else if(Mode==32){float n=fbm(p*3);float rays=pow(saturate(.5+.5*sin(n*18+atan2(p.y,p.x)*9+t)),6);col+=float3(.15,.35,.8)*rays*.35;col+=float3(.2,.45,.95)*pow(n,5)*.15;}
    else if(Mode==33){float2 z=p*1.8;float2 c=float2(.25*sin(t*.21),.25*cos(t*.17));float v=0;for(int i=0;i<8;i++){z=float2(z.x*z.x-z.y*z.y,2*z.x*z.y)+c;v+=exp(-4*length(z));}col=pal(v*.2+t*.02)*min(v*.08,.8);}
    else if(Mode==34){float n=fbm(p*5+t*.2);float e=pow(saturate(abs(sin(n*14+p.x*9-t))),8);col+=float3(.15,.55,1)*e*.5;}
    else if(Mode==35){float2 g=p*7;float2 f=abs(frac(g)-.5);float h=1-max(f.x,f.y);float cell=pow(saturate(1-h*2),3);col=lerp(float3(.02,.04,.07),float3(.08,.35,.62),cell);col+=float3(.25,.55,1)*pow(h,8)*.25;}
    else if(Mode==36){float n=hash21(uv*Resolution+floor(t*30));col=float3(.025,.035,.055)+n*.045;}
    else if(Mode==37){float bars=.5+.5*sin((p.y+t*.15)*Resolution.y*.055);float grain=hash21(uv*Resolution+floor(t*12))*.08;col=float3(.025,.035,.06)+bars*.035+grain;}
    else if(Mode==38){float d=length(p);float flare=pow(saturate(1-d*2),5);float cross=pow(saturate(1-abs(p.x*p.y)*900),7);col+=float3(.55,.75,1)*(flare*.35+cross*.35);}
    float3 base=float3(.004,.008,.016);
    return lerp(base,col,saturate(Intensity));
}
float4 PS(VSOut i) : SV_TARGET { return float4(saturate(effect(i.UV)),1); }
)HLSL";
}

ShaderPack::~ShaderPack(){ Shutdown(); }

bool ShaderPack::Initialize(ID3D11Device* device){
    if(!device) return false;
    Shutdown();
    m_Device=device; m_Device->AddRef();
    ID3DBlob* vsBlob=nullptr; ID3DBlob* psBlob=nullptr; ID3DBlob* err=nullptr;
    HRESULT hr=D3DCompile(kShader, strlen(kShader), "RavenXDShaderPack", nullptr,nullptr,"VS","vs_5_0",D3DCOMPILE_OPTIMIZATION_LEVEL3,0,&vsBlob,&err);
    if(FAILED(hr)){ if(err)err->Release(); Shutdown(); return false; }
    hr=D3DCompile(kShader, strlen(kShader), "RavenXDShaderPack", nullptr,nullptr,"PS","ps_5_0",D3DCOMPILE_OPTIMIZATION_LEVEL3,0,&psBlob,&err);
    if(FAILED(hr)){ if(err)err->Release(); vsBlob->Release(); Shutdown(); return false; }
    hr=m_Device->CreateVertexShader(vsBlob->GetBufferPointer(),vsBlob->GetBufferSize(),nullptr,&m_VertexShader);
    if(SUCCEEDED(hr))hr=m_Device->CreatePixelShader(psBlob->GetBufferPointer(),psBlob->GetBufferSize(),nullptr,&m_PixelShader);
    vsBlob->Release(); psBlob->Release(); if(FAILED(hr)){Shutdown();return false;}

    D3D11_BUFFER_DESC bd{}; bd.ByteWidth=sizeof(Constants); bd.Usage=D3D11_USAGE_DYNAMIC; bd.BindFlags=D3D11_BIND_CONSTANT_BUFFER; bd.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
    if(FAILED(m_Device->CreateBuffer(&bd,nullptr,&m_Constants))){Shutdown();return false;}
    D3D11_BLEND_DESC blend{}; blend.RenderTarget[0].BlendEnable=FALSE; blend.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
    if(FAILED(m_Device->CreateBlendState(&blend,&m_Blend))){Shutdown();return false;}
    D3D11_RASTERIZER_DESC rast{}; rast.FillMode=D3D11_FILL_SOLID; rast.CullMode=D3D11_CULL_NONE; rast.DepthClipEnable=TRUE;
    if(FAILED(m_Device->CreateRasterizerState(&rast,&m_Raster))){Shutdown();return false;}
    D3D11_DEPTH_STENCIL_DESC depth{}; depth.DepthEnable=FALSE; depth.StencilEnable=FALSE;
    if(FAILED(m_Device->CreateDepthStencilState(&depth,&m_Depth))){Shutdown();return false;}
    return true;
}

void ShaderPack::Shutdown(){
    if(m_Depth){m_Depth->Release();m_Depth=nullptr;} if(m_Raster){m_Raster->Release();m_Raster=nullptr;} if(m_Blend){m_Blend->Release();m_Blend=nullptr;} if(m_Constants){m_Constants->Release();m_Constants=nullptr;} if(m_PixelShader){m_PixelShader->Release();m_PixelShader=nullptr;} if(m_VertexShader){m_VertexShader->Release();m_VertexShader=nullptr;} if(m_Device){m_Device->Release();m_Device=nullptr;}
}

void ShaderPack::Render(ID3D11DeviceContext* context, ID3D11RenderTargetView* target, float width, float height, float time, int mode, float intensity){
    if(!context||!target||!Ready())return;
    mode=std::clamp(mode,0,Count()-1); intensity=std::clamp(intensity,0.0f,1.5f);
    D3D11_MAPPED_SUBRESOURCE mapped{};
    if(FAILED(context->Map(m_Constants,0,D3D11_MAP_WRITE_DISCARD,0,&mapped)))return;
    auto* c=reinterpret_cast<Constants*>(mapped.pData); c->resolution[0]=width;c->resolution[1]=height;c->time=time;c->intensity=intensity;c->mode=mode;c->pad[0]=c->pad[1]=c->pad[2]=0;context->Unmap(m_Constants,0);
    D3D11_VIEWPORT vp{};vp.Width=width;vp.Height=height;vp.MinDepth=0;vp.MaxDepth=1;context->RSSetViewports(1,&vp);
    float blendFactor[4]={0,0,0,0};context->OMSetRenderTargets(1,&target,nullptr);context->OMSetBlendState(m_Blend,blendFactor,0xffffffff);context->OMSetDepthStencilState(m_Depth,0);context->RSSetState(m_Raster);context->IASetInputLayout(nullptr);context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);context->VSSetShader(m_VertexShader,nullptr,0);context->PSSetShader(m_PixelShader,nullptr,0);context->PSSetConstantBuffers(0,1,&m_Constants);context->Draw(3,0);
}

const char* ShaderPack::Name(int index){ if(index<0||index>=Count())index=0; return kNames[index]; }
const char* ShaderPack::Id(int index){ if(index<0||index>=Count())index=0; return kIds[index]; }
}
