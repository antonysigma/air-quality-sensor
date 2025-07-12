#pragma once
#include <cstdint>

namespace dsu {

enum Registry : uint8_t {
    DSIR = 0x01,
    DSSD = 0x02,
    DSDX = 0x10,
    DSDY = 0x11,
    DSAL = 0x38,
    DSAH = 0x39
};

enum OpCode : uint8_t {
    // Multiply
    XMULUU = 0b0100'0100,
    XMULSS = 0b0111'0100,
    XMULSU = 0b0110'0100,

    // multiply add
    XMADUU = 0b010'00110,
    XMADSS = 0b011'10110,
    XMADSU = 0b011'00110,

    // Division
    DIV = 0b1011'0000,

    // Modulus
    MOD = 0b1011'0001,

    // Clear accumulator
    CLRA = 0b1000'0000,

    // Bit shifts
    SHRA = 0b1100'0000,
    SHLA = 0b1101'0000,
    SALA = 0b1111'0000
};

template <typename T>
concept Int32 = std::is_integral_v<T> && (sizeof(T) == sizeof(uint32_t));

template <typename T>
concept Int16 = std::is_integral_v<T> && (sizeof(T) == sizeof(uint16_t));

template <OpCode opcode, Int32 T, Int16 U, Int16 V>
inline T
offloadToCoProcessor(U x, V y) {
    T result;
    asm volatile(
        // Send address
        "out %[dsdx], %A[operand_x] \n\t"
        "out %[dsdy], %A[operand_y]   \n\t"
        // Send opcode
        "ldi r22, %[opcode]       \n\t"
        "out %[dsir], r22         \n\t"
        // Read result
        "in %A[result], %[dsal]          \n\t"
        "in %C[result], %[dsah]          \n\t"
        : [result] "=r"(result)
        : [operand_x] "r"(x),    //
          [operand_y] "r"(y),    //
          [opcode] "M"(opcode),  //
          [dsdx] "I"(DSDX),      //
          [dsdy] "I"(DSDY),      //
          [dsir] "I"(DSIR),      //
          [dsal] "I"(DSAL),      //
          [dsah] "I"(DSAH)
        : "r22");
    return result;
}

inline uint32_t
mulsi3(uint32_t x, uint32_t y) {
    int32_t result;
    asm volatile(
        // First partial product: x_lo * y_hi
        R"""(out %[dsdx], %A[x]
     out %[dsdy], %C[y]
     ldi r24, %[xmuluu]
     out %[dsir], r24
)"""

        // Second partial product: x_hi * y_lo
        R"(out %[dsdx], %C[x]
     out %[dsdy], %A[y]
     ldi r24, %[xmaduu]
     out %[dsir], r24
)"

        // Shift left 8 bits twice
        R"(ldi r25, %[shra8]
     out %[dsir], r25
     out %[dsir], r25
)"
        ""

        // Final product: x_lo * y_lo
        R"(out %[dsdx], %A[x]
     out %[dsdy], %A[y]
     out %[dsir], r24
)"

        // Read result
        R"(in %A[result], %[dsal]
     in %C[result], %[dsah]
)"
        ""
        : [result] "=r"(result)
        : [x] "r"(x),            //
          [y] "r"(y),            //
          [dsdx] "I"(DSDX),      //
          [dsdy] "I"(DSDY),      //
          [dsir] "I"(DSIR),      //
          [dsal] "I"(DSAL),      //
          [dsah] "I"(DSAH),      //
          [xmuluu] "M"(XMULUU),  //
          [xmaduu] "M"(XMADUU),  //
          [shra8] "M"(SHRA | 8)
        : "r24", "r25");
    return result;
}

constexpr auto xmuluu = [](uint16_t x, uint16_t y) {
    return offloadToCoProcessor<XMULUU, uint32_t>(x, y);
};
constexpr auto xmulsu = [](int16_t x, uint16_t y) {
    return offloadToCoProcessor<XMULSU, int32_t>(x, y);
};
constexpr auto xmulss = [](int16_t x, int16_t y) {
    return offloadToCoProcessor<XMULSS, int32_t>(x, y);
};

}  // namespace dsu