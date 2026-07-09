#include "Font.h"

#include "assets/AssetDatabase.h"
#include "assets/File.h"
#include "assets/FontLoader.h"
#include "rendering/Material.h"
#include "rendering/ShaderPropertyId.h"

using namespace core;
using namespace core::assets;

AssetRef<gfx::Pipeline> Font::_defaultFontPipeline = AssetRef<gfx::Pipeline>::null();

Font::Font() :
    _fontMetrics(), _glyphDataRenderBuffer(128_kB) { }

AssetRef<gfx::Pipeline> Font::getOrCreateDefaultPipeline()
{
    using namespace gfx;

    if (_defaultFontPipeline.isNull())
    {
        constexpr PipelineDescriptor desc{
            .depthTest = false,
            .blend = true,
            .blendSource = BlendOption::One,
            .blendDestination = BlendOption::OneMinusSourceAlpha,
            .backfaceCulling = BackfaceCulling::Back,
        };
        _defaultFontPipeline = Pipeline::create("MSDF font", desc, "shaders/font.vert", "shaders/font.frag");
    }

    return _defaultFontPipeline;
}

Font::~Font()
{
    // TODO fix this: deleting a font asset should also delete the associated texture, but only when the asset database
    //  is *not* already being cleaned up. Depending on the initialization order, fonts might be cleaned up *after* the
    //  textures (or even the database instance) making this unsafe to perform.

    // AssetDatabase::deleteAsset(_fontTexture);
}

const GlyphMetrics& Font::getGlyphMetrics(char32_t codepoint) const
{
    if (codepoint <= 256)
    {
        return _glyphMetrics[codepoint];
    }

    return _glyphMetrics['?'];
}

AssetRef<Font> Font::loadFromFile(std::string_view path, AssetRef<gfx::Pipeline> fontPipeline)
{
    if (const auto existing = AssetDatabase::tryGetAsset<Font>(AssetId::idFromPath(path)))
    {
        return existing;
    }

    auto& storage = AssetDatabase::instance->getStorage<Font>();
    auto [mem, index] = storage.allocate_uninitialized();
    Font* font = ::new(mem) Font();

    FontLoader& fontLoader = AssetDatabase::instance->_fontLoader;
    fontLoader.loadFontAtlas(path, *font);

    if (fontPipeline.isNull())
    {
        fontPipeline = getOrCreateDefaultPipeline();
    }

    font->_fontMaterial = fontPipeline->newMaterialInstance(fmt::format("FontMaterial-{}", path));
    font->_fontMaterial->setTexture2D("msdfAtlas"_spid, font->_fontTexture);
    font->_fontMaterial->setFloat("emRange"_spid, font->getFontMetrics().emRange);
    font->_fontMaterial->setBuffer("GlyphBuffer"_spid, &(font->_glyphDataRenderBuffer));

    return AssetDatabase::registerAsset(path, font, index);
}
