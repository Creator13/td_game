#include "EcsUI.h"

#include <flecs.h>
#include <tracy/Tracy.hpp>

#include "assets/MeshPrimitives.h"
#include "core/EcsCore.h"
#include "rendering/EcsRendering.h"
#include "rendering/Pipeline.h"

using namespace math;
using namespace core::ui;
using namespace core::gfx;

namespace
{
    void registerComponents(flecs::world& ecs) { }

    constexpr mat4 make2dTRS(vec2 pos, float rot, vec2 size)
    {
        const float wdt = size.x;
        const float hgt = size.y;
        const float cx = pos.x + wdt * 0.5f;
        const float cz = pos.y + hgt * 0.5f;
        const float cosR = math::cos(rot);
        const float sinR = math::sin(rot);

        return mat4(
            wdt * cosR, 0, -hgt * sinR, cx,
            0, 1, 0, 0,
            wdt * sinR, 0, hgt * cosR, cz,
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

    PipelineDescriptor desc;
    desc.blend = false;
    desc.backfaceCulling = BackfaceCulling::Back;
    desc.depthTest = false;
    auto uiRenderPipeline = Pipeline::create("UI sprite", desc, "shaders/basic.vert", "shaders/ui.frag");
    _uiMaterial = uiRenderPipeline->newMaterialInstance("UI material");

    _uiQuad = Mesh::createView("UI quad",
        assets::mesh_primitives::QUAD_VERTICES,
        assets::mesh_primitives::QUAD_INDICES,
        assets::mesh_primitives::QUAD_BOUNDS);

    ecs.system<const Rect>("Rect collection system")
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
                    command.modelMatrix = make2dTRS(rect.offset, rect.rotation, rect.size);
                    command.queue = DrawCommand::RenderQueue::UI;

                    renderer->submitDrawCommand(command);
                }
            }
        });
}
