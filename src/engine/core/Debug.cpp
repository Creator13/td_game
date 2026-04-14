#include "core/Debug.h"

#include <glad/gl.h>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#include "Constants.h"
#include "assets/AssetDatabase.h"
#include "assets/ShaderLoader.h"
#include "../assets/Mesh.h"

using namespace core;
using namespace math;
using namespace core::debug;

namespace
{
    constexpr size_t MAX_VERTICES = 20'000;
    DebugRenderer* globalDebugRenderer;

    gl::program_t compileDebugShader()
    {
        using namespace core::assets;

        const char* vertexShaderSource = R"(
                #version 430 core

                layout(location = 0) in vec3 aPosition;
                layout(location = 1) in vec4 aColor;

                uniform mat4 uViewProjection;

                out vec4 vColor;

                void main()
                {
                    gl_Position = uViewProjection * vec4(aPosition, 1.0);
                    vColor = aColor;
                }
            )";

        const char* fragmentShaderSource = R"(
                #version 430 core

                in vec4 vColor;
                out vec4 FragColor;

                void main()
                {
                    FragColor = vColor;
                }
            )";

        gl::shader_t vId = ShaderLoader::compileFromSource(vertexShaderSource, GL_VERTEX_SHADER).value();
        gl::shader_t fId = ShaderLoader::compileFromSource(fragmentShaderSource, GL_FRAGMENT_SHADER).value();
        const gl::program_t pId = ShaderLoader::linkShaderProgram({vId, fId}).value();

        glDeleteShader(vId);
        glDeleteShader(fId);

        return pId;
    }
}

DebugRenderer::DebugRenderer()
    : _viewportData(), _vertCount(0)
{
    glCreateVertexArrays(1, &_vao);
    glCreateBuffers(1, &_vbo);

    GLbitfield persistentFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

    glNamedBufferStorage(_vbo, MAX_VERTICES * sizeof(DebugVertex), nullptr, persistentFlags);
    const auto rawBufPtr = glMapNamedBufferRange(_vbo, 0, MAX_VERTICES * sizeof(DebugVertex), persistentFlags);
    _mappedVertexBuffer = std::span{static_cast<DebugVertex*>(rawBufPtr), MAX_VERTICES};

    glVertexArrayVertexBuffer(_vao, 0, _vbo, 0, sizeof(DebugVertex));

    glEnableVertexArrayAttrib(_vao, 0);
    glVertexArrayAttribFormat(_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(DebugVertex, pos));
    glVertexArrayAttribBinding(_vao, 0, 0);

    glEnableVertexArrayAttrib(_vao, 1);
    glVertexArrayAttribFormat(_vao, 1, 4, GL_FLOAT, GL_FALSE, offsetof(DebugVertex, color));
    glVertexArrayAttribBinding(_vao, 1, 0);

    _debugShader = compileDebugShader();
}

DebugRenderer::~DebugRenderer()
{
    glUnmapNamedBuffer(_vbo);
    _mappedVertexBuffer = { };
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
}

void DebugRenderer::copyViewportData(const gfx::ViewportData& viewportData)
{
    this->_viewportData = viewportData;
}

void DebugRenderer::submitLine(const DebugVertex& a, const DebugVertex& b)
{
    if (_vertCount + 2 >= MAX_VERTICES)
    {
        spdlog::warn("Max debug elements reached");
        return;
    }

    _mappedVertexBuffer[_vertCount] = a;
    _mappedVertexBuffer[_vertCount + 1] = b;
    _vertCount += 2;
}

void DebugRenderer::submitRect(const DebugVertex& a, const DebugVertex& b, const DebugVertex& c, const DebugVertex& d)
{
    if (_vertCount + 12 >= MAX_VERTICES)
    {
        spdlog::warn("Max debug elements reached");
        return;
    }

    // edges
    _mappedVertexBuffer[_vertCount] = a;
    _mappedVertexBuffer[_vertCount + 1] = b;

    _mappedVertexBuffer[_vertCount + 2] = b;
    _mappedVertexBuffer[_vertCount + 3] = c;

    _mappedVertexBuffer[_vertCount + 4] = c;
    _mappedVertexBuffer[_vertCount + 5] = d;

    _mappedVertexBuffer[_vertCount + 6] = d;
    _mappedVertexBuffer[_vertCount + 7] = a;

    // diags
    _mappedVertexBuffer[_vertCount + 8] = a;
    _mappedVertexBuffer[_vertCount + 9] = c;

    _mappedVertexBuffer[_vertCount + 10] = b;
    _mappedVertexBuffer[_vertCount + 11] = d;

    _vertCount += 12;
}

void DebugRenderer::render()
{
    ZoneScopedN("DebugRenderer::render()");
    TracyGpuZone("DebugRenderer::render()");

    if (_vertCount == 0) return;

    glUseProgram(_debugShader);
    glBindVertexArray(_vao);

    const mat4 viewProjection = _viewportData.getCombinedViewProjectionMatrix();
    const gl::Int vpLocation = glGetUniformLocation(_debugShader, "uViewProjection");
    glUniformMatrix4fv(vpLocation, 1, GL_FALSE, viewProjection.m);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glDrawArrays(GL_LINES, 0, static_cast<gl::Sizei>(_vertCount));

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    _vertCount = 0;
}

void DebugRenderer::submitScreenSpaceLine(vec2 a, vec2 b, Color color)
{
    submitLine(
        {_viewportData.screenToWorld(a, 0.001f), color},
        {_viewportData.screenToWorld(b, 0.001f), color}
    );
}

void DebugRenderer::submitScreenSpaceRect(rect rect, Color color)
{
    const float x0 = rect.offset.x, y0 = rect.offset.y;
    const float x1 = rect.offset.x + rect.extents.x, y1 = rect.offset.y + rect.extents.y;

    // place the rect just in front of the near plane
    constexpr float depth = 0.001f;

    // TODO fix screen space debug drawing flicker: the _viewportData here is *last* frame's viewport data and not *this* frame's. Either delay
    //  drawing, or change frame order such that the viewport data is stable from the beginning of the frame until the end of it.
    const vec3 tl = _viewportData.screenToWorld({x0, y0}, depth);
    const vec3 tr = _viewportData.screenToWorld({x1, y0}, depth);
    const vec3 br = _viewportData.screenToWorld({x1, y1}, depth);
    const vec3 bl = _viewportData.screenToWorld({x0, y1}, depth);

    submitRect({tl, color}, {tr, color}, {br, color}, {bl, color});
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
void debug::drawCameraFrustum(vec3 pos, ecs::PerspectiveCameraData& camera) { }

void debug::draw2DRect(rect rect, Color color)
{
    globalDebugRenderer->submitScreenSpaceRect(rect, color);
}

void debug::draw2DLine(vec2 start, vec2 end, Color color)
{
    globalDebugRenderer->submitScreenSpaceLine(start, end, color);
}
