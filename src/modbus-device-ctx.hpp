#pragma once

#include <cstdint>
#include <lua.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include "modbus-device.hpp"

class ModbusDeviceContext {
   public:
	enum class Format : uint8_t {
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
		str_ab,
		str_ba,
		str_a,
		str_b
	};

	struct Mapping {
		double scale = 1.0;
		uint16_t addr;
		Format format;
		size_t length;		// Only for string formats
		bool trim = false;	// Only for string formats
		bool isInput = false;
	};

	ModbusDeviceContext(
		std::shared_ptr<ModbusDevice> device,
		const std::unordered_map<std::string, Mapping>&& mappings);

	/**
	 * Pushes the value of the given mapping onto the Lua stack.
	 * @param L The Lua state.
	 * @param name The name of the mapping.
	 * @note The value is pushed onto the stack.
	 */
	void luaRead(lua_State* L, const std::string& name);

	/**
	 * Writes a value from the Lua stack to the given mapping.
	 * @param L The Lua state.
	 * @param name The name of the mapping.
	 * @param index The index of the value on the Lua stack.
	 * @note The value is popped from the stack.
	 */
	void luaWrite(lua_State* L, const char* name, int index);

   private:
	std::shared_ptr<ModbusDevice> m_device;
	std::unordered_map<std::string, Mapping> m_mappings;
};
