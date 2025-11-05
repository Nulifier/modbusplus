#pragma once

#include <cstdint>
#include <vector>
#include "mapping.hpp"

namespace value_utils {
std::vector<uint16_t> pack_coils_to_u16(const uint8_t* coils, int nb);

inline uint16_t map_byte_order(uint16_t value,
							   Mapping::ValueDefOrder ab_order) {
	if (ab_order == Mapping::ValueDefOrder::ba) {
		return (value >> 8) | (value << 8);
	}
	return value;
}

inline uint16_t unmap_byte_order(uint16_t value,
								 Mapping::ValueDefOrder ab_order) {
	// Same as map_byte_order since swapping is symmetric
	return map_byte_order(value, ab_order);
}

inline uint32_t map_byte_order(uint32_t value, Mapping::ValueDefOrder order) {
	const uint32_t a = value & 0xFF000000;
	const uint32_t b = value & 0x00FF0000;
	const uint32_t c = value & 0x0000FF00;
	const uint32_t d = value & 0x000000FF;

	switch (order) {
		case Mapping::ValueDefOrder::abcd:
			// No change needed
			return value;
		case Mapping::ValueDefOrder::dcba:
			return (d << 24) | (c << 8) | (b >> 8) | (a >> 24);
		case Mapping::ValueDefOrder::badc:
			return (b << 8) | (a >> 8) | (d << 8) | (c >> 8);
		case Mapping::ValueDefOrder::cdab:
			return ((c | d) << 16) | ((a | b) >> 16);
		default:
			return value;  // Unsupported order, return as is
	}
}

inline uint32_t unmap_byte_order(uint32_t value, Mapping::ValueDefOrder order) {
	const uint32_t a = value & 0xFF000000;
	const uint32_t b = value & 0x00FF0000;
	const uint32_t c = value & 0x0000FF00;
	const uint32_t d = value & 0x000000FF;

	switch (order) {
		case Mapping::ValueDefOrder::abcd:
			// No change needed
			return value;
		case Mapping::ValueDefOrder::dcba:
			return (a >> 24) | (b >> 8) | (c << 8) | (d << 24);
		case Mapping::ValueDefOrder::badc:
			return (a >> 8) | (b << 8) | (c >> 8) | (d << 8);
		case Mapping::ValueDefOrder::cdab:
			return ((a | b) >> 16) | ((c | d) << 16);
		default:
			return value;  // Unsupported order, return as is
	}
}

inline uint64_t map_byte_order(uint64_t value, Mapping::ValueDefOrder order) {
	const uint64_t a = value & 0xFF00000000000000;
	const uint64_t b = value & 0x00FF000000000000;
	const uint64_t c = value & 0x0000FF0000000000;
	const uint64_t d = value & 0x000000FF00000000;
	const uint64_t e = value & 0x00000000FF000000;
	const uint64_t f = value & 0x0000000000FF0000;
	const uint64_t g = value & 0x000000000000FF00;
	const uint64_t h = value & 0x00000000000000FF;

	switch (order) {
		case Mapping::ValueDefOrder::abcdefgh:
			// No change needed
			return value;
		default:
			return value;  // Unsupported order, return as is
	}
}

inline uint64_t unmap_byte_order(uint64_t value, Mapping::ValueDefOrder order) {
	switch (order) {
		case Mapping::ValueDefOrder::abcdefgh:
			// No change needed
			return value;
		default:
			return value;  // Unsupported order, return as is
	}
}
}  // namespace value_utils
