#pragma once

#include "assets/AssetRef.h"
#include "assets/AssetTraits.h"
#include "assets/Texture.h"
#include "core/Color.h"
#include "math/vec3.h"
#include "math/vec4.h"
#include "rendering/PipelineLayout.h"
#include "rendering/ShaderPropertyId.h"

namespace math
{
    struct mat4;
}

namespace core
{
    namespace gfx
    {
        struct ShaderLayout;
        class Pipeline;
        class Renderer;
    }

    class Material;

    template<>
    struct assets::AssetTraits<Material>
    {
        static constexpr AssetType type = AssetType::Material;
    };

    class Material
    {
        friend gfx::Renderer;
        friend gfx::Pipeline;

        // Note: AssetRef has stable pointer so we can just cache the references to the layout & pipeline here.
        // Beware if this ever changes.
        const gfx::Pipeline& _pipeline;
        const gfx::ShaderLayout& _layout;

        std::unordered_map<gfx::ShaderPropertyId, assets::AssetRef<Texture>> _textures;
        std::vector<u8> _materialBlockData;
        gl::buffer_t _uboHandle;

        mutable bool _dirty;

        // Private constructor from pipeline object takes a Pipeline& and not an AssetRef because it should only be called by the owning pipeline.
        explicit Material(const gfx::Pipeline& pipeline);
        void constructBuffers(); // constructor helper

        template<typename T>
        void setUniform(gfx::ShaderPropertyId id, const T& value, gl::enum_t expectedGlType);

        void flushUboChangesToGpu();

    public:
        ~Material();

        void setFloat(gfx::ShaderPropertyId id, float value);
        void setInt(gfx::ShaderPropertyId id, int value);

        void setVec2(gfx::ShaderPropertyId id, math::vec2 value);
        void setVec3(gfx::ShaderPropertyId id, math::vec3 value);
        void setVec4(gfx::ShaderPropertyId id, math::vec4 value);
        void setColor(gfx::ShaderPropertyId id, Color value);

        void setMat4(gfx::ShaderPropertyId id, const math::mat4& value);

        void setTexture2D(gfx::ShaderPropertyId id, assets::AssetRef<Texture> tex);
    };

#include "Material.inl"
}
