/**********************************************************************
* Author:	jaron.ho
* Date:		2014-03-25
* Brief:	used in lua for packet crypto
**********************************************************************/
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

extern int luaopen_packetcrypto(lua_State* L);

#ifdef __cplusplus
}
#endif
#include "tolua++.h"
