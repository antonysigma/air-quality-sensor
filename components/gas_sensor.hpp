#pragma once

#include "ScioSense_ENS160.h"
#include "callbacks.hpp"
#include "core.hpp"
#include "data-models/environment.hpp"
#include "serial.hpp"

namespace components {

template <class Indicator, uint8_t i2c_addr = ENS160_I2CADDR_1>
struct GasSensor {
    // TODO(antony): Handroll our own ENS160 library.
    static inline ScioSense_ENS160 sensor{i2c_addr};
    static inline bool has_sensor{false};

    static bool set(const data_models::EnvironmentData data) {
        return sensor.set_envdata(data.temperature, data.humidity);
    }

    constexpr static auto ping_gas_sensor = flow::action("ping_gas_sensor"_sc, []() {
        has_sensor = sensor.setMode(ENS160_OPMODE_IDLE);
        if (!has_sensor) {
            Indicator::setMode(Indicator::FAST);
            return;
        }
        sensor.setMode(ENS160_OPMODE_STD);
    });

    static data_models::AirQuality read() {
        sensor.measure(false);
        sensor.measureRaw(false);

        return {sensor.getAQI(), sensor.getTVOC(), sensor.geteCO2()};
    }

    constexpr static auto config = cib::config(                                       //
        cib::extend<RuntimeInit>(                                                     //
            core::enable_interrupt >> ping_gas_sensor >> SerialPort::wait_for_serial  //
            ));
};
}  // namespace components