/**********************************************************************
* Author:	jaron.ho
* Date:		2014-03-25
* Brief:	used in lua for crypto with algorithm RC4
**********************************************************************/
#include "luarc4.h"
#include "RC4.h"
#include "memory.h"


int rc4_crypto(lua_State* L)
{
	const char *data = (char*)luaL_checkstring(L, 1);
	unsigned long dataSize = (unsigned long)luaL_checklong(L, 2);
	if (NULL == data || 0 == dataSize)
		return 0;

	const char *key = (char*)luaL_checkstring(L, 3);
	if (NULL == key)
	{
		lua_pushstring(L, data);
		return 1;
	}

	char *input = new char[dataSize + 1];
	memset(input, 0, dataSize + 1);
	memcpy(input, data, dataSize);
	rc4_crypto((unsigned char*)input, dataSize, (unsigned char*)key);
	lua_pushstring(L, input);
	delete []input;
	input = NULL;
	return 1;
}

int luaopen_rc4(lua_State* L)
{
	lua_register(L, "rc4_crypto", rc4_crypto);
	return 0;
}
