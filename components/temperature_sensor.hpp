#pragma once

#include "Adafruit_AHTX0.h"
#include "callbacks.hpp"
#include "core.hpp"
#include "data-models/environment.hpp"
#include "serial.hpp"

namespace components {

template <class SerialPort>
struct TemperatureSensor {
    static inline Adafruit_AHTX0 sensor{};
    static inline bool has_sensor{false};

    static data_models::environment_data_t read() {
        sensors_event_t humidity, temperature;
        sensor.getEvent(&humidity, &temperature);
        return {temperature.temperature, humidity.relative_humidity};
    }

    constexpr static auto ping_temperature_sensor =
        flow::action("ping_temperature_sensor"_sc, []() { has_sensor = sensor.begin(); });

    constexpr static auto config = cib::config(                                               //
        cib::extend<RuntimeInit>(                                                             //
            core::enable_interrupt >> ping_temperature_sensor >> SerialPort::wait_for_serial  //
            ),
        cib::extend<MainLoop>([](const uint32_t current_ms) {
            static auto prev_ms = Millis();
            if (current_ms - prev_ms < 5000) {
                return;
            }
            prev_ms = current_ms;

            if (!has_sensor) {
                Serial.print(F("AHT21 not available\n"));
                return;
            }

            SerialPort::print(read());
        }));
};
}  // namespace components