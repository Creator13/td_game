#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "rendering/shader.h"

// TODO Might abstract this into a file namespace
namespace fs = std::filesystem;

namespace res
{
    std::optional<std::string> readFileText(const fs::path& path);

    using namespace render;
    shader::ShaderProgramData loadShader(const fs::path& vertPath, const fs::path& fragPath);
}
