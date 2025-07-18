#pragma once
#include <array>

#include "utils/pgm_string.hpp"

namespace mcu_tests {

struct TestCase {
    void (*func)(){nullptr};
    utils::PGMStringHelper name{};
};

struct TestRegistry {
    static constexpr uint16_t max_size = 5;
    static inline std::array<TestCase, max_size> test_case{};
    static inline uint16_t size{0};

    static void addTest(TestCase&& tc) {
        if (size >= max_size) {
            return;
        }

        test_case[size++] = std::move(tc);
    }
};

template <class S>
struct TapReporter {
    static inline uint16_t test_counter{0};

    constexpr TapReporter() { S::print(PSTR2("TAP version 13\n")); }

    constexpr ~TapReporter() {
        S::print(PSTR2("1.."));
        S::print(test_counter);
        S::write('\n');
    }

    TapReporter(const TapReporter&) = delete;
    TapReporter& operator=(const TapReporter&) = delete;
    TapReporter(TapReporter&&) = delete;
    TapReporter& operator=(TapReporter&&) = delete;

    template <bool is_pass>
    static constexpr void print(const utils::PGMStringHelper condition) {
        if constexpr (is_pass) {
            S::print(PSTR2("ok "));
        } else {
            S::print(PSTR2("not ok "));
        }

        S::print(++test_counter);
        S::print(PSTR2(" - "));
        S::print(condition);
        S::write('\n');
    }

    static constexpr void pass(utils::PGMStringHelper condition) { print<true>(condition); }

    static constexpr void fail(utils::PGMStringHelper condition) { print<false>(condition); }
};

template <class S>
constexpr void
runAllTests() {
    TapReporter<S> reporter{};

    // std::for_each(TestRegistry::test_case.begin(),
    // TestRegistry::test_case.begin() + TestRegistry::size,
    //[](const auto& tc) {
    for (const auto& tc : TestRegistry::test_case) {
        if (tc.func == nullptr) {
            break;
        }
        S::print(PSTR2("# "));
        S::print(tc.name);
        S::write('\n');
        tc.func();
    }
}

template <typename T>
constexpr inline void
requireEqImpl(T lhs, T rhs, const utils::PGMStringHelper line_info) {
    static_assert(std::is_integral_v<T>);

    if (lhs == rhs) [[likely]] {
        TapReporter<SERIAL_PORT_IMPL>::pass(line_info);
    } else {
        SERIAL_PORT_IMPL::print(PSTR2("# "));
        SERIAL_PORT_IMPL::print(lhs);
        SERIAL_PORT_IMPL::print(PSTR2(" == "));
        SERIAL_PORT_IMPL::print(rhs);
        SERIAL_PORT_IMPL::write('\n');
        TapReporter<SERIAL_PORT_IMPL>::fail(line_info);
    }
}

}  // namespace mcu_tests

#define TEST_CASE(name)                                                                       \
    void name##_impl();                                                                       \
    void name##_register() {                                                                  \
        ::mcu_tests::TestRegistry::addTest(::mcu_tests::TestCase{name##_impl, PSTR2(#name)}); \
    }                                                                                         \
    const auto name##_registered = (name##_register(), true);                                 \
    void name##_impl()

// end define TEST_CASE

#define LINESTR1(file, line) file ":" #line
#define LINESTR(file, line) LINESTR1(file, line)
#define LINE LINESTR(__FILE__, __LINE__)

#define REQUIRE(cond)                                                    \
    {                                                                    \
        const auto line_info = (PSTR2(LINE " " #cond));                  \
        if (cond) [[likely]] {                                           \
            ::mcu_tests::TapReporter<SERIAL_PORT_IMPL>::pass(line_info); \
        } else {                                                         \
            ::mcu_tests::TapReporter<SERIAL_PORT_IMPL>::fail(line_info); \
        }                                                                \
    }

#define REQUIRE_EQ(lhs, rhs) \
    ::mcu_tests::requireEqImpl((lhs), (rhs), PSTR2(LINE " " #lhs " == " #rhs));
