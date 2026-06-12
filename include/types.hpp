#pragma once
#include <cstdint>

namespace aeo {

using result_t = int64_t;
using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

constexpr const result_t SUCCESS = 0x0;
constexpr const result_t E_INVAL = 0x1;

inline bool is_error(result_t result) {
  bool is_error = true;
  switch (result) {
  case SUCCESS:
    is_error = false;
    break;
  case -E_INVAL:
    is_error = true;
    break;
  default:
    is_error = true;
    break;
  }

  return is_error;
}
} // namespace aeo
