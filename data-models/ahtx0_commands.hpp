#pragma once
#include "environment.hpp"
#include "utils/big_endian.hpp"

namespace commands {

namespace ahtx0 {

#pragma pack(push, 1)
struct SoftResetCmd {
    uint8_t cmd{0xBA};
    uint8_t padding[2]{0, 0};
};
static_assert(sizeof(SoftResetCmd) == 3);

struct CalibrateCmd {
    uint8_t cmd{0xE1};
    uint8_t reg{0x08};
    uint8_t padding{0};
};
static_assert(sizeof(CalibrateCmd) == 3);

struct TriggerCmd {
    uint8_t cmd{0xAC};
    uint8_t reg{0x33};
    uint8_t padding{0};
};
static_assert(sizeof(TriggerCmd) == 3);

struct Status {
    uint8_t value{0xFF};

    [[nodiscard]] constexpr bool isBusy() const { return value & 0x80u; }
    [[nodiscard]] constexpr bool isCalibrated() const { return value & 0x08u; }
};

struct Measurements {
#if 0
    // C-Struct bit fields not compatible to little-endian systems.
    uint8_t : 8;
    uint32_t humidity:20;
    uint32_t temperature:20;
#endif

    uint8_t : 8;
    union {
        struct {
            uint8_t : 8;
            be::UInt32<20, 0> raw{};
        } temperature;
        be::UInt32<20, 12> humidity_raw{};
    } data{};

    [[nodiscard]] constexpr utils::Rational<> temperature() const {
        using R = utils::Rational<>;
        const auto decoded = static_cast<uint32_t>(data.temperature.raw);
        // const auto celcius = decoded * (200.0f / 100'000L) - 50;
        return R{decoded, 0x100'000UL} * 200 + (-50UL);
    }

    [[nodiscard]] constexpr utils::Rational<> humidity() const {
        using R = utils::Rational<>;
        const auto decoded = static_cast<uint32_t>(data.humidity_raw);
        // const auto percentage = decoded * (100.0f / 0x100'000);
        return R{decoded, 0x100'000UL} * 100;
    }

    [[nodiscard]] constexpr explicit operator data_models::EnvironmentData() const {
        return {.temperature = temperature(), .humidity = humidity()};
    }
};
#pragma pack(pop)

static_assert(sizeof(Measurements) == sizeof(uint8_t) * 6);
static_assert(sizeof(Measurements::data) == sizeof(uint8_t) * 5);
static_assert(__builtin_offsetof(Measurements, data.temperature.raw) == 2);
static_assert(__builtin_offsetof(Measurements, data.humidity_raw) == 1);

}  // namespace ahtx0

}  // namespace commands