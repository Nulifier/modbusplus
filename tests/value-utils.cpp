#include "../src/value-utils.hpp"
#include <gtest/gtest.h>

TEST(value_utils, map_byte_order_16) {
	using namespace value_utils;

	const uint16_t value = 0x1234;

	EXPECT_EQ(map_byte_order(value, Mapping::ValueDefOrder::ab), 0x1234);
	EXPECT_EQ(map_byte_order(value, Mapping::ValueDefOrder::ba), 0x3412);
}

TEST(value_utils, unmap_byte_order_16) {
	using namespace value_utils;

	const uint16_t value = 0x1234;

	EXPECT_EQ(unmap_byte_order(value, Mapping::ValueDefOrder::ab), 0x1234);
	EXPECT_EQ(unmap_byte_order(value, Mapping::ValueDefOrder::ba), 0x3412);
}

TEST(value_utils, map_byte_order_32) {
	using namespace value_utils;

	const uint32_t value = 0x12345678;

	EXPECT_EQ(map_byte_order(value, Mapping::ValueDefOrder::abcd), 0x12345678);
	EXPECT_EQ(map_byte_order(value, Mapping::ValueDefOrder::dcba), 0x78563412);
	EXPECT_EQ(map_byte_order(value, Mapping::ValueDefOrder::badc), 0x34127856);
	EXPECT_EQ(map_byte_order(value, Mapping::ValueDefOrder::cdab), 0x56781234);
}

TEST(value_utils, unmap_byte_order_32) {
	using namespace value_utils;

	const uint32_t value = 0x12345678;

	EXPECT_EQ(
		unmap_byte_order((uint32_t)0x12345678, Mapping::ValueDefOrder::abcd),
		value);
	EXPECT_EQ(
		unmap_byte_order((uint32_t)0x78563412, Mapping::ValueDefOrder::dcba),
		value);
	EXPECT_EQ(
		unmap_byte_order((uint32_t)0x34127856, Mapping::ValueDefOrder::badc),
		value);
	EXPECT_EQ(
		unmap_byte_order((uint32_t)0x56781234, Mapping::ValueDefOrder::cdab),
		value);
}

TEST(value_utils, map_byte_order_64) {
	using namespace value_utils;

	const uint64_t value = 0x0123456789ABCDEF;

	EXPECT_EQ(map_byte_order(value, Mapping::ValueDefOrder::abcdefgh),
			  0x0123456789ABCDEF);
}

TEST(value_utils, unmap_byte_order_64) {
	using namespace value_utils;

	const uint64_t value = 0x0123456789ABCDEF;

	EXPECT_EQ(unmap_byte_order(value, Mapping::ValueDefOrder::abcdefgh),
			  0x0123456789ABCDEF);
}
