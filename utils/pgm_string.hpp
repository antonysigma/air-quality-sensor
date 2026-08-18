#pragma once
#include <cstdint>
#include <string_view>

#include "clamp.hpp"

namespace utils {

template <std::size_t N>
struct PGMString {
    std::array<char, N - 1> buffer{};

    consteval PGMString(const char (&s)[N]) { std::copy_n(s, N - 1, buffer.begin()); }
};

struct PGMStringHelper : std::string_view {};
}  // namespace utils

#define PSTR2(s)                                                                   \
    utils::PGMStringHelper {                                                       \
        []() {                                                                     \
            static const PROGMEM utils::PGMString message{s};                      \
            return std::string_view{message.buffer.data(), message.buffer.size()}; \
        }()                                                                        \
    }

// end define PSTR2
