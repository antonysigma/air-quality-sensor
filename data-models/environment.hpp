#pragma once
#include <cstdint>

#include "utils/rational.hpp"

namespace data_models {

struct EnvironmentData {
    utils::Rational<> temperature{};
    utils::Rational<> humidity{};
};

// using AirQuality = ens160_commands::AQIPredictions;

}  // namespace data_models