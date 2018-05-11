#pragma once

#include "callbacks.hpp"
#include "core.hpp"

namespace components {

static volatile uint32_t millis_value = 0;
uint32_t
Millis() {
    return millis_value;
}

template <uint8_t led_id>
struct Blink {
    constexpr static auto init_led = flow::action("init_led"_sc, []() { pinMode(led_id, OUTPUT); });

    // constexpr static auto setup_serial = flow::action("setup_serial"_sc, []() {
    //     Serial.begin(115'200);
    // });

    // constexpr static auto wait_for_serial = flow::action("wait_for_serial"_sc, []() {
    //     while(!Serial);
    // });

    constexpr static auto config = cib::config(
        cib::extend<RuntimeInit>(core::disable_interrupt >> init_led >> core::enable_interrupt),
        cib::extend<OnTimer0Interrupt>([]() { millis_value = millis_value + 1; }),  //
        cib::extend<MainLoop>([]() {
            // Serial.print(F("Hello world!\n"));

            static auto prev_ms = Millis();
            const auto current_ms = Millis();
            if (current_ms - prev_ms < 1000) {
                return;
            }
            prev_ms = current_ms;

            static uint8_t state = LOW;
            digitalWrite(led_id, state);
            state = !state;
        }));
};
}  // namespace components