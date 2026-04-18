#pragma once

#include "datatype.h"
#include "math/geom.h"

namespace core::assets
{
    struct FontMetrics
    {
        float ascenderY;
        float descenderY;
        float lineHeight;
        float emRange;
    };

    struct GlyphMetrics
    {
        enum class RenderDirective : u8 { Substitute, Render, Control };

        math::rect uvRect;
        math::rect quadRect;
        float advance;
        RenderDirective renderDirective = RenderDirective::Substitute;
    };
}
