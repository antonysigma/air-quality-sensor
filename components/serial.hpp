#pragma once

#include "callbacks.hpp"
#include "core.hpp"
#include "data-models/environment.hpp"

namespace components {

struct SerialPort {
    constexpr static auto setup_serial =
        flow::action("setup_serial"_sc, []() { Serial.begin(115'200); });

    constexpr static auto wait_for_serial =
        flow::action("wait_for_serial"_sc, []() { while (!Serial); });

    static void print(const data_models::air_quality_t aq) {
        Serial.print(F("AQI: "));
        Serial.print(aq.aqi);
        Serial.print(F("\tTVOC: "));
        Serial.print(aq.tvoc);
        Serial.print(F("ppb\teCO2: "));
        Serial.print(aq.eco2);
        Serial.print(F("ppm\n"));
    }

    constexpr static auto config = cib::config(cib::extend<RuntimeInit>(
        core::system_clk_init >> setup_serial >> core::enable_interrupt >> wait_for_serial));
};
}  // namespace components