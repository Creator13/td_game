#include "core/Assert.h"

#include <stacktrace>
#include <string>

void core::debug::handle_assert(const char* expression, const std::string& message, const std::source_location& location)
{
    spdlog::critical("Assertion Failed: {} | Message: {}", expression, message);
    spdlog::critical("File: {} ({}:{})", location.file_name(), location.line(), location.column());
    spdlog::critical("Function: {}", location.function_name());

    spdlog::critical("Stacktrace:\n{}", std::to_string(std::stacktrace::current()));

#if defined(_MSC_VER)
    __debugbreak();
#elif defined(__GNUC__) || defined(__clang__)
    __builtin_trap();
#endif

    std::abort();
}
