#pragma once
#include <cstdint>

#include "data-models/ens160_commands.hpp"

namespace data_models {

struct EnvironmentData {
    float temperature{};
    float humidity{};
};

using AirQuality = ens160_commands::AQIPredictions;

}  // namespace data_models

namespace display_commands {
using data_models::AirQuality;
using data_models::EnvironmentData;
struct Aqi {
    uint8_t value{};

    constexpr explicit Aqi(AirQuality aq) : value{aq.aqi} {}
};

struct TVoc {
    uint16_t value{};

    constexpr explicit TVoc(AirQuality aq) : value{aq.tvoc} {}
};

struct Celcius {
    float value{};

    constexpr explicit Celcius(EnvironmentData e) : value{e.temperature} {}
};
struct Humidity {
    float value{};

    constexpr explicit Humidity(EnvironmentData e) : value{e.humidity} {}
};
struct ECo2 {
    uint16_t value{};

    constexpr explicit ECo2(AirQuality aq) : value{aq.eco2} {}
};
}  // namespace display_commands