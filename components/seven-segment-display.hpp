#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

#include "callbacks.hpp"
#include "core.hpp"
#include "data-models/environment.hpp"

namespace components {

template <uint8_t i2c_addr = 0x70>
struct SevenSegDisplay {
    static inline Adafruit_7segment matrix{};

    constexpr static auto init_7seg_display =
        flow::action("init_7seg_display"_sc, []() { matrix.begin(i2c_addr); });

    constexpr static auto print_init_banner = flow::action("print_init_banner"_sc, []() {
        matrix.println("Init");
        matrix.writeDisplay();
    });

    static constexpr void print(const data_models::aqi_t aqi) {
        using namespace std::string_view_literals;
        for (const char& c : "Aq "sv) {
            matrix.write(c);
        }
        matrix.write('0' + aqi.value);
        matrix.println();
        matrix.writeDisplay();
    }

    static constexpr void print(const data_models::tvoc_t data) {
        matrix.println(data.value);
        matrix.writeDisplay();
    }

    constexpr static auto config = cib::config(  //
        cib::extend<RuntimeInit>(                //
            core::enable_interrupt >> init_7seg_display >> print_init_banner >>
            SerialPort::wait_for_serial  //
            ));
};
}  // namespace components