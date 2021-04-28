http://www.blogjava.net/ivanwan/archive/2012/02/08/369555.html

将LuaSocket静态编译到cocos2d-x目标执行文件中
Cocos2d-x已经提供了对Lua的基本支持，但除了Lua的基本库外，并没有捆绑一些常用库，例如LuaSocket。
经过一番尝试，终于搞定了此问题:)

获得LuaSocket源代码后，在cocos2d-x项目的libs/lua目录中建立子目录exts/luasocket，并将luasocket-2.0.2/src目录中所有的*.c/*.h文件，
拷贝到libs/lua/exts/luasocket目录中，在liblua工程里添加上面的目录及文件。注意：要去掉文件关联unix.h、unix.c、usocket.h、usocket.c
（这几个文件是在unix系统中才用到的，所以要去除关联）
在libs/lua/exts目录中建立文件：

lualoadexts.h
////////////////////////////////////////////////////////////
#ifndef _LUALOADEXTS_H_
#define _LUALOADEXTS_H_
#include "lauxlib.h"
void luax_initpreload(lua_State *L);
#endif	// _LUALOADEXTS_H_ 
////////////////////////////////////////////////////////////

lualoadexts.c
////////////////////////////////////////////////////////////
#include "lualoadexts.h"
#include "luasocket.h"
#include "mime.h"
static luaL_Reg luax_preload_list[] = 
{
	{"socket.core", luaopen_socket_core},
	{"mime.core", luaopen_mime_core},
	{NULL, NULL}
};
void luax_initpreload(lua_State *L)
{
	luaL_Reg* lib = luax_preload_list;
	luaL_findtable(L, LUA_GLOBALSINDEX, "package.preload", sizeof(luax_preload_list)/sizeof(luax_preload_list[0])-1);
	for (; lib->func; lib++)
	{
		lua_pushstring(L, lib->name);
		lua_pushcfunction(L, lib->func);
		lua_rawset(L, -3);
	}
	lua_pop(L, 1);
}
////////////////////////////////////////////////////////////

最后，打开libs/lua/cocos2dx_support/CCLuaStack.CCLuaStack::init()载入Lua标准库和扩展库的代码：
////////////////////////////////////////////////////////////
luax_initpreload(m_state);
////////////////////////////////////////////////////////////

LuaScoket除了C代码，还有一部分是Lua代码，所以需要将luasocket-2.0.2/src/*.lua复制到项目中，然后用下列Lua代码进行测试：
////////////////////////////////////////////////////////////
local socket = require("socket")
print("socket module:", socket)
print("socket.connect function:", socket.connect)
print("socket.bind function:", socket.bind)
print("\n")
print("io module:", io)
////////////////////////////////////////////////////////////


注意：要在工程里引入两个lib文件，WSock32.lib、WS2_32.lib（window系统需要用到这两个文件，否则编译会出错）


