#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

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
        auto afterOneSecond = [&](const TimeMs current_ms) -> bool {
            constexpr auto update_interval_ms = 1000;
            return current_ms.value - prev_ms >= update_interval_ms;
        };

        // Actions
        auto update_time = [&](const TimeMs current_ms) { prev_ms = current_ms.value; };

        // Actions
        constexpr auto print_tvoc_banner = []() {
            using namespace std::string_view_literals;
            Display::print("tVoC"sv);
        };

        constexpr auto print_tvoc_value = [](const data_models::air_quality_t& aq) {
            Display::print(data_models::tvoc_t{aq});
        };

        constexpr auto print_aqi = [](const data_models::air_quality_t& aq) {
            using namespace std::string_view_literals;
            Display::print("Aq "sv);
            Display::print(data_models::aqi_t{aq});
        };

        using namespace boost::sml;
        return make_transition_table(
            // clang-format off
            *"Begin"_s + event<TimeMs> / update_time = "Init"_s,
            "Init"_s + event<TimeMs>[afterOneSecond] / (update_time, print_tvoc_banner) = "TVOCBanner"_s,
            "TVOCBanner"_s + event<TimeMs>[afterOneSecond] / (update_time, print_tvoc_value) = "TVOCValue"_s,
            "TVOCValue"_s + event<TimeMs>[afterOneSecond] / (update_time, print_aqi) = "AQI"_s,
            "AQI"_s + event<TimeMs>[afterOneSecond] / (update_time, print_tvoc_banner) = "TVOCBanner"_s
            // clang-format on
        );
    }
};

}  // namespace internal

template <class Display>
struct impl {
    static inline data_models::air_quality_t air_quality{};
    static inline data_models::environment_data_t env{};

    static constexpr void update(const data_models::air_quality_t aq) { air_quality = aq; }

    static constexpr void update(const data_models::environment_data_t e) { env = e; }

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