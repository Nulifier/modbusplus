#pragma once

#include <lua.hpp>
#include "modbusplus-config.hpp"

// Library functions
static int lua_mbdevice_newRtu(lua_State* L);
static int lua_mbdevice_newTcp(lua_State* L);

// ModbusDevice methods
static int lua_mbdevice_gc(lua_State* L);
static int lua_mbdevice_connect(lua_State* L);
static int lua_mbdevice_close(lua_State* L);
static int lua_mbdevice_flush(lua_State* L);
static int lua_mbdevice_set_slave(lua_State* L);
static int lua_mbdevice_read_bits(lua_State* L);
static int lua_mbdevice_read_input_bits(lua_State* L);
static int lua_mbdevice_read_registers(lua_State* L);
static int lua_mbdevice_read_input_registers(lua_State* L);
static int lua_mbdevice_write_bit(lua_State* L);
static int lua_mbdevice_write_bits(lua_State* L);
static int lua_mbdevice_write_register(lua_State* L);
static int lua_mbdevice_write_registers(lua_State* L);
static int lua_mbdevice_new_ctx(lua_State* L);

// ModbusDeviceContext methods
static int lua_mbdevicectx_gc(lua_State* L);
static int lua_mbdevicectx_connect(lua_State* L);
static int lua_mbdevicectx_close(lua_State* L);
static int lua_mbdevicectx_read(lua_State* L);
static int lua_mbdevicectx_write(lua_State* L);

#ifdef LIBMODBUSPLUS_STACK_CHECK
#define STACK_START(fn_name, nargs)                             \
	int modbusplus_stack_top_##fn_name = lua_gettop(L) - nargs; \
	enum {}
#define STACK_END(fn_name, nresults)                                    \
	if (lua_gettop(L) != modbusplus_stack_top_##fn_name + nresults) {   \
		fprintf(stderr,                                                 \
				"Stack imbalance in %s: expected %d results, got %d\n", \
				#fn_name, nresults,                                     \
				lua_gettop(L) - modbusplus_stack_top_##fn_name);        \
	}                                                                   \
	enum {}
#else
#define STACK_START(name, nargs) enum {}
#define STACK_END(name, nresults) enum {}
#endif
