#pragma once

#include "Adafruit_AHTX0.h"
#include "callbacks.hpp"
#include "core.hpp"
#include "data-models/ahtx0_commands.hpp"
#include "data-models/environment.hpp"
#include "serial.hpp"

namespace components {

template <class Indicator, uint8_t i2c_addr = AHTX0_I2CADDR_DEFAULT>
struct TemperatureSensor {
    static inline Adafruit_AHTX0 sensor{};
    static inline bool has_sensor{false};

    static data_models::environment_data_t read() {
        namespace ahtx0 = commands::ahtx0;

        I2CPort::send(i2c_addr, ahtx0::TriggerCmd{});
        while (I2CPort::read<ahtx0::Status>(i2c_addr).isBusy()) {
            delay(10);
        }

        const auto measurements = I2CPort::read<ahtx0::Measurements>(i2c_addr);
        return {measurements.temperature(), measurements.humidity()};

        // sensors_event_t humidity, temperature;
        // sensor.getEvent(&humidity, &temperature);
        // return {temperature.temperature, humidity.relative_humidity};
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