#include "value-utils.hpp"

std::vector<uint16_t> value_utils::pack_coils_to_u16(const uint8_t* coils,
													 int nb) {
	int byte_count = (nb + 7) / 8;
	std::vector<uint16_t> packed(byte_count, 0);

	for (int i = 0; i < byte_count; ++i) {
		uint8_t b = 0;
		for (int bit = 0; bit < 8; ++bit) {
			int idx = i * 8 + bit;
			if (idx < nb && coils[idx]) {
				b |= (1u << bit);  // LSB-first
			}
		}
		packed[i] = static_cast<uint16_t>(b);  // Upper bits remain 0
	}

	return packed;
}
