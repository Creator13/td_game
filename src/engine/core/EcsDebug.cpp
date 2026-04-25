#include "EcsDebug.h"

#include "Debug.h"
#include "core/Time.h"
#include "core/ui/EcsUI.h"
#include "rendering/EcsRendering.h"

using namespace debug::ecs;
using namespace core;
using namespace core::ui;
using namespace core::assets;

struct RenderDebugEntity { };

engine_debug::engine_debug(flecs::world& ecs)
{
    _debugInfoFont = Font::loadFromFile("font/JetBrainsMono-Regular.ttf");

    ecs.module<engine_debug>("Debug module");
    ecs.component<DebugRendererSingleton>().add(flecs::Singleton);

    ecs.system<const gfx::ViewportData, DebugRendererSingleton>()
        .each([](const gfx::ViewportData& viewportData, const DebugRendererSingleton& dbg)
        {
            dbg.ptr->copyViewportData(viewportData);
        });

    ecs.entity("Rendering debug view")
        .set<Rect>({{10, 10}, {0, 0}, 0, anchor::topLeft})
        .emplace<Text>("")
        .set<TextRenderData>({_debugInfoFont, 12, Color::white})
        .add<RenderDebugEntity>();

    ecs.system<Text, const core::ecs::RendererSingleton&>()
        .with<RenderDebugEntity>()
        .each([](Text& text, const core::ecs::RendererSingleton& renderer)
        {
            auto renderStats = renderer.ptr->getFrameStats();

            std::string stats = fmt::format(
                "{:.3f}ms (avg:{:.3f}ms, 1%:{:.3f}ms) | {:.1f}fps\n"
                "Commands submitted: {} | draw calls: {} | pipeline binds: {}",
                time::deltaMs(), time::averageDeltaMs(), time::onePercentMs(), time::fps(),
                renderStats.numCommands, renderStats.numDrawCalls, renderStats.numPipelineBinds);
            text.setText(stats);
        });
}
