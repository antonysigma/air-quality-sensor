#pragma once

#include "callbacks.hpp"
#include "core.hpp"
#include "data-models/environment.hpp"
#include "utils/pgm_string.hpp"

namespace components {

struct SerialPort {
    constexpr static auto setup_serial =
        flow::action("setup_serial"_sc, []() { Serial.begin(115'200); });

    constexpr static auto wait_for_serial = flow::action("wait_for_serial"_sc, []() {
        while (!Serial) {
        }
    });

    static constexpr void print(const utils::PGMStringHelper message) {
        for (const char* p = message.begin(); p != message.end(); ++p) {
            Serial.write(pgm_read_byte(p));
        }
    }

    static void print(const data_models::AirQuality aq) {
        print(PSTR2("AQI: "));
        Serial.print(aq.aqi);
        print(PSTR2("\tTVOC: "));
        Serial.print(aq.tvoc);
        print(PSTR2("ppb\teCO2: "));
        Serial.print(aq.eco2);
        print(PSTR2("ppm\n"));
    }

    static void print(const data_models::EnvironmentData env) {
        constexpr auto printFloat = [&](const float v) {  // NOLINT(readability-identifier-naming)
            Serial.print(static_cast<uint8_t>(v));
            Serial.write('.');
            Serial.print(static_cast<uint8_t>(v * 10) % 10);
        };
        print(PSTR2("Temperature: "));
        printFloat(env.temperature);
        print(PSTR2("C\tRH: "));
        printFloat(env.humidity);
        print(PSTR2("%\n"));
    }

    constexpr static auto config = cib::config(cib::extend<RuntimeInit>(
        core::system_clk_init >> setup_serial >> core::enable_interrupt >> wait_for_serial));
};
}  // namespace components