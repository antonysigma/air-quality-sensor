#pragma once

#include "callbacks.hpp"
#include "core.hpp"
#include "data-models/environment.hpp"

namespace components {

struct SerialPort {
    constexpr static auto setup_serial =
        flow::action("setup_serial"_sc, []() { Serial.begin(115'200); });

    constexpr static auto wait_for_serial = flow::action("wait_for_serial"_sc, []() {
        while (!Serial) {
        }
    });

    static void print(const data_models::AirQuality aq) {
        Serial.print(F("AQI: "));
        Serial.print(aq.aqi);
        Serial.print(F("\tTVOC: "));
        Serial.print(aq.tvoc);
        Serial.print(F("ppb\teCO2: "));
        Serial.print(aq.eco2);
        Serial.print(F("ppm\n"));
    }

    static void print(const data_models::EnvironmentData env) {
        constexpr auto print = [&](const float v) {
            Serial.print(static_cast<uint8_t>(v));
            Serial.write('.');
            Serial.print(static_cast<uint8_t>(v * 10) % 10);
        };
        Serial.print(F("Temperature: "));
        print(env.temperature);
        Serial.print(F("C\tRH: "));
        print(env.humidity);
        Serial.print(F("%\n"));
    }

    constexpr static auto config = cib::config(cib::extend<RuntimeInit>(
        core::system_clk_init >> setup_serial >> core::enable_interrupt >> wait_for_serial));
};
}  // namespace components