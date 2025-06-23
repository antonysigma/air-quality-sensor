#pragma once

#include "Adafruit_AHTX0.h"
#include "callbacks.hpp"
#include "core.hpp"
#include "data-models/environment.hpp"
#include "serial.hpp"

namespace components {

template <class Indicator>
struct TemperatureSensor {
    static inline Adafruit_AHTX0 sensor{};
    static inline bool has_sensor{false};

    static data_models::environment_data_t read() {
        sensors_event_t humidity, temperature;
        sensor.getEvent(&humidity, &temperature);
        return {temperature.temperature, humidity.relative_humidity};
    }

    constexpr static auto ping_temperature_sensor =
        flow::action("ping_temperature_sensor"_sc, []() {
            has_sensor = sensor.begin();
            if (!has_sensor) {
                Indicator::setMode(Indicator::FAST);
            }
        });

    constexpr static auto config = cib::config(                                               //
        cib::extend<RuntimeInit>(                                                             //
            core::enable_interrupt >> ping_temperature_sensor >> SerialPort::wait_for_serial  //
            ));
};
}  // namespace components