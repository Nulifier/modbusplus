#pragma once

#include <cstdint>
#include <lua.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include "modbus-device.hpp"
#include "mapping.hpp"

class ModbusDeviceContext {
   public:
	ModbusDeviceContext(
		std::shared_ptr<ModbusDevice> device,
		std::shared_ptr<Mapping>&& mapping);

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
	 * @note The value is consumed from the stack.
	 * @note The value to write is at the top of the stack.
	 */
	void luaWrite(lua_State* L, const char* name);

   private:
	std::shared_ptr<ModbusDevice> m_device;
	std::shared_ptr<Mapping> m_mapping;
};
