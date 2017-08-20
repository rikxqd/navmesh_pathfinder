

#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "lua-nav.h"
#include "nav.h"

int _create(struct lua_State* L)
{
	double** vptr = (double**)lua_touserdata(L,1); 
	int vsize = lua_tointeger(L,2);
	int** pptr = (int**)lua_touserdata(L,3); 
	int psize = lua_tointeger(L,4);

	struct MeshContext* ctx = load_mesh(vptr,vsize,pptr, psize);

	lua_pushlightuserdata(L,ctx);
	return 1;
}

int _release(struct lua_State* L)
{
	return 0;
}

int _set_mask(struct lua_State* L)
{
	struct MeshContext* ctx = (struct MeshContext*)lua_touserdata(L,1);
	int mask = lua_tointeger(L,2);
	int enable = lua_tointeger(L,3);
	set_mask(&ctx->mask_ctx,mask,enable);

	return 0;
}

int _find_path(struct lua_State* L)
{
	struct MeshContext* ctx = (struct MeshContext*)lua_touserdata(L,1);
	struct vector3 start,over;
	start.x = lua_tonumber(L,2);
	start.z = lua_tonumber(L,3);
	over.x = lua_tonumber(L,4);
	over.z = lua_tonumber(L,5);

	struct PathContext* path = astar_find(ctx,&start,&over);

	//lua_createtable(L,64,0);
	//for(int i = 0; i < path->offset;i++)
	//{
	//	lua_pushinteger(L,i+1);
	//	lua_createtable(L,0,2);
	//	lua_pushnumber(L,path->wp[i].x);
	//	lua_setfield(L,-2,"x");
	//	lua_pushnumber(L,path->wp[i].z);
	//	lua_setfield(L,-2,"z");
	//	lua_settable(L,-3);
	//}
	lua_pushlightuserdata(L,path);
	return 1;
}

int _raycast(struct lua_State* L)
{
	struct MeshContext* ctx = (struct MeshContext*)lua_touserdata(L,1);
	struct vector3 start,over,result;
	start.x = lua_tonumber(L,2);
	start.z = lua_tonumber(L,3);
	over.x = lua_tonumber(L,4);
	over.z = lua_tonumber(L,5);

	bool ok = raycast(ctx,&start,&over,&result);
	if (ok)
	{
		lua_pushboolean(L,1);
		lua_pushnumber(L,result.x);
		lua_pushnumber(L,result.z);
		return 3;
	}

	lua_pushboolean(L,0);
	return 1;
}

int luaopen_nav(lua_State *L)
{
	luaL_Reg l[] = 
	{
		{"create", _create},
		{"release", _release},
		{"find_path", _find_path},
		{"raycast", _raycast},
		{"set_mask", _set_mask},
		{ NULL, NULL }
	};

	lua_createtable(L, 0, (sizeof(l)) / sizeof(luaL_Reg) - 1);
	luaL_setfuncs(L, l, 0);
	return 1;
}
