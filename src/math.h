#ifndef MATH_H
#define MATH_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

inline u32 next_power_of_two(u32 x) {
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return x + 1;
}

extern inline u32 next_power_of_two(u32 x);

#ifdef __cplusplus
}
#endif
#endif // MATH_H
