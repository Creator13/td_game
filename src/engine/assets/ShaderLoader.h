#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "datatype.h"
#include "assets/AssetId.h"

namespace core::assets
{
    class ShaderLoader
    {
        std::unordered_map<AssetId, gl::shader_t> _shaderStageCache;
        gl::program_t _errorShaderId;

        static gl::program_t compileInternalErrorShader();
        [[nodiscard]] static std::string preprocessShader(std::string_view sourceString);

    public:
        ~ShaderLoader();

        static std::optional<gl::shader_t> compileFromSource(std::string_view source, gl::enum_t shaderType);
        static std::optional<gl::program_t> linkShaderProgram(std::initializer_list<gl::shader_t> shaderIds);

        std::optional<gl::shader_t> loadShaderStageFromFile(std::string_view path, gl::enum_t stageType, bool preprocess);
        gl::program_t glProgramFromFiles(std::string_view vertPath, std::string_view fragPath);

        gl::program_t getErrorShader();

        void cleanCache();
    };
}
