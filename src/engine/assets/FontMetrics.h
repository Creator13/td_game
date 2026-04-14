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
    };

    struct GlyphMetrics
    {
        bool occupied = false;
        u32 codepoint;
        math::rect uvRect;
        math::rect quadRect;
        float advance;
    };
}
