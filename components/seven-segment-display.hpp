#pragma once

#include <boost/sml.hpp>

#include "callbacks.hpp"
#include "core.hpp"
#include "data-models/environment.hpp"
#include "data-models/ht16k33_commands.hpp"
#include "data-models/timing.hpp"
#include "serial.hpp"
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
    const uint16_t index =
        utils::Min(static_cast<uint16_t>(c - ' '), seven_seg_fonttable.size() - 1);
    return seven_seg_fonttable[index];  // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
}

struct WriteBuffer : public utils::SmallVector<uint8_t, 5> {
    constexpr void write(const char c) {
        emplaceBack(getFont(c));
        // Skip the middle colon (:) LED
        if (size() == 2) {
            emplaceBack(getFont(' '));
        }
    }

    static constexpr WriteBuffer make(const std::string_view message) {
        WriteBuffer write_buffer{};
        for (const auto& c : message) {
            write_buffer.write(c);
        }
        return write_buffer;
    }

    static constexpr WriteBuffer make(const uint16_t value) {
        using namespace std::string_view_literals;
        WriteBuffer write_buffer{};
        write_buffer.resize(5);

        for (auto [i, v] = std::pair<int8_t, uint16_t>{3, value}; i >= 0; i--, v /= 10) {
            const uint8_t digit = v % 10;
            const uint8_t cursor = (i <= 1 ? i : i + 1);
            write_buffer[cursor] = getFont((v > 0) ? static_cast<char>('0' + digit) : ' ');
        }
        return write_buffer;
    }

    static constexpr WriteBuffer make(const float value, const bool flush_right) {
        WriteBuffer write_buffer{};
        write_buffer.resize(5);

        const uint8_t scale_factor = (value < 100) ? 10 : 1;
        const auto first_three_digits = static_cast<uint16_t>(value * scale_factor);

        static_assert(uint8_t{1} / 10 < 1);
        const uint8_t end_position = flush_right + 3;
        for (auto [i, mag] = std::make_pair<uint8_t, uint16_t>(  // NOLINT(bugprone-infinite-loop)
                 flush_right, 100);
             (i < end_position) && (mag >= 1); ++i, mag /= 10) {
            const uint8_t digit = (first_three_digits / mag) % 10;

            const auto cursor = (i <= 1 ? i : i + 1);
            const auto print_decimal_point = (mag == scale_factor);
            const uint8_t dot = print_decimal_point ? (1 << 7) : 0;
            write_buffer[cursor] = getFont(static_cast<char>('0' + digit)) | dot;
        }

        return write_buffer;
    }
};
}  // namespace internal

template <class I2CPort, uint8_t i2c_addr = 0x70>
struct Impl {
    constexpr static auto init_7seg_display = flow::action("init_7seg_display"_sc, []() {
        using namespace ht16k33_commands;
        I2CPort::send(i2c_addr, OscillatorOn{});
        I2CPort::send(i2c_addr, NoBlink{});
        I2CPort::send(i2c_addr, Brightness{4});
    });

    static constexpr void println(const std::string_view message) {
        using ht16k33_commands::WriteDisplay;

        const auto write_buffer = internal::WriteBuffer::make(message).sanitizedBuffer();
        I2CPort::send(i2c_addr, WriteDisplay{write_buffer});
    }

    constexpr static auto print_init_banner = flow::action("print_init_banner"_sc, []() {
        using namespace std::string_view_literals;
        println("Init"sv);
    });

    static constexpr void print(const display_commands::TVoc tvoc) {
        using ht16k33_commands::WriteDisplay;
        const auto write_buffer = internal::WriteBuffer::make(tvoc.value).sanitizedBuffer();
        I2CPort::send(i2c_addr, WriteDisplay{write_buffer});
    }

    static constexpr void print(const display_commands::Aqi data) {
        using namespace std::string_view_literals;
        using ht16k33_commands::WriteDisplay;

        auto write_buffer = internal::WriteBuffer::make("Aq "sv);
        write_buffer.write(static_cast<char>('0' + data.value));

        I2CPort::send(i2c_addr, WriteDisplay{write_buffer.sanitizedBuffer()});
    }

    static constexpr void print(const display_commands::Celcius deg_c) {
        using ht16k33_commands::WriteDisplay;
        auto write_buffer = internal::WriteBuffer::make(deg_c.value, false);
        write_buffer[4] = internal::getFont('C');
        I2CPort::send(i2c_addr, WriteDisplay{write_buffer.sanitizedBuffer()});
    }

    static constexpr void print(const display_commands::Humidity humidity) {
        using ht16k33_commands::WriteDisplay;
        auto write_buffer = internal::WriteBuffer::make(humidity.value, true);
        write_buffer[0] = internal::getFont('H');
        I2CPort::send(i2c_addr, WriteDisplay{write_buffer.sanitizedBuffer()});
    }

    static constexpr void print(const display_commands::ECo2 co2_concentration) {
        using ht16k33_commands::WriteDisplay;
        const auto write_buffer =
            internal::WriteBuffer::make(co2_concentration.value).sanitizedBuffer();
        I2CPort::send(i2c_addr, WriteDisplay{write_buffer});
    }

    constexpr static auto config = cib::config(  //
        cib::extend<RuntimeInit>(                //
            core::enable_interrupt >> init_7seg_display >> print_init_banner >>
            SerialPort::wait_for_serial  //
            ));
};
}  // namespace seven_seg_display
}  // namespace components