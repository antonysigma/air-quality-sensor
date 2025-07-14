#include "callbacks.hpp"
#include "components/core.hpp"
#include "components/serial.hpp"

using SerialPort = ::components::serial_port::Impl;
#define SERIAL_PORT_IMPL SerialPort
#include "mcu_tests.hpp"

namespace {
struct RegisteredInterfaces {
    static constexpr auto config = cib::config(  //
        cib::exports<RuntimeInit>,               //
        cib::exports<MainLoop>,                  //
        cib::exports<OnTimer0Interrupt>          //
    );
};

using namespace components;

struct Project {
    static constexpr auto config = cib::components<  //
        RegisteredInterfaces,                        //
        core::Impl,                                  //
        SerialPort                                   //
        >;
};

constexpr cib::nexus<Project> nexus{};

}  // namespace

TEST_CASE(SmokeTest) {
    REQUIRE_EQ(1 + 2, 3);
    REQUIRE(1 + 2 == 3);
}

TEST_CASE(SmokeTest2) {
    REQUIRE_EQ(1 + 2, 3);
    REQUIRE(1 * 2 != 1);
}

ISR(TIMER0_COMPA_vect) { nexus.service<OnTimer0Interrupt>(); }
int
main() {
    nexus.service<RuntimeInit>();

    ::mcu_tests::run_all_tests<SerialPort>();

    for (;;) {
    }

    return 0;
}
