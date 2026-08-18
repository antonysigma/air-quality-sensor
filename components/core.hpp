#pragma once

#include <util/atomic.h>
#include <wiring_private.h>

#include "callbacks.hpp"
#include "config.h"
#include "utils/log2.hpp"

namespace components {

struct WallClock {
    static inline volatile uint16_t value{
        0};  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    static units::Millisecond<uint16_t> now() {  // NOLINT(readability-identifier-naming)
        uint16_t safe_copy;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { safe_copy = value; }
        return {safe_copy};
    }

    static void sleepFor(
        const units::Millisecond<uint16_t> duration) {  // NOLINT(readability-identifier-naming)
        for (const auto prev_time = now(); now() - prev_time < duration;) {
        }
    }
};

namespace core {

template <uint16_t factor>
consteval uint8_t
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

template <uint16_t prescaler_value, units::Microsecond<uint32_t> time_interval,
          typename T = uint8_t>
consteval T
topRegisterValue() {
    static_assert(static_cast<uint64_t>(system_freq * time_interval) % prescaler_value == 0,
                  "Period must be a multiple of prescaler");

    constexpr auto overflow_value =
        static_cast<uint32_t>(system_freq * time_interval / prescaler_value - 1);
    static_assert(overflow_value <= std::numeric_limits<T>::max(),
                  "Top register value overflowed; insufficient clock prescaler value?");
    return static_cast<T>(overflow_value);
}
static_assert(topRegisterValue<64, 1'000_us>() == 0xF9);

template <units::KiloHertz timer_freq>
consteval uint8_t
clockPrescaler() {
    constexpr auto division = static_cast<uint32_t>(oscillator_freq / timer_freq);

    static_assert(division <= 256);
    static_assert(__builtin_popcount(division) == 1, "Must be power of two");
    return utils::log2(division);
}
static_assert(clockPrescaler<16e3_kHz>() == 0x01);
static_assert(clockPrescaler<8e3_kHz>() == 0x02);
static_assert(clockPrescaler<4e3_kHz>() == 0x03);

static constexpr auto disable_interrupt = flow::action<"disable_interrupt">([]() { cli(); });
static constexpr auto enable_interrupt = flow::action<"enable_interrupt">([]() { sei(); });

constexpr static auto system_clk_init = flow::action<"system_clk_init">([]() {
    // Enable clock prescaler pin change.
    CLKPR = 0x80;

    // With 4 clock cycles, update the clock division factor.
    CLKPR = clockPrescaler<system_freq>();
});

static constexpr auto timer0_init = flow::action<"timer0_init">([]() {
    constexpr auto prescaler = 64UL;

    TCCR0A = TCCR0A | (1u << WGM01);                                        // Set the CTC mode
    OCR0A = topRegisterValue<prescaler, timer0.interrupt_interval>();       // Set the value for 1ms
    TIMSK0 = TIMSK0 | (1u << OCIE0A);  // Set the interrupt request

    TCCR0B = TCCR0B | prescalerRegisterValue<prescaler>();  // Set the prescale 1/64 clock
});

struct Impl {
    constexpr static auto config = cib::config(  //
        cib::extend<RuntimeInit>(
            *disable_interrupt >> *system_clk_init >> *timer0_init >> *enable_interrupt,  //
            disable_interrupt >> enable_interrupt                                         //
            ),
        cib::extend<OnTimer0Interrupt>([]() { WallClock::value = WallClock::value + 1; })  //
    );
};

}  // namespace core
}  // namespace components
