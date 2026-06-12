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
constexpr const result_t E_FAIL = 0x2;

inline const char *result_to_string(result_t result) {
  const char *error_string = nullptr;
  switch (result) {
  case SUCCESS:
    error_string = "success";
    break;
  case -E_INVAL:
    error_string = "invalid arguments";
    break;
  case -E_FAIL:
    error_string = "failed";
    break;
  default:
    error_string = "unknown error";
  }

  return error_string;
}

inline bool is_error(result_t result) {
  bool is_error = true;
  switch (result) {
  case SUCCESS:
    is_error = false;
    break;
  case -E_INVAL:
  case -E_FAIL:
  default:
    is_error = true;
    break;
  }

  return is_error;
}
} // namespace aeo
