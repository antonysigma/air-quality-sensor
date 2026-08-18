// Boost-SML must be included before Arduino.h
#include <boost/sml.hpp>
#include <cmath>

//
#include "callbacks.hpp"
#include "components/blink.hpp"
#include "components/core.hpp"
#include "components/gas_sensor.hpp"
#include "components/i2c_port.hpp"
#include "components/rolling-display.hpp"
#include "components/serial.hpp"
#include "components/seven-segment-display.hpp"
#include "components/temperature_sensor.hpp"
#include "components/the_main_app.hpp"
#include "components/gpio.hpp"

namespace {
struct RegisteredInterfaces {
    static constexpr auto config = cib::config(  //
        cib::exports<RuntimeInit>,               //
        cib::exports<MainLoop>,                  //
        cib::exports<OnTimer0Interrupt>          //
    );
};

using namespace components;

// clang-format off
using LedPin = Pin<'B', 5>;
using Timer1Pin = Pin<'B', 2>;
using Timer2Pin = Pin<'D', 3>;

static_assert(unique_pins<LedPin, Timer1Pin, Timer2Pin>);

using LED = GPIO<LedPin>;
using Heartbeat = Blink<LED>;
using SerialPort = serial_port::Impl;
using I2CPort = i2c_port::Impl;
using TS = TemperatureSensor<Heartbeat, I2CPort>;
using GS = GasSensor<Heartbeat, I2CPort>;
using SevenSegDisplay = seven_seg_display::Impl<I2CPort>;
using RollingDisplay = rolling_display::Impl<SevenSegDisplay>;

struct Project {
    static constexpr auto config = cib::components<
        RegisteredInterfaces,
        core::Impl,
        SerialPort,
        Heartbeat,
        I2CPort,
        TS,
        GS,
        SevenSegDisplay,
        RollingDisplay,
        controllers::TheMainApp<TS, GS, RollingDisplay, SerialPort>
    >;
};
// clang-format on

constexpr cib::nexus<Project> nexus{};

}  // namespace

ISR(TIMER0_COMPA_vect) { nexus.service<OnTimer0Interrupt>(); }

int
main() {
    nexus.service<RuntimeInit>();

    for (;;) {
        nexus.service<MainLoop>(WallClock::now());
    }

    return 0;
}
