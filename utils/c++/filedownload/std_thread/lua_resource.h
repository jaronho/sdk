/**********************************************************************
* Author:	jaron.ho
* Date:		2014-02-14
* Brief:	资源下载
**********************************************************************/
#ifndef _LUA_RESOURCE_H_
#define _LUA_RESOURCE_H_

#include <string>
#include <algorithm>
#include "AutoUpdate/FileDownload.h"
#include "AutoUpdate/ResourceUpdate.h"
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

const char* callLuaGlobalFunction(lua_State* L, const char* globalFuncName, const char* sig, ...);

/**********************************************************************
************************** 资源下载模块
**********************************************************************/

enum ResDownloadListenerType
{
	RDLT_PROGRESS = 1,						// 单个资源下载进度
	RDLT_SUCCESS,							// 单个资源下载完成
	RDLT_TOTAL_PROGRESS,					// 资源列表下载进度
	RDLT_TOTAL_SUCCESS,						// 所有资源下载完成
	RDLT_ERROR								// 资源下载出错
};

class ResDownload : public FileDownloadListener
{
public:
	ResDownload(void);
	~ResDownload(void);

public:
	// 下载进度
	virtual void onProgress(FileDownloadCode code, const std::string& fileURL, const std::string& buffer, double totalToDownload, double nowDownloaded);

	// 下载成功
	virtual void onSuccess(FileDownloadCode code, const std::string& fileURL, const std::string& buffer);

	// 下载失败
	virtual void onError(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer);

public:
	void listen(void);

	void setPath(const std::string& path);

	void setTimeout(unsigned int connectTimeout, unsigned int downloadTimeout);

	void excute(const std::vector<std::string>& fileUrlVec);

	void openLua(lua_State* L);

	void registerLuaHandler(ResDownloadListenerType listenerType, const std::string& globalFuncName);

private:
	FileDownload mFileDownload;						// 文件下载
	lua_State* mLuaState;							// lua脚本虚拟机
	std::string mProgressFuncName;					// 单个文件下载进度脚本处理函数名
	std::string mSuccessFuncName;					// 单个文件下载完成脚本处理函数名
	std::string mTotalProgressFuncName;				// 文件列表下载进度脚本处理函数名
	std::string mTotalSuccessFuncName;				// 所有文件下载完成脚本处理函数名
	std::string mErrorFuncName;						// 文件下载出错脚本处理函数名
};

void lua_resdownload_register(lua_State* L, const std::string& path = "");

/**********************************************************************
************************** 资源更新模块
**********************************************************************/

enum ResUpdateListenerType
{
	RULT_CHECK_VERSION_FAILED = 1,			// 版本检测失败
	RULT_NEW_VERSION,						// 发现新版本
	RULT_NO_NEW_VERSION,					// 没有新版本
	RULT_CHECK_UPDATE_LIST_FAILED,			// 更新列表检测失败
	RULT_UPDATE_LIST,						// 发现更新列表
	RULT_NO_UPDATE_LIST,					// 没有更新列表
	RULT_PROGRESS,							// 单个文件下载进度
	RULT_SUCCESS,							// 单个文件下载完成
	RULT_TOTAL_PROGRESS,					// 文件列表下载进度
	RULT_TOTAL_SUCCESS,						// 所有文件下载完成
	RULT_ERROR								// 文件下载出错
};

class ResUpdate : public ResourceUpdateListener
{
public:
	ResUpdate(void);
	~ResUpdate(void);

public:
	// 版本检测失败
	virtual void onCheckVersionFailed(const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer);

	// 发现新版本
	virtual void onNewVersion(const std::string& curVersion, const std::string& newVersion);

	// 没有新版本
	virtual void onNoNewVersion(const std::string& curVersion);

	// 更新列表检测失败
	virtual void onCheckUpdateListFailed(const std::string& fileURL, int curlCode, int responseCode, const std::string errorBuffer);

	// 发现更新列表
	virtual void onUpdateList(long updateCount, long updateSize);

	// 没有更新列表
	virtual void onNoUpdateList(void);

	// 单个文件下载进度
	virtual void onPogress(const std::string& fileURL, double totalSize, double curSize);

	// 单个文件下载完成
	virtual void onSuccess(const std::string& fileURL);

	// 文件列表下载进度
	virtual void onTotalProgress(const std::string& fileURL, int totalCount, int curCount);

	// 所有文件下载完成
	virtual void onTotalSuccess(void);

	// 文件下载出错
	virtual void onError(const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer);

public:
	void listen(void);

	void setTimeout(unsigned int connectTimeout, unsigned int downloadTimeout);

	void setNative(const std::string& path, const std::string& nativeVersionFile, const std::string& nativeMd5File);

	void checkVersion(const std::string& url, const std::string& checkVersionFile, const std::string& checkMd5File);

	void checkUpdate(bool deepCheck = true);

	void startUpdate(void);

	void record(void);

	void openLua(lua_State* L);

	void registerLuaHandler(ResUpdateListenerType listenerType, const std::string& globalFuncName);

private:
	ResourceUpdate* mResourceUpdate;				// 资源更新
	lua_State* mLuaState;							// lua脚本虚拟机
	std::string mCheckVersionFailedFuncName;		// 版本检测失败脚本处理函数名
	std::string mNewVersionFuncName;				// 发现新版本脚本处理函数名
	std::string mNoNewVersionFuncName;				// 没有新版本脚本处理函数名
	std::string mCheckUpdateListFailedFuncName;		// 更新列表检测失败脚本处理函数名
	std::string mUpdateListFuncName;				// 发现更新列表脚本处理函数名
	std::string mNoUpdateListFuncName;				// 没有更新列表脚本处理函数名
	std::string mProgressFuncName;					// 单个文件下载进度脚本处理函数名
	std::string mSuccessFuncName;					// 单个文件下载完成脚本处理函数名
	std::string mTotalProgressFuncName;				// 文件列表下载进度脚本处理函数名
	std::string mTotalSuccessFuncName;				// 所有文件下载完成脚本处理函数名
	std::string mErrorFuncName;						// 文件下载出错脚本处理函数名
};

void lua_resupdate_register(lua_State* L, const std::string& path = "", const std::string& nativeVersionFile = "", const std::string& nativeMd5File = "");

#endif	// _LUA_RESOURCE_H_
