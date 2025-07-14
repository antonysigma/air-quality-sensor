#include "callbacks.hpp"
#include "components/core.hpp"
#include "components/serial.hpp"
#include "dsu_funcs2.hpp"

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

ISR(TIMER0_COMPA_vect) { nexus.service<OnTimer0Interrupt>(); }

#define unlikely(expr) __builtin_expect(!!(expr), 0)

TEST_CASE(UInt32Mul) {
    using dsu::xmuluu;
    REQUIRE_EQ(xmuluu(3, 5), 15);

    for (uint16_t x = 0; x < 512; x++) {
        for (uint16_t y = 0; y < 512; y++) {
            if (unlikely(xmuluu(x, y) != static_cast<uint32_t>(x) * y)) {
                REQUIRE_EQ(xmuluu(x, y), static_cast<uint32_t>(x) * y);
                return;
            }
        }
    }
}

TEST_CASE(Int32Mul) {
    using dsu::xmulss;

    for (int16_t x = -50; x < 50; x++) {
        for (int16_t y = -50; y < 50; y++) {
            if (unlikely(xmulss(x, y) != static_cast<int32_t>(x) * y)) {
                REQUIRE_EQ(xmulss(x, y), static_cast<int32_t>(x) * y);
                return;
            }
        }
    }
}

TEST_CASE(Int32_UInt32Mul) {
    using dsu::xmulsu;
    for (int16_t x = -50; x < 50; x++) {
        for (uint16_t y = 0; y < 100; y++) {
            if (unlikely(xmulsu(x, y) != static_cast<int32_t>(x) * y)) {
                REQUIRE_EQ(xmulsu(x, y), static_cast<int32_t>(x) * y);
                return;
            }
        }
    }
}

TEST_CASE(Mulsi3Tests) {
    using dsu::mulsi3;
    for (uint32_t x = 10'000; x < 10'512; x++) {
        for (uint32_t y = 10'000; y < 10'512; y++) {
            if (unlikely(mulsi3(x, y) != static_cast<int32_t>(x) * y)) {
                REQUIRE_EQ(mulsi3(x, y), static_cast<int32_t>(x) * y);
                return;
            }
        }
    }
}

int
main() {
    {
        volatile uint8_t& DCSR{*reinterpret_cast<volatile uint8_t*>(0x20)};
        DCSR = DCSR | (1 << 7);
    }
    nexus.service<RuntimeInit>();

    ::mcu_tests::run_all_tests<SerialPort>();

    for (;;) {
    }

    return 0;
}
