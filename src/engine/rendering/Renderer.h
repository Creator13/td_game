#pragma once

#include <vector>

#include "datatype.h"
#include "assets/Mesh.h"
#include "rendering/DataLayout.h"
#include "rendering/Framebuffer.h"
#include "rendering/GraphicsBuffer.h"
#include "rendering/Material.h"
#include "rendering/MeshGpuHandle.h"
#include "rendering/RenderingDataStructures.h"

namespace core::gfx
{
    class Pipeline;

    struct DrawCommand
    {
        enum class RenderQueue : u8 { OPAQUE, UI, SHADOW, INVALID };

        u64 sortKey;
        gpu::MeshGpuHandle mesh;
        assets::AssetRef<Material> material = assets::AssetRef<Material>::null();
        RenderQueue queue = RenderQueue::INVALID;
        InstanceData instanceData;

        u16 getPipelineId() const { return (sortKey >> 32) & 0xFFFF; }
        u16 getMaterialId() const { return (sortKey >> 16) & 0xFFFF; }
        u16 getMeshId() const { return sortKey & 0xFFFF; }
    };

    struct EnvironmentSettings
    {
        float ambientIntensity = .01f;
    };

    static_assert(std::is_trivially_destructible_v<DrawCommand>, "DrawCommand should be a trivially destructible type for good performance.");

    class Renderer
    {
        using CommandQueue = std::vector<DrawCommand>;

        struct FrameStats
        {
            i32 numDrawCalls = 0;
            i32 numPipelineBinds = 0;

            i32 numCommands = 0;
            i32 numShadowCasters = 0;

            i32 numDirLights = 0;
            i32 numSpotlights = 0;
            i32 numPointLights = 0;

            u64 triCount = 0;
        } _frameStats;

        ViewportData _viewportData = ViewportData();
        ShadowData _shadowData = ShadowData();
        EnvironmentSettings _environmentSettings = EnvironmentSettings();
        LightingDataBlock _lightingDataBlock;
        Color _clearColor;
        int _shadowMapResolution;

        CommandQueue _shadowCommandQueue;
        CommandQueue _opaqueCommandQueue;
        CommandQueue _uiCommandQueue;

        gl::buffer_t _viewportDataUboHandle;
        gl::buffer_t _frameDataUboHandle;
        gl::buffer_t _lightingDataUboHandle;

        GraphicsBuffer _instanceDataBuffer;

        Framebuffer _mainFramebuffer;
        Framebuffer _shadowFramebuffer;
        std::array<Framebuffer, 2> _pingPongFramebuffers;

        gl::Uint _defaultSampler = 0; // TODO define samplers as a type
        gl::Uint _shadowSampler = 0;
        gl::vert_arr_t _emptyVao = 0;

        assets::AssetRef<Material> _depthMaterial;
        assets::AssetRef<Material> _fullscreenBlitEffect;
        std::vector<assets::AssetRef<Material>> _postEffects;

    public:
        Renderer();

        void init();

        [[nodiscard]] const FrameStats& getFrameStats() const noexcept { return _frameStats; }

        void setViewportData(const ViewportData& viewportData);
        void setShadowData(const ShadowData& shadowData);
        void setEnvironmentSettings(const EnvironmentSettings& env);
        void setPostEffectStack(const std::vector<assets::AssetRef<Material>>& stack);
        void setClearColor(const Color& clearColor);

        void submitPointLight(const LightingDataBlock::PointLight& light);
        void submitDirectionalLight(const LightingDataBlock::DirectionalLight& light);
        void submitSpotlight(const LightingDataBlock::Spotlight& spotlight);

        void submitShadowCommand(assets::AssetRef<Mesh> mesh, const math::mat4& transform);
        void submitDrawCommand(const DrawCommand& command);

        void renderFrame();

        static u64 buildSortKey(assets::AssetRef<Material> material, assets::AssetRef<Mesh> mesh);

    private:
        void bindPipeline(const Pipeline& pipeline);
        void bindMaterial(assets::AssetRef<Material> material);
        void bindPassData(const ViewportDataBlock& passData) const;
        void bindFrameData(const FrameDataBlock& passData) const;
        void bindLightingData(const LightingDataBlock& lightingData) const;

        void resizeFrameBuffers(int newWidth, int newHeight);

        void appendInstanceData(CommandQueue& queue);

        void executePass(const ViewportDataBlock& passData, CommandQueue& queue, usize instanceIndex);
        void executePostEffectStack();
        void executePostEffect(assets::AssetRef<Material> material, const Framebuffer& src, gl::framebuffer_t dst);

        void resetLightingData();

        static void sortCommandList(std::vector<DrawCommand>& queue);
    };
}
