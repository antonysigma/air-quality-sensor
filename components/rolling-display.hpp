#pragma once

#include <boost/sml.hpp>

#include "callbacks.hpp"
#include "core.hpp"
#include "data-models/environment.hpp"
#include "data-models/timing.hpp"

namespace components {

namespace rolling_display {

namespace internal {
template <class Display>
struct StateMachine {
    uint32_t prev_ms{};

    auto operator()() {
        using data_models::TimeMs;
        // Guards
        auto after_one_second = [&](const TimeMs current_ms) -> bool {
            constexpr auto update_interval_ms = 1000;
            return current_ms.value - prev_ms >= update_interval_ms;
        };

        // Actions
        auto update_time = [&](const TimeMs current_ms) { prev_ms = current_ms.value; };

        // Actions
        constexpr auto print_tvoc_banner = []() {
            using namespace std::string_view_literals;
            Display::println("tVoC"sv);
        };

        constexpr auto print_tvoc_value = [](const data_models::AirQuality& aq) {
            Display::print(display_commands::TVoc{aq});
        };

        constexpr auto print_aqi = [](const display_commands::AirQuality& aq) {
            using namespace std::string_view_literals;
            Display::print(display_commands::Aqi{aq});
        };

        constexpr auto print_temperature = [](const display_commands::EnvironmentData& env) {
            using namespace std::string_view_literals;
            Display::print(display_commands::Celcius{env});
        };

        constexpr auto print_humidity = [](const display_commands::EnvironmentData& env) {
            using namespace std::string_view_literals;
            Display::print(display_commands::Humidity{env});
        };

        constexpr auto print_eco2_banner = []() {
            using namespace std::string_view_literals;
            Display::println("eCO2"sv);
        };

        constexpr auto print_eco2_value = [](const display_commands::AirQuality& aq) {
            using namespace std::string_view_literals;
            Display::print(display_commands::ECo2{aq});
        };

        using namespace boost::sml;
        return make_transition_table(
            // clang-format off
            *"Begin"_s + event<TimeMs> / update_time = "Init"_s,
            "Init"_s + event<TimeMs>[after_one_second] / (update_time, print_eco2_banner) = "eCO2Banner"_s,
            "eCO2Banner"_s + event<TimeMs>[after_one_second] / (update_time, print_eco2_value) = "eCO2Value"_s,
            "eCO2Value"_s + event<TimeMs>[after_one_second] / (update_time, print_tvoc_banner) = "TVOCBanner"_s,
            "TVOCBanner"_s + event<TimeMs>[after_one_second] / (update_time, print_tvoc_value) = "TVOCValue"_s,
            "TVOCValue"_s + event<TimeMs>[after_one_second] / (update_time, print_aqi) = "AQI"_s,
            "AQI"_s + event<TimeMs>[after_one_second] / (update_time, print_temperature) = "TemperatureValue"_s,
            "TemperatureValue"_s + event<TimeMs>[after_one_second] / (update_time, print_humidity) = "HumidityValue"_s,
            "HumidityValue"_s + event<TimeMs>[after_one_second] / (update_time, print_eco2_banner) = "eCO2Banner"_s
            // clang-format on
        );
    }
};

}  // namespace internal

template <class Display>
struct Impl {
    static inline data_models::AirQuality air_quality{};
    static inline data_models::EnvironmentData env{};

    static constexpr void update(const data_models::AirQuality aq) { air_quality = aq; }

    static constexpr void update(const data_models::EnvironmentData e) { env = e; }

    constexpr static auto config = cib::config(  //
        cib::extend<MainLoop>([](const uint32_t current_ms) {
            using dispatch_t = boost::sml::dispatch<boost::sml::back::policies::branch_stm>;

            static boost::sml::sm<rolling_display::internal::StateMachine<Display>, dispatch_t>
                state_machine{air_quality, env};

            state_machine.process_event(data_models::TimeMs{current_ms});
        }));
};
}  // namespace rolling_display

}  // namespace components