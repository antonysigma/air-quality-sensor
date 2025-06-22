#pragma once

#include "ScioSense_ENS160.h"
#include "callbacks.hpp"
#include "core.hpp"
#include "serial.hpp"

namespace components {

struct I2CPort {
    constexpr static auto setup_i2c = flow::action("setup_i2c"_sc, []() { Wire.begin(); });

    constexpr static auto config = cib::config(cib::extend<RuntimeInit>(  //
        core::system_clk_init >> setup_i2c >> core::enable_interrupt      //
        ));
};
}  // namespace components