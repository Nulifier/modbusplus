#pragma once

#include <cstdint>
#include <lua.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include "mapping.hpp"
#include "modbus-device.hpp"

class ModbusDeviceContext {
   public:
	ModbusDeviceContext(std::shared_ptr<ModbusDevice> device,
						std::shared_ptr<Mapping>&& mapping,
						int deviceId = -1) noexcept;

	ModbusDevice& getDevice() const noexcept { return *m_device; }

	/**
	 * Gets the device ID for the context.
	 * @return The device ID.
	 */
	int getDeviceId() const noexcept { return m_deviceId; }

	/**
	 * Pushes the value of the given mapping onto the Lua stack.
	 * @param L The Lua state.
	 * @param name The name of the mapping.
	 * @note The value is pushed onto the stack.
	 */
	void luaRead(lua_State* L, const char* name);

	/**
	 * Writes a value from the Lua stack to the given mapping.
	 * @param L The Lua state.
	 * @param name The name of the mapping.
	 * @note The value is consumed from the stack.
	 * @note The value to write is at the top of the stack.
	 */
	void luaWrite(lua_State* L, const char* name);

   private:
	int m_deviceId = 0;

	std::shared_ptr<ModbusDevice> m_device;
	std::shared_ptr<Mapping> m_mapping;
};
