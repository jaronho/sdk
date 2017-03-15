/**********************************************************************
* Author:	jaron.ho
* Date:		2014-03-25
* Brief:	used in lua for packet crypto
**********************************************************************/
#include "luarpacketcrypto.h"
#include "PacketCrypto.h"
#include <memory.h>
#include <stdlib.h>

unsigned long g_packet_xor_key = 0x00000000;
unsigned long g_packet_shift_key = 0x00000000;

int packet_crypto_init(lua_State* L)
{
	g_packet_xor_key = (unsigned long)luaL_checklong(L, 1);
	g_packet_shift_key = (unsigned long)luaL_checklong(L, 2);
	return 0;
}

int packet_crypto_encode(lua_State* L)
{
	const char* data = (char*)luaL_checkstring(L, 1);
	if (NULL == data)
	{
		return 0;
	}
	unsigned int dataSize = strlen(data);
	unsigned int destSize = 0;
	static unsigned char packetEncodeCount = 0;
	char* dest = PacketCryptoEncode(data, dataSize, &destSize, g_packet_xor_key, g_packet_shift_key, ++packetEncodeCount);
	if (NULL == dest)
	{
		lua_pushstring(L, "");
	}
	else
	{
		lua_pushlstring(L, dest, destSize);
		free(dest);
		dest = NULL;
	}
	return 1;
}

int packet_crypto_decode(lua_State* L)
{
	const char* p = (char*)luaL_checkstring(L, 1);
	if (NULL == p)
	{
		return 0;
	}
	unsigned int dataSize = strlen(p);
	char* data = (char*)malloc((dataSize + 1)*sizeof(char));
	memset(data, 0, dataSize + 1);
	memcpy(data, p, dataSize);
	unsigned int destSize = 0;
	unsigned char packetEncodeCount = 0;
	char* dest = PacketCryptoDecode(data, dataSize, &destSize, g_packet_xor_key, g_packet_shift_key, &packetEncodeCount);
	if (NULL == dest)
	{
		lua_pushstring(L, "");
	}
	else
	{
		lua_pushlstring(L, dest, destSize);
		free(dest);
		dest = NULL;
	}
	if (data)
	{
		free(data);
		data = NULL;
	}
	return 1;
}

int luaopen_packetcrypto(lua_State* L)
{
	lua_register(L, "packet_init", packet_crypto_init);
	lua_register(L, "packet_encode", packet_crypto_encode);
	lua_register(L, "packet_decode", packet_crypto_decode);
	return 0;
}
