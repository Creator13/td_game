#pragma once
#include <string_view>

#include "datatype.h"
#include "assets/AssetRef.h"
#include "assets/Font.h"

namespace msdfgen
{
    class FreetypeHandle;
}

namespace core::assets
{
    class FontLoader
    {
        msdfgen::FreetypeHandle* _ftHandle;
        const u32 _numWorkerThreads;

        bool validateFontFile(std::string_view path) const;
        static u32 getWorkerThreadCount();

    public:
        FontLoader();
        ~FontLoader();

        FontLoader(FontLoader&& other) = delete;
        FontLoader(const FontLoader&) = delete;

        // struct FontLoadResult
        // {
        //     std::vector<unsigned char> atlasPixels;
        //     int atlasWidth, atlasHeight;
        //     TextureFormat atlasTextureFormat;
        //     FontMetrics metrics;
        //     std::array<GlyphMetrics, 256> glyphMetrics;
        // };

        bool loadFontAtlas(std::string_view path, Font& outFont);
    };
}
