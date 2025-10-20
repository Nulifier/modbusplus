#include "modbus-device-ctx.hpp"
#include <cstring>
#include <stdexcept>
#include <vector>

ModbusDeviceContext::ModbusDeviceContext(std::shared_ptr<ModbusDevice> device,
										 std::shared_ptr<Mapping>&& mapping)
	: m_device(std::move(device)), m_mapping(std::move(mapping)) {}

void ModbusDeviceContext::luaRead(lua_State* L, const std::string& name) {
	// Get mapping
	const auto& def = m_mapping->getValueDef(name);

	std::vector<uint16_t> regsBuffer(def.length, 0);
	if (def.format != Mapping::ValueDefFormat::bit) {
		// Pre-read registers if needed
		if (def.type == Mapping::ValueDefType::input) {
			m_device->readInputRegisters(def.addr, def.length,
										 regsBuffer.data());
		} else {
			m_device->readRegisters(def.addr, def.length, regsBuffer.data());
		}
	}

	switch (def.format) {
		case Mapping::ValueDefFormat::bit: {
			// Read single bit
			uint8_t value = 0;
			// Since there is only a single bit, we don't need any compatibility
			// flags
			if (def.type == Mapping::ValueDefType::input) {
				m_device->readInputBits(def.addr, 1, &value);
			} else {
				m_device->readBits(def.addr, 1, &value);
			}
			lua_pushboolean(L, value != 0);
			return;
		}
		case Mapping::ValueDefFormat::u16:
		case Mapping::ValueDefFormat::i16: {
			// Read single register
			uint16_t value = regsBuffer[0];

			// Handle byte order if needed (only ab and ba for 16-bit)
			if (def.order == Mapping::ValueDefOrder::ba) {
				value = (value >> 8) | (value << 8);
			}

			int16_t value_s = static_cast<int16_t>(value);

			// Handle scale if not 1.0
			if (def.scale != 1.0) {
				value = static_cast<uint16_t>(static_cast<double>(value) *
											  def.scale);
				value_s = static_cast<int16_t>(static_cast<double>(value_s) *
											   def.scale);
			}

			// If there is a linked enum definition, we should return the
			// corresponding string
			if (!def.linked.empty()) {
				const auto& enumDef = m_mapping->getEnumDef(def.linked);
				int64_t lookupValue = static_cast<int64_t>(value);
				auto it = enumDef.find(lookupValue);
				if (it != enumDef.end()) {
					lua_pushlstring(L, it->second.c_str(), it->second.size());
				} else {
					throw std::runtime_error("Enum value not found for value " +
											 std::to_string(lookupValue) +
											 " in mapping: " + name);
				}
				return;
			}

			if (def.format == Mapping::ValueDefFormat::i16) {
				lua_pushinteger(L, static_cast<int16_t>(value));
			} else {
				lua_pushinteger(L, value);
			}
			return;
		}
		case Mapping::ValueDefFormat::u32:
		case Mapping::ValueDefFormat::i32: {
			// Read two registers
			uint32_t value = (static_cast<uint32_t>(regsBuffer[0]) << 16) |
							 static_cast<uint32_t>(regsBuffer[1]);

			// Handle byte order if needed
			switch (def.order) {
				case Mapping::ValueDefOrder::abcd:
					// No change needed
					break;
				case Mapping::ValueDefOrder::dcba:
					value = value >> 24 | ((value >> 8) & 0x0000FF00) |
							((value << 8) & 0x00FF0000) | (value << 24);
					break;
				case Mapping::ValueDefOrder::badc:
					value = ((value >> 8) & 0x0000FF00) |
							(value << 8) & 0x00FF0000 |
							(value >> 24) & 0x000000FF |
							(value << 24) & 0xFF000000;
					break;
				case Mapping::ValueDefOrder::cdab:
					value = ((value >> 16) & 0x0000FFFF) |
							((value << 16) & 0xFFFF0000);
					break;
				default:
					throw std::runtime_error(
						"Unsupported byte order for 32-bit in luaRead for "
						"mapping: " +
						name);
			}

			int32_t value_s = static_cast<int32_t>(value);

			// Handle scale if not 1.0
			if (def.scale != 1.0) {
				value = static_cast<uint32_t>(static_cast<double>(value) *
											  def.scale);
				value_s = static_cast<int32_t>(static_cast<double>(value_s) *
											   def.scale);
			}

			// If there is a linked enum definition, we should return the
			// corresponding string
			if (!def.linked.empty()) {
				const auto& enumDef = m_mapping->getEnumDef(def.linked);
				int64_t lookupValue = static_cast<int64_t>(value);
				auto it = enumDef.find(lookupValue);
				if (it != enumDef.end()) {
					lua_pushlstring(L, it->second.c_str(), it->second.size());
				} else {
					throw std::runtime_error("Enum value not found for value " +
											 std::to_string(lookupValue) +
											 " in mapping: " + name);
				}
				return;
			}

			if (def.format == Mapping::ValueDefFormat::i32) {
				lua_pushinteger(L, static_cast<int32_t>(value));
			} else {
				lua_pushinteger(L, value);
			}
			return;
		}
		case Mapping::ValueDefFormat::f32: {
			// Read two registers
			uint32_t value_raw = (static_cast<uint32_t>(regsBuffer[0]) << 16) |
								 static_cast<uint32_t>(regsBuffer[1]);

			// Handle byte order if needed
			switch (def.order) {
				case Mapping::ValueDefOrder::abcd:
					// No change needed
					break;
				case Mapping::ValueDefOrder::dcba:
					value_raw =
						value_raw >> 24 | ((value_raw >> 8) & 0x0000FF00) |
						((value_raw << 8) & 0x00FF0000) | (value_raw << 24);
					break;
				case Mapping::ValueDefOrder::badc:
					value_raw = ((value_raw >> 8) & 0x0000FF00) |
								(value_raw << 8) & 0x00FF0000 |
								(value_raw >> 24) & 0x0000FFFF |
								(value_raw << 24);
					break;
				case Mapping::ValueDefOrder::cdab:
					value_raw = ((value_raw >> 16) & 0xFFFF) |
								((value_raw << 16) & 0xFFFF);
					break;
				default:
					throw std::runtime_error(
						"Unsupported byte order for f32 in luaRead for "
						"mapping: " +
						name);
			}

			// Convert to float
			float value;
			memcpy(&value, &value_raw, sizeof(float));

			// Handle scale if not 1.0
			if (def.scale != 1.0) {
				value =
					static_cast<float>(static_cast<double>(value) * def.scale);
			}

			lua_pushnumber(L, static_cast<lua_Number>(value));
			return;
		}
		case Mapping::ValueDefFormat::f64: {
			// Read four registers
			uint64_t value_raw = (static_cast<uint64_t>(regsBuffer[0]) << 48) |
								 (static_cast<uint64_t>(regsBuffer[1]) << 32) |
								 (static_cast<uint64_t>(regsBuffer[2]) << 16) |
								 static_cast<uint64_t>(regsBuffer[3]);

			// Handle byte order if needed
			switch (def.order) {
				case Mapping::ValueDefOrder::abcdefgh:
					// No change needed
					break;
				default:
					throw std::runtime_error(
						"Unsupported byte order for f64 in luaRead for "
						"mapping: " +
						name);
			}

			// Handle scale if not 1.0
			double value;
			memcpy(&value, &value_raw, sizeof(double));
			if (def.scale != 1.0) {
				value = value * def.scale;
			}

			lua_pushnumber(L, static_cast<lua_Number>(value));
			return;
		}
		case Mapping::ValueDefFormat::str: {
			// Strings depend heavily on length and order
			std::string strValue;
			strValue.resize(def.length * 2, '\0');

			switch (def.order) {
				case Mapping::ValueDefOrder::ab:
					printf("DEBUG: Reading string with ab order and length %d\n", def.length);
					for (size_t i = 0; i < def.length; ++i) {
						strValue[i * 2] = static_cast<char>(regsBuffer[i] >> 8);
						strValue[i * 2 + 1] =
							static_cast<char>(regsBuffer[i] & 0x00FF);
					}
					break;
				case Mapping::ValueDefOrder::ba:
					for (size_t i = 0; i < def.length; ++i) {
						strValue[i * 2 + 1] =
							static_cast<char>(regsBuffer[i] >> 8);
						strValue[i * 2] =
							static_cast<char>(regsBuffer[i] & 0x00FF);
					}
					break;
				case Mapping::ValueDefOrder::a:
					for (size_t i = 0; i < def.length; ++i) {
						strValue[i] = static_cast<char>(regsBuffer[i] >> 8);
					}
					break;
				case Mapping::ValueDefOrder::b:
					for (size_t i = 0; i < def.length; ++i) {
						strValue[i] = static_cast<char>(regsBuffer[i] & 0x00FF);
					}
					break;
				default:
					throw std::runtime_error(
						"Unsupported byte order for str in luaRead for "
						"mapping: " +
						name);
			}

			// Trim any trailing null characters
			size_t endPos = strValue.find('\0');
			if (endPos != std::string::npos) {
				strValue.resize(endPos);
			}
			
			lua_pushlstring(L, strValue.c_str(), strValue.size());

			return;
		}
		case Mapping::ValueDefFormat::bitfield: {
			const auto& bitfieldDef = m_mapping->getBitfieldDef(def.linked);
			// Loop through values in the def
			lua_newtable(L);
			for (const auto& [bitPos, bitName] : bitfieldDef) {
				uint16_t regIndex = bitPos / 16;
				uint16_t bitInReg = bitPos % 16;
				uint16_t regValue = regsBuffer[regIndex];
				bool bitSet = (regValue & (1u << bitInReg)) != 0;
				lua_pushboolean(L, bitSet);
				lua_setfield(L, -2, bitName.c_str());
			}
			return;
		}
		default:
			throw std::runtime_error(
				"Unsupported format in luaRead for mapping: " + name);
	}
}

void ModbusDeviceContext::luaWrite(lua_State* L, const char* name) {
	// Just consume the value for now
	lua_pop(L, 1);
}
