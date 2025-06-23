#pragma once

#include "ScioSense_ENS160.h"
#include "callbacks.hpp"
#include "core.hpp"
#include "data-models/environment.hpp"
#include "serial.hpp"

namespace components {

template <class Indicator, class SerialPort, uint8_t i2c_addr = ENS160_I2CADDR_1>
struct GasSensor {
    static inline ScioSense_ENS160 sensor{i2c_addr};
    static inline bool has_sensor{false};

    static bool setEnvironmentData(const data_models::environment_data_t data) {
        return sensor.set_envdata(data.temperature, data.humidity);
    }

    constexpr static auto ping_gas_sensor = flow::action("ping_gas_sensor"_sc, []() {
        has_sensor = sensor.setMode(ENS160_OPMODE_IDLE);
        if (has_sensor) {
            sensor.setMode(ENS160_OPMODE_STD);
            Indicator::setMode(Indicator::SLOW);
        } else {
            Indicator::setMode(Indicator::FAST);
        }
    });

    constexpr static auto config = cib::config(                                       //
        cib::extend<RuntimeInit>(                                                     //
            core::enable_interrupt >> ping_gas_sensor >> SerialPort::wait_for_serial  //
            ),
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

            SerialPort::print(
                data_models::air_quality_t{sensor.getAQI(), sensor.getTVOC(), sensor.geteCO2()});
        }));
};
}  // namespace components