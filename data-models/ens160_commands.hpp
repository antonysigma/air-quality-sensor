#pragma once
#include "environment.hpp"

namespace ens160_commands {

#pragma pack(push, 1)
struct SetMode {
    uint8_t cmd{0x10};
    enum mode_t : uint8_t { IDLE = 0x01, STANDARD = 0x02, LOW_POWER = 0x03 } mode;

    constexpr explicit SetMode(uint8_t m) : mode{m} {}
};

using R = utils::Rational<>;

struct SetEnvData {
    uint8_t cmd{0x13};
    uint16_t temperature{};
    uint16_t humidity{};

    constexpr explicit SetEnvData(data_models::EnvironmentData e)
        : temperature{static_cast<uint16_t>((e.temperature + R{27315, 100}) + 64UL)},
          humidity{static_cast<uint16_t>(e.humidity * 512)} {}
};

struct GetStatus {
    uint8_t cmd{0x20};
};

struct Status {
    uint8_t buffer{};

    [[nodiscard]] constexpr bool isNewData() const { return 0x02 & buffer; }
};

struct ReadAQIPrediction {
    uint8_t cmd{0x21};
};

struct AQIPredictions {
    uint8_t aqi{};
    uint16_t tvoc{};
    uint16_t eco2{};
    uint16_t aci500{};
};
#pragma pack(pop)
}  // namespace ens160_commands