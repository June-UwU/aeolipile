#pragma once
#include <exception>

#define AEO_ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define AEO_ASSERT(predicate, format, ...)                                     \
  if (false == (predicate)) {                                                  \
    TLOGF(format, __VA_ARGS__);                                                \
    std::terminate();                                                          \
  }
