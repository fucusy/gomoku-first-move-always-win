#ifndef MATH_EXTENSION_H
#define MATH_EXTENSION_H

#include <cstdint>

// add two values with respect to infinity being represented as UINT16_MAX
inline uint16_t inf_plus(uint16_t a, uint16_t b) { uint32_t sum = uint32_t(a) + uint32_t(b); return sum > UINT16_MAX ? UINT16_MAX : sum; }

// add two values with respect to infinity being represented as UINT32_MAX
inline uint32_t inf_plus(uint32_t a, uint32_t b) { uint64_t sum = uint64_t(a) + uint64_t(b); return uint32_t(sum > UINT32_MAX ? UINT32_MAX : sum); }

#endif