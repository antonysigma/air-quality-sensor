#pragma once
#include "core.hpp"
#include "data-models/timing.hpp"
#include "utils/pgm_string.hpp"

namespace controllers {

using units::literals::operator""_ms;
template <class TemperatureSensor, class GasSensor, class Display, class SerialPort>
struct TheMainApp {
    static constexpr auto update_interval{30'000_ms};

    constexpr static auto read_air_quality = [](const data_models::TimeMs current_ms) {
        static units::Millisecond<uint16_t> prev_ms{};

        // Eliminate the 32-bit guard variable for non-zero static initialization.
        static bool is_first_time{true};
        if (is_first_time) {
            prev_ms = current_ms - update_interval;
            is_first_time = false;
        }

        if (current_ms - prev_ms < update_interval) {
            return;
        }
        prev_ms = current_ms;

        if (!GasSensor::has_sensor) {
            SerialPort::print(PSTR2("ENS160 not available\n"));
            return;
        }

        const auto measurements = GasSensor::read();
        Display::update(measurements);
        constexpr auto print = [](const ens160_commands::AQIPredictions aq) {
            SerialPort::print(PSTR2("AQI: "));
            SerialPort::print(aq.aqi);
            SerialPort::print(PSTR2("\tTVOC: "));
            SerialPort::print(aq.tvoc);
            SerialPort::print(PSTR2("ppb\teCO2: "));
            SerialPort::print(aq.eco2);
            SerialPort::print(PSTR2("ppm\n"));
        };
        print(measurements);
    };

    constexpr static auto read_temperature = [](const data_models::TimeMs current_ms) {
        static units::Millisecond<uint16_t> prev_ms{};

        // Eliminate the 32-bit guard variable for non-zero static initialization.
        static bool is_first_time{true};
        if (is_first_time) {
            prev_ms = current_ms - update_interval;
            is_first_time = false;
        }

        if (current_ms - prev_ms < update_interval) {
            return;
        }
        prev_ms = current_ms;

        if (!TemperatureSensor::has_sensor) {
            SerialPort::print(PSTR2("AHTX0 not available\n"));
            return;
        }

        const auto data = TemperatureSensor::read();
        GasSensor::set(data);
        Display::update(data);

        constexpr auto print_float =
            [&](const utils::Rational<> v) {  // NOLINT(readability-identifier-naming)
                const auto truncated = static_cast<uint16_t>(v * 10);
                SerialPort::print(truncated / 10);
                SerialPort::write('.');
                SerialPort::write('0' + (truncated % 10));
            };

        constexpr auto print = [=](const data_models::EnvironmentData env) {
            SerialPort::print(PSTR2("Temperature: "));
            print_float(env.temperature);
            SerialPort::print(PSTR2("C\tRH: "));
            print_float(env.humidity);
            SerialPort::print(PSTR2("%\n"));
        };

        print(data);
    };

    constexpr static auto config = cib::config(   //
        cib::extend<MainLoop>(read_air_quality),  //
        cib::extend<MainLoop>(read_temperature)   //
    );
};
}  // namespace controllers
