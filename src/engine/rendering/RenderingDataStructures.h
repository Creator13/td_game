#pragma once

#include "datatype.h"
#include "core/Color.h"
#include "math/mat4.h"

namespace core::gfx
{
    struct ViewportData
    {
        Color clearColor;

        math::mat4 projectionMatrix = math::mat4::identity;
        math::mat4 viewMatrix = math::mat4::identity;

        u16 pixelWidth, pixelHeight;

        math::mat4 getCombinedViewProjectionMatrix() const;
        math::mat4 getScreenSpaceProjectionMatrix() const;
        math::vec3 screenToWorld(math::vec2 pixelPos, float depth = 0.5f) const;
        math::vec2 worldToScreen(math::vec3 worldPos) const;
    };
}
