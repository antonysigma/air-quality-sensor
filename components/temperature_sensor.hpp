#pragma once

#include "callbacks.hpp"
#include "core.hpp"
#include "data-models/ahtx0_commands.hpp"
#include "data-models/environment.hpp"
#include "serial.hpp"

namespace components {

template <class Indicator, class I2CPort, uint8_t i2c_addr = 0x38>
struct TemperatureSensor {
    static inline bool has_sensor{false};

    static data_models::EnvironmentData read() {
        namespace ahtx0 = commands::ahtx0;

        I2CPort::send(i2c_addr, ahtx0::TriggerCmd{});
        while (I2CPort::template read<ahtx0::Status>(i2c_addr).isBusy()) {
            delay(10);
        }

        const auto measurements = I2CPort::template read<ahtx0::Measurements>(i2c_addr);
        return {measurements.temperature(), measurements.humidity()};
    }

    constexpr static void wait() {
        using commands::ahtx0::Status;
        while (I2CPort::template read<Status>(i2c_addr).isBusy()) {
            delay(10);
        }
    }

    constexpr static auto ping_temperature_sensor =
        flow::action("ping_temperature_sensor"_sc, []() {
            namespace ahtx0 = commands::ahtx0;
            // Wait for device to power up.
            delay(20);

            I2CPort::send(i2c_addr, ahtx0::SoftResetCmd{});
            delay(20);
            wait();

            I2CPort::send(i2c_addr, ahtx0::CalibrateCmd{});
            wait();

            has_sensor = I2CPort::template read<ahtx0::Status>(i2c_addr).isCalibrated();
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