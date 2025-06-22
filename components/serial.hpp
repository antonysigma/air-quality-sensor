#pragma once

#include "callbacks.hpp"
#include "core.hpp"

namespace components {

struct SerialPort {
    constexpr static auto setup_serial =
        flow::action("setup_serial"_sc, []() { Serial.begin(115'200); });

    constexpr static auto wait_for_serial =
        flow::action("wait_for_serial"_sc, []() { while (!Serial); });

    constexpr static auto config = cib::config(cib::extend<RuntimeInit>(
        core::system_clk_init >> setup_serial >> core::enable_interrupt >> wait_for_serial));
};
}  // namespace components