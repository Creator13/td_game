#include "core/Debug.h"

#include <spdlog/spdlog.h>

#include "Constants.h"
#include "assets/AssetDatabase.h"

using namespace core;
using namespace math;
using namespace core::debug;
using namespace graphics;

namespace
{
    constexpr size_t MAX_VERTICES = 20'000;
    DebugRenderer* globalDebugRenderer;
}

DebugRenderer::DebugRenderer() : vertCount(0)
{
    glCreateVertexArrays(1, &vao);
    glCreateBuffers(1, &vbo);

    GLbitfield persistentFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

    glNamedBufferStorage(vbo, MAX_VERTICES * sizeof(DebugVertex), nullptr, persistentFlags);
    const auto rawBufPtr = glMapNamedBufferRange(vbo, 0, MAX_VERTICES * sizeof(Vertex), persistentFlags);
    mappedVertexBuffer = std::span{static_cast<DebugVertex*>(rawBufPtr), MAX_VERTICES};

    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(DebugVertex));

    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(DebugVertex, pos));
    glVertexArrayAttribBinding(vao, 0, 0);

    glEnableVertexArrayAttrib(vao, 1);
    glVertexArrayAttribFormat(vao, 1, 4, GL_FLOAT, GL_FALSE, offsetof(DebugVertex, color));
    glVertexArrayAttribBinding(vao, 1, 0);

    debugShader = assets::AssetDatabase::getShaderProgram(assets::idFromPath("@internal/shader/debug"));
}

DebugRenderer::~DebugRenderer()
{
    glUnmapNamedBuffer(vbo);
    mappedVertexBuffer = { };
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

void DebugRenderer::setMatrices(const mat4& view, const mat4& projection)
{
    this->viewMatrix = view;
    this->projectionMatrix = projection;
}

void DebugRenderer::submitLine(const DebugVertex& a, const DebugVertex& b)
{
    if (vertCount + 2 >= MAX_VERTICES)
    {
        spdlog::warn("Max debug elements reached");
        return;
    }

    mappedVertexBuffer[vertCount] = a;
    mappedVertexBuffer[vertCount + 1] = b;
    vertCount += 2;
}

void DebugRenderer::submitRect(const DebugVertex& a, const DebugVertex& b, const DebugVertex& c, const DebugVertex& d)
{
    if (vertCount + 8 >= MAX_VERTICES)
    {
        spdlog::warn("Max debug elements reached");
        return;
    }

    // edges
    mappedVertexBuffer[vertCount] = a;
    mappedVertexBuffer[vertCount + 1] = b;

    mappedVertexBuffer[vertCount + 2] = b;
    mappedVertexBuffer[vertCount + 3] = c;

    mappedVertexBuffer[vertCount + 4] = c;
    mappedVertexBuffer[vertCount + 5] = d;

    mappedVertexBuffer[vertCount + 6] = d;
    mappedVertexBuffer[vertCount + 7] = a;

    // diags
    mappedVertexBuffer[vertCount + 8] = a;
    mappedVertexBuffer[vertCount + 9] = c;

    mappedVertexBuffer[vertCount + 10] = b;
    mappedVertexBuffer[vertCount + 11] = d;

    vertCount += 12;
}

void DebugRenderer::render()
{
    if (vertCount == 0) return;

    glUseProgram(debugShader.programId);
    glBindVertexArray(vao);

    const mat4 viewProjection = projectionMatrix * constants::COORDINATE_BASIS * viewMatrix;
    const GLint vpLocation = glGetUniformLocation(debugShader.programId, "uViewProjection");
    glUniformMatrix4fv(vpLocation, 1, GL_FALSE, viewProjection.m);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertCount));

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    vertCount = 0;
}

void debug::bindDebugRenderer(DebugRenderer& renderer)
{
    globalDebugRenderer = &renderer;
}

void debug::drawLine(vec3 start, vec3 end, Color color)
{
    globalDebugRenderer->submitLine({start, color}, {end, color});
}

void debug::drawRay(vec3 origin, vec3 direction, Color color)
{
    globalDebugRenderer->submitLine({origin, color}, {origin + direction, color});
}

void debug::drawPlane(plane plane, Color color) { }
void debug::drawCameraFrustum(math::vec3 pos, ecs::PerspectiveCameraData& camera)
{

}
