#pragma once
#include <cstdint>

namespace ens160_commands {

#pragma pack(push, 1)
struct SetMode {
    const uint8_t cmd{0x10};
    enum mode_t : uint8_t { IDLE = 0x01, STANDARD = 0x02, LOW_POWER = 0x03 } mode;

    constexpr explicit SetMode(uint8_t m) : mode{m} {}
};

struct SetEnvData {
    const uint8_t cmd{0x13};
    uint16_t temperature{};
    uint16_t humidity{};

    constexpr explicit SetEnvData(float t, float h)
        : temperature{static_cast<uint16_t>((t + 273.15f) + 64.0f)},
          humidity{static_cast<uint16_t>(h * 512.0f)} {}
};

struct GetStatus {
    const uint8_t cmd{0x20};
};

struct Status {
    uint8_t buffer{};

    [[nodiscard]] constexpr bool isNewData() const { return 0x02 & buffer; }
};

struct ReadAQIPrediction {
    const uint8_t cmd{0x21};
};

struct AQIPredictions {
    uint8_t aqi{};
    uint16_t tvoc{};
    uint16_t eco2{};
    uint16_t aci500{};
};
#pragma pack(pop)
}  // namespace ens160_commands