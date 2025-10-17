#pragma once

#include <cstdint>
#include <vector>

namespace value_utils {
std::vector<uint16_t> pack_coils_to_u16(const uint8_t* coils, int nb);
}
