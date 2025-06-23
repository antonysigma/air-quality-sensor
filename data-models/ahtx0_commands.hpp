#pragma once
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

    constexpr bool isBusy() const { return value & 0x80; }
    constexpr bool isCalibrated() const { return value & 0x08; }
};

struct Measurements {
    uint8_t padding{};
    union {
        uint8_t buffer[5];
        struct {
            uint8_t padding{};
            be::UInt32 raw{};
        } temperature;
        struct {
            be::UInt32 raw{};
            uint8_t padding{};
        } humidity;
    } data{};

    constexpr float temperature() const {
        const auto masked = static_cast<uint32_t>(data.temperature.raw) & 0x0F'FFFF;
        return static_cast<float>(masked) * (200.0f / 0x100'000) - 50;
    }

    constexpr float humidity() const {
        const auto masked = static_cast<uint32_t>(data.humidity.raw) >> 12;
        return static_cast<float>(masked) * (100.0f / 0x100'000);
    }
};
#pragma pack(pop)

static_assert(sizeof(Measurements) == sizeof(uint8_t) * 6);
static_assert(sizeof(Measurements::data) == sizeof(uint8_t) * 5);
static_assert(offsetof(Measurements, data.temperature.raw) == 2);
static_assert(offsetof(Measurements, data.humidity.raw) == 1);

}  // namespace ahtx0

}  // namespace commands