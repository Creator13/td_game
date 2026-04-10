#pragma once

#include <vector>

#include "datatype.h"
#include "assets/Mesh.h"
#include "math/mat4.h"
#include "rendering/DataLayout.h"
#include "rendering/GraphicsBuffer.h"
#include "rendering/Material.h"
#include "rendering/MeshGpuHandle.h"
#include "rendering/RenderingDataStructures.h"

namespace core::gfx
{
    class Pipeline;

    struct DrawCommand
    {
        enum class RenderQueue { OPAQUE, UI, SHADOW, INVALID };

        u64 sortKey;
        gpu::MeshGpuHandle mesh;
        assets::AssetRef<Material> material;
        math::mat4 modelMatrix;
        RenderQueue queue = RenderQueue::INVALID;

        u16 getPipelineId() const { return (sortKey >> 32) & 0xFFFF; }
        u16 getMaterialId() const { return (sortKey >> 16) & 0xFFFF; }
        u16 getMeshId() const { return sortKey & 0xFFFF; }
    };

    static_assert(std::is_trivially_destructible_v<DrawCommand>, "DrawCommand should be a trivially destructible type for good performance.");

    class Renderer
    {
        using CommandQueue = std::vector<DrawCommand>;

        ViewportData _viewportData = ViewportData();

        CommandQueue _opaqueCommandQueue;
        CommandQueue _uiCommandQueue;

        gl::buffer_t _frameDataUboHandle;
        GraphicsBuffer _instanceDataBuffer;

        gl::framebuffer_t _mainFramebuffer;
        gl::Uint _defaultSampler;

    public:
        Renderer();

        void setViewportData(const ViewportData& params);
        void submitDrawCommand(const DrawCommand& command);
        void renderFrame();

        static u64 buildSortKey(assets::AssetRef<Material> material, assets::AssetRef<Mesh> mesh);

    private:
        void bindPipeline(const Pipeline& pipeline);
        void bindMaterial(assets::AssetRef<Material> material);
        void bindPassData(const PassDataBlock& passData) const;

        void appendInstanceData(CommandQueue& queue);
        void executePass(const PassDataBlock& passData, CommandQueue& queue, usize instanceIndex);

        static void sortCommandList(std::vector<DrawCommand>& queue);
    };
}
