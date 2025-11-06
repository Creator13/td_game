#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "rendering/shader.h"

// TODO Might abstract this into a file namespace
namespace fs = std::filesystem;

namespace res
{
    void initResources(std::string_view resourceRoot);
    void unloadResources();

    fs::path getPath(const fs::path& path);
    std::optional<std::string> readFileText(const fs::path& path);

    graphics::shader::ShaderProgramData loadShader(const fs::path& vertPath, const fs::path& fragPath);
}
