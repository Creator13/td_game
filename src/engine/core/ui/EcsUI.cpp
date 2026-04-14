#include "EcsUI.h"

#include <flecs.h>
#include <tracy/Tracy.hpp>

#include "assets/MeshPrimitives.h"
#include "core/Debug.h"
#include "core/EcsCore.h"
#include "core/EcsDebug.h"
#include "rendering/EcsRendering.h"
#include "rendering/Pipeline.h"
#include "math/geom.h"

using namespace math;
using namespace core::ui;
using namespace core::gfx;

namespace
{
    void registerComponents(flecs::world& ecs) { }

    constexpr mat4 make2dRectTRS(rect rect, float rot, vec2 anchor = anchor::middleCenter)
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
}

Text::Text(std::string_view text)
    : text(text) { }

engine_ui::engine_ui(flecs::world& ecs)
{
    ecs.import<ecs::engine_core>();

    registerComponents(ecs);

    auto tex = Texture::loadFromFile("tex/uv_checker.png", TextureFormat::RGBA8_SRGB, false);

    PipelineDescriptor desc;
    desc.blend = false;
    desc.backfaceCulling = BackfaceCulling::None;
    desc.depthTest = false;
    auto uiRenderPipeline = Pipeline::create("UI sprite", desc, "shaders/basic.vert", "shaders/ui.frag");
    _uiMaterial = uiRenderPipeline->newMaterialInstance("UI material");
    _uiMaterial->setTexture2D("_texture"_spid, tex);

    _uiQuad = Mesh::createView("UI quad",
        assets::mesh_primitives::QUAD_VERTICES,
        assets::mesh_primitives::QUAD_INDICES,
        assets::mesh_primitives::QUAD_BOUNDS);

    ecs.system<const Rect>("Rect collection system")
        .without<Text>()
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
                    command.modelMatrix = make2dRectTRS(rect, rect.rotation, rect.anchor);
                    command.queue = DrawCommand::RenderQueue::UI;

                    renderer->submitDrawCommand(command);
                }
            }
        });

    ecs.system<const Rect, const Text, const TextRenderData>("Text rendering system").run([this](flecs::iter& it)
    {
        ZoneScopedN("UI font rendering system");

        const auto& [renderer] = it.world().get<const ecs::RendererSingleton>();

        const assets::AssetRef<Mesh> glyphMesh = _uiQuad;

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
                const float fontSize = renderData.size;

                debug::draw2DRect(rect, Color::fromSrgb(SrgbColor::magenta));

                DrawCommand baseCommand;
                baseCommand.mesh = glyphMesh->gpuHandle;
                baseCommand.material = renderData.font->getMaterial();
                baseCommand.sortKey = Renderer::buildSortKey(baseCommand.material, glyphMesh);
                baseCommand.queue = DrawCommand::RenderQueue::UI;

                mat4 baseTransform = make2dRectTRS({rect.offset, vec2::one}, rect.rotation, rect.anchor);

                const assets::FontMetrics& fontMetrics = renderData.font->getFontMetrics();
                float ascender = fontMetrics.ascenderY * fontSize;
                float descender = fontMetrics.descenderY * fontSize;
                float baselineHeight = ascender;
                vec2 pen = vec2(0, baselineHeight);
                debug::draw2DLine(rect.offset + pen, rect.offset + pen + vec2(rect.size.x, 0), Color::fromSrgb(SrgbColor::cyan));
                debug::draw2DLine(rect.offset + vec2(0, baselineHeight - ascender), rect.offset + vec2(0, baselineHeight - ascender) + vec2(rect.size.x, 0), Color::fromSrgb(SrgbColor::red));
                debug::draw2DLine(rect.offset + vec2(0, baselineHeight - descender), rect.offset + vec2(0, baselineHeight - descender) + vec2(rect.size.x, 0), Color::fromSrgb(SrgbColor::green));
                for (const auto c : text.text)
                {
                    DrawCommand command = baseCommand;

                    const assets::GlyphMetrics& glyphMetrics = renderData.font->getGlyphMetrics(c);
                    math::rect glyphRect = glyphMetrics.quadRect;
                    glyphRect.offset *= fontSize;
                    glyphRect.extents *= fontSize;
                    glyphRect = translate(glyphRect, pen);
                    command.modelMatrix = baseTransform * make2dRectTRS(glyphRect, 0, anchor::topLeft);
                    pen.x += glyphMetrics.advance * fontSize;

                    renderer->submitDrawCommand(command);
                }
            }
        }
    });
}
