#pragma once
#include <cstdint>

namespace data_models {

struct environment_data_t {
    float temperature{};
    float humidity{};
};

struct air_quality_t {
    uint8_t aqi{};
    uint16_t tvoc{};
    uint16_t eco2{};
};

struct aqi_t {
    uint8_t value{};

    constexpr aqi_t(air_quality_t aq) : value{aq.aqi} {}
};
}  // namespace data_models