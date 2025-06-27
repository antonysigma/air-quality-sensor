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

    static constexpr void print(const uint16_t value) {
        for(auto [i, v] = std::pair<int8_t, uint16_t>{3, value};
            i >= 0; i--, v /= 10
        ) {
            const uint8_t digit = v % 10;
            const uint8_t cursor = (i <= 1 ? i : i + 1);
            matrix.writeDigitAscii(cursor, (v > 0) ? '0' + digit : ' ', false);
        }
    }

    static constexpr void print(const display_commands::tvoc_t tvoc) {
        print(tvoc.value);
        matrix.println();
        matrix.writeDisplay();
    }

    static constexpr void print(const display_commands::aqi_t data) {
        matrix.print("Aq ");
        matrix.write('0' + data.value);
        matrix.println();
        matrix.writeDisplay();
    }

    static constexpr void print(const float value, const bool flush_right) {
        const uint8_t scale_factor = (value < 100) ? 10 : 1;
        const auto first_three_digits = static_cast<uint16_t>(value * scale_factor);

        static_assert(uint8_t{1} / 10 < 1);
        for (auto [i, mag] = std::make_pair<uint8_t, uint16_t>(flush_right, 100);
             mag >= 1 && i < flush_right + 3; mag /= 10, i++) {
            const uint8_t digit = (first_three_digits / mag) % 10;

            const auto cursor = (i <= 1 ? i : i + 1);
            const auto print_decimal_point = (mag == scale_factor);
            matrix.writeDigitAscii(cursor, '0' + digit, print_decimal_point);
        }
    }

    static constexpr void print(const display_commands::celcius_t deg_c) {
        print(deg_c.value, false);
        matrix.writeDigitAscii(4, 'C');
        matrix.println();
        matrix.writeDisplay();
    }

    static constexpr void print(const display_commands::humidity_t humidity) {
        print(humidity.value, true);
        matrix.writeDigitAscii(0, 'H');
        matrix.println();
        matrix.writeDisplay();
    }

    static constexpr void print(const display_commands::eco2_t co2_concentration) {
        print(co2_concentration.value);
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