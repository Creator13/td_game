#pragma once
#include <cstdint>
#include <fmt/format.h>

struct FormattableBytes {
    uint64_t bytes;
    explicit FormattableBytes(uint64_t b) : bytes(b) {}
};


template <>
struct fmt::formatter<FormattableBytes> {
    // Format spec: optional ".N" precision, then optional presentation character
    // 's' = SI (kB, MB, GB) — default
    // 'b' = binary (KiB, MiB, GiB)
    // 'r' = raw bytes, no conversion
    //
    // Examples: "{}"      -> auto-trim decimals (old default behavior)
    //           "{:.2}"   -> SI, always 2 decimals
    //           "{:.0b}"  -> binary, always 0 decimals (whole numbers)
    //           "{:.3r}"  -> raw bytes, 3 decimals (no-op since raw has no fraction)
    char presentation = 's';
    int precision = -1;  // -1 means "auto-trim trailing zeros" (legacy behavior)

    constexpr auto parse(fmt::format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();

        if (it != end && *it == '.') {
            ++it;
            int value = 0;
            bool has_digits = false;
            while (it != end && *it >= '0' && *it <= '9') {
                has_digits = true;
                value = value * 10 + (*it - '0');
                ++it;
            }
            if (!has_digits)
                throw fmt::format_error("missing precision digits for ByteSize");
            precision = value;
        }

        if (it != end && (*it == 's' || *it == 'b' || *it == 'r'))
            presentation = *it++;
        if (it != end && *it != '}')
            throw fmt::format_error("invalid format spec for ByteSize");
        return it;
    }

    auto format(const FormattableBytes& bs, fmt::format_context& ctx) const {
        if (presentation == 'r')
            return fmt::format_to(ctx.out(), "{}B", bs.bytes);

        const bool binary = (presentation == 'b');
        const double base  = binary ? 1024.0 : 1000.0;

        static constexpr const char* si_suffixes[]  = {"B","kB","MB","GB","TB","PB"};
        static constexpr const char* bin_suffixes[]  = {"B","KiB","MiB","GiB","TiB","PiB"};
        const auto& suffixes = binary ? bin_suffixes : si_suffixes;

        int tier = 0;
        double val = static_cast<double>(bs.bytes);

        while (val >= base && tier < 5) {
            val /= base;
            ++tier;
        }

        if (precision >= 0) {
            // Fixed precision requested explicitly, e.g. "{:.2}"
            return fmt::format_to(ctx.out(), "{:.{}f}{}", val, precision, suffixes[tier]);
        }

        // Default: trim trailing zeros — no decimal for whole numbers
        if (val == std::floor(val))
            return fmt::format_to(ctx.out(), "{:.0f}{}", val, suffixes[tier]);
        else
            return fmt::format_to(ctx.out(), "{:.2f}{}", val, suffixes[tier]);
    }
};
