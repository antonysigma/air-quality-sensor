#pragma once
#include <cstdint>
#include <type_traits>

namespace utils {

template <typename T>
concept Integral = std::is_integral_v<T> && (sizeof(T) <= 32);

template <typename T = uint32_t>
struct Rational {
    T num{0};
    T den{1};

    constexpr Rational operator*(const Rational rhs) const {
        return {num * rhs.num, den * rhs.den};
    }

    constexpr Rational operator*(const T rhs) const { return {num * rhs, den}; }

    constexpr Rational operator/(const uint32_t rhs) const { return {num, den * rhs}; }

    constexpr Rational operator+(const Rational rhs) const {
        return {num * rhs.den + rhs.num * den, den * rhs.den};
    }

    constexpr Rational operator+(const T rhs) const { return {num + rhs * den, den}; }

    constexpr Rational operator-(const Rational rhs) const { return operator+(-rhs); }

    constexpr Rational operator-() const { return {-num, den}; }

    constexpr bool operator<(const T rhs) const { return num < rhs * den; }

    template <Integral U>
    constexpr explicit operator U() const {
        return num / den;
    }
};
}  // namespace utils