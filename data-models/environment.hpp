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

}  // namespace data_models

namespace display_commands {
using data_models::air_quality_t;
using data_models::environment_data_t;
struct aqi_t {
    uint8_t value{};

    constexpr aqi_t(air_quality_t aq) : value{aq.aqi} {}
};

struct tvoc_t {
    uint16_t value{};

    constexpr tvoc_t(air_quality_t aq) : value{aq.tvoc} {}
};

struct celcius_t {
    float value{};

    constexpr celcius_t(environment_data_t e) : value{e.temperature} {}
};
struct humidity_t {
    float value{};

    constexpr humidity_t(environment_data_t e) : value{e.humidity} {}
};
struct eco2_t {
    uint16_t value{};

    constexpr eco2_t(air_quality_t aq) : value{aq.eco2} {}
};
}  // namespace display_commands