#pragma once

#include "ScioSense_ENS160.h"
#include "callbacks.hpp"
#include "core.hpp"
#include "serial.hpp"

namespace components {

template <class Indicator, uint8_t i2c_addr = ENS160_I2CADDR_1>
struct GasSensor {
    static inline ScioSense_ENS160 sensor{i2c_addr};
    static inline bool has_sensor{false};

    constexpr static auto ping_gas_sensor = flow::action("ping_gas_sensor"_sc, []() {
        has_sensor = sensor.setMode(ENS160_OPMODE_IDLE);
        if (has_sensor) {
            sensor.setMode(ENS160_OPMODE_STD);
            Indicator::setMode(Indicator::SLOW);
        } else {
            Indicator::setMode(Indicator::FAST);
        }
    });

    constexpr static auto config =
        cib::config(cib::extend<RuntimeInit>(core::enable_interrupt >> ping_gas_sensor >>
                                             SerialPort::wait_for_serial),
                    cib::extend<MainLoop>([](const uint32_t current_ms) {
                        static auto prev_ms = Millis();
                        if (current_ms - prev_ms < 1000) {
                            return;
                        }
                        prev_ms = current_ms;

                        if (!has_sensor) {
                            Serial.print(F("ENS160 not available\n"));
                            return;
                        }

                        sensor.measure(false);
                        sensor.measureRaw(false);

                        Serial.print(F("AQI: "));
                        Serial.print(sensor.getAQI());
                        Serial.write('\t');
                        Serial.print(F("TVOC: "));
                        Serial.print(sensor.getTVOC());
                        Serial.print(F("ppb\t"));
                        Serial.print(F("eCO2: "));
                        Serial.print(sensor.geteCO2());
                        Serial.print(F("ppm\t"));
                        Serial.print(F("R HP0: "));
                        Serial.print(sensor.getHP0());
                        Serial.print(F("Ohm\t"));
                        Serial.print(F("R HP1: "));
                        Serial.print(sensor.getHP1());
                        Serial.print(F("Ohm\t"));
                        Serial.print(F("R HP2: "));
                        Serial.print(sensor.getHP2());
                        Serial.print(F("Ohm\t"));
                        Serial.print(F("R HP3: "));
                        Serial.print(sensor.getHP3());
                        Serial.println(F("Ohm"));
                    }));
};
}  // namespace components