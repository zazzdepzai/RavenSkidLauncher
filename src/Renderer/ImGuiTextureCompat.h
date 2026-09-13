#pragma once
#include "imgui.h"
#include <d3d11.h>

namespace Renderer {
inline ImTextureID ToImTexture(ID3D11ShaderResourceView* view) noexcept {
    return reinterpret_cast<ImTextureID>(view);
}
}
