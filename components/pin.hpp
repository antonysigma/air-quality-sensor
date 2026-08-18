#pragma once

#include <avr/io.h>
#include <stdint.h>

#include <type_traits>

namespace components {

template <char port, uint8_t bit>
struct Pin {
    static_assert(bit < 8, "A port pin bit must be in the range 0..7");

    static constexpr char port_id = port;
    static constexpr uint8_t bit_index = bit;
    static constexpr uint8_t bit_mask = static_cast<uint8_t>(1u << bit);
};

namespace pin_detail {

template <char port>
struct Port;

template <>
struct Port<'B'> {
    static void configureOutput(const uint8_t mask) { DDRB |= mask; }
    static void set(const uint8_t mask) { PORTB |= mask; }
    static void clear(const uint8_t mask) { PORTB &= static_cast<uint8_t>(~mask); }
};

template <>
struct Port<'D'> {
    static void configureOutput(const uint8_t mask) { DDRD |= mask; }
    static void set(const uint8_t mask) { PORTD |= mask; }
    static void clear(const uint8_t mask) { PORTD &= static_cast<uint8_t>(~mask); }
};

template <typename First, typename... Rest>
struct UniquePins
    : std::bool_constant<(!std::is_same_v<First, Rest> && ...) && UniquePins<Rest...>::value> {};

template <typename Last>
struct UniquePins<Last> : std::true_type {};

}  // namespace pin_detail

template <typename Pin>
void
configureOutput() {
    pin_detail::Port<Pin::port_id>::configureOutput(Pin::bit_mask);
}

template <typename... Pins>
inline constexpr bool unique_pins = pin_detail::UniquePins<Pins...>::value;

}  // namespace components
