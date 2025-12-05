#pragma once

#include "utils/units.hpp"

using units::literals::operator""_kHz;
using units::literals::operator""_ms;

struct Timer {
    units::Microsecond<uint32_t> interrupt_interval{};
    units::KiloHertz freq{};
    enum usage_t : std::uint8_t { INTERRUPT, PHASE_CORRECT_PWM } usage{INTERRUPT};
};

// Chipset LGT8 has 32MHz clock.
constexpr auto oscillator_freq = 32e3_kHz;
constexpr auto system_freq = 16e3_kHz;

// Arduino SDK's Serial and millis depends on the 1kHz clock.
constexpr Timer timer0{1_ms, 16e3_kHz, Timer::INTERRUPT};
