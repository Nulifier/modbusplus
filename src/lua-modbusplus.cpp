#include "lua-modbusplus.h"
#include <cstring>
#include <vector>
#include "lauxlib.h"
#include "lua-modbusplus-private.hpp"
#include "modbus-device-ctx.hpp"
#include "modbus-device.hpp"

static const char* MODBUS_DEVICE_METATABLE = "modbusplus.device";
static const char* MODBUS_DEVICE_CTX_METATABLE = "modbusplus.device.ctx";

std::shared_ptr<ModbusDevice> getModbusDevice(lua_State* L, int index) {
	void* udata = luaL_checkudata(L, index, MODBUS_DEVICE_METATABLE);
	if (!udata) {
		luaL_error(L, "Invalid ModbusDevice userdata");
		return nullptr;
	}
	auto ptr = static_cast<std::shared_ptr<ModbusDevice>*>(udata);
	if (!ptr || !*ptr) {
		luaL_error(L, "ModbusDevice is null");
		return nullptr;
	}
	return *ptr;
}

std::shared_ptr<ModbusDeviceContext> getModbusDeviceCtx(lua_State* L,
														int index) {
	void* udata = luaL_checkudata(L, index, MODBUS_DEVICE_CTX_METATABLE);
	if (!udata) {
		luaL_error(L, "Invalid ModbusDeviceContext userdata");
		return nullptr;
	}
	auto ptr = static_cast<std::shared_ptr<ModbusDeviceContext>*>(udata);
	if (!ptr || !*ptr) {
		luaL_error(L, "ModbusDeviceContext is null");
		return nullptr;
	}
	return *ptr;
}

luaL_reg library_methods[] = {
	{"newRtu", lua_mbdevice_newRtu},
	{"newTcp", lua_mbdevice_newTcp},
	{NULL, NULL} /* sentinel */
};
luaL_reg device_methods[] = {
	{"__gc", lua_mbdevice_gc},
	{"raw_connect", lua_mbdevice_connect},
	{"raw_close", lua_mbdevice_close},
	{"raw_flush", lua_mbdevice_flush},
	{"raw_set_slave", lua_mbdevice_set_slave},
	{"raw_read_bits", lua_mbdevice_read_bits},
	{"raw_read_input_bits", lua_mbdevice_read_input_bits},
	{"raw_read_registers", lua_mbdevice_read_registers},
	{"raw_read_input_registers", lua_mbdevice_read_input_registers},
	{"raw_write_bit", lua_mbdevice_write_bit},
	{"raw_write_bits", lua_mbdevice_write_bits},
	{"raw_write_register", lua_mbdevice_write_register},
	{"raw_write_registers", lua_mbdevice_write_registers},
	{NULL, NULL} /* sentinel */
};
luaL_reg device_ctx_methods[] = {
	{"__gc", lua_mbdevicectx_gc},
	{NULL, NULL} /* sentinel */
};

int luaopen_modbusplus(lua_State* L) {
	STACK_START(luaopen_modbusplus, 1);

	// Pop module name
	lua_pop(L, 1);

	// Create device metatable
	if (luaL_newmetatable(L, MODBUS_DEVICE_METATABLE)) {
		luaL_register(L, nullptr, device_methods);
		lua_setfield(L, -1, "__index");	 // metatable.__index = metatable
	} else {
		lua_pop(L, 1);	// pop existing metatable
	}

	// Create device ctx metatable
	if (luaL_newmetatable(L, MODBUS_DEVICE_CTX_METATABLE)) {
		luaL_register(L, nullptr, device_ctx_methods);
		lua_setfield(L, -1, "__index");	 // metatable.__index = metatable
	} else {
		lua_pop(L, 1);	// pop existing metatable
	}

	// Create library table
	lua_newtable(L);
	luaL_register(L, nullptr, library_methods);

	STACK_END(luaopen_modbusplus, 1);

	return 1;
}

int lua_mbdevice_newRtu(lua_State* L) {
	STACK_START(lua_mbdevice_newRtu, 1);

	// The config is passed as a table
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_getfield(L, -1, "device");
	lua_getfield(L, -2, "baud");
	lua_getfield(L, -3, "parity");
	lua_getfield(L, -4, "data_bits");
	lua_getfield(L, -5, "stop_bits");

#ifdef MODBUSPLUS_COMPAT_HWSW_FLOWCONTROL
	lua_getfield(L, -6, "flowctrl");
	const char* flowctrlStr = luaL_optstring(L, -1, "None");
	ModbusDeviceRtu::FlowControl flowctrl;
	if (strcmp(flowctrlStr, "None") == 0) {
		flowctrl = ModbusDeviceRtu::FlowControl::None;
	} else if (strcmp(flowctrlStr, "HW") == 0) {
		flowctrl = ModbusDeviceRtu::FlowControl::Hardware;
	} else if (strcmp(flowctrlStr, "SW") == 0) {
		flowctrl = ModbusDeviceRtu::FlowControl::Software;
	} else {
		return luaL_error(L, "Invalid flowctrl: %s", flowctrlStr);
	}
#endif

	// STACK: config, device, baud, parity, data_bits, stop_bits

	// Validate parameters
	const char* deviceName = luaL_checkstring(L, 2);
	int baud = luaL_checkinteger(L, 3);
	const char* parityStr = luaL_optstring(L, 4, "N");
	int dataBits = luaL_optinteger(L, 5, 8);
	int stopBits = luaL_optinteger(L, 6, 1);

	// STACK: config, device, baud, parity, data_bits, stop_bits
	lua_pop(L, 6);

	// Validate parity
	ModbusDeviceRtu::Parity parity;
	if (parityStr[0] == 'N') {
		parity = ModbusDeviceRtu::Parity::None;
	} else if (parityStr[0] == 'E') {
		parity = ModbusDeviceRtu::Parity::Even;
	} else if (parityStr[0] == 'O') {
		parity = ModbusDeviceRtu::Parity::Odd;
	} else {
		return luaL_error(L, "Invalid parity: %s", parityStr);
	}

	// Validate data bits
	ModbusDeviceRtu::DataBits dataBitsEnum;
	if (dataBits < 5 || dataBits > 8) {
		return luaL_error(L, "Invalid data_bits: %d", dataBits);
	} else {
		dataBitsEnum = static_cast<ModbusDeviceRtu::DataBits>(dataBits);
	}

	// Validate stop bits
	ModbusDeviceRtu::StopBits stopBitsEnum;
	if (stopBits < 1 || stopBits > 2) {
		return luaL_error(L, "Invalid stop_bits: %d", stopBits);
	} else {
		stopBitsEnum = static_cast<ModbusDeviceRtu::StopBits>(stopBits);
	}

// Create ModbusDeviceRtu instance
#ifndef MODBUSPLUS_COMPAT_HWSW_FLOWCONTROL
	auto device = std::make_shared<ModbusDeviceRtu>(deviceName, baud, parity,
													dataBitsEnum, stopBitsEnum);
#else
	auto device = std::make_shared<ModbusDeviceRtu>(
		deviceName, baud, parity, dataBitsEnum, stopBitsEnum, flowctrl);
#endif

	// Allocate userdata
	void* udata = lua_newuserdata(L, sizeof(std::shared_ptr<ModbusDevice>));

	// Construct the shared_ptr in the userdata (placement new)
	new (udata) std::shared_ptr<ModbusDevice>(device);

	// Set the userdata's metatable
	luaL_getmetatable(L, MODBUS_DEVICE_METATABLE);
	lua_setmetatable(L, -2);

	STACK_END(lua_mbdevice_newRtu, 1);

	return 1;  // Return the userdata
}

int lua_mbdevice_newTcp(lua_State* L) {
	STACK_START(lua_mbdevice_newTcp, 1);

	// The config is passed as a table
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_getfield(L, -1, "ip");
	lua_getfield(L, -2, "port");

	// STACK: config, ip, port

	// Validate parameters
	const char* ip = luaL_checkstring(L, 2);
	int port = luaL_optinteger(L, 3, 502);

	// STACK: config, ip, port
	lua_pop(L, 3);

	// Create ModbusDeviceTcp instance
	auto device = std::make_shared<ModbusDeviceTcp>(ip, port);

	// Allocate userdata
	void* udata = lua_newuserdata(L, sizeof(std::shared_ptr<ModbusDevice>));

	// Construct the shared_ptr in the userdata (placement new)
	new (udata) std::shared_ptr<ModbusDevice>(device);

	// Set the userdata's metatable
	luaL_getmetatable(L, MODBUS_DEVICE_METATABLE);
	lua_setmetatable(L, -2);

	STACK_END(lua_mbdevice_newTcp, 1);

	return 1;  // Return the userdata
}

int lua_mbdevice_gc(lua_State* L) {
	STACK_START(lua_mbdevice_gc, 1);

	// Get the userdata
	void* udata = luaL_checkudata(L, 1, MODBUS_DEVICE_METATABLE);
	if (udata) {
		// Call the destructor for the shared_ptr
		auto ptr = static_cast<std::shared_ptr<ModbusDevice>*>(udata);
		ptr->reset();
	}

	lua_pop(L, 1);

	STACK_END(lua_mbdevice_gc, 0);

	return 0;
}

int lua_mbdevice_connect(lua_State* L) {
	STACK_START(lua_mbdevice_connect, 0);

	auto ptr = getModbusDevice(L, 1);

	// STACK: device
	lua_pop(L, 1);

	try {
		ptr->connect();
	} catch (const std::exception& ex) {
		return luaL_error(L, "Failed to connect: %s", ex.what());
	}

	STACK_END(lua_mbdevice_connect, 0);

	return 0;
}

int lua_mbdevice_close(lua_State* L) {
	STACK_START(lua_mbdevice_close, 0);

	auto ptr = getModbusDevice(L, 1);

	// STACK: device
	lua_pop(L, 1);

	ptr->close();

	STACK_END(lua_mbdevice_close, 0);

	return 0;
}

int lua_mbdevice_flush(lua_State* L) {
	STACK_START(lua_mbdevice_flush, 1);

	auto ptr = getModbusDevice(L, 1);

	// STACK: device
	lua_pop(L, 1);

	unsigned int rc;
	try {
		rc = ptr->flush();
	} catch (const std::exception& ex) {
		return luaL_error(L, "Failed to flush: %s", ex.what());
	}
	lua_pushinteger(L, rc);

	STACK_END(lua_mbdevice_flush, 1);

	return 1;
}

int lua_mbdevice_set_slave(lua_State* L) {
	STACK_START(lua_mbdevice_set_slave, 0);

	auto ptr = getModbusDevice(L, 1);
	int slave = luaL_checkinteger(L, 2);

	// STACK: device, slave
	lua_pop(L, 2);

	try {
		ptr->setSlave(slave);
	} catch (const std::exception& ex) {
		return luaL_error(L, "Failed to set slave: %s", ex.what());
	}

	STACK_END(lua_mbdevice_set_slave, 0);

	return 0;
}

int lua_mbdevice_read_bits(lua_State* L) {
	STACK_START(lua_mbdevice_read_bits, 3);

	auto ptr = getModbusDevice(L, 1);
	int addr = luaL_checkinteger(L, 2);
	int nb = luaL_checkinteger(L, 3);

	// STACK: device, addr, nb
	lua_pop(L, 3);

	if (nb <= 0) {
		return luaL_error(L, "Number of bits to read must be positive");
	}

// Allocate buffer for bits
#ifdef MODBUSPLUS_COMPAT_READ_REG_8BIT
	std::vector<uint8_t> buffer(nb / 8 + (nb % 8 ? 1 : 0), 0);
#else
	std::vector<uint8_t> buffer(nb, 0);
#endif

	unsigned int rc;
	try {
		rc = ptr->readBits(addr, buffer.size(), buffer.data());
	} catch (const std::exception& ex) {
		return luaL_error(L, "Failed to read bits: %s", ex.what());
	}

	// Push results as a table of booleans
	lua_newtable(L);
#ifdef MODBUSPLUS_COMPAT_READ_REG_8BIT
	for (int i = 0; i < static_cast<int>(nb); ++i) {
		int byteIndex = i / 8;
		int bitIndex = 7 - i % 8;  // MSB first
		int bit = (buffer[byteIndex] >> bitIndex) & 0x01;
		lua_pushboolean(L, bit);
		lua_rawseti(L, -2, i + 1);
	}
#else
	for (int i = 0; i < static_cast<int>(nb); ++i) {
		lua_pushboolean(L, buffer[i]);
		lua_rawseti(L, -2, i + 1);
	}
#endif

	STACK_END(lua_mbdevice_read_bits, 1);

	return 1;  // Return the table of bits
}

int lua_mbdevice_read_input_bits(lua_State* L) {
	STACK_START(lua_mbdevice_read_input_bits, 3);

	auto ptr = getModbusDevice(L, 1);
	int addr = luaL_checkinteger(L, 2);
	int nb = luaL_checkinteger(L, 3);

	// STACK: device, addr, nb
	lua_pop(L, 3);

	if (nb <= 0) {
		return luaL_error(L, "Number of input bits to read must be positive");
	}

	// Allocate buffer for bits
#ifdef MODBUSPLUS_COMPAT_READ_REG_8BIT
	std::vector<uint8_t> buffer(nb / 8 + (nb % 8 ? 1 : 0), 0);
#else
	std::vector<uint8_t> buffer(nb, 0);
#endif

	unsigned int rc;
	try {
		rc = ptr->readInputBits(addr, buffer.size(), buffer.data());
	} catch (const std::exception& ex) {
		return luaL_error(L, "Failed to read input bits: %s", ex.what());
	}

	// Push results as a table of booleans
	lua_newtable(L);
#ifdef MODBUSPLUS_COMPAT_READ_REG_8BIT
	for (int i = 0; i < static_cast<int>(nb); ++i) {
		int byteIndex = i / 8;
		int bitIndex = 7 - i % 8;  // MSB first
		int bit = (buffer[byteIndex] >> bitIndex) & 0x01;
		lua_pushboolean(L, bit);
		lua_rawseti(L, -2, i + 1);
	}
#else
	for (int i = 0; i < static_cast<int>(nb); ++i) {
		lua_pushboolean(L, buffer[i]);
		lua_rawseti(L, -2, i + 1);
	}
#endif

	STACK_END(lua_mbdevice_read_input_bits, 1);

	return 1;  // Return the table of input bits
}

int lua_mbdevice_read_registers(lua_State* L) {
	STACK_START(lua_mbdevice_read_registers, 3);

	auto ptr = getModbusDevice(L, 1);
	int addr = luaL_checkinteger(L, 2);
	int nb = luaL_checkinteger(L, 3);

	// STACK: device, addr, nb
	lua_pop(L, 3);

	if (nb <= 0) {
		return luaL_error(L, "Number of registers to read must be positive");
	}

	// Allocate buffer for registers
	std::vector<uint16_t> buffer(nb, 0);

	unsigned int rc;
	try {
		rc = ptr->readRegisters(addr, buffer.size(), buffer.data());
	} catch (const std::exception& ex) {
		return luaL_error(L, "Failed to read registers: %s", ex.what());
	}

	// Push results as a table of integers
	lua_newtable(L);
	for (int i = 0; i < static_cast<int>(rc); ++i) {
		lua_pushinteger(L, buffer[i]);
		lua_rawseti(L, -2, i + 1);
	}

	STACK_END(lua_mbdevice_read_registers, 1);

	return 1;  // Return the table of registers
}

int lua_mbdevice_read_input_registers(lua_State* L) {
	STACK_START(lua_mbdevice_read_input_registers, 3);

	auto ptr = getModbusDevice(L, 1);
	int addr = luaL_checkinteger(L, 2);
	int nb = luaL_checkinteger(L, 3);

	// STACK: device, addr, nb
	lua_pop(L, 3);

	if (nb <= 0) {
		return luaL_error(L,
						  "Number of input registers to read must be positive");
	}

	// Allocate buffer for registers
	std::vector<uint16_t> buffer(nb, 0);

	unsigned int rc;
	try {
		rc = ptr->readInputRegisters(addr, buffer.size(), buffer.data());
	} catch (const std::exception& ex) {
		return luaL_error(L, "Failed to read input registers: %s", ex.what());
	}

	// Push results as a table of integers
	lua_newtable(L);
	for (int i = 0; i < static_cast<int>(rc); ++i) {
		lua_pushinteger(L, buffer[i]);
		lua_rawseti(L, -2, i + 1);
	}

	STACK_END(lua_mbdevice_read_input_registers, 1);

	return 1;  // Return the table of input registers
}

int lua_mbdevice_write_bit(lua_State* L) {
	STACK_START(lua_mbdevice_write_bit, 3);

	auto ptr = getModbusDevice(L, 1);
	int addr = luaL_checkinteger(L, 2);
	int value = lua_toboolean(L, 3) ? 1 : 0;

	// STACK: device, addr, value
	lua_pop(L, 3);

	unsigned int rc;
	try {
		rc = ptr->writeBit(addr, static_cast<uint8_t>(value));
	} catch (const std::exception& ex) {
		return luaL_error(L, "Failed to write bit: %s", ex.what());
	}
	lua_pushinteger(L, rc);

	STACK_END(lua_mbdevice_write_bit, 1);

	return 1;  // Return number of bits written (should be 1)
}

int lua_mbdevice_write_bits(lua_State* L) {
	STACK_START(lua_mbdevice_write_bits, 3);

	auto ptr = getModbusDevice(L, 1);
	int addr = luaL_checkinteger(L, 2);
	luaL_checktype(L, 3, LUA_TTABLE);

	// STACK: device, addr, bits_table
	int nb = lua_objlen(L, 3);
	if (nb <= 0) {
		return luaL_error(L, "Number of bits to write must be positive");
	}

	// Read bits from the table
	std::vector<uint8_t> buffer(nb, 0);
	for (int i = 0; i < nb; ++i) {
		lua_rawgeti(L, 3, i + 1);
		buffer[i] = lua_toboolean(L, -1) ? 1 : 0;
		lua_pop(L, 1);	// pop the bit value
	}

	// STACK: device, addr, bits_table
	lua_pop(L, 3);

	unsigned int rc;
	try {
		rc = ptr->writeBits(addr, buffer.size(), buffer.data());
	} catch (const std::exception& ex) {
		return luaL_error(L, "Failed to write bits: %s", ex.what());
	}
	lua_pushinteger(L, rc);

	STACK_END(lua_mbdevice_write_bits, 1);

	return 1;  // Return number of bits written
}

int lua_mbdevice_write_register(lua_State* L) {
	STACK_START(lua_mbdevice_write_register, 3);

	auto ptr = getModbusDevice(L, 1);
	int addr = luaL_checkinteger(L, 2);
	int value = luaL_checkinteger(L, 3);
	if (value < 0 || value > 0xFFFF) {
		return luaL_error(L, "Register value must be between 0 and 65535");
	}

	// STACK: device, addr, value
	lua_pop(L, 3);

	unsigned int rc;
	try {
		rc = ptr->writeRegister(addr, static_cast<uint16_t>(value));
	} catch (const std::exception& ex) {
		return luaL_error(L, "Failed to write register: %s", ex.what());
	}
	lua_pushinteger(L, rc);

	STACK_END(lua_mbdevice_write_register, 1);

	return 1;  // Return number of registers written (should be 1)
}

int lua_mbdevice_write_registers(lua_State* L) {
	STACK_START(lua_mbdevice_write_registers, 3);

	auto ptr = getModbusDevice(L, 1);
	int addr = luaL_checkinteger(L, 2);
	luaL_checktype(L, 3, LUA_TTABLE);

	// STACK: device, addr, registers_table
	int nb = lua_objlen(L, 3);
	if (nb <= 0) {
		return luaL_error(L, "Number of registers to write must be positive");
	}

	// Read registers from the table
	std::vector<uint16_t> buffer(nb, 0);
	for (int i = 0; i < nb; ++i) {
		lua_rawgeti(L, 3, i + 1);
		int value = luaL_checkinteger(L, -1);
		if (value < 0 || value > 0xFFFF) {
			return luaL_error(
				L,
				"Register value at index %d must be between 0 and "
				"65535",
				i + 1);
		}
		buffer[i] = static_cast<uint16_t>(value);
		lua_pop(L, 1);	// pop the register value
	}

	// STACK: device, addr, registers_table
	lua_pop(L, 3);

	unsigned int rc;
	try {
		rc = ptr->writeRegisters(addr, buffer.size(), buffer.data());
	} catch (const std::exception& ex) {
		return luaL_error(L, "Failed to write registers: %s", ex.what());
	}
	lua_pushinteger(L, rc);

	STACK_END(lua_mbdevice_write_registers, 1);

	return 1;  // Return number of registers written
}

ModbusDeviceContext::Format parseMappingFormat(lua_State* L,
											   const std::string& fmt) {
	if (fmt == "bit") {
		return ModbusDeviceContext::Format::bit;
	} else if (fmt == "u16") {
		return ModbusDeviceContext::Format::u16;
	} else if (fmt == "i16") {
		return ModbusDeviceContext::Format::i16;
	} else if (fmt == "u32") {
		return ModbusDeviceContext::Format::u32;
	} else if (fmt == "i32") {
		return ModbusDeviceContext::Format::i32;
	} else if (fmt == "u64") {
		return ModbusDeviceContext::Format::u64;
	} else if (fmt == "i64") {
		return ModbusDeviceContext::Format::i64;
	} else if (fmt == "f32") {
		return ModbusDeviceContext::Format::f32;
	} else if (fmt == "f64") {
		return ModbusDeviceContext::Format::f64;
	} else {
		return luaL_error(L, "Invalid mapping format: %s", fmt.c_str()),
			   ModbusDeviceContext::Format::bit;  // Unreachable, but avoids
												  // compiler warning
	}
}

int lua_mbdevice_new_ctx(lua_State* L) {
	STACK_START(lua_mbdevice_new_ctx, 1);

	auto ptr = getModbusDevice(L, 1);
	int slave = luaL_checkinteger(L, 2);
	luaL_checktype(L, 3, LUA_TTABLE);

	// Read mapping configurations from the table
	std::unordered_map<std::string, ModbusDeviceContext::Mapping> mappings;
	lua_pushnil(L);	 // first key
	while (lua_next(L, 3) != 0) {
		// STACK: device, slave, mapping_table, key, value
		if (lua_type(L, -2) != LUA_TSTRING) {
			return luaL_error(L, "Mapping name must be a string");
		}
		const char* name = lua_tostring(L, -2);
		if (lua_type(L, -1) != LUA_TTABLE) {
			return luaL_error(L, "Mapping configuration must be a table");
		}

		ModbusDeviceContext::Mapping mapping;
		lua_getfield(L, -1, "addr");
		mapping.addr = luaL_checkinteger(L, -1);
		lua_getfield(L, -2, "format");
		mapping.format = parseMappingFormat(L, luaL_checkstring(L, -1));
		lua_getfield(L, -3, "type");
		if (lua_isnil(L, -1)) {
			mapping.isInput = false;  // default
		} else {
			mapping.isInput = (std::string(luaL_checkstring(L, -1)) == "input");
		}
		lua_getfield(L, -4, "scale");
		if (lua_isnil(L, -1)) {
			mapping.scale = 1.0;  // default
		} else {
			mapping.scale = luaL_checknumber(L, -1);
		}
		lua_pop(L, 4);	// pop addr, format, type, scale

		mappings[name] = mapping;
		lua_pop(L, 1);	// pop value
	}

	// STACK: device, slave, mapping_table
	lua_pop(L, 1);

	// Create ModbusDeviceContext instance
	auto ctx = std::make_shared<ModbusDeviceContext>(ptr, std::move(mappings));

	// Allocate userdata
	void* udata =
		lua_newuserdata(L, sizeof(std::shared_ptr<ModbusDeviceContext>));

	// Construct the shared_ptr in the userdata (placement new)
	new (udata) std::shared_ptr<ModbusDeviceContext>(ctx);

	// Set the userdata's metatable
	luaL_getmetatable(L, MODBUS_DEVICE_CTX_METATABLE);
	lua_setmetatable(L, -2);

	STACK_END(lua_mbdevice_new_ctx, 1);

	return 1;  // Return the userdata
}

int lua_mbdevicectx_gc(lua_State* L) {
	STACK_START(lua_mbdevicectx_gc, 1);

	// Get the userdata
	auto ptr = getModbusDeviceCtx(L, 1);
	ptr.reset();

	// STACK: ctx
	lua_pop(L, 1);

	STACK_END(lua_mbdevicectx_gc, 0);

	return 0;
}

int lua_mbdevicectx_read(lua_State* L) {
	STACK_START(lua_mbdevicectx_read, 2);

	auto ctx = getModbusDeviceCtx(L, 1);
	const char* name = luaL_checkstring(L, 2);

	// STACK: ctx, name
	lua_pop(L, 2);

	try {
		ctx->luaRead(L, name);
	} catch (const std::exception& ex) {
		return luaL_error(L, "Failed to read mapping '%s': %s", name,
						  ex.what());
	}

	STACK_END(lua_mbdevicectx_read, 1);

	return 1;  // Return the value
}

int lua_mbdevicectx_write(lua_State* L) {
	STACK_START(lua_mbdevicectx_write, 3);

	auto ctx = getModbusDeviceCtx(L, 1);
	const char* name = luaL_checkstring(L, 2);
	// Value is at index 3

	// STACK: ctx, name, value
	lua_pop(L, 3);

	try {
		ctx->luaWrite(L, name, 3);
	} catch (const std::exception& ex) {
		return luaL_error(L, "Failed to write mapping '%s': %s", name,
						  ex.what());
	}

	STACK_END(lua_mbdevicectx_write, 0);

	return 0;
}
