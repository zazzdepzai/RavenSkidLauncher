#pragma once
#include <d3d11.h>
#include <string>

namespace UI::Effects {
class ShaderPack {
public:
    ShaderPack() = default;
    ~ShaderPack();
    ShaderPack(const ShaderPack&) = delete;
    ShaderPack& operator=(const ShaderPack&) = delete;

    bool Initialize(ID3D11Device* device);
    void Shutdown();
    void Render(ID3D11DeviceContext* context, ID3D11RenderTargetView* target,
                float width, float height, float time, int mode, float intensity);
    bool Ready() const { return m_PixelShader != nullptr; }
    static constexpr int Count() { return 39; }
    static const char* Name(int index);
    static const char* Id(int index);

private:
    ID3D11Device* m_Device = nullptr;
    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;
    ID3D11Buffer* m_Constants = nullptr;
    ID3D11BlendState* m_Blend = nullptr;
    ID3D11RasterizerState* m_Raster = nullptr;
    ID3D11DepthStencilState* m_Depth = nullptr;
};
}
