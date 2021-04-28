/*
** Lua binding: ResourceUpdate
** Generated automatically by tolua++-1.0.92 on 12/01/15 13:57:57.
*/

#include <stdlib.h>
#include <string>
#include <map>
#include "LuaResource.h"
#include "ResourceDownload.h"
#include "ResourceUpdate.h"
#include "../MD5/MD5.h"

const char* RESDOWNLOAD_LUA_REFID_FUNCTION_MAPPING = "resdownload_lua_function_refid_mapping";
static int sResDownloadLuaFunctionRefId = 0;
typedef std::map<unsigned int, unsigned int> RES_HANDLER;			// <flag, handler>
typedef std::map<unsigned int, RES_HANDLER> RES_HANDLER_MAP;		// <id, <flag, handler>>
static RES_HANDLER_MAP sResHandlerMap;

void openLuaFunction(lua_State* L) {
	static bool isOpened = false;
	if (L && !isOpened) {
		isOpened = true;
		lua_pushstring(L, RESDOWNLOAD_LUA_REFID_FUNCTION_MAPPING);
		lua_newtable(L);
		lua_rawset(L, LUA_REGISTRYINDEX);
	}
}

int registerLuaFunction(lua_State* L, int lo) {
	openLuaFunction(L);
	// function at lo
	if (NULL == L || !lua_isfunction(L, lo)) {
		return 0;
	}
	++sResDownloadLuaFunctionRefId;
	lua_pushstring(L, RESDOWNLOAD_LUA_REFID_FUNCTION_MAPPING);
	lua_rawget(L, LUA_REGISTRYINDEX);					// stack: fun ... refid_fun
	lua_pushinteger(L, sResDownloadLuaFunctionRefId);	// stack: fun ... refid_fun refid
	lua_pushvalue(L, lo);								// stack: fun ... refid_fun refid fun
	lua_rawset(L, -3);									// refid_fun[refid] = fun, stack: fun ... refid_ptr
	lua_pop(L, 1);										// stack: fun ...
	return sResDownloadLuaFunctionRefId;
}

bool getLuaFunction(lua_State* L, int refid) {
	openLuaFunction(L);
	if (NULL == L || refid <= 0) {
		return false;
	}
	lua_pushstring(L, RESDOWNLOAD_LUA_REFID_FUNCTION_MAPPING);
	lua_rawget(L, LUA_REGISTRYINDEX);					// stack: ... refid_fun
	lua_pushinteger(L, refid);							// stack: ... refid_fun refid
	lua_rawget(L, -2);									// stack: ... refid_fun fun
	lua_remove(L, -2);									// stack: ... fun
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 1);
		return false;
	}
	return true;
}

void removeLuaFunction(lua_State* L, int refid) {
	openLuaFunction(L);
	if (NULL == L || refid <= 0) {
		return;
	}
	lua_pushstring(L, RESDOWNLOAD_LUA_REFID_FUNCTION_MAPPING);
	lua_rawget(L, LUA_REGISTRYINDEX);					// stack: ... refid_fun
	lua_pushinteger(L, refid);							// stack: ... refid_fun refid
	lua_pushnil(L);										// stack: ... refid_fun refid nil
	lua_rawset(L, -3);									// refid_fun[refid] = fun, stack: ... refid_ptr
	lua_pop(L, 1);										// stack: ...
}

const char* callLuaFunction(lua_State* L, int refid, const char* sig, ...) {
	openLuaFunction(L);
	static char errorbuffer[512];
	memset(errorbuffer, 0, sizeof(errorbuffer));
	if (NULL == L || refid <= 0 || NULL == sig) {
		sprintf(errorbuffer, "invalid args");
		return errorbuffer;
	}
	va_list vl;
	va_start(vl, sig);
	int narg = 0;										// number of arguments
	while (*sig) {										// push arguments
		switch (*sig++) {
		case 'i': lua_pushinteger(L, va_arg(vl, int)); break;					// int argument
		case 'l': lua_pushnumber(L, va_arg(vl, long)); break;					// long argument
		case 'd': lua_pushnumber(L, va_arg(vl, double)); break;					// double argument
		case 's': lua_pushstring(L, va_arg(vl, const char*)); break;			// string argument
		case 'b': lua_pushboolean(L, va_arg(vl, int) > 0); break;				// bool argument
		case '>': goto endwhile;
		default: sprintf(errorbuffer, "invalid option (%c)", *(sig - 1)); return errorbuffer;
		}
		narg++;
		luaL_checkstack(L, 1, "too many arguments");
	} endwhile:
	if (getLuaFunction(L, refid)) {						// L: ... arg1 arg2 ... func
        if (narg > 0) {
            lua_insert(L, -(narg + 1));						// L: ... func arg1 arg2 ...
        }
		int functionIndex = -(narg + 1);
		if (!lua_isfunction(L, functionIndex)) {
			lua_pop(L, narg + 1);							// remove function and arguments
			return errorbuffer;
		}
		int traceback = 0;
		lua_getglobal(L, "__G__TRACKBACK__");				// L: ... func arg1 arg2 ... G
		if (!lua_isfunction(L, -1)) {
			lua_pop(L, 1);									// L: ... func arg1 arg2 ...
		} else {
			lua_insert(L, functionIndex - 1);				// L: ... G func arg1 arg2 ...
			traceback = functionIndex - 1;
		}
		if (lua_pcall(L, narg, 1, traceback)) {				// L: ... error
			if (0 == traceback) {
				sprintf(errorbuffer, "%s", lua_tostring(L, - 1));
				lua_pop(L, 1);								// remove error message from stack
			} else {											// L: ... G error
				lua_pop(L, 2);								// remove __G__TRACKBACK__ and error message from stack
			}
			return errorbuffer;
		}
		// get return value
		int ret = 0;
		if (lua_isnumber(L, -1)) {
			ret = (int)lua_tointeger(L, -1);
		} else if (lua_isboolean(L, -1)) {
			ret = (int)lua_toboolean(L, -1);
		}
		// remove return value from stack
		lua_pop(L, 1);										// L: ... [G]
		if (traceback) {
			lua_pop(L, 1);									// remove __G__TRACKBACK__ from stack
		}
    }
    lua_settop(L, 0);
	va_end(vl);
	return errorbuffer;
}

bool isExistResObjectHandler(unsigned int id) {
	return sResHandlerMap.end() != sResHandlerMap.find(id);
}

void addResObjectHandler(unsigned int id) {
	RES_HANDLER_MAP::iterator mapIter = sResHandlerMap.find(id);
	if (sResHandlerMap.end() == mapIter) {
		RES_HANDLER rh;
		sResHandlerMap.insert(std::make_pair(id, rh));
	}
}

void removeResObjectHandler(lua_State* L, unsigned int id) {
	RES_HANDLER_MAP::iterator mapIter = sResHandlerMap.find(id);
	if (sResHandlerMap.end() != mapIter) {
		RES_HANDLER::iterator iter = mapIter->second.begin();
		for (; mapIter->second.end() != iter; ++iter) {
			removeLuaFunction(L, iter->second);
		}
		sResHandlerMap.erase(mapIter);
	}
}

void updateResObjectHandler(lua_State* L, unsigned int id, unsigned int flag, unsigned int handler) {
	RES_HANDLER_MAP::iterator mapIter = sResHandlerMap.find(id);
	if (sResHandlerMap.end() == mapIter) {
		RES_HANDLER rh;
		rh.insert(std::make_pair(flag, handler));
		sResHandlerMap.insert(std::make_pair(id, rh));
	} else {
		RES_HANDLER::iterator iter = mapIter->second.find(flag);
		if (mapIter->second.end() == iter) {
			mapIter->second.insert(std::make_pair(flag, handler));
		} else {
			removeLuaFunction(L, iter->second);
			iter->second = handler;
		}
	}
}

/* function to release collected object via destructor */
#ifdef __cplusplus
static int tolua_collect_ResourceDownload(lua_State* tolua_S) {
	ResourceDownload* self = (ResourceDownload*)tolua_tousertype(tolua_S, 1, 0);
	if (self && isExistResObjectHandler(self->getId())) {
		removeResObjectHandler(tolua_S, self->getId());
		Mtolua_delete(self);
	}
	return 0;
}
#endif

/* method: create of class  ResourceDownload */
#ifndef TOLUA_DISABLE_tolua_ResourceDownload_ResourceDownload_create00
static int tolua_ResourceDownload_ResourceDownload_create00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertable(tolua_S, 1, "ResourceDownload", 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 2, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'create'.", &tolua_err);
		return 0;
	}
	ResourceDownload* tolua_ret = (ResourceDownload*)Mtolua_new((ResourceDownload)());
	if (!tolua_ret) {
		tolua_error(tolua_S, "invalid 'tolua_ret' in function 'create'", NULL);
		return 0;
	}
	tolua_pushusertype(tolua_S, (void*)tolua_ret, "ResourceDownload");
	tolua_register_gc(tolua_S, lua_gettop(tolua_S));
	addResObjectHandler(tolua_ret->getId());
	return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  ResourceDownload */
#ifndef TOLUA_DISABLE_tolua_ResourceDownload_ResourceDownload_new00
static int tolua_ResourceDownload_ResourceDownload_new00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertable(tolua_S, 1, "ResourceDownload", 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 2, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'new'.", &tolua_err);
		return 0;
	}
	ResourceDownload* tolua_ret = (ResourceDownload*)Mtolua_new((ResourceDownload)());
	if (!tolua_ret) {
		tolua_error(tolua_S, "invalid 'tolua_ret' in function 'new'", NULL);
		return 0;
	}
	tolua_pushusertype(tolua_S, (void*)tolua_ret, "ResourceDownload");
	addResObjectHandler(tolua_ret->getId());
	return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* method: delete of class  ResourceDownload */
#ifndef TOLUA_DISABLE_tolua_ResourceDownload_ResourceDownload_delete00
static int tolua_ResourceDownload_ResourceDownload_delete00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertype(tolua_S, 1, "ResourceDownload", 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 2, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'delete'.", &tolua_err);
		return 0;
	}
	ResourceDownload* self = (ResourceDownload*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'delete'", NULL);
		return 0;
	}
	removeResObjectHandler(tolua_S, self->getId());
	Mtolua_delete(self);
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: isnull of class  ResourceDownload */
#ifndef TOLUA_DISABLE_tolua_ResourceDownload_ResourceDownload_isnull00
static int tolua_ResourceDownload_ResourceDownload_isnull00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertype(tolua_S, 1, "ResourceDownload", 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 2, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'isnull'.", &tolua_err);
		return 0;
	}
	ResourceDownload* self = (ResourceDownload*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_pushboolean(tolua_S, (int)true);
	} else {
		tolua_pushboolean(tolua_S, (int)false);
	}
	return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* method: isdoing of class  ResourceDownload */
#ifndef TOLUA_DISABLE_tolua_ResourceDownload_ResourceDownload_isdoing00
static int tolua_ResourceDownload_ResourceDownload_isdoing00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertype(tolua_S, 1, "ResourceDownload", 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 2, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'isdoing'.", &tolua_err);
		return 0;
	}
	ResourceDownload* self = (ResourceDownload*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'isdoing'", NULL);
		return 0;
	}
	bool isDownloading = self->isDownloading();
	tolua_pushboolean(tolua_S, (int)isDownloading);
	return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* method: listen of class  ResourceDownload */
#ifndef TOLUA_DISABLE_tolua_ResourceDownload_ResourceDownload_listen00
static int tolua_ResourceDownload_ResourceDownload_listen00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertype(tolua_S, 1, "ResourceDownload", 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 2, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'listen'.", &tolua_err);
		return 0;
	}
	ResourceDownload* self = (ResourceDownload*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'listen'", NULL);
		return 0;
	}
	self->listen();
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: setTimeout of class  ResourceDownload */
#ifndef TOLUA_DISABLE_tolua_ResourceDownload_ResourceDownload_setTimeout00
static int tolua_ResourceDownload_ResourceDownload_setTimeout00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertype(tolua_S, 1, "ResourceDownload", 0, &tolua_err) ||
		!tolua_isnumber(tolua_S, 2, 0, &tolua_err) ||
		!tolua_isnumber(tolua_S, 3, 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 4, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setTimeout'.", &tolua_err);
		return 0;
	}
	ResourceDownload* self = (ResourceDownload*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setTimeout'", NULL);
		return 0;
	}
	unsigned int connectTimeout = (unsigned int)tolua_tonumber(tolua_S, 2, 0);
	unsigned int downloadTimeout = (unsigned int)tolua_tonumber(tolua_S, 3, 0);
	self->setTimeout(connectTimeout, downloadTimeout);
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: setErrorCB of class  ResourceDownload */
#ifndef TOLUA_DISABLE_tolua_ResourceDownload_ResourceDownload_setErrorCB00
static int tolua_ResourceDownload_ResourceDownload_setErrorCB00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	tolua_err.index = 2;
	tolua_err.array = 0;
	tolua_err.type = "function";
	if (!tolua_isusertype(tolua_S, 1, "ResourceDownload", 0, &tolua_err) ||
		!lua_isfunction(tolua_S, 2) ||
		!tolua_isnoobj(tolua_S, 3, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setErrorCB'.", &tolua_err);
		return 0;
	}
	ResourceDownload* self = (ResourceDownload*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setErrorCB'", NULL);
		return 0;
	}
	int handler = registerLuaFunction(tolua_S, 2);
	updateResObjectHandler(tolua_S, self->getId(), 1, handler);
	RDErrorCB callback = [=](const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer)->void{
		if (getLuaFunction(tolua_S, handler)) {
			callLuaFunction(tolua_S, handler, "siis", fileURL.c_str(), curlCode, responseCode, errorBuffer.c_str());
		}
	};
	self->setErrorCB(callback);
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: setProgressCB of class  ResourceDownload */
#ifndef TOLUA_DISABLE_tolua_ResourceDownload_ResourceDownload_setProgressCB00
static int tolua_ResourceDownload_ResourceDownload_setProgressCB00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	tolua_err.index = 2;
	tolua_err.array = 0;
	tolua_err.type = "function";
	if (!tolua_isusertype(tolua_S, 1, "ResourceDownload", 0, &tolua_err) ||
		!lua_isfunction(tolua_S, 2) ||
		!tolua_isnoobj(tolua_S, 3, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setProgressCB'.", &tolua_err);
		return 0;
	}
	ResourceDownload* self = (ResourceDownload*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setProgressCB'", NULL);
		return 0;
	}
	int handler = registerLuaFunction(tolua_S, 2);
	updateResObjectHandler(tolua_S, self->getId(), 2, handler);
	RDProgressCB callback = [=](const std::string& fileURL, double totalSize, double currSize)->void{
		if (getLuaFunction(tolua_S, handler)) {
			callLuaFunction(tolua_S, handler, "sdd", fileURL.c_str(), totalSize, currSize);
		}
	};
	self->setProgressCB(callback);
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: setSuccessCB of class  ResourceDownload */
#ifndef TOLUA_DISABLE_tolua_ResourceDownload_ResourceDownload_setSuccessCB00
static int tolua_ResourceDownload_ResourceDownload_setSuccessCB00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	tolua_err.index = 2;
	tolua_err.array = 0;
	tolua_err.type = "function";
	if (!tolua_isusertype(tolua_S, 1, "ResourceDownload", 0, &tolua_err) ||
		!lua_isfunction(tolua_S, 2) ||
		!tolua_isnoobj(tolua_S, 3, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setSuccessCB'.", &tolua_err);
		return 0;
	}
	ResourceDownload* self = (ResourceDownload*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setSuccessCB'", NULL);
		return 0;
	}
	int handler = registerLuaFunction(tolua_S, 2);
	updateResObjectHandler(tolua_S, self->getId(), 3, handler);
	RDSuccessCB callback = [=](const std::string& fileURL)->void{
		if (getLuaFunction(tolua_S, handler)) {
			callLuaFunction(tolua_S, handler, "s", fileURL.c_str());
		}
	};
	self->setSuccessCB(callback);
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: setTotalProgressCB of class  ResourceDownload */
#ifndef TOLUA_DISABLE_tolua_ResourceDownload_ResourceDownload_setTotalProgressCB00
static int tolua_ResourceDownload_ResourceDownload_setTotalProgressCB00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	tolua_err.index = 2;
	tolua_err.array = 0;
	tolua_err.type = "function";
	if (!tolua_isusertype(tolua_S, 1, "ResourceDownload", 0, &tolua_err) ||
		!lua_isfunction(tolua_S, 2) ||
		!tolua_isnoobj(tolua_S, 3, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setTotalProgressCB'.", &tolua_err);
		return 0;
	}
	ResourceDownload* self = (ResourceDownload*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setTotalProgressCB'", NULL);
		return 0;
	}
	int handler = registerLuaFunction(tolua_S, 2);
	updateResObjectHandler(tolua_S, self->getId(), 4, handler);
	RDTotalProgressCB callback = [=](const std::string& fileURL, int totalCount, int currCount)->void{
		if (getLuaFunction(tolua_S, handler)) {
			callLuaFunction(tolua_S, handler, "sii", fileURL.c_str(), totalCount, currCount);
		}
	};
	self->setTotalProgressCB(callback);
	return 0;	
}
#endif //#ifndef TOLUA_DISABLE

/* method: setTotalSuccessCB of class  ResourceDownload */
#ifndef TOLUA_DISABLE_tolua_ResourceDownload_ResourceDownload_setTotalSuccessCB00
static int tolua_ResourceDownload_ResourceDownload_setTotalSuccessCB00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	tolua_err.index = 2;
	tolua_err.array = 0;
	tolua_err.type = "function";
	if (!tolua_isusertype(tolua_S, 1, "ResourceDownload", 0, &tolua_err) ||
		!lua_isfunction(tolua_S, 2) ||
		!tolua_isnoobj(tolua_S, 3, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setTotalSuccessCB'.", &tolua_err);
		return 0;
	}
	ResourceDownload* self = (ResourceDownload*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setTotalSuccessCB'", NULL);
		return 0;
	}
	int handler = registerLuaFunction(tolua_S, 2);
	updateResObjectHandler(tolua_S, self->getId(), 5, handler);
	RDTotalSuccessCB callback = [=](void)->void{
		if (getLuaFunction(tolua_S, handler)) {
			callLuaFunction(tolua_S, handler, "");
		}
	};
	self->setTotalSuccessCB(callback);
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: setDownloadPath of class  ResourceDownload */
#ifndef TOLUA_DISABLE_tolua_ResourceDownload_ResourceDownload_setDownloadPath00
static int tolua_ResourceDownload_ResourceDownload_setDownloadPath00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertype(tolua_S, 1, "ResourceDownload", 0, &tolua_err) ||
		!tolua_isstring(tolua_S, 2, 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 3, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setDownloadPath'.", &tolua_err);
		return 0;
	}
	ResourceDownload* self = (ResourceDownload*)tolua_tousertype(tolua_S, 1, 0);
	const char* path = tolua_tocppstring(tolua_S, 2, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setDownloadPath'", NULL);
		return 0;
	}
	self->setDownloadPath(path);
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: excute of class  ResourceDownload */
#ifndef TOLUA_DISABLE_tolua_ResourceDownload_ResourceDownload_excute00
static int tolua_ResourceDownload_ResourceDownload_excute00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertype(tolua_S, 1, "ResourceDownload", 0, &tolua_err) ||
		(!tolua_isstring(tolua_S, 2, 0, &tolua_err) && !tolua_istable(tolua_S, 2, 0, &tolua_err)) ||
		!tolua_isstring(tolua_S, 3, 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 4, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'excute'.", &tolua_err);
		return 0;
	}
	ResourceDownload* self = (ResourceDownload*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'excute'", NULL);
		return 0;
	}
	std::vector<std::string> fileUrlVector;
	if (tolua_isstring(tolua_S, 2, 0, &tolua_err)) {
		const char* fileUrl = tolua_tostring(tolua_S, 2, 0);
		fileUrlVector.push_back(fileUrl);
	} else if (tolua_istable(tolua_S, 2, 0, &tolua_err)) {
		size_t len = lua_objlen(tolua_S, 2);
		for (size_t i=0; i<len; ++i) {
			lua_pushnumber(tolua_S, i + 1);
			lua_gettable(tolua_S, 2);
			if (!tolua_isstring(tolua_S, -1, 0, &tolua_err)) {
				tolua_error(tolua_S, "#ferror in function 'excute'.", &tolua_err);
				return 0;
			}
			const char* value = tolua_tostring(tolua_S, -1, 0);
			fileUrlVector.push_back(value);
			lua_pop(tolua_S, 1);
		}
	}
	const char* cacheSuffix = tolua_tostring(tolua_S, 3, 0);
	bool tolua_ret = self->excute(fileUrlVector, cacheSuffix);
	tolua_pushboolean(tolua_S, (int)tolua_ret);
	return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
int tolua_ResourceDownload_open(lua_State* tolua_S) {
	tolua_open(tolua_S);
	tolua_module(tolua_S, NULL, 0);
	tolua_beginmodule(tolua_S, NULL);
		tolua_usertype(tolua_S, "ResourceDownload");
	#ifdef __cplusplus
		tolua_cclass(tolua_S, "ResourceDownload", "ResourceDownload", "FileDownloadListener", tolua_collect_ResourceDownload);
	#else
		tolua_cclass(tolua_S, "ResourceDownload", "ResourceDownload", "FileDownloadListener", NULL);
	#endif
		tolua_beginmodule(tolua_S, "ResourceDownload");
			tolua_function(tolua_S, "create", tolua_ResourceDownload_ResourceDownload_create00);
			tolua_function(tolua_S, "new", tolua_ResourceDownload_ResourceDownload_new00);
			tolua_function(tolua_S, "delete", tolua_ResourceDownload_ResourceDownload_delete00);
			tolua_function(tolua_S, "isnull", tolua_ResourceDownload_ResourceDownload_isnull00);
			tolua_function(tolua_S, "isdoing", tolua_ResourceDownload_ResourceDownload_isdoing00);
			tolua_function(tolua_S, "listen", tolua_ResourceDownload_ResourceDownload_listen00);
			tolua_function(tolua_S, "setTimeout", tolua_ResourceDownload_ResourceDownload_setTimeout00);
			tolua_function(tolua_S, "setErrorCB", tolua_ResourceDownload_ResourceDownload_setErrorCB00);
			tolua_function(tolua_S, "setProgressCB", tolua_ResourceDownload_ResourceDownload_setProgressCB00);
			tolua_function(tolua_S, "setSuccessCB", tolua_ResourceDownload_ResourceDownload_setSuccessCB00);
			tolua_function(tolua_S, "setTotalProgressCB", tolua_ResourceDownload_ResourceDownload_setTotalProgressCB00);
			tolua_function(tolua_S, "setTotalSuccessCB", tolua_ResourceDownload_ResourceDownload_setTotalSuccessCB00);
			tolua_function(tolua_S, "setDownloadPath", tolua_ResourceDownload_ResourceDownload_setDownloadPath00);
			tolua_function(tolua_S, "excute", tolua_ResourceDownload_ResourceDownload_excute00);
		tolua_endmodule(tolua_S);
	tolua_endmodule(tolua_S);
	return 1;
}

/* function to release collected object via destructor */
#ifdef __cplusplus
static int tolua_collect_ResourceUpdate(lua_State* tolua_S) {
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (self && isExistResObjectHandler(self->getId())) {
		removeResObjectHandler(tolua_S, self->getId());
		self->record(true);
		Mtolua_delete(self);
	}
	return 0;
}
#endif

/* method: create of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_create00
static int tolua_ResourceUpdate_ResourceUpdate_create00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertable(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 2, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'create'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* tolua_ret = (ResourceUpdate*)Mtolua_new((ResourceUpdate)());
	if (!tolua_ret) {
		tolua_error(tolua_S, "invalid 'tolua_ret' in function 'create'", NULL);
		return 0;
	}
	tolua_pushusertype(tolua_S, (void*)tolua_ret, "ResourceUpdate");
	tolua_register_gc(tolua_S, lua_gettop(tolua_S));
	addResObjectHandler(tolua_ret->getId());
	return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_new00
static int tolua_ResourceUpdate_ResourceUpdate_new00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertable(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 2, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'new'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* tolua_ret = (ResourceUpdate*)Mtolua_new((ResourceUpdate)());
	if (!tolua_ret) {
		tolua_error(tolua_S, "invalid 'tolua_ret' in function 'create'", NULL);
		return 0;
	}
	tolua_pushusertype(tolua_S, (void*)tolua_ret, "ResourceUpdate");
	addResObjectHandler(tolua_ret->getId());
	return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* method: delete of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_delete00
static int tolua_ResourceUpdate_ResourceUpdate_delete00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 2, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'delete'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'delete'", NULL);
		return 0;
	}
	removeResObjectHandler(tolua_S, self->getId());
	self->record(true);
	Mtolua_delete(self);
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: isnull of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_isnull00
static int tolua_ResourceUpdate_ResourceUpdate_isnull00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 2, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'isnull'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_pushboolean(tolua_S, (int)true);
	} else {
		tolua_pushboolean(tolua_S, (int)false);
	}
	return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* method: isdoing of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_isdoing00
static int tolua_ResourceUpdate_ResourceUpdate_isdoing00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 2, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'isdoing'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'isdoing'", NULL);
		return 0;
	}
	bool isDownloading = self->isDownloading();
	tolua_pushboolean(tolua_S, (int)isDownloading);
	return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* method: listen of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_listen00
static int tolua_ResourceUpdate_ResourceUpdate_listen00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 2, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'listen'." ,&tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'listen'", NULL);
		return 0;
	}
	self->listen();
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: setTimeout of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_setTimeout00
static int tolua_ResourceUpdate_ResourceUpdate_setTimeout00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!tolua_isnumber(tolua_S, 2, 0, &tolua_err) ||
		!tolua_isnumber(tolua_S, 3 ,0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 4, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setTimeout'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setTimeout'", NULL);
		return 0;
	}
	unsigned int connectTimeout = (unsigned int)tolua_tonumber(tolua_S, 2, 0);
	unsigned int downloadTimeout = (unsigned int)tolua_tonumber(tolua_S, 3, 0);
	self->setTimeout(connectTimeout, downloadTimeout);
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: setCheckMD5 of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_setCheckMD500
static int tolua_ResourceUpdate_ResourceUpdate_setCheckMD500(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!tolua_isboolean(tolua_S, 2, 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 3, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setCheckMD5'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setCheckMD5'", NULL);
		return 0;
	}
	bool isCheckMD5 = tolua_toboolean(tolua_S, 2, 0) ? true : false;
	if (isCheckMD5) {
		RUMd5CheckFunc func = [=](const std::string& fileName)->std::string{
			std::string md5value;
			unsigned long fileLength = 0;
			char* fileData = (char*)ResourceUpdate::getFileData(fileName, &fileLength);
			if (fileData) {
				md5value = MD5_sign((unsigned char*)fileData, fileLength);
				delete fileData;
				fileData = NULL;
			}
			return md5value;
		};
		self->setFileMd5CheckFunc(func);
	} else {
		self->setFileMd5CheckFunc(nullptr);
	}
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: setUpdateListErrorCB of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_setUpdateListErrorCB00
static int tolua_ResourceUpdate_ResourceUpdate_setUpdateListErrorCB00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	tolua_err.index = 2;
	tolua_err.array = 0;
	tolua_err.type = "function";
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!lua_isfunction(tolua_S, 2) ||
		!tolua_isnoobj(tolua_S, 3, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setUpdateListErrorCB'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setUpdateListErrorCB'", NULL);
		return 0;
	}
	int handler = registerLuaFunction(tolua_S, 2);
	updateResObjectHandler(tolua_S, self->getId(), 1, handler);
	RUUpdateListErrorCB callback = [=](const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer)->void{
		if (getLuaFunction(tolua_S, handler)) {
			callLuaFunction(tolua_S, handler, "siis", fileURL.c_str(), curlCode, responseCode, errorBuffer.c_str());
		}
	};
	self->setUpdateListErrorCB(callback);
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: setUpdateListNotFoundCB of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_setUpdateListNotFoundCB00
static int tolua_ResourceUpdate_ResourceUpdate_setUpdateListNotFoundCB00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	tolua_err.index = 2;
	tolua_err.array = 0;
	tolua_err.type = "function";
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!lua_isfunction(tolua_S, 2) ||
		!tolua_isnoobj(tolua_S, 3, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setUpdateListNotFoundCB'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setUpdateListNotFoundCB'", NULL);
		return 0;
	}
	int handler = registerLuaFunction(tolua_S, 2);
	updateResObjectHandler(tolua_S, self->getId(), 2, handler);
	RUUpdateListNotFoundCB callback = [=](void)->void{
		if (getLuaFunction(tolua_S, handler)) {
			callLuaFunction(tolua_S, handler, "");
		}
	};
	self->setUpdateListNotFoundCB(callback);
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: setUpdateListCB of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_setUpdateListCB00
static int tolua_ResourceUpdate_ResourceUpdate_setUpdateListCB00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	tolua_err.index = 2;
	tolua_err.array = 0;
	tolua_err.type = "function";
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!lua_isfunction(tolua_S, 2) ||
		!tolua_isnoobj(tolua_S, 3, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setUpdateListCB'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setUpdateListCB'", NULL);
		return 0;
	}
	int handler = registerLuaFunction(tolua_S, 2);
	updateResObjectHandler(tolua_S, self->getId(), 3, handler);
	RUUpdateListCB callback = [=](long updateCount, long updateSize)->void{
		if (getLuaFunction(tolua_S, handler)) {
			callLuaFunction(tolua_S, handler, "ll", updateCount, updateSize);
		}
	};
	self->setUpdateListCB(callback);
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: setErrorCB of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_setErrorCB00
static int tolua_ResourceUpdate_ResourceUpdate_setErrorCB00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	tolua_err.index = 2;
	tolua_err.array = 0;
	tolua_err.type = "function";
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!lua_isfunction(tolua_S, 2) ||
		!tolua_isnoobj(tolua_S, 3, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setErrorCB'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setErrorCB'", NULL);
		return 0;
	}
	int handler = registerLuaFunction(tolua_S, 2);
	updateResObjectHandler(tolua_S, self->getId(), 4, handler);
	RUErrorCB callback = [=](const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer)->void{
		if (getLuaFunction(tolua_S, handler)) {
			callLuaFunction(tolua_S, handler, "siis", fileURL.c_str(), curlCode, responseCode, errorBuffer.c_str());
		}
	};
	self->setErrorCB(callback);
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: setProgressCB of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_setProgressCB00
static int tolua_ResourceUpdate_ResourceUpdate_setProgressCB00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	tolua_err.index = 2;
	tolua_err.array = 0;
	tolua_err.type = "function";
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!lua_isfunction(tolua_S, 2) ||
		!tolua_isnoobj(tolua_S, 3, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setProgressCB'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setProgressCB'", NULL);
		return 0;
	}
	int handler = registerLuaFunction(tolua_S, 2);
	updateResObjectHandler(tolua_S, self->getId(), 5, handler);
	RUProgressCB callback = [=](const std::string& fileURL, double totalSize, double currSize)->void{
		if (getLuaFunction(tolua_S, handler)) {
			callLuaFunction(tolua_S, handler, "sdd", fileURL.c_str(), totalSize, currSize);
		}
	};
	self->setProgressCB(callback);
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: setSuccessCB of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_setSuccessCB00
static int tolua_ResourceUpdate_ResourceUpdate_setSuccessCB00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	tolua_err.index = 2;
	tolua_err.array = 0;
	tolua_err.type = "function";
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!lua_isfunction(tolua_S, 2) ||
		!tolua_isnoobj(tolua_S, 3, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setSuccessCB'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setSuccessCB'", NULL);
		return 0;
	}
	int handler = registerLuaFunction(tolua_S, 2);
	updateResObjectHandler(tolua_S, self->getId(), 6, handler);
	RUSuccessCB callback = [=](const std::string& fileURL)->void{
		if (getLuaFunction(tolua_S, handler)) {
			callLuaFunction(tolua_S, handler, "s", fileURL.c_str());
		}
	};
	self->setSuccessCB(callback);
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: setTotalProgressCB of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_setTotalProgressCB00
static int tolua_ResourceUpdate_ResourceUpdate_setTotalProgressCB00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	tolua_err.index = 2;
	tolua_err.array = 0;
	tolua_err.type = "function";
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!lua_isfunction(tolua_S, 2) ||
		!tolua_isnoobj(tolua_S, 3, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setTotalProgressCB'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setTotalProgressCB'", NULL);
		return 0;
	}
	int handler = registerLuaFunction(tolua_S, 2);
	updateResObjectHandler(tolua_S, self->getId(), 7, handler);
	RUTotalProgressCB callback = [=](const std::string& fileURL, int totalCount, int currCount)->void{
		if (getLuaFunction(tolua_S, handler)) {
			callLuaFunction(tolua_S, handler, "sii", fileURL.c_str(), totalCount, currCount);
		}
	};
	self->setTotalProgressCB(callback);
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: setTotalSuccessCB of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_setTotalSuccessCB00
static int tolua_ResourceUpdate_ResourceUpdate_setTotalSuccessCB00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	tolua_err.index = 2;
	tolua_err.array = 0;
	tolua_err.type = "function";
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!lua_isfunction(tolua_S, 2) ||
		!tolua_isnoobj(tolua_S, 3, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setTotalSuccessCB'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setTotalSuccessCB'", NULL);
		return 0;
	}
	int handler = registerLuaFunction(tolua_S, 2);
	updateResObjectHandler(tolua_S, self->getId(), 8, handler);
	RUTotalSuccessCB callback = [=](void)->void{
		if (getLuaFunction(tolua_S, handler)) {
			callLuaFunction(tolua_S, handler, "");
		}
	};
	self->setTotalSuccessCB(callback);
	return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: setNative of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_setNative00
static int tolua_ResourceUpdate_ResourceUpdate_setNative00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!tolua_isstring(tolua_S, 2, 0, &tolua_err) ||
		!tolua_isstring(tolua_S, 3, 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 4, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'setNative'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'setNative'", NULL);
		return 0;
	}
	const char* path = tolua_tostring(tolua_S, 2, 0);
	const char* nativeMd5File = tolua_tostring(tolua_S, 3, 0);
	bool tolua_ret = self->setNative(path, nativeMd5File);
	tolua_pushboolean(tolua_S, (int)tolua_ret);
	return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* method: checkUpdate of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_checkUpdate00
static int tolua_ResourceUpdate_ResourceUpdate_checkUpdate00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!tolua_isstring(tolua_S, 2, 0, &tolua_err) ||
		(!tolua_isstring(tolua_S, 3, 0, &tolua_err) && !tolua_istable(tolua_S, 3, 0, &tolua_err)) ||
		!tolua_isnoobj(tolua_S, 4, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'checkUpdate'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'checkUpdate'", NULL);
		return 0;
	}
	const char* url = tolua_tostring(tolua_S, 2, 0);
	std::vector<std::string> md5FileVector;
	if (tolua_isstring(tolua_S, 3, 0, &tolua_err)) {
		const char* checkMd5File = tolua_tostring(tolua_S, 3, 0);
		md5FileVector.push_back(checkMd5File);
	} else if (tolua_istable(tolua_S, 3, 0, &tolua_err)) {
		size_t len = lua_objlen(tolua_S, 3);
		for (size_t i=0; i<len; ++i) {
			lua_pushnumber(tolua_S, i + 1);
			lua_gettable(tolua_S, 3);
			if (!tolua_isstring(tolua_S, -1, 0, &tolua_err)) {
				tolua_error(tolua_S, "#ferror in function 'checkUpdate'.", &tolua_err);
				return 0;
			}
			const char* value = tolua_tostring(tolua_S, -1, 0);
			md5FileVector.push_back(value);
			lua_pop(tolua_S, 1);
		}
	}
	bool tolua_ret = self->checkUpdate(url, md5FileVector);
	tolua_pushboolean(tolua_S, (int)tolua_ret);
	return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* method: startUpdate of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_startUpdate00
static int tolua_ResourceUpdate_ResourceUpdate_startUpdate00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!tolua_isstring(tolua_S, 2, 0, &tolua_err) ||
		!tolua_isboolean(tolua_S, 3, 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 4, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'startUpdate'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'startUpdate'", NULL);
		return 0;
	}
	const char* cacheSuffix = tolua_tostring(tolua_S, 2, 0);
	bool removeInvalidFileFlag = tolua_toboolean(tolua_S, 3, 0) ? true : false;
	bool tolua_ret = self->startUpdate(cacheSuffix, removeInvalidFileFlag);
	tolua_pushboolean(tolua_S, (int)tolua_ret);
	return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* method: record of class  ResourceUpdate */
#ifndef TOLUA_DISABLE_tolua_ResourceUpdate_ResourceUpdate_record00
static int tolua_ResourceUpdate_ResourceUpdate_record00(lua_State* tolua_S) {
	tolua_Error tolua_err;
	if (!tolua_isusertype(tolua_S, 1, "ResourceUpdate", 0, &tolua_err) ||
		!tolua_isboolean(tolua_S, 2, 0, &tolua_err) ||
		!tolua_isnoobj(tolua_S, 3, &tolua_err)) {
		tolua_error(tolua_S, "#ferror in function 'record'.", &tolua_err);
		return 0;
	}
	ResourceUpdate* self = (ResourceUpdate*)tolua_tousertype(tolua_S, 1, 0);
	if (!self || !isExistResObjectHandler(self->getId())) {
		tolua_error(tolua_S, "invalid 'self' in function 'record'", NULL);
		return 0;
	}
	bool immediatelyFlag = tolua_toboolean(tolua_S, 2, 0) ? true : false;
	bool tolua_ret = self->record(immediatelyFlag);
	tolua_pushboolean(tolua_S, (int)tolua_ret);
	return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
int tolua_ResourceUpdate_open(lua_State* tolua_S) {
	tolua_open(tolua_S);
	tolua_module(tolua_S, NULL, 0);
	tolua_beginmodule(tolua_S, NULL);
	tolua_usertype(tolua_S, "ResourceUpdate");
	#ifdef __cplusplus
		tolua_cclass(tolua_S, "ResourceUpdate", "ResourceUpdate", "FileDownloadListener", tolua_collect_ResourceUpdate);
	#else
		tolua_cclass(tolua_S, "ResourceUpdate", "ResourceUpdate", "FileDownloadListener", NULL);
	#endif
		tolua_beginmodule(tolua_S, "ResourceUpdate");
			tolua_function(tolua_S, "create", tolua_ResourceUpdate_ResourceUpdate_create00);
			tolua_function(tolua_S, "new", tolua_ResourceUpdate_ResourceUpdate_new00);
			tolua_function(tolua_S, "delete", tolua_ResourceUpdate_ResourceUpdate_delete00);
			tolua_function(tolua_S, "isnull", tolua_ResourceUpdate_ResourceUpdate_isnull00);
			tolua_function(tolua_S, "isdoing", tolua_ResourceUpdate_ResourceUpdate_isdoing00);
			tolua_function(tolua_S, "listen", tolua_ResourceUpdate_ResourceUpdate_listen00);
			tolua_function(tolua_S, "setTimeout", tolua_ResourceUpdate_ResourceUpdate_setTimeout00);
			tolua_function(tolua_S, "setCheckMD5", tolua_ResourceUpdate_ResourceUpdate_setCheckMD500);
			tolua_function(tolua_S, "setUpdateListErrorCB", tolua_ResourceUpdate_ResourceUpdate_setUpdateListErrorCB00);
			tolua_function(tolua_S, "setUpdateListNotFoundCB", tolua_ResourceUpdate_ResourceUpdate_setUpdateListNotFoundCB00);
			tolua_function(tolua_S, "setUpdateListCB", tolua_ResourceUpdate_ResourceUpdate_setUpdateListCB00);
			tolua_function(tolua_S, "setErrorCB", tolua_ResourceUpdate_ResourceUpdate_setErrorCB00);
			tolua_function(tolua_S, "setProgressCB", tolua_ResourceUpdate_ResourceUpdate_setProgressCB00);
			tolua_function(tolua_S, "setSuccessCB", tolua_ResourceUpdate_ResourceUpdate_setSuccessCB00);
			tolua_function(tolua_S, "setTotalProgressCB", tolua_ResourceUpdate_ResourceUpdate_setTotalProgressCB00);
			tolua_function(tolua_S, "setTotalSuccessCB", tolua_ResourceUpdate_ResourceUpdate_setTotalSuccessCB00);
			tolua_function(tolua_S, "setNative", tolua_ResourceUpdate_ResourceUpdate_setNative00);
			tolua_function(tolua_S, "checkUpdate", tolua_ResourceUpdate_ResourceUpdate_checkUpdate00);
			tolua_function(tolua_S, "startUpdate", tolua_ResourceUpdate_ResourceUpdate_startUpdate00);
			tolua_function(tolua_S, "record", tolua_ResourceUpdate_ResourceUpdate_record00);
		tolua_endmodule(tolua_S);
	tolua_endmodule(tolua_S);
	return 1;
}

#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
int lua_resource_register(lua_State* L) {
	tolua_ResourceDownload_open(L);
	tolua_ResourceUpdate_open(L);
	return 1;
}
#endif
