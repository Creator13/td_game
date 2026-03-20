#include "assets/Shader.h"

#include <glad/gl.h>

#include "assets/AssetRef.h"
#include "assets/File.h"
#include "assets/ShaderLoader.h"
#include "util/PagedStorage.h"

using namespace core;
using namespace core::assets;

namespace
{
    ShaderLoader shaderLoader = ShaderLoader();
    util::PagedStorage<Shader, 128> shaderStorage = util::PagedStorage<Shader, 128>();
}

Shader::Shader(gl::program_t programId)
    : programId(programId),
      _layout(gfx::ShaderPipelineLayout::buildFromShader(programId)) { }

Shader::~Shader()
{
    glDeleteProgram(programId);
}

AssetRef<Shader> Shader::fromFiles(std::string_view vertPath, std::string_view fragPath)
{
    std::optional<gl::shader_t> vertId = shaderLoader.loadShaderStageFromFile(vertPath, GL_VERTEX_SHADER);
    std::optional<gl::shader_t> fragId = shaderLoader.loadShaderStageFromFile(fragPath, GL_FRAGMENT_SHADER);
    if (!vertId || !fragId)
    {
        return { };
    }

    std::optional<gl::program_t> sId = shaderLoader.linkShaderProgram({vertId.value(), fragId.value()});
    if (!sId) return { };

    Shader& ptr = shaderStorage.emplace(sId.value());

    const AssetId id = registerAsset(createCombinedShaderPath(vertPath, fragPath), AssetType::Shader, &ptr);
    return AssetRef(&ptr, id);
}

std::string Shader::createCombinedShaderPath(std::string_view vertPath, std::string_view fragPath)
{
    return fmt::format("{}|{}", vertPath, fragPath);
}
