#pragma once
#include <string_view>

namespace utils {

struct PGMStringHelper : std::string_view {};
}  // namespace utils

#define PSTR2(s)                                    \
    utils::PGMStringHelper {                        \
        []() {                                      \
            static const PROGMEM char message[]{s}; \
            return std::string_view{message};       \
        }()                                         \
    }

// end define PSTR2