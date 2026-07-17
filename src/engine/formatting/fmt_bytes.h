#pragma once
#include <cstdint>
#include <fmt/format.h>

struct FormattableBytes {
    uint64_t bytes;
    explicit FormattableBytes(uint64_t b) : bytes(b) {}
};

template <>
struct fmt::formatter<FormattableBytes> {
    // Format spec: optional presentation character
    // 's' = SI (kB, MB, GB) — default
    // 'b' = binary (KiB, MiB, GiB)
    // 'r' = raw bytes, no conversion
    char presentation = 's';

    constexpr auto parse(fmt::format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 's' || *it == 'b' || *it == 'r'))
            presentation = *it++;
        if (it != end && *it != '}')
            throw fmt::format_error("invalid format spec for ByteSize");
        return it;
    }

    auto format(const FormattableBytes& bs, fmt::format_context& ctx) const {
        if (presentation == 'r')
            return fmt::format_to(ctx.out(), "{} B", bs.bytes);

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

        // Trim trailing zeros: use no decimal for whole numbers
        if (val == std::floor(val))
            return fmt::format_to(ctx.out(), "{:.0f} {}", val, suffixes[tier]);
        else
            return fmt::format_to(ctx.out(), "{:.2f} {}", val, suffixes[tier]);
    }
};