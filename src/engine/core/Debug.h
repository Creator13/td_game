#pragma once

#include "assets/Shader.h"
#include "math/geom.h"
#include "math/mat4.h"
#include "math/vec3.h"
#include "rendering/Color.h"

namespace core::ecs {
    struct PerspectiveCameraData;
}

namespace core::debug
{
    struct DebugVertex
    {
        math::vec3 pos;
        graphics::Color color;
    };

    struct DebugRenderer
    {
        math::mat4 viewMatrix = math::mat4::identity;
        math::mat4 projectionMatrix = math::mat4::identity;

        // VAO and VBO lifetimes are managed by this class;
        gl::Uint vao;
        gl::Uint vbo;

        // Buffer map and shader are non-owning
        std::span<DebugVertex> mappedVertexBuffer;
        gl::program_t debugShader;

        usize vertCount;

        DebugRenderer();
        ~DebugRenderer();

        void setMatrices(const math::mat4& view, const math::mat4& projection);
        void submitLine(const DebugVertex& a, const DebugVertex& b);
        void submitRect(const DebugVertex& a, const DebugVertex& b, const DebugVertex& c, const DebugVertex& d);
        void render();
    };

    void bindDebugRenderer(DebugRenderer& renderer);

    void drawLine(math::vec3 start, math::vec3 end, graphics::Color color);
    void drawRay(math::vec3 origin, math::vec3 direction, graphics::Color color);
    void drawPlane(math::plane, graphics::Color color);
    void drawCameraFrustum(math::vec3 pos, ecs::PerspectiveCameraData& camera);
}

