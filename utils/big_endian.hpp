#pragma once
#include <cstdint>

namespace be {

template <uint8_t bit_depth = 32, uint8_t shifts = 0>
class UInt32 {
    uint32_t buffer;

   public:
    constexpr UInt32() : buffer{0} {}
    constexpr explicit UInt32(uint32_t v) : buffer{v} {}
    static_assert(bit_depth <= 32);
    static_assert(shifts <= 32 - bit_depth);

    constexpr explicit operator uint32_t() const {
        const auto decoded = ((buffer & 0x000000FFu) << 24) | ((buffer & 0x0000FF00u) << 8) |
                             ((buffer & 0x00FF0000u) >> 8) | ((buffer & 0xFF000000u) >> 24);

        return (decoded >> shifts) & ((1ULL << bit_depth) - 1);
    }
};
static_assert(static_cast<uint32_t>(UInt32(0xdead'beef)) == 0xefbe'adde,
              "Invalid big endian to little endian conversion logic");

static_assert(static_cast<uint32_t>(UInt32<20>(0xdead'beef)) == 0x000e'adde,
              "Invalid big endian to little endian conversion logic");

static_assert(static_cast<uint32_t>(UInt32<20, 12>(0xdead'beef)) == 0xefbe'a,
              "Invalid big endian to little endian conversion logic");

}  // namespace be