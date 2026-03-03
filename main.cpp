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
#include "pdaqp-solver.h"

namespace {
struct RegisteredInterfaces {
    static constexpr auto config = cib::config(  //
        cib::exports<RuntimeInit>,               //
        cib::exports<MainLoop>,                  //
        cib::exports<OnTimer0Interrupt>          //
    );
};

using namespace components;

using Heartbeat = Blink<13>;
using SerialPort = serial_port::Impl;
using I2CPort = i2c_port::Impl;
using TS = TemperatureSensor<Heartbeat, I2CPort>;
using GS = GasSensor<Heartbeat, I2CPort>;
using SevenSegDisplay = seven_seg_display::Impl<I2CPort>;
using RollingDisplay = rolling_display::Impl<SevenSegDisplay>;

struct Project {
    static constexpr auto config = cib::components<  //
        RegisteredInterfaces,                        //
        core::Impl,                                  //
        SerialPort,                                  //
        Heartbeat,                                   //
        I2CPort,                                     //
        TS,                                          //
        GS,                                          //
        SevenSegDisplay,                             //
        RollingDisplay,                              //
        controllers::TheMainApp<TS, GS, RollingDisplay, SerialPort>>;
};

constexpr cib::nexus<Project> nexus{};

}  // namespace

ISR(TIMER0_COMPA_vect) { nexus.service<OnTimer0Interrupt>(); }

int
main() {
    nexus.service<RuntimeInit>();

    static_assert(sizeof(Parameter) % 2 == 0);
    const uint16_t raw[sizeof(Parameter) / 2] PROGMEM {};
    Parameter parameter{};

    std::transform(raw, raw + sizeof(Parameter), reinterpret_cast<uint16_t*>(&parameter), [](const uint16_t& c) {
        return pgm_read_u16(&c);
    });
    const auto solution = pdaqpSolver<true>(std::move(parameter));

    const auto* bytes{reinterpret_cast<const char*>(&solution)};
    std::for_each(bytes, bytes + sizeof(solution), [](const char& c) {
        SerialPort::write(c);
    });
    //for (;;) {
    //    const auto current_ms = components::Millis();
    //    nexus.service<MainLoop>(current_ms);
    //}

    return 0;
}