#pragma once

#include "callbacks.hpp"
#include "core.hpp"

namespace components {

// TODO: define a struct io_def_t instead of uint8_t.
template <uint8_t led_id>
struct Blink {
    constexpr static auto init_led = flow::action("init_led"_sc, []() { pinMode(led_id, OUTPUT); });

    constexpr static auto config = cib::config(
        cib::extend<RuntimeInit>(core::disable_interrupt >> init_led >> core::enable_interrupt),
        cib::extend<MainLoop>([]() {
            // TODO: decouple GPIO component and the application.

            static auto prev_ms = Millis();
            const auto current_ms = Millis();
            if (current_ms - prev_ms < 1000) {
                return;
            }
            prev_ms = current_ms;

            static uint8_t state = LOW;
            digitalWrite(led_id, state);
            state = !state;

            Serial.print(current_ms);
            Serial.print(F(" ms\n"));
        }));
};
}  // namespace components