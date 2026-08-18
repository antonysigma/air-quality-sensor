#pragma once

#include "callbacks.hpp"
#include "core.hpp"

namespace components {

template <typename LED>
struct Blink {
    enum blink_interval_t : uint16_t {  // NOLINT(performance-enum-size)
        FAST = 200,
        SLOW = 1000
    };
    static inline blink_interval_t blink_interval{SLOW};

    static void setMode(const blink_interval_t m) { blink_interval = m; }

    constexpr static auto init_led = flow::action<"init_led">([]() { LED::init(); });

    constexpr static auto config = cib::config(
        cib::extend<RuntimeInit>(core::disable_interrupt >> *init_led >> core::enable_interrupt),
        cib::extend<MainLoop>([](const units::Millisecond<uint16_t> current_ms) {
            using units::literals::operator""_ms;

            // TODO(antony): decouple GPIO component and the application.

            static units::Millisecond<uint16_t> prev_ms{};
            if (current_ms - prev_ms < blink_interval * 1_ms) {
                return;
            }
            prev_ms = current_ms;

            static bool state = false;
            LED::digitalWrite(state);
            state = !state;
        }));
};
}  // namespace components
