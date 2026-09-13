#pragma once

#include "imgui.h"
#include <cmath>
#include <cstdint>

namespace UI::Effects {

inline void DrawIceOverlay(ImDrawList* draw, ImVec2 min, ImVec2 max, float time)
{
    if (!draw) return;
    const float w = max.x - min.x;
    const float h = max.y - min.y;

    // Moving cold sheen.
    const float sweep = std::fmod(time * 70.0f, w + h + 220.0f) - 110.0f;
    draw->AddQuadFilled(
        ImVec2(min.x + sweep, min.y),
        ImVec2(min.x + sweep + 70.0f, min.y),
        ImVec2(min.x + sweep + 70.0f - h * 0.22f, max.y),
        ImVec2(min.x + sweep - h * 0.22f, max.y),
        IM_COL32(210, 238, 255, 11));

    // Subtle crystalline shards around the edges.
    for (int i = 0; i < 18; ++i) {
        const float t = static_cast<float>(i) / 18.0f;
        const float x = min.x + std::fmod(t * w + std::sin(time * 0.35f + i) * 18.0f + 40.0f, (w > 1.0f ? w : 1.0f));
        const float y = min.y + std::fmod(t * h + std::cos(time * 0.28f + i * 1.7f) * 22.0f + 25.0f, (h > 1.0f ? h : 1.0f));
        const float r = 2.0f + static_cast<float>(i % 3);
        const ImU32 c = IM_COL32(225, 245, 255, 28 + (i % 4) * 7);
        draw->AddLine(ImVec2(x-r, y), ImVec2(x+r, y), c, 1.0f);
        draw->AddLine(ImVec2(x, y-r), ImVec2(x, y+r), c, 1.0f);
    }

    // Icy border highlight.
    draw->AddRect(min, max, IM_COL32(205, 238, 255, 30), 14.0f, 0, 1.0f);
}

}
