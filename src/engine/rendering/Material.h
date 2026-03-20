#pragma once

#include "assets/AssetRef.h"
#include "assets/Shader.h"
#include "assets/Texture.h"
#include "core/Color.h"
#include "math/vec3.h"
#include "math/vec4.h"

namespace math {
    struct mat4;
}

namespace core
{
    namespace gfx {
        class Pipeline;
        class Renderer;
    }

    struct Material
    {
        friend gfx::Renderer;
        friend gfx::Pipeline;

        const gfx::Pipeline& pipeline;

        explicit Material(const gfx::Pipeline& pipeline);
        ~Material();

        void setFloat(gfx::ShaderPropertyId id, float value);
        void setInt(gfx::ShaderPropertyId id, int value);

        void setVec2(gfx::ShaderPropertyId id, math::vec2 value);
        void setVec3(gfx::ShaderPropertyId id, math::vec3 value);
        void setVec4(gfx::ShaderPropertyId id, math::vec4 value);
        void setColor(gfx::ShaderPropertyId id, Color value);

        void setMat4(gfx::ShaderPropertyId id, const math::mat4& value);

        void setTexture2D(gfx::ShaderPropertyId id, assets::AssetRef<Texture> tex);

    private:
        // Note: AssetRef has stable pointer so we can just cache the reference to the layout here.
        // Beware if this ever changes.
        const gfx::ShaderPipelineLayout& _layout;

        std::vector<u8> _materialBlockDdata;
        gl::buffer_t _uboHandle;

        bool _dirty;
    };
}
