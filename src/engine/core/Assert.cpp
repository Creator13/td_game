#include "core/Assert.h"

#include <string>
#if defined(_WIN32)
#include <stacktrace>
#elif defined(__linux__)
        // Linux fallback using backtrace
#include <execinfo.h>
#include <sstream>
#endif

    inline std::string get_stacktrace() {
#if defined(_WIN32)
        return std::to_string(std::stacktrace::current());
#elif defined(__linux__)
        constexpr int max_frames = 32;
        void* frames[max_frames];
        int n = backtrace(frames, max_frames);
        char** symbols = backtrace_symbols(frames, n);

        std::ostringstream oss;
        if (symbols) {
            for (int i = 0; i < n; ++i) {
                oss << symbols[i] << "\n";
            }
            free(symbols);
        } else {
            oss << "Stacktrace unavailable\n";
        }
        return oss.str();
#else
        return "Stacktrace unsupported on this platform\n";
#endif
}

void core::debug::handle_assert(const char* expression, const std::string& message, const std::source_location& location)
{
    spdlog::critical("Assertion Failed: {} | Message: {}", expression, message);
    spdlog::critical("File: {} ({}:{})", location.file_name(), location.line(), location.column());
    spdlog::critical("Function: {}", location.function_name());

    spdlog::critical("Stacktrace:\n{}", get_stacktrace());

#if defined(_MSC_VER)
    __debugbreak();
#elif defined(__GNUC__) || defined(__clang__)
    __builtin_trap();
#endif

    std::abort();
}
