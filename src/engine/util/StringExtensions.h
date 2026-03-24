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
}
