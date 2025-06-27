#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

#include <boost/sml.hpp>

#include "callbacks.hpp"
#include "core.hpp"
#include "data-models/environment.hpp"
#include "data-models/timing.hpp"

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

    static constexpr void print(const std::string_view message) {
        for (const auto& c : message) {
            matrix.write(c);
        }
        matrix.println();
        matrix.writeDisplay();
    }

    static constexpr void print(const display_commands::tvoc_t data) {
        matrix.println(data.value);
        matrix.writeDisplay();
    }

    static constexpr void print(const display_commands::aqi_t data) {
        matrix.print("Aq ");
        matrix.write('0' + data.value);
        matrix.println();
        matrix.writeDisplay();
    }

    static constexpr void print(const display_commands::celcius_t deg_c) {
        // constexpr auto decimal_place = 1;
        // constexpr uint8_t scale_factor = std::pow(10, decimal_place);
        // const auto rounded = static_cast<uint8_t>(deg_c.value * scale_factor);

        // static_assert(uint8_t{1} / 10 < 1);
        // for(auto [i, mag] = std::pair<uint8_t, uint8_t>{0, 100}; mag >= 1; mag /= 10, i++) {
        //     const auto digit = (rounded / mag) %10;
        //     if (digit <= 0 && i == 0) {
        //         matrix.writeDigitAscii(0, ' ');
        //         continue;
        //     }
        //     const auto print_decimal_point = (mag == 10);
        //     matrix.writeDigitNum(i <= 1 ? i : i+1, digit, print_decimal_point);
        // }
        matrix.print(deg_c.value);
        matrix.writeDigitAscii(4, 'C');
        matrix.println();
        matrix.writeDisplay();
    }

    static constexpr void print(const display_commands::humidity_t humidity) {
        matrix.print(humidity.value);
        matrix.writeDigitAscii(4, 'H');
        matrix.println();
        matrix.writeDisplay();
    }

    constexpr static auto config = cib::config(  //
        cib::extend<RuntimeInit>(                //
            core::enable_interrupt >> init_7seg_display >> print_init_banner >>
            SerialPort::wait_for_serial  //
            ));
};
}  // namespace components