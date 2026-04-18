#include "FontLoader.h"

#include <ft2build.h>
#include <tracy/Tracy.hpp>
#include <msdf-atlas-gen/msdf-atlas-gen.h>
#include FT_FREETYPE_H

#include "Logging.h"
#include "assets/AssetDatabase.h"
#include "assets/File.h"

using namespace core;
using namespace core::assets;

namespace
{
    msdfgen::Shape createFallbackDiamond()
    {
        msdfgen::Shape shape;
        msdfgen::Contour& contour = shape.addContour();

        // Define a diamond centered on the baseline
        // Points: Bottom, Right, Top, Left
        msdfgen::Point2 pB(0.5, -0.1);
        msdfgen::Point2 pR(0.9, 0.4);
        msdfgen::Point2 pT(0.5, 0.9);
        msdfgen::Point2 pL(0.1, 0.4);

        // Add edges (Linear segments)
        contour.addEdge(new msdfgen::LinearSegment(pB, pR));
        contour.addEdge(new msdfgen::LinearSegment(pR, pT));
        contour.addEdge(new msdfgen::LinearSegment(pT, pL));
        contour.addEdge(new msdfgen::LinearSegment(pL, pB));

        // MSDF shapes must have oriented contours for the distance field to work
        shape.inverseYAxis = false;
        return shape;
    }

    const msdf_atlas::Charset& getDefaultCharset()
    {
        static const msdf_atlas::Charset charset = []()
        {
            msdf_atlas::Charset cs = msdf_atlas::Charset::ASCII;
            for (msdf_atlas::unicode_t c = 0x00A0; c <= 0x00FF; c++)
            {
                cs.add(c);
            }
            cs.add(0xFFFD); // fallback character (�)
            return cs;
        }();
        return charset;
    }
}

bool FontLoader::validateFontFile(std::string_view path) const
{
    // Hack to get the underlying FT_Library from msdfgen's FreetypeHandle, because there is no need to reinitialize the library each time for validation
    // (alternative would be to store two FT_Libraries, one for validation and the other for msdfgen::FreetypeHandle).
    const FT_Library lib = *reinterpret_cast<FT_Library*>(ftHandle);

    FT_Face face;
    const FT_Error err = FT_New_Face(lib, path.data(), 0, &face);

    if (err)
    {
        spdlog::error("Failed to load font file at \"{}\": {}", path, FT_Error_String(err));
    }
    FT_Done_Face(face);

    return err == 0;
}

u32 FontLoader::getWorkerThreadCount()
{
    const u32 numOsThreads = std::thread::hardware_concurrency();
    if (numOsThreads == 0) return 2;
    return math::max(2, numOsThreads - 2);
}

FontLoader::FontLoader()
    : ftHandle(msdfgen::initializeFreetype()), _numWorkerThreads(getWorkerThreadCount()) { }

FontLoader::~FontLoader()
{
    msdfgen::deinitializeFreetype(ftHandle);
}

bool FontLoader::loadFontAtlas(std::string_view path, Font& outFont)
{
    file::fs::path fullPath = AssetDatabase::resolveResourcePath(path);

    if (!validateFontFile(fullPath.string()))
    {
        return false;
    }
    msdfgen::FontHandle* srcFont = msdfgen::loadFont(ftHandle, fullPath.string().c_str());

    std::vector<msdf_atlas::GlyphGeometry> glyphs;
    msdf_atlas::FontGeometry geometry(&glyphs);
    geometry.loadCharset(srcFont, 1.0, getDefaultCharset());

    for (auto& glyph : glyphs)
    {
        constexpr double maxCornerAngle = 3.0;
        glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);
    }

    constexpr double pxRange = 2;

    msdf_atlas::TightAtlasPacker packer;
    packer.setDimensionsConstraint(msdf_atlas::DimensionsConstraint::POWER_OF_TWO_RECTANGLE);
    packer.setMinimumScale(24);
    packer.setPixelRange(pxRange);
    packer.setMiterLimit(1);
    if (packer.pack(glyphs.data(), glyphs.size()) != 0)
    {
        spdlog::error("Failed to pack glyphs for font {}", path);
        return false;
    }

    constexpr int depth = 4;
    int width, height;
    packer.getDimensions(width, height);
    msdf_atlas::ImmediateAtlasGenerator<
        float,
        depth,
        msdf_atlas::mtsdfGenerator,
        msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, depth>
    > generator(width, height);

    msdf_atlas::GeneratorAttributes attributes;
    generator.setAttributes(attributes);
    generator.setThreadCount(_numWorkerThreads);
    generator.generate(glyphs.data(), glyphs.size());

    auto& fontMetrics = geometry.getMetrics();
    outFont._fontMetrics = FontMetrics{
        .ascenderY = static_cast<float>(fontMetrics.ascenderY),
        .descenderY = static_cast<float>(fontMetrics.descenderY),
        .lineHeight = static_cast<float>(fontMetrics.lineHeight),
        .emRange = static_cast<float>(pxRange / packer.getScale()),
    };

    msdfgen::BitmapConstRef<msdf_atlas::byte, depth> bmp = generator.atlasStorage();

    for (const auto& glyph : glyphs)
    {
        msdfgen::unicode_t codepoint = glyph.getCodepoint();
        if (codepoint > 255)
        {
            continue; // Skip non-(ascii or latin1) characters, will be substituted for now TODO implement wider ranges, dynamic?
        }

        GlyphMetrics& current = outFont._glyphMetrics[codepoint];
        current.renderDirective = GlyphMetrics::RenderDirective::Render;
        current.advance = static_cast<float>(glyph.getAdvance());

        double planeL, planeB, planeR, planeT;
        glyph.getQuadPlaneBounds(planeL, planeB, planeR, planeT);

        double atlasL, atlasB, atlasR, atlasT;
        glyph.getQuadAtlasBounds(atlasL, atlasB, atlasR, atlasT);

        current.quadRect.offset = math::vec2(planeL, -planeB);
        current.quadRect.extents = math::vec2(planeR - planeL, -(planeT - planeB));

        current.uvRect.offset = math::vec2(atlasL / bmp.width, atlasB / bmp.height);
        current.uvRect.extents = math::vec2((atlasR - atlasL) / bmp.width, (atlasT - atlasB) / bmp.height);
    }

    // 2nd pass: set all valid control characters to skip rendering
    for (usize cp = 0; cp < 256; cp++)
    {
        auto& glyphMetric = outFont._glyphMetrics[cp];

        if (glyphMetric.renderDirective == GlyphMetrics::RenderDirective::Render) continue; // Already dealt with

        if (cp < 0x0021
            || cp == 0x007f
            || (cp >= 0x0080 && cp <= 0x009f)
            || cp == 0x00ad)
        {
            glyphMetric.renderDirective = GlyphMetrics::RenderDirective::Control;
        }
        else
        {
            glyphMetric.renderDirective = GlyphMetrics::RenderDirective::Substitute;
        }
    }

    outFont._fontTexture = Texture::create(fmt::format("FontAtlas-{}", fullPath.filename().string()), bmp.width, bmp.height, TextureFormat::RGBA8_UNORM, false, true);
    outFont._fontTexture->uploadExternalData(
        bmp.pixels,
        texture_util::getGlPixelFormat(TextureFormat::RGBA8_UNORM),
        texture_util::getGlPixelDataType(TextureFormat::RGBA8_UNORM),
        false);

    msdfgen::destroyFont(srcFont);
    return true;
}
