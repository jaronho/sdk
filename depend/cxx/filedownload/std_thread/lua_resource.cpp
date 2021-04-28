/**********************************************************************
* Author:	jaron.ho
* Date:		2014-02-14
* Brief:	资源下载
**********************************************************************/
#include "lua_resource.h"

const char* callLuaGlobalFunction(lua_State* L, const char* globalFuncName, const char* sig, ...)
{
	static char errorbuffer[512];
	memset(errorbuffer, 0, sizeof(errorbuffer));
	if (NULL == L || NULL == globalFuncName || 0 == strcmp(globalFuncName, "") || NULL == sig)
	{
		sprintf(errorbuffer, "invalid args");
		return errorbuffer;
	}
	lua_getglobal(L, globalFuncName);		// get function
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
		sprintf(errorbuffer, "error running function '%s': %s", globalFuncName, lua_tostring(L, -1));
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

/**********************************************************************
************************** 资源下载模块
**********************************************************************/

ResDownload::ResDownload(void)
{
	mFileDownload.setListener(this);
	mLuaState = NULL;
}

ResDownload::~ResDownload(void)
{
}

void ResDownload::onProgress(FileDownloadCode code, const std::string& fileURL, const std::string& buffer, double totalToDownload, double nowDownloaded)
{
	if (FDC_FILE_PROGRESS == code)
	{
		callLuaGlobalFunction(mLuaState, mProgressFuncName.c_str(), "ssdd", fileURL.c_str(), buffer.c_str(), totalToDownload, nowDownloaded);
	}
	else if (FDC_LIST_PROGRESS == code)
	{
		callLuaGlobalFunction(mLuaState, mTotalProgressFuncName.c_str(), "ssdd", fileURL.c_str(), buffer.c_str(), totalToDownload, nowDownloaded);
	}
}

void ResDownload::onSuccess(FileDownloadCode code, const std::string& fileURL, const std::string& buffer)
{
	if (FDC_FILE_SUCCESS == code)
	{
		callLuaGlobalFunction(mLuaState, mSuccessFuncName.c_str(), "ss", fileURL.c_str(), buffer.c_str());
	}
	else if (FDC_LIST_SUCCESS == code)
	{
		callLuaGlobalFunction(mLuaState, mTotalSuccessFuncName.c_str(), "ss", fileURL.c_str(), buffer.c_str());
	}
}

void ResDownload::onError(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer)
{
	if (FDC_DOWNLOAD_ERROR == code)
	{
		callLuaGlobalFunction(mLuaState, mErrorFuncName.c_str(), "siis", fileURL.c_str(), curlCode, responseCode, buffer.c_str());
	}
}

void ResDownload::listen(void)
{
	mFileDownload.listenMessage();
}

void ResDownload::setPath(const std::string& path)
{
	if (path.empty())
		return;

	mFileDownload.setStoragePath(path);
}

void ResDownload::setTimeout(unsigned int connectTimeout, unsigned int downloadTimeout)
{
	mFileDownload.setConnectTimeout(connectTimeout);
	mFileDownload.setDownloadTimeout(downloadTimeout);
}

void ResDownload::excute(const std::vector<std::string>& fileUrlVec)
{
	if (fileUrlVec.empty())
		return;

	mFileDownload.download(fileUrlVec);
}

void ResDownload::openLua(lua_State* L)
{
	mLuaState = L;
}

void ResDownload::registerLuaHandler(ResDownloadListenerType listenerType, const std::string& globalFuncName)
{
	switch (listenerType)
	{
	case RDLT_PROGRESS: mProgressFuncName = globalFuncName; break;
	case RDLT_SUCCESS: mSuccessFuncName = globalFuncName; break;
	case RDLT_TOTAL_PROGRESS: mTotalProgressFuncName = globalFuncName; break;
	case RDLT_TOTAL_SUCCESS: mTotalSuccessFuncName = globalFuncName; break;
	case RDLT_ERROR: mErrorFuncName = globalFuncName; break;
	}
}

static ResDownload* sResDownload = NULL;

static int res_download_listen(lua_State* L)
{
	if (sResDownload)
	{
		sResDownload->listen();
	}
	return 0;
}

static int res_download_setPath(lua_State* L)
{
	const char* path = (char*)luaL_checkstring(L, 1);
	if (sResDownload)
	{
		sResDownload->setPath(path);
	}
	return 0;
}

static int res_download_setTimeout(lua_State* L)
{
	unsigned int connectTimeout = (unsigned int)luaL_checknumber(L, 1);
	unsigned int downloadTimeout = (unsigned int)luaL_checknumber(L, 2);
	if (sResDownload)
	{
		sResDownload->setTimeout(connectTimeout, downloadTimeout);
	}
	return 0;
}

static int res_download_excute(lua_State* L)
{
	const char* fileUrl = (char*)luaL_checkstring(L, 1);
	std::vector<std::string> urlVec;
	urlVec.push_back(fileUrl);
	if (sResDownload)
	{
		sResDownload->excute(urlVec);
	}
	return 0;
}

static int res_download_setListener(lua_State* L)
{
	int listenerType = (int)luaL_checknumber(L, 1);
	const char* globalFuncName = (char*)luaL_checkstring(L, 2);
	if (sResDownload)
	{
		sResDownload->registerLuaHandler((ResDownloadListenerType)listenerType, NULL == globalFuncName ? "" : globalFuncName);
	}
	return 0;
}

void lua_resdownload_register(lua_State* L, const std::string& path /*= ""*/)
{
	if (sResDownload)
		return;

	// 透露接口给lua
	static const luaL_reg R[] =
	{
		{"listen",			res_download_listen},
		{"setPath",			res_download_setPath},
		{"setTimeout",		res_download_setTimeout},
		{"excute",			res_download_excute},
		{"setListener",		res_download_setListener},
		{NULL,	NULL}
	};
	luaL_openlib(L, "ResDownload", R, 0);
	// 实例化
	sResDownload = new ResDownload();
	sResDownload->setPath(path);
	sResDownload->openLua(L);
}

/**********************************************************************
************************** 资源更新模块
**********************************************************************/

ResUpdate::ResUpdate(void)
{
	mResourceUpdate = new ResourceUpdate(this);
	mLuaState = NULL;
}

ResUpdate::~ResUpdate(void)
{
	if (mResourceUpdate)
	{
		delete mResourceUpdate;
		mResourceUpdate = NULL;
	}
}

void ResUpdate::onCheckVersionFailed(const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer)
{
	callLuaGlobalFunction(mLuaState, mCheckVersionFailedFuncName.c_str(), "siis", fileURL.c_str(), curlCode, responseCode, errorBuffer.c_str());
}

void ResUpdate::onNewVersion(const std::string& curVersion, const std::string& newVersion)
{
	callLuaGlobalFunction(mLuaState, mNewVersionFuncName.c_str(), "ss", curVersion.c_str(), newVersion.c_str());
}

void ResUpdate::onNoNewVersion(const std::string& curVersion)
{
	callLuaGlobalFunction(mLuaState, mNoNewVersionFuncName.c_str(), "s", curVersion.c_str());
}

void ResUpdate::onCheckUpdateListFailed(const std::string& fileURL, int curlCode, int responseCode, const std::string errorBuffer)
{
	callLuaGlobalFunction(mLuaState, mCheckUpdateListFailedFuncName.c_str(), "siis", fileURL.c_str(), curlCode, responseCode,errorBuffer.c_str());
}

void ResUpdate::onUpdateList(long updateCount, long updateSize)
{
	callLuaGlobalFunction(mLuaState, mUpdateListFuncName.c_str(), "ll", updateCount, updateSize);
}

void ResUpdate::onNoUpdateList(void)
{
	callLuaGlobalFunction(mLuaState, mNoUpdateListFuncName.c_str(), "");
}

void ResUpdate::onPogress(const std::string& fileURL, double totalSize, double curSize)
{
	callLuaGlobalFunction(mLuaState, mProgressFuncName.c_str(), "sdd", fileURL.c_str(), totalSize, curSize);
}

void ResUpdate::onSuccess(const std::string& fileURL)
{
	callLuaGlobalFunction(mLuaState, mSuccessFuncName.c_str(), "s", fileURL.c_str());
}

void ResUpdate::onTotalProgress(const std::string& fileURL, int totalCount, int curCount)
{
	callLuaGlobalFunction(mLuaState, mTotalProgressFuncName.c_str(), "sii", fileURL.c_str(), totalCount, curCount);
}

void ResUpdate::onTotalSuccess(void)
{
	callLuaGlobalFunction(mLuaState, mTotalSuccessFuncName.c_str(), "");
}

void ResUpdate::onError(const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer)
{
	callLuaGlobalFunction(mLuaState, mErrorFuncName.c_str(), "siis", fileURL.c_str(), curlCode, responseCode, errorBuffer.c_str());
}

void ResUpdate::listen(void)
{
	if (mResourceUpdate)
	{
		mResourceUpdate->listen();
	}
}

void ResUpdate::setTimeout(unsigned int connectTimeout, unsigned int downloadTimeout)
{
	if (mResourceUpdate)
	{
		mResourceUpdate->setTimeout(connectTimeout, downloadTimeout);
	}
}

void ResUpdate::setNative(const std::string& path, const std::string& nativeVersionFile, const std::string& nativeMd5File)
{
	if (mResourceUpdate && !path.empty() && !nativeVersionFile.empty() && !nativeMd5File.empty())
	{
		mResourceUpdate->setNative(path, nativeVersionFile, nativeMd5File);
	}
}

void ResUpdate::checkVersion(const std::string& url, const std::string& checkVersionFile, const std::string& checkMd5File)
{
	if (mResourceUpdate && !url.empty() && !checkVersionFile.empty() && !checkMd5File.empty())
	{
		mResourceUpdate->checkVersion(url, checkVersionFile, checkMd5File);
	}
}

void ResUpdate::checkUpdate(bool deepCheck /*= true*/)
{
	if (mResourceUpdate)
	{
		mResourceUpdate->checkUpdate(deepCheck);
	}
}

void ResUpdate::startUpdate(void)
{
	if (mResourceUpdate)
	{
		mResourceUpdate->startUpdate();
	}
}

void ResUpdate::record(void)
{
	if (mResourceUpdate)
	{
		mResourceUpdate->record();
	}
}

void ResUpdate::openLua(lua_State* L)
{
	mLuaState = L;
}

void ResUpdate::registerLuaHandler(ResUpdateListenerType listenerType, const std::string& globalFuncName)
{
	switch (listenerType)
	{
	case RULT_CHECK_VERSION_FAILED: mCheckVersionFailedFuncName = globalFuncName; break;
	case RULT_NEW_VERSION: mNewVersionFuncName = globalFuncName; break;
	case RULT_NO_NEW_VERSION: mNoNewVersionFuncName = globalFuncName; break;
	case RULT_CHECK_UPDATE_LIST_FAILED: mCheckUpdateListFailedFuncName = globalFuncName; break;
	case RULT_UPDATE_LIST: mUpdateListFuncName = globalFuncName; break;
	case RULT_NO_UPDATE_LIST: mNoUpdateListFuncName = globalFuncName; break;
	case RULT_PROGRESS: mProgressFuncName = globalFuncName; break;
	case RULT_SUCCESS: mSuccessFuncName = globalFuncName; break;
	case RULT_TOTAL_PROGRESS: mTotalProgressFuncName = globalFuncName; break;
	case RULT_TOTAL_SUCCESS: mTotalSuccessFuncName = globalFuncName; break;
	case RULT_ERROR: mErrorFuncName = globalFuncName; break;
	}
}

static ResUpdate* sResUpdate = NULL;

static int res_update_listen(lua_State* L)
{
	if (sResUpdate)
	{
		sResUpdate->listen();
	}
	return 0;
}

static int res_update_setTimeout(lua_State* L)
{
	unsigned int connectTimeout = (unsigned int)luaL_checknumber(L, 1);
	unsigned int downloadTimeout = (unsigned int)luaL_checknumber(L, 2);
	if (sResUpdate)
	{
		sResUpdate->setTimeout(connectTimeout, downloadTimeout);
	}
	return 0;
}

static int res_update_setNative(lua_State* L)
{
	const char* path = (char*)luaL_checkstring(L, 1);
	const char* nativeVersionFile = (char*)luaL_checkstring(L, 2);
	const char* nativeMd5File = (char*)luaL_checkstring(L, 3);
	if (sResUpdate && path && nativeVersionFile && nativeMd5File)
	{
		sResUpdate->setNative(path, nativeVersionFile, nativeMd5File);
	}
	return 0;
}

static int res_update_checkVersion(lua_State* L)
{
	const char* url = (char*)luaL_checkstring(L, 1);
	const char* checkVersionFile = (char*)luaL_checkstring(L, 2);
	const char* checkMd5File = (char*)luaL_checkstring(L, 3);
	if (sResUpdate && url && checkVersionFile && checkMd5File)
	{
		sResUpdate->checkVersion(url, checkVersionFile, checkMd5File);
	}
	return 0;
}

static int res_update_checkUpdate(lua_State* L)
{
	int deepCheck = luaL_checknumber(L, 1);
	if (sResUpdate)
	{
		sResUpdate->checkUpdate(deepCheck ? true : false);
	}
	return 0;
}

static int res_update_startUpdate(lua_State* L)
{
	if (sResUpdate)
	{
		sResUpdate->startUpdate();
	}
	return 0;
}

static int res_update_record(lua_State* L)
{
	if (sResUpdate)
	{
		sResUpdate->record();
	}
	return 0;
}

static int res_update_setListener(lua_State* L)
{
	int listenerType = (int)luaL_checknumber(L, 1);
	const char* globalFuncName = (char*)luaL_checkstring(L, 2);
	if (sResUpdate)
	{
		sResUpdate->registerLuaHandler((ResUpdateListenerType)listenerType, NULL == globalFuncName ? "" : globalFuncName);
	}
	return 0;
}

void lua_resupdate_register(lua_State* L, const std::string& path /*= ""*/, const std::string& nativeVersionFile /*= ""*/, const std::string& nativeMd5File /*= ""*/)
{
	if (sResUpdate)
		return;

	// 透露接口给lua
	static const luaL_reg R[] =
	{
		{"listen",			res_update_listen},
		{"setTimeout",		res_update_setTimeout},
		{"setNative",		res_update_setNative},
		{"checkVersion",	res_update_checkVersion},
		{"checkUpdate",		res_update_checkUpdate},
		{"startUpdate",		res_update_startUpdate},
		{"record",			res_update_record},
		{"setListener",		res_update_setListener},
		{"addListener",		res_update_setListener},	// deprecated
		{NULL,	NULL}
	};
	luaL_openlib(L, "ResUpdate", R, 0);
	// 实例化
	sResUpdate = new ResUpdate();
	sResUpdate->setNative(path, nativeVersionFile, nativeMd5File);
	sResUpdate->openLua(L);
}

