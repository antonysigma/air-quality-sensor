#pragma once
#include <span>

#include "callbacks.hpp"
#include "config.h"
#include "core.hpp"
#include "data-models/i2c_types.hpp"
#include "serial.hpp"

namespace components {

namespace i2c_port {

struct Prescaler {
    uint8_t raw{};
    constexpr explicit Prescaler(uint8_t rate)
        : raw{[=]() -> uint8_t {
              // Table 21-8: TWI bit rate prescaler
              switch (rate) {
                  case 1:
                      return 0b00;
                  case 4:
                      return 0b01;
                  case 16:
                      return 0b10;
                  case 64:
                      return 0b11;
                  default: {
                      __builtin_unreachable();
                      return 0;
                  }
              }
          }()} {}

    [[nodiscard]] constexpr uint8_t value() const {
        return 1u << (raw * 2);  // NOLINT(hicpp-signed-bitwise)
    }
};
static_assert(Prescaler{1}.value() == 1);
static_assert(Prescaler{4}.value() == 4);
static_assert(Prescaler{16}.value() == 16);
static_assert(Prescaler{64}.value() == 64);

template <Prescaler prescaler = 1, units::KiloHertz scl_freq>
constexpr uint8_t
computeI2CFrequency() {
    // Section 21.5.2 Bit rate generator unit
    return static_cast<uint8_t>((system_freq / scl_freq - 16) / 2 / prescaler.value());
}
static_assert(computeI2CFrequency<Prescaler{1}, 100.0_kHz>() == 72);

union ControlRegister {
    uint8_t buffer{};
    struct {
        // Section 21.7.1 Master transmitter mode
        uint8_t twie : 1 = 0;
        uint8_t : 1;
        uint8_t twen : 1 = 0;
        uint8_t twwc : 1 = 0;
        uint8_t twsto : 1 = 0;
        uint8_t twsta : 1 = 0;
        uint8_t twea : 1 = 0;
        uint8_t twint : 1 = 0;
    } fields;
};

void
wait() {
    while (!(ControlRegister{TWCR}.fields.twint)) {
    }
}

void
start() {
    TWCR = ControlRegister{.fields = {.twen = 1, .twsta = 1, .twint = 1}}.buffer;  // Send START
}

template <bool is_read>
void
writeAddress(const uint8_t address) {
    TWDR = (address << 1u) | is_read;                                  // LSB=0 for write
    TWCR = ControlRegister{.fields = {.twen = 1, .twint = 1}}.buffer;  // Start transmission
}

void
writeTo(const uint8_t data) {
    TWDR = data;
    TWCR = ControlRegister{.fields = {.twen = 1, .twint = 1}}.buffer;  // Start transmission
}

template <bool ack>
uint8_t
readFrom() {
    TWCR = ControlRegister{.fields = {.twen = 1, .twea = ack, .twint = 1}}.buffer;
    wait();
    return TWDR;
}

void
stop() {
    TWCR = ControlRegister{.fields = {.twen = 1, .twsto = 1, .twint = 1}}.buffer;  // Send STOP
}

struct Impl {
    constexpr static auto setup_i2c = flow::action("setup_i2c"_sc, []() {
        constexpr auto prescaler = Prescaler{1};
        TWSR = prescaler.value();
        TWBR = computeI2CFrequency<prescaler, 100.0_kHz>();
        TWCR = ControlRegister{.fields = {.twen = 1}}.buffer;
    });

    template <class Message>
    constexpr static i2c_types::status_code_t send(const uint8_t i2c_addr, const Message message) {
        start();
        wait();
        writeAddress<false>(i2c_addr);
        wait();
        for (const auto& c : std::span<const uint8_t>{reinterpret_cast<const uint8_t*>(&message),
                                                      sizeof(Message)}) {
            writeTo(c);
            wait();
        }

        // Table 21.2: Assembly code example
        auto status_code = static_cast<i2c_types::status_code_t>(TWSR & 0xF8);
        stop();
        return status_code;
    }

    template <class Message>
    constexpr static Message read(const uint8_t i2c_addr) {
        Message payload{};

        start();
        wait();
        writeAddress<true>(i2c_addr);
        wait();

        auto buffer = std::span<uint8_t>{reinterpret_cast<uint8_t*>(&payload), sizeof(Message)};
        for (auto iter = buffer.begin(); iter != buffer.end(); ++iter) {
            *iter = (iter + 1 != buffer.end()) ? readFrom<true>() : readFrom<false>();
            wait();
        }

        stop();
        return payload;
    }

    constexpr static auto config = cib::config(cib::extend<RuntimeInit>(  //
        core::system_clk_init >> setup_i2c >> core::enable_interrupt      //
        ));
};
}  // namespace i2c_port
}  // namespace components