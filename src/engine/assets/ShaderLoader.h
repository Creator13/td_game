#pragma once

#include <optional>
#include <unordered_map>

#include "datatype.h"
#include "assets/AssetId.h"

namespace core::assets
{
    class AssetDatabase;

    class ShaderLoader
    {
        std::unordered_map<AssetId, gl::shader_t> _shaderStageCache;
        static gl::program_t compileInternalErrorShader();

        gl::program_t _errorShaderId;

    public:
        static std::optional<gl::shader_t> compileFromSource(std::string_view source, gl::enum_t shaderType);
        static std::optional<gl::program_t> linkShaderProgram(std::initializer_list<gl::shader_t> shaderIds);

        std::optional<gl::shader_t> loadShaderStageFromFile(std::string_view path, gl::enum_t stageType);
        gl::program_t glProgramFromFiles(std::string_view vertPath, std::string_view fragPath);

        gl::program_t getErrorShader();
    };
}
