#pragma once
#include "types.hpp"
#include <medusa.hpp>

namespace aeo {
namespace logging {

result_t initialize();
result_t deinitialize();

}; // namespace logging
}; // namespace aeo

#define LOGL(format, ...)                                                      \
  medusa::write_log(medusa::log_level::log, format __VA_OPT__(, ) __VA_ARGS__)
#define LOGD(format, ...)                                                      \
  medusa::write_log(medusa::log_level::log, format __VA_OPT__(, ) __VA_ARGS__)
#define LOGP(format, ...)                                                      \
  medusa::write_log(medusa::log_level::paranoid,                               \
                    format __VA_OPT__(, ) __VA_ARGS__)
#define LOGW(format, ...)                                                      \
  medusa::write_log(medusa::log_level::warning,                                \
                    format __VA_OPT__(, ) __VA_ARGS__)
#define LOGF(format, ...)                                                      \
  medusa::write_log(medusa::log_level::fatal, format __VA_OPT__(, ) __VA_ARGS__)

#define TLOGL(format, ...)                                                     \
  medusa::write_timed_log(medusa::log_level::log,                              \
                          format __VA_OPT__(, ) __VA_ARGS__)
#define TLOGD(format, ...)                                                     \
  medusa::write_timed_log(medusa::log_level::debug,                            \
                          format __VA_OPT__(, ) __VA_ARGS__)
#define TLOGP(format, ...)                                                     \
  medusa::write_timed_log(medusa::log_level::paranoid,                         \
                          format __VA_OPT__(, ) __VA_ARGS__)
#define TLOGW(format, ...)                                                     \
  medusa::write_timed_log(medusa::log_level::warning,                          \
                          format __VA_OPT__(, ) __VA_ARGS__)
#define TLOGF(format, ...)                                                     \
  medusa::write_timed_log(medusa::log_level::fatal,                            \
                          format __VA_OPT__(, ) __VA_ARGS__)
