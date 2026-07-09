#include "EcsUI.h"

#include <flecs.h>
#include <simdutf.h>
#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

#include "assets/MeshPrimitives.h"
#include "core/Debug.h"
#include "core/EcsCore.h"
#include "math/geom.h"
#include "rendering/EcsRendering.h"
#include "rendering/Pipeline.h"

using namespace math;
using namespace core::assets;
using namespace core::gfx;
using namespace core::ui;

namespace
{
    struct alignas(16) GlyphInstanceData
    {
        vec2 uvOffset;
        vec2 uvExtents;
        core::Color color;
        float fontRenderSize;
    };

    void registerComponents(flecs::world& ecs) { }

    mat4 make2dRectTRS(rect rect, float rot, vec2 anchor = anchor::middleCenter)
    {
        rot *= DEG2RAD;

        const float cosR = math::cos(rot);
        const float sinR = math::sin(rot);

        const float scaledPivotX = anchor.x * rect.extents.x;
        const float scaledPivotY = anchor.y * rect.extents.y;

        const float offsetX = (scaledPivotX * cosR) - (scaledPivotY * sinR);
        const float offsetZ = (scaledPivotX * sinR) + (scaledPivotY * cosR);

        const float wdt = rect.extents.x;
        const float hgt = rect.extents.y;

        const float tx = rect.offset.x - offsetX;
        const float tz = rect.offset.y - offsetZ;

        return mat4(
            wdt * cosR, 0, -hgt * sinR, tx,
            0, 1, 0, 0,
            wdt * sinR, 0, hgt * cosR, tz,
            0, 0, 0, 1
        );
    }

    int sortByFontComparison(flecs::entity_t e1, const TextRenderData* t1, flecs::entity_t e2, const TextRenderData* t2)
    {
        (void) e1;
        (void) e2;
        // Basic id-based sorting, should probably be enough?
        return (t1->font.id().id > t2->font.id().id) - (t1->font.id().id < t2->font.id().id);
    }
}

Text::Text(std::string_view text)
{
    setText(text);
}

void Text::setText(std::string_view newText)
{
    ZoneScoped;

    const auto data = std::span{newText};
    const simdutf::encoding_type encoding = simdutf::autodetect_encoding(data);

    if (encoding == simdutf::UTF8)
    {
        text.resize(simdutf::utf32_length_from_utf8(data));
        simdutf::result res = simdutf::convert_utf8_to_utf32_with_errors(data, text);
        ENGINE_ASSERT(res.error == simdutf::SUCCESS, "Error converting input text to utf32 codepoints. Index {}, error: {}", res.count, magic_enum::enum_name(res.error));
    }
    else if (encoding == simdutf::UTF32_LE)
    {
        // Native encoding is a memcpy.
        text.assign(reinterpret_cast<const char32_t*>(newText.data()), newText.size() / 4);
    }
    else
    {
        ENGINE_ASSERT(false, "Unsupported encoding: {}", magic_enum::enum_name(encoding));
    }
}

engine_ui::engine_ui(flecs::world& ecs)
{
    ecs.import<ecs::engine_core>();

    registerComponents(ecs);

    auto tex = Texture::loadFromFile("tex/uv_checker.png", TextureFormat::RGBA8_SRGB, false, true);

    constexpr PipelineDescriptor desc{
        .depthTest = false,
        .blend = false,
        .backfaceCulling = BackfaceCulling::None, // Culling could technically be Back for all screenspace ui elements, its only required to be None for worldspace elements
    };

    auto uiRenderPipeline = Pipeline::create("UI sprite", desc, "shaders/basic.vert", "shaders/ui.frag");
    _uiMaterial = uiRenderPipeline->newMaterialInstance("UI material");
    _uiMaterial->setTexture2D("_texture"_spid, tex);

    _uiQuad = Mesh::createView("UI quad",
        mesh_primitives::QUAD_VERTICES,
        mesh_primitives::QUAD_INDICES,
        mesh_primitives::QUAD_BOUNDS);

    ecs.system<const Rect>("Rect collection system")
        .without<Text>()
        .kind(flecs::OnStore)
        .run([this](flecs::iter& it)
        {
            ZoneScopedN("UI geometry collection system");

            const auto& [renderer] = it.world().get<const ecs::RendererSingleton>();

            while (it.next())
            {
                auto f_rect = it.field<const Rect>(0);

                for (auto i : it)
                {
                    const Rect& rect = f_rect[i];

                    DrawCommand command;
                    command.mesh = _uiQuad->gpuHandle;
                    command.material = _uiMaterial;
                    command.sortKey = Renderer::buildSortKey(_uiMaterial, _uiQuad);
                    command.instanceData.transform = make2dRectTRS(rect, rect.rotation, rect.anchor);
                    command.queue = DrawCommand::RenderQueue::UI;

                    renderer->submitDrawCommand(command);
                }
            }
        });

    ecs.system<const Rect, const Text, const TextRenderData>("Text rendering system")
        .order_by(sortByFontComparison)
        .kind(flecs::OnStore)
        .run([this](flecs::iter& it)
        {
            ZoneScopedN("UI font rendering system");

            /* Font rendering system iterates all ui text entities. Glyph rendering relies on a decent amount of data
             * not passable through the standard material or custom instance data. We want to use a single GraphicsBuffer
             * for all texts that use the same font, so they can share a material and therefore an (instanced) draw call.
             * In order to achieve this, the system query is sorted by font. While iterating, we perform actions on the
             * same buffer until we reach a different font.
             */

            const auto& [renderer] = it.world().get<const ecs::RendererSingleton>();

            const AssetRef<Mesh> glyphMesh = _uiQuad;

            AssetRef<Font> currentFont;
            GraphicsBuffer* glyphDataBuffer = nullptr;
            usize glyphBufferIndex = 0;

            while (it.next())
            {
                auto f_rect = it.field<const Rect>(0);
                auto f_text = it.field<const Text>(1);
                auto f_renderData = it.field<const TextRenderData>(2);

                for (auto i : it)
                {
                    const Rect& rect = f_rect[i];
                    const Text& text = f_text[i];
                    const TextRenderData& renderData = f_renderData[i];

                    // Reset the font and buffer index and flush the previous buffer
                    if (renderData.font != currentFont)
                    {
                        if (glyphDataBuffer != nullptr)
                        {
                            glyphDataBuffer->upload();
                        }

                        currentFont = renderData.font;
                        glyphDataBuffer = &currentFont->getGlyphDataBuffer();
                        glyphDataBuffer->clear();
                        glyphBufferIndex = 0;
                    }

                    const float fontSize = renderData.size;

                    DrawCommand baseCommand;
                    baseCommand.mesh = glyphMesh->gpuHandle;
                    baseCommand.material = renderData.font->getMaterial();
                    baseCommand.sortKey = Renderer::buildSortKey(baseCommand.material, glyphMesh);
                    baseCommand.queue = DrawCommand::RenderQueue::UI;

                    mat4 baseTransform = make2dRectTRS({rect.offset, vec2::one}, rect.rotation, rect.anchor);

                    const FontMetrics& fontMetrics = renderData.font->getFontMetrics();
                    float baselineHeight = fontMetrics.ascenderY * fontSize;

                    vec2 pen = vec2(0, baselineHeight);
                    usize col = 0; // Current character in the line, independent of actual character widths.
                    usize line = 0;
                    for (const auto c : text.text)
                    {
                        GlyphMetrics const* glyphMetrics = &renderData.font->getGlyphMetrics(c);
                        if (glyphMetrics->renderDirective == GlyphMetrics::RenderDirective::Control)
                        {
                            // Handle control characters before anything else
                            if (c == '\n')
                            {
                                pen.x = 0;
                                pen.y += fontMetrics.lineHeight * fontSize;
                                col = 0;
                                line++;
                            }
                            else if (c == ' ')
                            {
                                pen.x += glyphMetrics->advance * fontSize;
                                col++;
                            }
                            else if (c == '\t')
                            {
                                constexpr u32 tabWidth = 4;
                                const u32 advanceCount = ((col + tabWidth - 1) & -tabWidth) - col;
                                pen.x += glyphMetrics->advance * fontSize * advanceCount;
                            }
                            // And don't render this character
                            continue;
                        }
                        if (glyphMetrics->renderDirective == GlyphMetrics::RenderDirective::Substitute)
                        {
                            // This glyph should be rendered but does not have an associated glyph in the font.
                            // Replace the glyph with a question mark (TODO implement a proper tofu character)
                            glyphMetrics = &renderData.font->getGlyphMetrics('?');
                        }

                        DrawCommand command = baseCommand;

                        // TODO either:
                        //  - cache glyph quads in entity, calculating only on set (most text does not change)
                        //  - Offload to gpu.
                        math::rect glyphRect = glyphMetrics->quadRect;
                        glyphRect.offset *= fontSize;
                        glyphRect.extents *= fontSize;
                        glyphRect = translate(glyphRect, pen);
                        command.instanceData.transform = baseTransform * make2dRectTRS(glyphRect, 0, anchor::topLeft);
                        pen.x += glyphMetrics->advance * fontSize;

                        GlyphInstanceData instanceData;
                        instanceData.uvOffset = glyphMetrics->uvRect.offset;
                        instanceData.uvExtents = glyphMetrics->uvRect.extents;
                        instanceData.color = renderData.color;
                        instanceData.fontRenderSize = renderData.size;

                        ENGINE_ASSERT(glyphDataBuffer != nullptr, "A nullptr glyphbuffer at this point can only be the result of a corrupt font asset. (font asset id: {})", renderData.font.id());
                        glyphDataBuffer->append(instanceData);

                        command.instanceData.customData.c0 = glyphBufferIndex;

                        renderer->submitDrawCommand(command);
                        glyphBufferIndex++;
                    }
                }
            }

            if (glyphDataBuffer != nullptr)
            {
                glyphDataBuffer->upload();
            }
        });
}
