#include "Font.h"

#include "assets/AssetDatabase.h"
#include "assets/File.h"
#include "assets/FontLoader.h"
#include "rendering/Material.h"
#include "rendering/ShaderPropertyId.h"

using namespace core;
using namespace core::assets;

Font::Font() :
    _fontMetrics(), _glyphDataBuffer(128_kB) { }

Font::~Font()
{
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
    auto& storage = AssetDatabase::instance->getStorage<Font>();
    auto [mem, index] = storage.allocate_uninitialized();
    Font* font = ::new(mem) Font();

    FontLoader& fontLoader = AssetDatabase::instance->_fontLoader;
    fontLoader.loadFontAtlas(path, *font);

    font->_fontMaterial = fontPipeline->newMaterialInstance(fmt::format("FontMaterial-{}", path));
    font->_fontMaterial->setTexture2D("_msdfAtlas"_spid, font->_fontTexture);
    font->_fontMaterial->setFloat("screenPxRange"_spid, 2);
    font->_fontMaterial->setBuffer("GlyphBuffer"_spid, &font->_glyphDataBuffer);

    return AssetDatabase::registerAsset(path, font, index);
}
