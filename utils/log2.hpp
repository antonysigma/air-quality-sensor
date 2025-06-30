#pragma once
#include <cstdint>

namespace utils {

/** Find the position of the most significant bit. */
constexpr uint32_t
log2(uint32_t n) {
    uint32_t result = 0;
    while (n >>= 1) {
        ++result;
    }
    return result;
}
static_assert(log2(1) == 0);
static_assert(log2(0b10) == 1);
static_assert(log2(0b1000'000) == 6);
}  // namespace utils