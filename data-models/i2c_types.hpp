#pragma once
#include <cstdint>

namespace i2c_types {

/** Table 21-3: Status codes for master transmitter mode */
enum status_code_t : uint8_t {
    SUCCESS = 0,
    START_TRANSMITTED = 0x08,
    REPEATED_START_TRANSMITTED = 0x10,
    SLA_W_ACK = 0x18,
    SLA_W_NACK = 0x20,
    DATA_TRANSMITTED_ACK = 0x28,
    DATA_TRANSMITTED_NACK = 0x30,
    ARBITRATION_LOST = 0x38
};

}  // namespace i2c_types