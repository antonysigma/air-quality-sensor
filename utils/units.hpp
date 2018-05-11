#pragma once
#include <stdint.h>

namespace units {

template <typename T>
struct Microsecond {
    T value{};

    constexpr T operator/(const Microsecond<T>& rhs) const {
        static_assert(sizeof(T) >= 4);
        return value / rhs.value;
    }
};

struct KiloHertz {
    float value{};

    constexpr float operator/(const KiloHertz& rhs) const { return value / rhs.value; }

    template <typename T>
    constexpr float operator*(const Microsecond<T>& rhs) const {
        return value * rhs.value;
    }
};

namespace literals {
constexpr KiloHertz
operator""_kHz(long double x) {
    return {static_cast<float>(x)};
}

constexpr Microsecond<uint32_t>
operator""_ms(unsigned long long x) {
    return {static_cast<uint32_t>(x)};
}

}  // namespace literals
}  // namespace units