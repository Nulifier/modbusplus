#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

class Mapping {
   public:
	enum class ValueDefFormat : uint8_t {
		bit,
		u16,
		i16,
		u32,
		i32,
		u64,
		i64,
		f32,
		f64,
		str,
		bitfield
	};

	enum class ValueDefType : uint8_t { input, holding };

	enum class ValueDefOrder : uint8_t {
		a,
		b,
		ab,
		ba,
		abcd,
		dcba,
		badc,
		cdab,
		abcdefgh
	};

	struct ValueDef {
		double scale = 1.0;
		std::string linked;
		uint16_t addr;
		uint16_t length;
		ValueDefFormat format;
		ValueDefType type;
		ValueDefOrder order;
	};

	Mapping(const char* path);

	const ValueDef& getValueDef(const std::string& name) const;

	const std::unordered_map<uint16_t, std::string>& getBitfieldDef(
		const std::string& name) const;

	const std::unordered_map<int64_t, std::string>& getEnumDef(
		const std::string& name) const;

   private:
	std::unordered_map<std::string, ValueDef> m_values;
	std::unordered_map<std::string, std::unordered_map<uint16_t, std::string>>
		m_bitfields;
	std::unordered_map<std::string, std::unordered_map<int64_t, std::string>>
		m_enums;
};
