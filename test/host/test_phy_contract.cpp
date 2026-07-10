#include <type_traits>

#include "dali/phy/i_dali_phy.h"

static_assert(std::has_virtual_destructor_v<dali::IDaliPhy>);
static_assert(std::is_abstract_v<dali::IDaliPhy>);

int main() {
    return 0;
}
