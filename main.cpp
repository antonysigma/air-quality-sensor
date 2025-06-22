// Boost-SML must be included before Arduino.h
#include <boost/sml.hpp>
#include <cmath>

#include "callbacks.hpp"
#include "components/blink.hpp"
#include "components/core.hpp"
#include "components/gas_sensor.hpp"
#include "components/i2c_port.hpp"
#include "components/serial.hpp"

namespace {
struct registered_interfaces {
    static constexpr auto config = cib::config(  //
        cib::exports<RuntimeInit>,               //
        cib::exports<MainLoop>,                  //
        cib::exports<OnTimer0Interrupt>          //
    );
};

using namespace components;

using Heartbeat = Blink<13>;

struct project {
    static constexpr auto config = cib::components<  //
        registered_interfaces,                       //
        core::impl,                                  //
        SerialPort,                                  //
        Heartbeat,                                   //
        I2CPort,
        GasSensor<Heartbeat>  //
        >;
};

cib::nexus<project> nexus{};

}  // namespace

ISR(TIMER0_COMPA_vect) { nexus.service<OnTimer0Interrupt>(); }

int
main() {
    nexus.service<RuntimeInit>();

    for (;;) {
        const auto current_ms = components::Millis();
        nexus.service<MainLoop>(current_ms);
    }

    return 0;
}