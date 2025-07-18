#pragma once

#include "ens160_commands.hpp"
#include "environment.hpp"

namespace display_commands {
using AirQuality = ens160_commands::AQIPredictions;
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
    utils::Rational<> value{};

    constexpr explicit Celcius(EnvironmentData e) : value{e.temperature} {}
};
struct Humidity {
    utils::Rational<> value{};

    constexpr explicit Humidity(EnvironmentData e) : value{e.humidity} {}
};
struct ECo2 {
    uint16_t value{};

    constexpr explicit ECo2(AirQuality aq) : value{aq.eco2} {}
};
}  // namespace display_commands