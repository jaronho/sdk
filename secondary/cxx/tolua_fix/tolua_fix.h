#ifndef _TOLUA_FIX_H_
#define _TOLUA_FIX_H_
#include "tolua++.h"
TOLUA_API void toluafix_open(lua_State* L);
TOLUA_API int toluafix_ref_function(lua_State* L, int lo);
TOLUA_API void toluafix_get_function_by_refid(lua_State* L, int refid);
TOLUA_API void toluafix_remove_function_by_refid(lua_State* L, int refid);
TOLUA_API int toluafix_isfunction(lua_State* L, int lo, tolua_Error* err);
TOLUA_API void toluafix_stack_dump(lua_State* L, const char* label);
TOLUA_API const char* toluafix_call_function_by_refid(lua_State* L, int refid, const char* sig, ...);
TOLUA_API const char* toluafix_call_function_by_name(lua_State* L, const char* name, const char* sig, ...);
#endif	// _TOLUA_FIX_H_