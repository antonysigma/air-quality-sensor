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

TEST_CASE(UInt16Mul) {
    using dsu::xmuluu;
    REQUIRE_EQ(xmuluu(3, 5), 15UL);

    for (uint16_t x = 0; x < 512; x++) {
        for (uint16_t y = 0; y < 512; y++) {
            if (xmuluu(x, y) != static_cast<uint32_t>(x) * y) [[unlikely]] {
                REQUIRE_EQ(xmuluu(x, y), static_cast<uint32_t>(x) * y);
                return;
            }
        }
    }
}

TEST_CASE(Int16Mul) {
    using dsu::xmulss;
    REQUIRE_EQ(xmulss(-3, 5), -15L);

    for (int16_t x = -50; x < 50; x++) {
        for (int16_t y = -50; y < 50; y++) {
            if (xmulss(x, y) != static_cast<int32_t>(x) * y) [[unlikely]] {
                REQUIRE_EQ(xmulss(x, y), static_cast<int32_t>(x) * y);
                return;
            }
        }
    }
}

TEST_CASE(Int32_UInt32Mul) {
    using dsu::xmulsu;
    REQUIRE_EQ(xmulsu(-5, 3), -15L);

    for (int16_t x = -50; x < 50; x++) {
        for (uint16_t y = 0; y < 100; y++) {
            const auto expected = static_cast<int32_t>(x) * y;
            if (xmulsu(x, y) != expected) [[unlikely]] {
                REQUIRE_EQ(xmulsu(x, y), expected);
                return;
            }
        }
    }
}

TEST_CASE(Int32Mul) {
    using dsu::mulsi3;
    REQUIRE_EQ(mulsi3(-3L, 5L), -15L);

    constexpr int32_t offset{10'000};
    for (int32_t x = offset; x < offset + 512; x++) {
        for (int32_t y = offset; y < offset + 512; y++) {
            const auto expected = x * y;
            if (mulsi3(x, y) != expected) [[unlikely]] {
                std::array<char, 80> buffer;
                snprintf(buffer.data(), buffer.size(), "# %ld x %ld = %ld\n", x, y, expected);
                for(const char& c : buffer) {
                    if(c == '\0') {
                        break;
                    }
                    SerialPort::write(c);
                }
                REQUIRE_EQ(mulsi3(x, y), expected);
                return;
            }
        }
    }
}

int
main() {
    nexus.service<RuntimeInit>();

    ::mcu_tests::runAllTests<SerialPort>();

    for (;;) {
    }

    return 0;
}
