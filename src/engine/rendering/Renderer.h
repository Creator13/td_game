#pragma once

#include <vector>

#include "datatype.h"
#include "math/mat4.h"
#include "rendering/GrowableUbo.h"
#include "rendering/Material.h"
#include "rendering/RenderingDataStructures.h"
#include "rendering/pass/GeometryPass.h"

namespace core::gfx
{
    class Pipeline;

    struct DrawCommand
    {
        assets::AssetRef<Mesh> mesh;
        assets::AssetRef<Material> material;
        math::mat4 modelMatrix;
    };

    class Renderer
    {
        ViewportData _viewportData = ViewportData();

        std::vector<DrawCommand> _geometryCommandBuffer;
        std::vector<DrawCommand> _uiCommandBuffer;

        gl::buffer_t _frameDataUboHandle;
        GrowableUbo _perFrameUbo;

        gl::framebuffer_t _mainFramebuffer;

    public:
        Renderer();

        void setViewportData(const ViewportData& params);
        void submitSceneGeometry(const DrawCommand& command);
        void renderFrame();

    private:
        void bindPipeline(const Pipeline& pipeline);
        void bindMaterial(Material& material);
        void bindFrameData() const;

        void renderSceneGeometry();
        void renderUI();
    };
}
