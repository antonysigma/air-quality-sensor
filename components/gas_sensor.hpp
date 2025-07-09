#pragma once

#include "callbacks.hpp"
#include "core.hpp"
#include "data-models/ens160_commands.hpp"
#include "data-models/environment.hpp"
#include "data-models/i2c_types.hpp"
#include "serial.hpp"

namespace components {

template <class Indicator, class I2CPort, uint8_t i2c_addr = 0x53>
struct GasSensor {
    static inline bool has_sensor{false};
    static inline ens160_commands::AQIPredictions cached_measurements{};

    static constexpr void set(const data_models::EnvironmentData data) {
        using ens160_commands::SetEnvData;
        I2CPort::send(i2c_addr, SetEnvData{data.temperature, data.humidity});
    }

    constexpr static auto ping_gas_sensor = flow::action("ping_gas_sensor"_sc, []() {
        using namespace ens160_commands;
        using namespace i2c_types;

        using M = SetMode;
        has_sensor = I2CPort::send(i2c_addr, SetMode{M::IDLE}) == DATA_TRANSMITTED_ACK;
        if (!has_sensor) {
            Indicator::setMode(Indicator::FAST);
            return;
        }
        I2CPort::send(i2c_addr, SetMode{M::STANDARD});
    });

    static constexpr data_models::AirQuality read() {
        using namespace ens160_commands;
        I2CPort::send(i2c_addr, GetStatus{});
        const auto status = I2CPort::template read<Status>(i2c_addr);
        if (status.isNewData()) {
            I2CPort::send(i2c_addr, ReadAQIPrediction{});
            cached_measurements = I2CPort::template read<AQIPredictions>(i2c_addr);
        }

        return cached_measurements;
    }

    constexpr static auto config = cib::config(                                       //
        cib::extend<RuntimeInit>(                                                     //
            core::enable_interrupt >> ping_gas_sensor >> SerialPort::wait_for_serial  //
            ));
};
}  // namespace components