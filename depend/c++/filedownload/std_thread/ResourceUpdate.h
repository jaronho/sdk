/**********************************************************************
* Author:	jaron.ho
* Date:		2014-02-14
* Brief:	资源更新
**********************************************************************/
#ifndef _RESOURCE_UPDATE_H_
#define _RESOURCE_UPDATE_H_

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include "FileDownload.h"

// 线程类型(内部使用)
enum RUPthreadType
{
	RAUPT_NONE = 0,
	RAUPT_UPDATE_LIST,					// 发现更新列表
	RAUPT_NO_UPDATE_LIST,				// 没有更新列表
	RAUPT_UPDATE_LIST_DOWNLOADED		// 更新列表全部下载完成
};

// 资源更新操作状态
enum ResourceUpdateOperateStatus
{
	RUOS_CAN_CHECK_VERSION,				// 可执行版本检查
	RUOS_IN_CHECK_VERSION,				// 在执行版本检查
	RUOS_CAN_CHECK_UPDATE,				// 可执行更新检查
	RUOS_IN_CHECK_UPDATE,				// 在执行更新检查
	RUOS_CAN_UPDATE,					// 可执行更新操作
	RUOS_IN_UPDATE,						// 在执行更新操作
};

// 资源更新信息结构(内部使用)
struct ResourceUpdateInfo
{
	std::string filename;				// 文件名
	std::string md5value;				// md5值
	long filesize;						// 文件大小
};

// 资源更新监听器(需要继承实现)
class ResourceUpdateListener
{
public:
	// 版本检测失败
	virtual void onCheckVersionFailed(const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer) {}
	// 发现新版本
	virtual void onNewVersion(const std::string& curVersion, const std::string& newVersion) {}
	// 没有新版本
	virtual void onNoNewVersion(const std::string& curVersion) {}

	// 更新列表检测失败
	virtual void onCheckUpdateListFailed(const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer) {}
	// 发现更新列表
	virtual void onUpdateList(long updateCount, long updateSize) {}
	// 没有更新列表
	virtual void onNoUpdateList(void) {}

	// 单个文件下载进度
	virtual void onPogress(const std::string& fileURL, double totalSize, double curSize) {}
	// 单个文件下载完成
	virtual void onSuccess(const std::string& fileURL) {}

	// 文件列表下载进度
	virtual void onTotalProgress(const std::string& fileURL, int totalCount, int curCount) {}
	// 文件列表下载完成
	virtual void onTotalSuccess(void) {}

	// 文件下载出错
	virtual void onError(const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer) {}
};

// 资源更新
class ResourceUpdate : public FileDownloadListener
{
public:
	ResourceUpdate(ResourceUpdateListener* listener = NULL);
	~ResourceUpdate(void);

	// 下载进度
	virtual void onProgress(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer, double totalToDownload, double nowDownloaded);

	// 下载成功
	virtual void onSuccess(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer);

	// 下载失败
	virtual void onError(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer);

public:
	// 返回换行标示符
	static std::string newLineString(void);

	// 分割字符串
	static std::vector<std::string> splitString(std::string str, const std::string& pattern);

	// 是否存在文件
	static bool existFile(const std::string& file);

	// 获取文件数据
	static char* getFileData(const std::string& file, unsigned long* fileSize);

	// 获取文件数据,分割每一行
	static std::vector<std::string> getFileDataEx(const std::string& file);

	// 写数据到文件
	static bool writeDataToFile(const char* data, unsigned long dataSize, const std::string& file);

public:
	// 每帧监听
	void listen(void);

	// 设置超时
	void setTimeout(unsigned int connectTimeout, unsigned int downloadTimeout);

	// step1:设置本地
	bool setNative(const std::string& path, const std::string& nativeVersionFile, const std::string& nativeMd5File);

	// step2:版本检查,url末尾必须已'/'结尾
	bool checkVersion(const std::string& url, const std::string& checkVersionFile, const std::string& checkMd5File);

	// step3:更新检查,deepCheck:版本号不一致时,若无更新列表,true(则不保存新版本号,下次继续检查版本号和更新列表),false(保存新版本号,下次只检查版本号)
	bool checkUpdate(bool deepCheck = true);

	// step4:开始更新
	bool startUpdate(void);

	// 记录更新进度
	bool record(void);

private:
	// 创建消息队列
	void createMessageQueue(void);

	// 销毁消息队列
	void destroyMessageQueue(void);

	// 发送消息
	void sendMessage(RUPthreadType msg);

	// 接收消息
	void recvMessage(void);

	void handleVersionCheckFileDownloaded(bool downloaded, const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer);

	void handleMd5CheckFileDownloaded(bool downloaded, const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer);

	void createUpdatePthread(void);

	void updateImpl(void);

	friend void* updateProcessFunc(void* ptr);

	// 更新完成
	void handleUpdateListDownloaded(void);

	void removeImpl(void);

	friend void* removeProcessFunc(void* ptr);

	// 记录进度
	void recordImpl(void);

	friend void* recordProcessFunc(void* ptr);

	// 移除校验文件
	void removeCheckFile(void);

	// 检验md5值
	bool checkMd5Value(const std::string& fileURL);

	// 解析更新文件列表
	long parseUpdateFileList(std::vector<std::string>& updateVec);

	// 解析移除文件列表
	void parseRemoveFileList(std::vector<std::string>& removeVec);

	// 解析MD5文件列表
	bool parseFileInfo(const std::string& filename, std::map<std::string, ResourceUpdateInfo>& infoVec);

private:
	unsigned int mId;											// 对象id
	FileDownload mFileDownload;									// 文件下载对象
	ResourceUpdateListener *mListener;							// 监听器
	bool mIsInnerCreatedListener;								// 是否为内部创建的监听器
	// 
	std::list<RUPthreadType> *mMessageList;						// 消息队列
	std::mutex mMessageQueueMutex;								// 消息队列互斥
	// 
	std::string mDownloadDir;									// 资源下载路径
	std::string mNativeVersionFile;								// 版本本地文件
	std::string mNativeMd5File;									// MD5本地文件
	std::string mCurrVersion;									// 当前版本号
	std::map<std::string, ResourceUpdateInfo> mNativeInfoVec;	// 本地文件信息
	// 
	std::string mURL;											// 资源下载的URL
	std::string mCheckVersionFile;								// 版本校验文件(网络上的文件)
	std::string mCheckMd5File;									// MD5校验文件(网络上的文件)
	std::string mNewVersion;									// 新版本号
	std::map<std::string, ResourceUpdateInfo> mCheckInfoVec;	// 校验文件信息
	std::vector<std::string> mDownloadedVec;					// 已下载的文件
	std::vector<std::string> mFileList;							// 要下载的文件列表
	long mTotalUpdateSize;										// 总下载文件大小
	ResourceUpdateOperateStatus mUpdateStatus;					// 更新操作状态
	bool mDeepCheckUpdate;										// 是否深度检查更新状态
};

#endif	// _RESOURCE_UPDATE_H_