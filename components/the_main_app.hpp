#pragma once
#include "core.hpp"

namespace controllers {

template <class TemperatureSensor, class GasSensor, class Display, class SerialPort>
struct TheMainApp {
    constexpr static auto read_air_quality = [](const uint32_t current_ms) {
        using components::Millis;

        static auto prev_ms = current_ms + 100'000UL;
        if (current_ms - prev_ms < 30'000UL) {
            return;
        }
        prev_ms = current_ms;

        if (!GasSensor::has_sensor) {
            Serial.print(F("ENS160 not available\n"));
            return;
        }

        const auto measurements = GasSensor::read();
        Display::update(measurements);
        SerialPort::print(measurements);
    };

    constexpr static auto read_temperature = [](const uint32_t current_ms) {
        using components::Millis;

        static auto prev_ms = current_ms + 100'000UL;
        if (current_ms - prev_ms < 30'000UL) {
            return;
        }
        prev_ms = current_ms;

        if (!TemperatureSensor::has_sensor) {
            Serial.print(F("AHTX0 not available\n"));
            return;
        }

        const auto data = TemperatureSensor::read();
        GasSensor::set(data);
        Display::update(data);
        SerialPort::print(data);
    };

    constexpr static auto config = cib::config(   //
        cib::extend<MainLoop>(read_air_quality),  //
        cib::extend<MainLoop>(read_temperature)   //
    );
};
}  // namespace controllers