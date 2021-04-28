/**********************************************************************
* Author:	jaron.ho
* Date:		2012-9-9
* Brief:	used in lua for parse .xml file
**********************************************************************/
#include "luaxml.h"
#include "rapidxml.hpp"
#include "Cocos2dxLuaLoader.h"


static int parseXml(lua_State* L, const char* buffer)
{
	if (NULL == buffer || 0 == strcmp(buffer, ""))
		return 0;

	rapidxml::xml_document<> doc;
	doc.parse<0>(const_cast<char*>(buffer));

	unsigned int index = 0;
	lua_newtable(L);
	rapidxml::xml_node<> *root = doc.first_node();
	for (rapidxml::xml_node<> *row = root->first_node(); row; row = row->next_sibling())
	{
		lua_newtable(L);
		for (rapidxml::xml_node<> *col = row->first_node(); col; col = col->next_sibling())
		{
			lua_pushstring(L, col->name());
			lua_pushstring(L, col->value());
			lua_rawset(L, -3);
		}
		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

static int loadXmlFile(lua_State* L)
{
	const char *filename = (char*)luaL_checkstring(L, 1);
	if (NULL == filename)
	{
		luaL_error(L, "error in function 'loadXmlFile', arg1 is not string");

		showError("error in function 'loadXmlFile', arg1 is not string");
		return 0;
	}
	std::string content = decrypt_file(filename, check_need_decrypt());
	int res = parseXml(L, content.c_str());
	if (0 == res)
	{
		luaL_error(L, "error in function 'loadXmlFile', parse xml file %s failed", filename);

		static const int MAX_LEN = cocos2d::kMaxLogLen + 1;
		char szBuf[MAX_LEN];
		memset(szBuf, 0, MAX_LEN);
		sprintf(szBuf, "error in function 'loadXmlFile', parse xml file %s failed", filename);
		showError(szBuf);
		return 0;
	}
	return 1;
}

int luaopen_xml(lua_State* L)
{
	lua_register(L, "loadXmlFile", loadXmlFile);
	return 0;
}
