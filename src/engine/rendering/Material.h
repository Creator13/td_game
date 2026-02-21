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

        void setFloat(ShaderProperty id, float value);
        void setInt(ShaderProperty id, int value);

        void setVec2(ShaderProperty id, math::vec2 value);
        void setVec3(ShaderProperty id, math::vec3 value);
        void setVec4(ShaderProperty id, math::vec4 value);
        void setMat4(ShaderProperty id, const math::mat4& value);
        void setColor(ShaderProperty id, Color value);

        void setTexture2D(ShaderProperty id, assets::AssetRef<Texture> tex);

    private:
        // Note: AssetRef has stable pointer so we can just cache the reference to the layout here.
        // Beware if this ever changes.
        const ShaderPipelineLayout& _layout;

        std::vector<u8> _materialBlockDdata;
        gl::buffer_t _ubo;

        bool _dirty;
    };
}
