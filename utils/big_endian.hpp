#pragma once
#include <cstdint>

namespace be {

class UInt32 {
    uint32_t buffer;

   public:
    constexpr UInt32() : buffer{0} {}
    constexpr explicit UInt32(uint32_t v) : buffer{v} {}

    constexpr explicit operator uint32_t() const {
        return ((buffer & 0x000000FFu) << 24) | ((buffer & 0x0000FF00u) << 8) |
               ((buffer & 0x00FF0000u) >> 8) | ((buffer & 0xFF000000u) >> 24);
    }
};
static_assert(static_cast<uint32_t>(UInt32(0xdeadbeef)) == 0xefbeadde,
              "Invalid big endian to little endian conversion logic");

}  // namespace be