#include "tolua_fix.h"
#include <stdlib.h>
//--------------------------------------------------------------------
const char* TOLUA_REFID_FUNCTION_MAPPING = "toluafix_refid_function_mapping";
static int s_function_ref_id = 0;
//--------------------------------------------------------------------
TOLUA_API void toluafix_open(lua_State* L)
{
    lua_pushstring(L, TOLUA_REFID_FUNCTION_MAPPING);
    lua_newtable(L);
    lua_rawset(L, LUA_REGISTRYINDEX);
}
//--------------------------------------------------------------------
TOLUA_API int toluafix_ref_function(lua_State* L, int lo)
{
    // function at lo
    if (!lua_isfunction(L, lo))
	{
		return 0;
	}
    lua_pushstring(L, TOLUA_REFID_FUNCTION_MAPPING);
    lua_rawget(L, LUA_REGISTRYINDEX);                           /* stack: fun ... refid_fun */
    lua_pushinteger(L, ++s_function_ref_id);                    /* stack: fun ... refid_fun refid */
    lua_pushvalue(L, lo);                                       /* stack: fun ... refid_fun refid fun */
    lua_rawset(L, -3);                  /* refid_fun[refid] = fun, stack: fun ... refid_ptr */
    lua_pop(L, 1);                                              /* stack: fun ... */
    return s_function_ref_id;
}
//--------------------------------------------------------------------
TOLUA_API void toluafix_get_function_by_refid(lua_State* L, int refid)
{
    lua_pushstring(L, TOLUA_REFID_FUNCTION_MAPPING);
    lua_rawget(L, LUA_REGISTRYINDEX);                           /* stack: ... refid_fun */
    lua_pushinteger(L, refid);                                  /* stack: ... refid_fun refid */
    lua_rawget(L, -2);                                          /* stack: ... refid_fun fun */
    lua_remove(L, -2);                                          /* stack: ... fun */
}
//--------------------------------------------------------------------
TOLUA_API void toluafix_remove_function_by_refid(lua_State* L, int refid)
{
    lua_pushstring(L, TOLUA_REFID_FUNCTION_MAPPING);
    lua_rawget(L, LUA_REGISTRYINDEX);                           /* stack: ... refid_fun */
    lua_pushinteger(L, refid);                                  /* stack: ... refid_fun refid */
    lua_pushnil(L);                                             /* stack: ... refid_fun refid nil */
    lua_rawset(L, -3);                  /* refid_fun[refid] = fun, stack: ... refid_ptr */
    lua_pop(L, 1);                                              /* stack: ... */
}
//--------------------------------------------------------------------
// check lua value is funciton
TOLUA_API int toluafix_isfunction(lua_State* L, int lo, tolua_Error* err)
{
    if (lua_gettop(L) >= abs(lo) && lua_isfunction(L, lo))
    {
        return 1;
    }
    err->index = lo;
    err->array = 0;
    err->type = "[not function]";
    return 0;
}
//--------------------------------------------------------------------
TOLUA_API void toluafix_stack_dump(lua_State* L, const char* label)
{
    int i;
    int top = lua_gettop(L);
    printf("Total [%d] in lua stack: %s\n", top, label ? label : "");
    for (i = -1; i >= -top; --i)
    {
        int t = lua_type(L, i);
        switch (t)
        {
		case LUA_TSTRING:
			printf("  [%02d] string %s\n", i, lua_tostring(L, i));
			break;
		case LUA_TBOOLEAN:
			printf("  [%02d] boolean %s\n", i, lua_toboolean(L, i) ? "true" : "false");
			break;
		case LUA_TNUMBER:
			printf("  [%02d] number %g\n", i, lua_tonumber(L, i));
			break;
		default:
			printf("  [%02d] %s\n", i, lua_typename(L, t));
        }
    }
    printf("\n");
}
//--------------------------------------------------------------------
TOLUA_API const char* toluafix_call_function_by_refid(lua_State* L, int refid, const char* sig, ...)
{
	static char errorbuffer[512];
	memset(errorbuffer, 0, sizeof(errorbuffer));
	if (NULL == L || refid <= 0 || NULL == sig)
	{
		sprintf(errorbuffer, "invalid args");
		return errorbuffer;
	}
	va_list vl;
	va_start(vl, sig);
	int narg = 0;										// number of arguments
	while (*sig)										// push arguments
	{
		switch (*sig++)
		{
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
	if (toluafix_get_function_by_refid(L, refid))		// L: ... arg1 arg2 ... func
    {
        if (narg > 0)
        {
            lua_insert(L, -(narg + 1));						// L: ... func arg1 arg2 ...
        }
		int functionIndex = -(narg + 1);
		if (!lua_isfunction(L, functionIndex))
		{
			lua_pop(L, narg + 1);							// remove function and arguments
			return errorbuffer;
		}
		int traceback = 0;
		lua_getglobal(L, "__G__TRACKBACK__");				// L: ... func arg1 arg2 ... G
		if (!lua_isfunction(L, -1))
		{
			lua_pop(L, 1);									// L: ... func arg1 arg2 ...
		}
		else
		{
			lua_insert(L, functionIndex - 1);				// L: ... G func arg1 arg2 ...
			traceback = functionIndex - 1;
		}
		if (lua_pcall(L, narg, 1, traceback))				// L: ... error
		{
			if (0 == traceback)
			{
				sprintf(errorbuffer, lua_tostring(L, - 1));
				lua_pop(L, 1);								// remove error message from stack
			}
			else											// L: ... G error
			{
				lua_pop(L, 2);								// remove __G__TRACKBACK__ and error message from stack
			}
			return errorbuffer;
		}
		// get return value
		int ret = 0;
		if (lua_isnumber(L, -1))
		{
			ret = (int)lua_tointeger(L, -1);
		}
		else if (lua_isboolean(L, -1))
		{
			ret = (int)lua_toboolean(L, -1);
		}
		// remove return value from stack
		lua_pop(L, 1);										// L: ... [G]
		if (traceback)
		{
			lua_pop(L, 1);									// remove __G__TRACKBACK__ from stack
		}
    }
    lua_settop(L, 0);
	va_end(vl);
	return errorbuffer;
}
//--------------------------------------------------------------------
TOLUA_API const char* toluafix_call_function_by_name(lua_State* L, const char* name, const char* sig, ...)
{
	static char errorbuffer[512];
	memset(errorbuffer, 0, sizeof(errorbuffer));
	if (NULL == L || NULL == name || 0 == strcmp(name, "") || NULL == sig)
	{
		sprintf(errorbuffer, "invalid args");
		return errorbuffer;
	}
	lua_getglobal(L, name);		// get function
	va_list vl;
	va_start(vl, sig);
	int narg = 0;							// number of arguments
	while (*sig)							// push arguments
	{
		switch (*sig++)
		{
		case 'i': lua_pushnumber(L, va_arg(vl, int)); break;					// int argument
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
	int nres = strlen(sig);					// number of expected results
	if (0 != lua_pcall(L, narg, nres, 0))	// call function
	{
		sprintf(errorbuffer, "error running function '%s': %s", name, lua_tostring(L, -1));
		return errorbuffer;
	}
	nres = -nres;							// stack index of first result
	while (*sig)							// get results
	{
		switch (*sig++)
		{
		case 'i': if (lua_isnumber(L, nres)) { *va_arg(vl, int*) = (int)lua_tonumber(L, nres); } break;						// int result
		case 'l': if (lua_isnumber(L, nres)) { *va_arg(vl, long*) = (long)lua_tonumber(L, nres); } break;					// long result
		case 'd': if (lua_isnumber(L, nres)) { *va_arg(vl, double*) = (double)lua_tonumber(L, nres); } break;				// double result
		case 's': if (lua_isstring(L, nres)) { *va_arg(vl, const char**) = lua_tostring(L, nres); } break;					// string result
		case 'b': if (lua_isboolean(L, nres)) { *va_arg(vl, bool*) = 0 == lua_toboolean(L, nres) ? false : true; } break;	// bool result
		default: sprintf(errorbuffer, "invalid option (%c)", *(sig - 1)); return errorbuffer;
		}
		nres++;
	}
	va_end(vl);
	return errorbuffer;
}
//--------------------------------------------------------------------