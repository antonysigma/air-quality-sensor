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