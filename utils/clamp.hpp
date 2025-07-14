#pragma once

namespace utils {

template <typename T>
constexpr T
clamp(const T x, const T vmin, const T vmax) {
    if (x < vmin) {
        return vmin;
    }

    if (x > vmax) {
        return vmax;
    }

    return x;
}

template <typename T>
constexpr T
Min(const T a, const T b) {  // NOLINT(readability-identifier-naming)
    return a < b ? a : b;
}

template <typename T>
constexpr T
Max(const T a, const T b) {  // NOLINT(readability-identifier-naming)
    return a > b ? a : b;
}
}  // namespace utils