#pragma once

#include <source_location>
#include <string>

namespace core::debug
{
    void handle_assert(const char* expression, const std::string& message, const std::source_location& location);
}

#ifdef DEBUG_BUILD
#define ENGINE_ASSERT(condition, ...) \
    do { \
        if (!(condition)) { \
            core::debug::handle_assert(#condition, fmt::format("" __VA_ARGS__), std::source_location::current()); \
        }\
    } while (false)
#else
#define ENGINE_ASSERT(condition, ...) ((void)0)
#endif
