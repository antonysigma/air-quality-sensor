#pragma once
#include <cstdint>

extern "C" {

uint16_t dsu_muluu(uint16_t, uint16_t);   // NOLINT(readability-identifier-naming)
uint32_t dsu_xmuluu(uint16_t, uint16_t);  // NOLINT(readability-identifier-naming)
int32_t dsu_xmulss(int16_t, int16_t);     // NOLINT(readability-identifier-naming)
}