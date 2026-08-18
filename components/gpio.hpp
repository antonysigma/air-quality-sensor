#pragma once

#include "pin.hpp"

namespace components {

template <typename Pin>
struct GPIO {
    static constexpr void init() { configureOutput<Pin>(); }

    static constexpr void digitalWrite(const bool high) {
        if (high) {
            pin_detail::Port<Pin::port_id>::set(Pin::bit_mask);
        } else {
            pin_detail::Port<Pin::port_id>::clear(Pin::bit_mask);
        }
    }
};

}  // namespace components
