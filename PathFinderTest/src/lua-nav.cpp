

#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "lua-nav.h"
#include "nav.h"

struct scene_nav_context
{
	struct nav_mesh_context* ctx;
	int id;
};

int _release(struct lua_State* L)
{
	struct scene_nav_context* userdata = (struct scene_nav_context*)lua_touserdata(L, 1);
	release_mesh(userdata->ctx);
	return 0;
}

int _set_mask(struct lua_State* L)
{
	struct nav_mesh_context* ctx = (struct nav_mesh_context*)lua_touserdata(L, 1);
	int mask = lua_tointeger(L, 2);
	int enable = lua_tointeger(L, 3);
	set_mask(&ctx->mask_ctx, mask, enable);

	return 0;
}

int _find(struct lua_State* L)
{
	struct scene_nav_context* userdata = (struct scene_nav_context*)lua_touserdata(L, 1);

	struct vector3 start, over;
	start.x = lua_tonumber(L, 2);
	start.z = lua_tonumber(L, 3);
	over.x = lua_tonumber(L, 4);
	over.z = lua_tonumber(L, 5);

	struct nav_path_context* path = astar_find(userdata->ctx, &start, &over);

	lua_createtable(L, 64, 0);
	for (int i = 0; i < path->offset; i++)
	{
		lua_pushinteger(L, i + 1);
		lua_createtable(L, 0, 2);
		lua_pushnumber(L, path->wp[i].x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, path->wp[i].y);
		lua_setfield(L, -2, "y");
		lua_pushnumber(L, path->wp[i].z);
		lua_setfield(L, -2, "z");
		lua_settable(L, -3);
	}

	return 1;
}

int _raycast(struct lua_State* L)
{
	struct scene_nav_context* userdata = (struct scene_nav_context*)lua_touserdata(L, 1);
	struct vector3 start, over, result;
	start.x = lua_tonumber(L, 2);
	start.z = lua_tonumber(L, 3);
	over.x = lua_tonumber(L, 4);
	over.z = lua_tonumber(L, 5);

	bool ok = raycast(userdata->ctx, &start, &over, &result);
	if (ok)
	{
		lua_pushboolean(L, 1);
		lua_pushnumber(L, result.x);
		lua_pushnumber(L, result.z);
		return 3;
	}

	lua_pushboolean(L, 0);
	return 1;
}


int _create(struct lua_State* L)
{
	int scene = lua_tointeger(L, 1);
	double** vptr = (double**)lua_touserdata(L,2); 
	int vsize = lua_tointeger(L,3);
	int** pptr = (int**)lua_touserdata(L,4); 
	int psize = lua_tointeger(L,5);

	struct nav_mesh_context* ctx = load_mesh(vptr,vsize,pptr, psize);
	struct scene_nav_context* userdata = (struct scene_nav_context*)lua_newuserdata(L, sizeof(scene_nav_context));
	userdata->ctx = ctx;
	userdata->id = scene;
	
	lua_newtable(L);
	lua_pushcfunction(L, _release);
	lua_setfield(L, -2, "__gc");

	lua_newtable(L);
	lua_pushcfunction(L, _find);
	lua_setfield(L, -2, "find");
	lua_pushcfunction(L, _raycast);
	lua_setfield(L, -2, "raycast");
	lua_pushcfunction(L, _set_mask);
	lua_setfield(L, -2, "set_mask");

	lua_setfield(L, -2, "__index");

	lua_setmetatable(L, -2);

	return 1;
}


int luaopen_nav_core(lua_State *L)
{
	luaL_Reg l[] = 
	{
		{"create", _create},
		{ NULL, NULL }
	};

	lua_createtable(L, 0, (sizeof(l)) / sizeof(luaL_Reg) - 1);
	luaL_setfuncs(L, l, 0);
	return 1;
}
