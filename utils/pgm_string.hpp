#pragma once
#include <cstdint>
#include <string_view>

#include "clamp.hpp"

namespace utils {

struct PGMStringHelper : std::string_view {};
}  // namespace utils

#define PSTR2(s)                                                                              \
    utils::PGMStringHelper {                                                                  \
        []() {                                                                                \
            static constexpr PROGMEM char message[]{s};                                       \
            constexpr size_t length = utils::Max(static_cast<int32_t>(std::size(s)) - 1, 0L); \
            return std::string_view{&(message[0]), length};                                   \
        }()                                                                                   \
    }

// end define PSTR2
