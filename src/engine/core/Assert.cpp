#include "core/Assert.h"

#include <cpptrace/basic.hpp>

void core::debug::handle_assert(const char* expression, const std::string_view& message, const std::source_location& location)
{
    spdlog::critical("Assertion Failed: {} | Message: {}", expression, message);

    cpptrace::stacktrace trace = cpptrace::generate_trace(1);
    trace.print();

#if defined(_MSC_VER)
    __debugbreak();
    std::abort();
#elif defined(__GNUC__) || defined(__clang__)
    __builtin_trap();
#endif
}
