#pragma once

#include <fmt/format.h>

template <typename T>
concept IsVec = requires(T v) { v.x; v.y; };

template <IsVec T>
struct fmt::formatter<T> {
    // Parse format specifiers (e.g., {:.2f}) to pass through to float formatting
    fmt::formatter<float> float_formatter;

    constexpr auto parse(fmt::parse_context<>& ctx) {
        return float_formatter.parse(ctx);
    }

    template <typename FormatContext>
    auto format(const T& v, FormatContext& ctx) const {
        auto out = ctx.out();
        out = fmt::format_to(out, "(");

        // Format x and y
        out = float_formatter.format(v.x, ctx);
        out = fmt::format_to(out, ", ");
        out = float_formatter.format(v.y, ctx);

        // Format z if available
        if constexpr (requires { v.z; }) {
            out = fmt::format_to(out, ", ");
            out = float_formatter.format(v.z, ctx);
        }

        // Format w if available
        if constexpr (requires { v.w; }) {
            out = fmt::format_to(out, ", ");
            out = float_formatter.format(v.w, ctx);
        }

        return fmt::format_to(out, ")");
    }
};