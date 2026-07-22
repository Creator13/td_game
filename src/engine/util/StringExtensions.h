#pragma once

#include <string>
#include <string_view>

namespace util::string
{
    inline std::string replace(std::string_view src, std::string_view from, std::string_view to)
    {
        std::string result{src};
        std::size_t pos = 0;
        while ((pos = result.find(from, pos)) != std::string::npos)
        {
            result.replace(pos, from.size(), to);
            pos += to.size();
        }
        return result;
    }

    using ReplacePair = std::pair<std::string_view, std::string_view>;
    inline std::string multiReplace(std::string_view src, std::initializer_list<ReplacePair> replacements)
    {
        std::string result{src};
        for (auto [from, to] : replacements)
        {
            std::size_t pos = 0;
            while ((pos = result.find(from, pos)) != std::string::npos)
            {
                result.replace(pos, from.size(), to);
                pos += to.size();
            }
        }
        return result;
    }

    constexpr std::string_view trimWhitespace(std::string_view s)
    {
        const auto start = s.find_first_not_of(" \t\r\n");
        if (start == std::string_view::npos) return "";
        const auto end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    inline bool containsCaseInsensitive(std::string_view in, std::string_view toFind)
    {
        const auto it = std::ranges::search(in, toFind,
            [](uint8_t a, uint8_t b)
            {
                return std::tolower(a) == std::tolower(b);
            }).begin();
        return it != in.end();
    }
}
