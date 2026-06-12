#include "aeolipile.hpp"
#include "types.hpp"

int main() {
  aeo::result_t result = aeo::initialize_early_infrastructure();
  if (true == aeo::is_error(result)) {
    return -1;
  }
  return 0;
}
