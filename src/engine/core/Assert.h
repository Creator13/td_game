#pragma once

#include <source_location>
#include <string_view>

#include "Logging.h"

namespace core::debug
{
    [[noreturn]] void handle_assert(const char* expression, const std::string_view& message, const std::source_location& location);
}

#ifdef DEBUG_BUILD
#define ENGINE_ASSERT(condition, ...) \
    do { \
        if (!(condition)) { \
            core::debug::handle_assert(#condition, fmt::format("" __VA_ARGS__), std::source_location::current()); \
        } \
    } while (false)
#else
#define ENGINE_ASSERT(condition, ...) ((void)0)
#endif

#if defined(_MSC_VER)
#define ENGINE_UNREACHABLE() __assume(false)
#else
#define ENGINE_UNREACHABLE() __builtin_unreachable()
#endif