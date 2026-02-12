#pragma once

#include <vector>

#include "datatype.h"
#include "core/Color.h"
#include "math/mat4.h"
#include "rendering/pass/GeometryPass.h"

namespace core
{
    struct Material;
}

namespace core::gfx
{
    enum class BackfaceCulling : u8 { Back, Front, None };

    struct ViewportData
    {
        Color clearColor;

        math::mat4 projectionMatrix = math::mat4::identity;
        math::mat4 viewMatrix = math::mat4::identity;

        u16 pixelWidth, pixelHeight;

        math::mat4 getCombinedViewProjectionMatrix() const;
    };

    struct DrawCommand
    {
        assets::AssetRef<Mesh> mesh;
        Material* material;
        math::mat4 modelMatrix;
    };

    struct RenderPass
    {
        bool depth;
        BackfaceCulling backfaceCulling;
    };

    class Renderer
    {
        ViewportData _viewportData = ViewportData();

        std::vector<DrawCommand> _geometryCommandBuffer;
        std::vector<DrawCommand> _uiCommandBuffer;

        gl::Uint _sampler;
        gl::framebuffer_t _mainFramebuffer;

    public:
        Renderer();

        void copyViewportData(const ViewportData& params);
        void submitSceneGeometry(const DrawCommand& command);
        void submitUI();
        void renderFrame();

    private:
        void setPass(const RenderPass& pass);

        void renderSceneGeometry();
        void renderUI();
    };
}
