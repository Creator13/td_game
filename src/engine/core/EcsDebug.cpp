#include "EcsDebug.h"

#include "Debug.h"
#include "assets/AssetDatabase.h"
#include "core/Time.h"
#include "core/ui/EcsUI.h"
#include "formatting/fmt_bytes.h"
#include "rendering/EcsRendering.h"

using namespace debug::ecs;
using namespace core;
using namespace core::ui;
using namespace core::assets;

namespace {
    struct RenderDebugEntity { };
}

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
            auto assetStats = AssetDatabase::stats();

            std::string stats = fmt::format(std::locale("en_US.UTF-8"),
                "{:.3f}ms (avg:{:.3f}ms, 1%:{:.3f}ms) | {:.1f}fps\n"
                "Commands submitted: {} | tri count: {:L} | draw calls: {} | pipeline binds: {}\n"
                "Asset storage: {} items, {:.2b} (Pl:{}/{:.1b} | Mat:{}/{:.1b} | Tex:{}/{:.1b} | Mesh:{}/{:.1b} | Font:{}/{:.1b})",
                time::deltaMs(), time::averageDeltaMs(), time::onePercentMs(), time::fps(),
                renderStats.numCommands, renderStats.triCount, renderStats.numDrawCalls, renderStats.numPipelineBinds,
                assetStats.totalAssetCount, FormattableBytes(assetStats.totalBytesUsed),
                assetStats.pipelineStats.itemCount, FormattableBytes(assetStats.pipelineStats.bytesUsed),
                assetStats.materialStats.itemCount, FormattableBytes(assetStats.materialStats.bytesUsed),
                assetStats.textureStats.itemCount, FormattableBytes(assetStats.textureStats.bytesUsed),
                assetStats.meshStats.itemCount, FormattableBytes(assetStats.meshStats.bytesUsed),
                assetStats.fontStats.itemCount, FormattableBytes(assetStats.fontStats.bytesUsed)
                );
            text.setText(stats);
        });
}
