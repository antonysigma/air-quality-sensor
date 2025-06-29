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
Min(const T a, const T b) {
    return a < b ? a : b;
}
}  // namespace utils