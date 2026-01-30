#include "assets/File.h"

#include <fstream>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace file
{
    fs::path getExecutableDir()
    {
#ifdef _WIN32
        wchar_t buffer[1024];
        GetModuleFileNameW(nullptr, buffer, 1024);
        return fs::path(buffer).parent_path();
#else // Linux
        char buffer[1024];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1)
        {
            buffer[len] = '\0';
            return fs::path(buffer).parent_path();
        }
        else
        {
            throw std::runtime_error("Failed to get executable path");
        }
#endif
    }

    std::optional<std::string> readFileText(const fs::path& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) return std::nullopt;

        const auto size = file.tellg();
        std::string text = std::string(size, '\0');
        file.seekg(0);
        file.read(text.data(), size);
        return text;
    }

    std::optional<std::vector<uint8_t>> readFileBinary(const fs::path& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) return std::nullopt;

        const auto size = file.tellg();
        if (size < 0) return std::nullopt;

        std::vector<uint8_t> buffer(size);
        file.seekg(0);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
        {
            return std::nullopt;
        }
        return buffer;
    }
}
