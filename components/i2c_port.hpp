#pragma once

extern "C" {
#include <utility/twi.h>
}

#include "callbacks.hpp"
#include "core.hpp"
#include "serial.hpp"

namespace components {

struct I2CPort {
    constexpr static auto setup_i2c = flow::action("setup_i2c"_sc, []() { twi_init(); });

    template <class Message>
    constexpr static uint8_t send(const uint8_t i2c_addr, const Message message) {
        return twi_writeTo(i2c_addr,
                           const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(&message)),
                           sizeof(Message), 1, 1);
    }

    template <class Message>
    constexpr static Message read(const uint8_t i2c_addr) {
        Message payload{};
        twi_readFrom(i2c_addr, reinterpret_cast<uint8_t*>(&payload), sizeof(Message), true);
        return payload;
    }

    constexpr static auto config = cib::config(cib::extend<RuntimeInit>(  //
        core::system_clk_init >> setup_i2c >> core::enable_interrupt      //
        ));
};
}  // namespace components