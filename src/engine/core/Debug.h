#pragma once

#include "datatype.h"
#include "core/Color.h"
#include "math/geom.h"
#include "math/mat4.h"
#include "math/vec3.h"
#include "rendering/RenderingDataStructures.h"

namespace core::ecs {
    struct PerspectiveCameraData;
}

namespace core::debug
{
    struct DebugVertex
    {
        math::vec3 pos;
        Color color;
    };

    class DebugRenderer
    {
        gfx::ViewportData _viewportData;

        // VAO and VBO lifetimes are managed by this class;
        gl::Uint _vao;
        gl::Uint _vbo;

        // Buffer map and shader are non-owning
        std::span<DebugVertex> _mappedVertexBuffer;
        gl::program_t _debugShader;

        usize _vertCount;

    public:
        DebugRenderer();
        ~DebugRenderer();

        void copyViewportData(const gfx::ViewportData& viewportData);

        void submitLine(const DebugVertex& a, const DebugVertex& b);
        void submitRect(const DebugVertex& a, const DebugVertex& b, const DebugVertex& c, const DebugVertex& d);

        void submitScreenSpaceLine(math::vec2 a, math::vec2 b, Color color);
        void submitScreenSpaceRect(math::rect rect, Color color);

        void render();
    };

    void bindDebugRenderer(DebugRenderer& renderer);

    void drawLine(math::vec3 start, math::vec3 end, Color color);
    void drawRay(math::vec3 origin, math::vec3 direction, Color color);
    void drawPlane(math::plane, Color color);
    void drawCameraFrustum(math::vec3 pos, ecs::PerspectiveCameraData& camera);
    void draw2DRect(math::rect rect, Color color);
    void draw2DLine(math::vec2 start, math::vec2 end, Color color);
}

