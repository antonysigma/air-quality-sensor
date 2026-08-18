#pragma once
#include <compare>
#include <cstdint>

namespace units {

template <typename T>
struct Microsecond {
    T value{};

    constexpr T operator/(const Microsecond<T>& rhs) const {
        static_assert(sizeof(T) >= 4);
        return value / rhs.value;
    }

    constexpr Microsecond<T> operator+(const Microsecond<T>& rhs) const {
        static_assert(sizeof(T) >= 4);
        return {value + rhs.value};
    }
};

template <typename T>
struct Millisecond {
    T value;

    constexpr Millisecond<T> operator+(const Millisecond<T>& rhs) const {
        return {value + rhs.value};
    }

    constexpr Millisecond<T> operator-(const Millisecond<T>& rhs) const {
        return {value - rhs.value};
    }

    // Automatically generates all 6 relational operators
    auto operator<=>(const Millisecond&) const = default;
};

template <typename Integer, typename T>
    requires std::convertible_to<Integer, T>
constexpr units::Millisecond<T>
operator*(const Integer& a, const units::Millisecond<T>& b) {
    return {a * b.value};
}

struct Hertz {
    float value{};

    template <typename T>
    constexpr float operator*(const Microsecond<T>& rhs) const {
        return value * rhs.value * 1e-6f;
    }
};

struct KiloHertz {
    float value{};

    constexpr float operator/(const KiloHertz& rhs) const { return value / rhs.value; }

    constexpr float operator/(const Hertz& rhs) const { return value * 1e3f / rhs.value; }

    template <typename T>
    constexpr float operator*(const Microsecond<T>& rhs) const {
        return value * rhs.value * 1e-3f;
    }
};

namespace literals {
constexpr KiloHertz
operator""_kHz(long double x) {
    return {static_cast<float>(x)};
}

constexpr Hertz
operator""_Hz(long double x) {
    return {static_cast<float>(x)};
}

constexpr Millisecond<uint16_t>
operator""_ms(unsigned long long x) {
    return {static_cast<uint16_t>(x)};
}

constexpr Microsecond<uint32_t>
operator""_us(unsigned long long x) {
    return {static_cast<uint32_t>(x)};
}

}  // namespace literals
}  // namespace units
