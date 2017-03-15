/**********************************************************************
* Author:	jaron.ho
* Date:		2012-9-9
* Brief:	used in lua for parse .xml file
**********************************************************************/
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

extern int luaopen_xml(lua_State* L);

#ifdef __cplusplus
}
#endif
#include "tolua++.h"
