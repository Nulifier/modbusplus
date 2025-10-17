#pragma once

#include <lua.hpp>
#include "lua-modbusplus-export.h"

#ifdef __cplusplus
extern "C" {
#endif

LUA_MODBUSPLUS_EXPORT int luaopen_modbusplus(lua_State* L);

#ifdef __cplusplus
}
#endif
