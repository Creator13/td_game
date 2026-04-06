#pragma once

#include <vector>

#include "datatype.h"
#include "math/mat4.h"
#include "rendering/GraphicsBuffer.h"
#include "rendering/GrowableUbo.h"
#include "rendering/Material.h"
#include "rendering/RenderingDataStructures.h"
#include "rendering/pass/GeometryPass.h"

namespace core::gfx
{
    class Pipeline;

    struct DrawCommand
    {
        u64 sortKey;
        assets::AssetRef<Mesh> mesh;
        assets::AssetRef<Material> material;
        math::mat4 modelMatrix;

        u16 getPipelineId() const { return (sortKey >> 32) & 0xFFFF; }
        u16 getMaterialId() const { return (sortKey >> 16) & 0xFFFF; }
        u16 getMeshId() const { return sortKey & 0xFFFF; }
    };

    class Renderer
    {
        ViewportData _viewportData = ViewportData();

        std::vector<DrawCommand> _geometryCommandBuffer;
        std::vector<DrawCommand> _uiCommandBuffer;

        gl::buffer_t _frameDataUboHandle;
        GraphicsBuffer _instanceDataBuffer;

        gl::framebuffer_t _mainFramebuffer;
        gl::Uint _defaultSampler;

    public:
        Renderer();

        void setViewportData(const ViewportData& params);
        void submitSceneGeometry(const DrawCommand& command);
        void renderFrame();

    private:
        void bindPipeline(const Pipeline& pipeline);
        void bindMaterial(assets::AssetRef<Material> material);
        void bindFrameData() const;

        void sortCommandList();
        void bindInstanceData();

        void renderSceneGeometry();
        void renderUI();
    };
}
