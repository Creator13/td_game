#pragma once

#include <vector>

#include "datatype.h"
#include "assets/Mesh.h"
#include "math/mat4.h"
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

    static_assert(std::is_trivially_destructible_v<DrawCommand>, "DrawCommand should be a trivially destructible type for good performance.");

    class Renderer
    {
        struct FrameStats
        {
            i32 numDrawCalls = 0;
            i32 numPipelineBinds = 0;
            i32 numCommands = 0;
            u64 triCount = 0;
        };

        using CommandQueue = std::vector<DrawCommand>;

        ViewportData _viewportData = ViewportData();
        LightingDataBlock _lightingData = LightingDataBlock();
        Color _clearColor;

        CommandQueue _opaqueCommandQueue;
        CommandQueue _uiCommandQueue;

        gl::buffer_t _viewportDataUboHandle;
        gl::buffer_t _frameDataUboHandle;
        gl::buffer_t _lightingDataUboHandle;

        GraphicsBuffer _instanceDataBuffer;

        Framebuffer _mainFramebuffer;
        std::array<Framebuffer, 2> _pingPongFramebuffers;

        gl::Uint _defaultSampler = 0; // TODO define samplers as a type
        gl::vert_arr_t _emptyVao = 0;

        assets::AssetRef<Material> _fullscreenBlitEffect = assets::AssetRef<Material>::null();
        std::vector<assets::AssetRef<Material>> _postEffects;

        FrameStats _frameStats;

    public:
        Renderer();

        void init(int fbWidth, int fbHeight);

        [[nodiscard]] const FrameStats& getFrameStats() const noexcept { return _frameStats; }

        void setViewportData(const ViewportData& params);
        void setLightData(const LightingDataBlock& lightingData);
        void submitDrawCommand(const DrawCommand& command);
        void renderFrame();

        void setPostEffectStack(const std::vector<assets::AssetRef<Material>>& stack);
        void setClearColor(const Color& clearColor);

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

        static void sortCommandList(std::vector<DrawCommand>& queue);
    };
}
