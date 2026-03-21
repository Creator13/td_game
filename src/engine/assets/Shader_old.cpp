#include "assets/Shader_old.h"

#include "assets/ShaderLoader.h"
#include "util/PagedStorage.h"

using namespace core;
using namespace core::assets;

namespace
{
    ShaderLoader shaderLoader = ShaderLoader();
}

Shader::Shader(gl::program_t programId)
    : programId(programId) { }

Shader::~Shader() { }

Shader Shader::fromFiles(std::string_view vertPath, std::string_view fragPath)
{
    return Shader(0);
}

std::string Shader::createCombinedShaderPath(std::string_view vertPath, std::string_view fragPath)
{
    return fmt::format("{}|{}", vertPath, fragPath);
}
