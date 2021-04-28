/**********************************************************************
* Author:	jaron.ho
* Date:		2015-12-01
* Brief:	resource for lua
**********************************************************************/
#ifndef _LUA_RESOURCE_H_
#define _LUA_RESOURCE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "tolua++.h"

int lua_resource_register(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif	// _LUA_RESOURCE_H_