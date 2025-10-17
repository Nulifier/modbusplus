#include "modbus-device-ctx.hpp"

ModbusDeviceContext::ModbusDeviceContext(
	std::shared_ptr<ModbusDevice> device,
	const std::unordered_map<std::string, Mapping>&& mappings)
	: m_device(std::move(device)), m_mappings(std::move(mappings)) {}

void ModbusDeviceContext::luaRead(lua_State* L, const std::string& name) {}

void ModbusDeviceContext::luaWrite(lua_State* L, const char* name, int index) {}
