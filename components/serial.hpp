#pragma once

#include "callbacks.hpp"
#include "core.hpp"
#include "data-models/environment.hpp"
#include "utils/pgm_string.hpp"

namespace components {

namespace serial_port {

template <units::KiloHertz rate>
constexpr uint16_t
baudrateRegisterValue() {
    return static_cast<uint16_t>(system_freq / rate + 1) / 2 - 1;
}

struct UcsrARegister {
    /** Multi-processor mode */
    uint8_t mpme : 1;

    /** double-speed mode */
    uint8_t u2x : 1;
    uint8_t pe : 1;
    uint8_t dor : 1;
    uint8_t fe : 1;
    uint8_t udre : 1;
    uint8_t txc : 1;
    uint8_t rxc : 1;
};

union UcsrBRegister {
    uint8_t buffer{};

    struct {
        uint8_t txb8 : 1 = 0;
        uint8_t rxb8 : 1 = 0;
        uint8_t ucsz2 : 1 = 0;

        /** Transmission enable */
        uint8_t txen : 1 = 0;

        /** Receive enable */
        uint8_t rxen : 1 = 0;

        /** Data register empty interrupt request */
        uint8_t udrie : 1 = 0;

        /** Transmission completion interrupt request */
        uint8_t txcie : 1 = 0;

        /** Receive completion interrupt request */
        uint8_t rxcie : 1 = 0;
    } fields;
};
static_assert(sizeof(UcsrBRegister) == 1);

union UcsrCRegister {
    uint8_t buffer{};

    struct {
        uint8_t ucpol : 1 = 0;
        uint8_t ucsz : 2 = 0;
        uint8_t usbs : 1 = 0;
        uint8_t upm : 2 = 0;
        uint8_t umsel : 2 = 0;
    } fields;
};
static_assert(sizeof(UcsrCRegister) == 1);

auto& ucsra{*reinterpret_cast<volatile UcsrARegister*>(0xC0)};
}  // namespace serial_port

struct SerialPort {
    constexpr static auto setup_serial = flow::action("setup_serial"_sc, []() {
        using serial_port::UcsrBRegister;
        using serial_port::UcsrCRegister;
        // auto& ucsrc{*reinterpret_cast<volatile uint8_t*>(0xC2)};
        // ucsrc = UcsrCRegister{.fields={.ucpol=1, .ucsz=0b11, .usbs=1, .upm=0b00,
        // .umsel=0b01}}.buffer;

        // auto& ucsrb{*reinterpret_cast<volatile uint8_t*>(0xC1)};
        // ucsrb = UcsrBRegister{.fields={.txen=1,.rxen=1}}.buffer;

        // UCSR0C = (USART_UMSEL0 << UMSEL00 ) | (USART_UPM0 << UPM00) | (USART_USBS0 << USBS0) |
        //		((USART_UCSZ0 & 3) << UCSZ00 ) | (USART_UCPOL0 << UCPOL0);

        // UCSR0B = (USART_RXEN << RXEN0) | (USART_TXEN << TXEN0) | (USART_UCSZ0 & 0x4) |
        //		(USART_RXC << RXCIE0) | (USART_TXC << TXCIE0) | (USART_UDRE << UDRIE0);

        using namespace units::literals;
        constexpr auto baudrate = 115.200_kHz;
        Serial.begin(baudrate / 1e-3_kHz);

        // auto& ubrr{*reinterpret_cast<volatile uint16_t*>(0xC5)};
        // ubrr = serial_port::baudrateRegisterValue<baudrate>();
    });

    static void wait() {
        using serial_port::ucsra;
        while (!ucsra.udre) {
        }
    }

    constexpr static auto wait_for_serial = flow::action("wait_for_serial"_sc, []() { wait(); });

    static void write(const char c) {
        wait();
        UDR0 = c;
    }

    static constexpr void print(const utils::PGMStringHelper message) {
        for (const char* p = message.begin(); p != message.end(); ++p) {
            write(pgm_read_byte(p));
        }
    }

    template <typename T>
    static constexpr void print(T v) {
        static_assert(std::is_integral_v<T> && sizeof(T) <= sizeof(uint16_t));
        if (v > 9999) {
            print(PSTR2("OvEr"));
            return;
        }

        if (v < 0) {
            print(PSTR2(" NeG"));
            return;
        }

        std::array<char, 4> digits{' ', ' ', ' ', ' '};
        for (auto iter = digits.rbegin(); v > 0 && iter != digits.rend(); ++iter, v /= 10) {
            *iter = '0' + (v % 10);
        }

        for (const auto& d : digits) {
            if (d == ' ') {
                continue;
            }
            write(d);
        }
    }

    static void print(const data_models::AirQuality aq) {
        print(PSTR2("AQI: "));
        print(aq.aqi);
        print(PSTR2("\tTVOC: "));
        print(aq.tvoc);
        print(PSTR2("ppb\teCO2: "));
        print(aq.eco2);
        print(PSTR2("ppm\n"));
    }

    static void print(const data_models::EnvironmentData env) {
        constexpr auto printFloat = [&](const float v) {  // NOLINT(readability-identifier-naming)
            print(static_cast<uint8_t>(v));
            write('.');
            write('0' + (static_cast<uint8_t>(v * 10) % 10));
        };
        print(PSTR2("Temperature: "));
        printFloat(env.temperature);
        print(PSTR2("C\tRH: "));
        printFloat(env.humidity);
        print(PSTR2("%\n"));
    }

    constexpr static auto config = cib::config(cib::extend<RuntimeInit>(
        core::system_clk_init >> setup_serial >> core::enable_interrupt >> wait_for_serial));
};
}  // namespace components