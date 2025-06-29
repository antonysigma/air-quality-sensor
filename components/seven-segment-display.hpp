#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

#include <boost/sml.hpp>

#include "callbacks.hpp"
#include "core.hpp"
#include "data-models/environment.hpp"
#include "data-models/ht16k33_commands.hpp"
#include "data-models/timing.hpp"
#include "utils/small_vector.hpp"

namespace components {

namespace seven_seg_display {

namespace internal {

constexpr std::array<uint8_t, 96> seven_seg_fonttable{
    0b00000000,  // (space)
    0b10000110,  // !
    0b00100010,  // "
    0b01111110,  // #
    0b01101101,  // $
    0b11010010,  // %
    0b01000110,  // &
    0b00100000,  // '
    0b00101001,  // (
    0b00001011,  // )
    0b00100001,  // *
    0b01110000,  // +
    0b00010000,  // ,
    0b01000000,  // -
    0b10000000,  // .
    0b01010010,  // /
    0b00111111,  // 0
    0b00000110,  // 1
    0b01011011,  // 2
    0b01001111,  // 3
    0b01100110,  // 4
    0b01101101,  // 5
    0b01111101,  // 6
    0b00000111,  // 7
    0b01111111,  // 8
    0b01101111,  // 9
    0b00001001,  // :
    0b00001101,  // ;
    0b01100001,  // <
    0b01001000,  // =
    0b01000011,  // >
    0b11010011,  // ?
    0b01011111,  // @
    0b01110111,  // A
    0b01111100,  // B
    0b00111001,  // C
    0b01011110,  // D
    0b01111001,  // E
    0b01110001,  // F
    0b00111101,  // G
    0b01110110,  // H
    0b00110000,  // I
    0b00011110,  // J
    0b01110101,  // K
    0b00111000,  // L
    0b00010101,  // M
    0b00110111,  // N
    0b00111111,  // O
    0b01110011,  // P
    0b01101011,  // Q
    0b00110011,  // R
    0b01101101,  // S
    0b01111000,  // T
    0b00111110,  // U
    0b00111110,  // V
    0b00101010,  // W
    0b01110110,  // X
    0b01101110,  // Y
    0b01011011,  // Z
    0b00111001,  // [
    0b01100100,  //
    0b00001111,  // ]
    0b00100011,  // ^
    0b00001000,  // _
    0b00000010,  // `
    0b01011111,  // a
    0b01111100,  // b
    0b01011000,  // c
    0b01011110,  // d
    0b01111011,  // e
    0b01110001,  // f
    0b01101111,  // g
    0b01110100,  // h
    0b00010000,  // i
    0b00001100,  // j
    0b01110101,  // k
    0b00110000,  // l
    0b00010100,  // m
    0b01010100,  // n
    0b01011100,  // o
    0b01110011,  // p
    0b01100111,  // q
    0b01010000,  // r
    0b01101101,  // s
    0b01111000,  // t
    0b00011100,  // u
    0b00011100,  // v
    0b00010100,  // w
    0b01110110,  // x
    0b01101110,  // y
    0b01011011,  // z
    0b01000110,  // {
    0b00110000,  // |
    0b01110000,  // }
    0b00000001,  // ~
    0b00000000,  // del
};

constexpr uint8_t
getFont(const char c) {
    const uint16_t index = c - ' ';
    if (index > seven_seg_fonttable.size()) {
        return getFont(' ');
    }
    return seven_seg_fonttable[index];
}

class WriteBuffer : public utils::SmallVector<uint8_t, 5> {
   public:
    void write(const char c) {
        emplaceBack(getFont(c));
        // Skip the middle colon (:) LED
        if (size() == 2) {
            emplaceBack(getFont(' '));
        }
    }
};
}  // namespace internal

template <class I2CPort, uint8_t i2c_addr = 0x70>
struct Impl {
    static inline Adafruit_7segment matrix{};

    constexpr static auto init_7seg_display = flow::action("init_7seg_display"_sc, []() {
        using namespace ht16k33_commands;
        matrix.begin(i2c_addr);
        // I2CPort::send(i2c_addr, OscillatorOn{});
        // I2CPort::send(i2c_addr, NoBlink{});
        I2CPort::send(i2c_addr, Brightness{4});
    });

    // static constexpr void flush() {
    //     using ht16k33_commands::WriteDisplay;
    //     I2CPort::send(i2c_addr, WriteDisplay{buffer.sanitizedBuffer()});
    //     buffer.clear();
    // }

    static constexpr void println(const std::string_view message) {
        using ht16k33_commands::WriteDisplay;
        internal::WriteBuffer write_buffer{};
        for (const auto& c : message) {
            write_buffer.write(c);
        }
        I2CPort::send(i2c_addr, WriteDisplay{write_buffer.sanitizedBuffer()});
    }

    constexpr static auto print_init_banner = flow::action("print_init_banner"_sc, []() {
        using namespace std::string_view_literals;
        println("Init"sv);
    });

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
        using namespace std::string_view_literals;
        using ht16k33_commands::WriteDisplay;

        internal::WriteBuffer write_buffer{};
        for (const auto& c : "Aq "sv) {
            write_buffer.write(c);
        }
        write_buffer.write('0' + data.value);
        I2CPort::send(i2c_addr, WriteDisplay{write_buffer.sanitizedBuffer()});
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
}  // namespace seven_seg_display
}  // namespace components