#pragma once

#if defined(__clang__)
    // Clang and clang-cl both support constexpr cmath
    #define MATH_CONSTEXPR_CMATH constexpr
#elif defined(__GNUC__) && (__GNUC__ >= 14)
    // adjust if needed
    #define MATH_CONSTEXPR_CMATH constexpr
#else
    // MSVC and older GCC
    #define MATH_CONSTEXPR_CMATH
#endif