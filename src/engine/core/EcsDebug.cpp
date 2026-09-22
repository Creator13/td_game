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
using namespace core::ecs;
using namespace math;

namespace
{
    // TODO extract this to be globally accessible when more places need this formatting (wow maybe even by default?)
    struct EnglishNumberPunctuation : std::numpunct<char>
    {
    protected:
        char do_decimal_point() const override { return '.'; }
        char do_thousands_sep() const override { return ','; }
        std::string do_grouping() const override { return "\3"; }
    };

    std::locale enLoc(std::locale::classic(), new EnglishNumberPunctuation);

    struct RenderDebugEntity { };

    struct DebugSystemTag { };

    constexpr auto COMMANDS_PLOT_NAME = "Commands";
    constexpr auto DRAW_CALLS_PLOT_NAME = "Draw calls";
}

engine_debug::engine_debug(flecs::world& ecs)
{
    TracyPlotConfig(COMMANDS_PLOT_NAME, tracy::PlotFormatType::Number, false, true, tracy::Color::Orange2);
    TracyPlotConfig(DRAW_CALLS_PLOT_NAME, tracy::PlotFormatType::Number, false, true, tracy::Color::OrangeRed2);

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

    ecs.system<Text, const RendererSingleton&>()
        .with<RenderDebugEntity>()
        .each([](Text& text, const RendererSingleton& renderer)
        {
            auto renderStats = renderer.ptr->getFrameStats();
            auto assetStats = AssetDatabase::stats();

            std::string stats = fmt::format(enLoc,
                "{:.3f}ms (avg:{:.3f}ms, 1%:{:.3f}ms) | {:.1f}fps\n"
                "Commands submitted: {} | shadow casters: {} | tri count: {:L} | draw calls: {} | pipeline binds: {} | lights: {} dir, {} point, {} spot\n"
                "Asset storage: {} items, {:.2b} (Pl:{}/{:.1b} | Mat:{}/{:.1b} | Tex:{}/{:.1b} | Mesh:{}/{:.1b} | Font:{}/{:.1b})",
                time::deltaMs(), time::averageDeltaMs(), time::onePercentMs(), time::fps(),
                renderStats.numCommands, renderStats.numShadowCasters, renderStats.triCount, renderStats.numDrawCalls, renderStats.numPipelineBinds,
                renderStats.numDirLights, renderStats.numPointLights, renderStats.numSpotlights,
                assetStats.totalAssetCount, FormattableBytes(assetStats.totalBytesUsed),
                assetStats.pipelineStats.itemCount, FormattableBytes(assetStats.pipelineStats.bytesUsed),
                assetStats.materialStats.itemCount, FormattableBytes(assetStats.materialStats.bytesUsed),
                assetStats.textureStats.itemCount, FormattableBytes(assetStats.textureStats.bytesUsed),
                assetStats.meshStats.itemCount, FormattableBytes(assetStats.meshStats.bytesUsed),
                assetStats.fontStats.itemCount, FormattableBytes(assetStats.fontStats.bytesUsed)
            );
            text.setText(stats);

            // TODO move this somewhere else (maybe?)
            TracyPlot(COMMANDS_PLOT_NAME, static_cast<i64>(renderStats.numCommands));
            TracyPlot(DRAW_CALLS_PLOT_NAME, static_cast<i64>(renderStats.numDrawCalls));
        })
        .add<DebugSystemTag>();

    ecs.system<const BoxBoundsData, const MeshRenderData>()
        .each([](const BoxBoundsData& data, const MeshRenderData& renderData)
        {
            Color color = Color::magenta;
            switch (renderData.cullReason)
            {
                case CullReason::None:
                    color = Color::deepPink;
                    break;
                case CullReason::Frustum:
                    color = Color::darkSeaGreen;
                    break;
                case CullReason::LOD:
                    color = Color::cadetBlue;
                    break;
            }
            core::debug::drawAABB(data.cachedWorldBounds, color);
        })
        .disable();

    ecs.system<const HierarchyTransform, const LightData>()
        .with<Gizmo>()
        .each([](const HierarchyTransform& transform, const LightData& light)
            {
                core::debug::drawRay(transform.getWorldPosition(), transform.getForward() * max(light.range, 1), light.color);
                if (light.type == LightData::Type::Spot)
                {
                    float halfAngle = light.cutoffDegrees * .5;

                    vec3 fwd = transform.getForward();
                    vec3 right = transform.getRight();
                    vec3 up = transform.getUp();
                    vec3 pos = transform.getWorldPosition();

                    vec3 dirUp = rotate(quaternion::angleAxis(-halfAngle, right), fwd);
                    vec3 dirDown = rotate(quaternion::angleAxis(halfAngle, right), fwd);

                    vec3 dirRight = rotate(quaternion::angleAxis(-halfAngle, up), fwd);
                    vec3 dirLeft = rotate(quaternion::angleAxis(halfAngle, up), fwd);

                    core::debug::drawRay(pos, dirUp * light.range, light.color);
                    core::debug::drawRay(pos, dirDown * light.range, light.color);
                    core::debug::drawRay(pos, dirLeft * light.range, light.color);
                    core::debug::drawRay(pos, dirRight * light.range, light.color);
                }
            }
        )
        .add<DebugSystemTag>();
}
