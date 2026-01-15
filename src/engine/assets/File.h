#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace file
{
    namespace fs = std::filesystem;

    fs::path getExecutableDir();
    std::optional<std::string> readFileText(const fs::path& path);
    std::optional<std::vector<uint8_t>> readFileBinary(const fs::path& path);
}
