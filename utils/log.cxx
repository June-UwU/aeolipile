#include "log.hpp"

namespace aeo {
namespace logging {

result_t initialize() {
  medusa::initialize({.log_file = "engine.log", .write_intro = false});
  return SUCCESS;
}

result_t deinitialize() {
  medusa::deinitialize();
  return SUCCESS;
}

}; // namespace logging
}; // namespace aeo
