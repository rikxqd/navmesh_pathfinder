#ifndef LUA_NAV_H
#define LUA_NAV_H
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};
int luaopen_nav(lua_State *L);

#endif