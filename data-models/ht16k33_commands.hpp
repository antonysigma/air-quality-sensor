#pragma once
#include <cstdint>

#include "utils/clamp.hpp"

namespace ht16k33_commands {
#pragma pack(push, 1)
struct Brightness {
    static constexpr uint8_t brightness_cmd{0xE0};
    uint8_t value;

    constexpr Brightness(uint8_t b) : value{brightness_cmd | utils::Min(b, uint8_t(15))} {}
};

struct BlinkRate {
    static constexpr uint8_t blink_cmd{0x80};
    static constexpr uint8_t display_on{0x01};

    uint8_t buffer;
    constexpr BlinkRate(uint8_t rate)
        : buffer{blink_cmd | display_on | (utils::Min(rate, uint8_t(2)) << 1)} {}
};

struct NoBlink : BlinkRate {
    constexpr NoBlink() : BlinkRate{0} {}
};

struct OscillatorOn {
    const uint8_t buffer{0x21};
};

struct AnodeCathode {
    uint8_t anode;
    uint8_t cathode;

    constexpr AnodeCathode(uint8_t r) : anode{r & uint8_t(0xff)}, cathode{r >> 8} {}
    constexpr AnodeCathode() : anode{}, cathode{} {}
};
static_assert(sizeof(AnodeCathode) == 2);

struct WriteDisplay {
    uint8_t starting_address{0x00};
    AnodeCathode buffer[5];
    AnodeCathode padding[3]{};

    constexpr WriteDisplay(const std::array<uint8_t, 5>& raw) {
        std::copy(raw.begin(), raw.end(), buffer);
    }
};
static_assert(sizeof(WriteDisplay) == 17);
#pragma pack(pop)
}  // namespace ht16k33_commands