#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

#include <boost/sml.hpp>

#include "callbacks.hpp"
#include "core.hpp"
#include "data-models/environment.hpp"
#include "data-models/timing.hpp"

namespace components {

namespace RollingDisplay {
namespace internal {

// Actions
constexpr auto print_tvoc_banner = [](Adafruit_7segment& matrix) {
    matrix.println("tVoC");
    matrix.writeDisplay();
};
constexpr auto print_tvoc_value = [](Adafruit_7segment& matrix,
                                     const data_models::air_quality_t& aq) {
    matrix.println(aq.tvoc);
    matrix.writeDisplay();
};

constexpr auto print_aqi = [](Adafruit_7segment& matrix, const data_models::air_quality_t& aq) {
    using namespace std::string_view_literals;
    for (const char& c : "Aq "sv) {
        matrix.write(c);
    }
    matrix.write('0' + aq.aqi);
    matrix.println();
    matrix.writeDisplay();
};

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

using dispatch_t = boost::sml::dispatch<boost::sml::back::policies::branch_stm>;

}  // namespace internal
}  // namespace RollingDisplay

template <uint8_t i2c_addr = 0x70>
struct SevenSegDisplay {
    static inline Adafruit_7segment matrix{};
    static inline data_models::air_quality_t air_quality{};
    static inline data_models::environment_data_t env{};
    static inline boost::sml::sm<RollingDisplay::internal::StateMachine,
                                 RollingDisplay::internal::dispatch_t>
        rolling_display{matrix, air_quality};

    constexpr static auto init_7seg_display =
        flow::action("init_7seg_display"_sc, []() { matrix.begin(i2c_addr); });

    constexpr static auto print_init_banner = flow::action("print_init_banner"_sc, []() {
        matrix.println("Init");
        matrix.writeDisplay();
    });

    static constexpr void update(const data_models::air_quality_t aq) { air_quality = aq; }

    static constexpr void update(const data_models::environment_data_t e) { env = e; }

    static constexpr void print(const data_models::tvoc_t data) {
        matrix.println("tVoC");
        // matrix.println(data.value);
        matrix.writeDisplay();
    }

    constexpr static auto config = cib::config(  //
        cib::extend<RuntimeInit>(                //
            core::enable_interrupt >> init_7seg_display >> print_init_banner >>
            SerialPort::wait_for_serial  //
            ),                           //
        cib::extend<MainLoop>([](const uint32_t current_ms) {
            rolling_display.process_event(data_models::TimeMs{current_ms});
        }));
};
}  // namespace components