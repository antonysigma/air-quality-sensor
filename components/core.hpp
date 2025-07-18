#pragma once

#include <wiring_private.h>

#include "callbacks.hpp"
#include "config.h"
#include "utils/log2.hpp"

namespace components {

static volatile uint32_t
    millis_value =  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    0;
uint32_t
Millis() {  // NOLINT(readability-identifier-naming)
    return millis_value;
}

namespace core {

template <uint16_t factor>
constexpr uint8_t
prescalerRegisterValue() {
    static_assert(factor == 8 || factor == 64 || factor == 256 || factor == 1024);
    switch (factor) {
        case 1024:
            return (1u << CS02) | (1u << CS00);
        case 256:
            return (1u << CS02);
        case 64:
            return (1u << CS01) | (1u << CS00);
        case 8:
            return (1u << CS01);
        default:
            return 0;
    }
}

template <uint16_t prescaler_value, units::Microsecond<uint32_t> time_interval>
constexpr uint8_t
overflowRegisterValue() {
    static_assert(static_cast<uint64_t>(timer0.freq * time_interval) % prescaler_value == 0,
                  "Timer0 period must be a multiple of prescaler");

    constexpr auto overflow_value =
        static_cast<uint32_t>(timer0.freq * time_interval / prescaler_value - 1);
    static_assert(overflow_value < 255,
                  "Overflow register value > 255; insufficient clock prescaler value?");
    return static_cast<uint8_t>(overflow_value);
}
static_assert(overflowRegisterValue<64, 1_ms>() == 0xF9);

template <units::KiloHertz timer_freq>
constexpr uint8_t
clockPrescaler() {
    constexpr auto division = static_cast<uint32_t>(oscillator_freq / timer_freq);

    static_assert(division <= 256);
    static_assert(__builtin_popcount(division) == 1, "Must be power of two");
    return utils::log2(division);
}
static_assert(clockPrescaler<16e3_kHz>() == 0x01);
static_assert(clockPrescaler<8e3_kHz>() == 0x02);
static_assert(clockPrescaler<4e3_kHz>() == 0x03);

static constexpr auto disable_interrupt = flow::action("disable_interrupt"_sc, []() { cli(); });
static constexpr auto enable_interrupt = flow::action("enable_interrupt"_sc, []() { sei(); });

constexpr static auto system_clk_init = flow::action("system_clk_init"_sc, []() {
    // Enable clock prescaler pin change.
    CLKPR = 0x80;

    // With 4 clock cycles, update the clock division factor.
    CLKPR = clockPrescaler<system_freq>();
});

static constexpr auto dsu_init = flow::action("dsu_init"_sc, []() {
    volatile uint8_t& dcsr{*reinterpret_cast<volatile uint8_t*>(0x20)};
    dcsr = dcsr | (1 << 7);
});

static constexpr auto timer0_init = flow::action("timer0_init"_sc, []() {
    constexpr auto prescaler = 64UL;

    TCCR0A = TCCR0A | (1u << WGM01);                                        // Set the CTC mode
    OCR0A = overflowRegisterValue<prescaler, timer0.interrupt_interval>();  // Set the value for 1ms
    TIMSK0 = TIMSK0 | (1u << OCIE0A);  // Set the interrupt request

    TCCR0B = TCCR0B | prescalerRegisterValue<prescaler>();  // Set the prescale 1/64 clock
});

struct Impl {
    constexpr static auto config = cib::config(  //
        cib::extend<RuntimeInit>(
            disable_interrupt >> system_clk_init >> timer0_init >> enable_interrupt,  //
            disable_interrupt >> dsu_init >> enable_interrupt                         //
            ),
        cib::extend<OnTimer0Interrupt>([]() { millis_value = millis_value + 1; })  //
    );
};

}  // namespace core
}  // namespace components