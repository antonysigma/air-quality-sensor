#include "dsu_funcs2.hpp"
#define VISIBLE __attribute__((visibility("default")))
#ifdef DSU_OVERRIDE_GCC
extern "C" {
VISIBLE uint32_t __umulhisi3(uint16_t x, uint16_t y) {
    return dsu::offloadToCoProcessor<dsu::XMULUU, uint32_t>(x, y);
}

VISIBLE int32_t __mulhisi3(int16_t x, int16_t y) {
    return dsu::offloadToCoProcessor<dsu::XMULSS, int32_t>(x, y);
}

VISIBLE int32_t __mulsi3(int32_t x, int32_t y) {
    return dsu::mulsi3(x, y);
}
}
#endif
