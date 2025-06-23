#pragma once

#include "ScioSense_ENS160.h"
#include "callbacks.hpp"
#include "core.hpp"
#include "serial.hpp"

namespace components {

struct I2CPort {
    constexpr static auto setup_i2c = flow::action("setup_i2c"_sc, []() { Wire.begin(); });

    template <class Message>
    constexpr static void send(const uint8_t i2c_addr, const Message message) {
        Wire.beginTransmission(i2c_addr);
        Wire.write(reinterpret_cast<const uint8_t*>(&message), sizeof(Message));
        Wire.endTransmission();
    }

    template <class Message>
    constexpr static Message read(const uint8_t i2c_addr) {
        Message payload{};
        Wire.requestFrom(i2c_addr, sizeof(Message));
        Wire.readBytes(reinterpret_cast<uint8_t*>(&payload), sizeof(Message));
        return payload;
    }

    constexpr static auto config = cib::config(cib::extend<RuntimeInit>(  //
        core::system_clk_init >> setup_i2c >> core::enable_interrupt      //
        ));
};
}  // namespace components