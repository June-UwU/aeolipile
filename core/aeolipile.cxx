#include "aeolipile.hpp"
#include "log.hpp"

namespace aeo {
result_t initialize_early_infrastructure() {
  aeo::logging::initialize();
  TLOGL("%s", "early initialization\n");
  return SUCCESS;
}
}; // namespace aeo
