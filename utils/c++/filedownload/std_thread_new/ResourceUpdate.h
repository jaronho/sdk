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
#include <functional>
#include "FileDownload.h"

// 资源更新回调类型定义
typedef std::function<std::string(const std::string& fileName)>															RUMd5CheckFunc;			// 文件MD5检查函数定义
typedef std::function<void(const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer)>	RUUpdateListErrorCB;	// 更新列表下载出错回调定义
typedef std::function<void(void)>																						RUUpdateListNotFoundCB;	// 无可更新列表回调定义
typedef std::function<void(long updateCount, long updateSize)>															RUUpdateListCB;			// 有可更新列表回调定义
typedef std::function<void(const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer)>	RUErrorCB;				// 单个文件下载出错回调定义
typedef std::function<void(const std::string& fileURL, double totalSize, double currSize)>								RUProgressCB;			// 单个文件下载进度回调定义
typedef std::function<void(const std::string& fileURL)>																	RUSuccessCB;			// 单个文件下载完成回调定义
typedef std::function<void(const std::string& fileURL, int totalCount, int currCount)>									RUTotalProgressCB;		// 文件列表下载进度回调定义
typedef std::function<void(void)>																						RUTotalSuccessCB;		// 文件列表下载完成回调定义

// 资源更新
class ResourceUpdate : public FileDownloadListener {
public:
	// 线程类型(内部使用)
	enum RUThreadType {
		RUTT_NONE = 0,
		RUTT_UPDATE_LIST_NOT_FOUND,		// 没有更新列表
		RUTT_UPDATE_LIST,				// 发现更新列表
		RUTT_UPDATE_LIST_ENDED			// 更新列表结束
	};

	// 资源更新信息结构(内部使用)
	struct ResourceUpdateInfo {
		std::string md5value;			// 文件md5值
		std::string filename;			// 文件名称
		long filesize;					// 文件大小
	};

public:
	ResourceUpdate(void);
	~ResourceUpdate(void);

	// 下载失败
	virtual void onError(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer);

	// 下载进度
	virtual void onProgress(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer, double totalToDownload, double nowDownloaded);

	// 下载成功
	virtual void onSuccess(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer);

public:
	// 返回换行标示符
	static std::string newLineString(void);

	// 分割字符串
	static std::vector<std::string> splitString(std::string str, const std::string& pattern);

	// 获取文件数据
	static char* getFileData(const std::string& file, unsigned long* fileSize);

	// 获取文件数据,分割每一行
	static std::vector<std::string> getFileDataEx(const std::string& file);

	// 写数据到文件
	static bool writeDataToFile(const char* data, unsigned long dataSize, const std::string& file);

public:
	// 获取id
	unsigned int getId(void);

	// 每帧监听
	void listen(void);

	// 设置超时
	void setTimeout(unsigned int connectTimeout, unsigned int downloadTimeout);

	// 设置文件MD5检查函数
	void setFileMd5CheckFunc(const RUMd5CheckFunc& func);

	// 设置回调(更新列表下载出错)
	void setUpdateListErrorCB(const RUUpdateListErrorCB& callback);

	// 设置回调(无可更新列表)
	void setUpdateListNotFoundCB(const RUUpdateListNotFoundCB& callback);

	// 设置回调(有可更新列表)
	void setUpdateListCB(const RUUpdateListCB& callback);

	// 设置回调(单个文件下载出错)
	void setErrorCB(const RUErrorCB& callback);

	// 设置回调(单个文件下载进度)
	void setProgressCB(const RUProgressCB& callback);

	// 设置回调(单个文件下载完成)
	void setSuccessCB(const RUSuccessCB& callback);

	// 设置回调(文件列表下载进度)
	void setTotalProgressCB(const RUTotalProgressCB& callback);

	// 设置回调(文件列表下载完成)
	void setTotalSuccessCB(const RUTotalSuccessCB& callback);

	// step1:设置本地,path末尾必须已'/'结尾
	bool setNative(const std::string& path, const std::string& nativeMd5File);

	// step2:更新检查,url末尾必须已'/'结尾
	bool checkUpdate(const std::string& url, const std::vector<std::string>& checkMd5FileList);

	// step3:更新开始
	bool startUpdate(const std::string& cacheSuffix = "", bool removeInvalidFileFlag = true);

	// 是否在下载中
	bool isDownloading(void);

	// 记录更新进度
	bool record(bool immediatelyFlag = true);

private:
	// 创建消息队列
	void createMessageQueue(void);

	// 销毁消息队列
	void destroyMessageQueue(void);

	// 发送消息
	void sendMessage(RUThreadType msg);

	// 接收消息
	void recvMessage(void);

	// 更新准备
	void handleUpdateListPrepare(void);

	friend void* updateListPrepareProcessFunc(void* ptr);

	void updateListPrepareImpl(void);

	// 更新结束
	void handleUpdateListEnded(void);

	friend void* updateListEndedProcessFunc(void* ptr);

	void updateListEndedImpl(void);

	// 记录进度
	friend void* recordProcessFunc(void* ptr);

	void recordImpl(void);

	// 检验md5值
	bool checkMd5Value(const std::string& fileURL);

	// 是否MD5校验文件
	bool isCheckMd5File(const std::string& fileURL);

	// 保存更新列表文件
	void saveUpdateFileList(const std::map<std::string, ResourceUpdateInfo>& infoVec, const std::string& fileName);

	// 解析更新文件列表
	long parseUpdateFileList(std::vector<std::string>& updateVec);

	// 解析无效文件列表
	void parseInvalidFileList(std::vector<std::string>& invalidVec);

	// 解析MD5文件列表
	bool parseFileInfo(const std::string& filename, std::map<std::string, ResourceUpdateInfo>& infoVec);

private:
	FileDownload mFileDownload;									// 文件下载对象
	std::list<RUThreadType>* mMessageList;						// 消息队列
	std::mutex mMessageQueueMutex;								// 消息队列互斥
	// 
	RUMd5CheckFunc mFileMd5CheckFunc;							// 文件MD5检查函数
	RUUpdateListErrorCB mUpdateListErrorCB;						// 更新列表下载错误回调
	RUUpdateListNotFoundCB mUpdateListNotFoundCB;				// 无可更新列表回调
	RUUpdateListCB mUpdateListCB;								// 有可更新列表回调
	RUErrorCB mErrorCB;											// 单个文件下载出错回调
	RUProgressCB mProgressCB;									// 单个文件下载进度回调
	RUSuccessCB mSuccessCB;										// 单个文件下载完成回调
	RUTotalProgressCB mTotalProgressCB;							// 文件列表下载进度回调
	RUTotalSuccessCB mTotalSuccessCB;							// 文件列表下载完成回调
	// 
	std::string mDIR;											// 资源保存路径
	std::string mNativeMd5File;									// MD5本地文件
	std::map<std::string, ResourceUpdateInfo> mNativeInfoVec;	// 本地文件信息
	// 
	std::string mURL;											// 资源获取URL
	std::vector<std::string> mCheckMd5FileList;					// MD5校验文件列表
	std::map<std::string, ResourceUpdateInfo> mCheckInfoVec;	// 校验文件信息
	// 
	std::string mCacheSuffix;									// 缓存后缀
	bool mIsRemoveInvalidFile;									// 是否删除无效文件
	std::vector<std::string> mUpdateFileList;					// 要更新的文件列表
	long mUpdateFileSize;										// 要更新的文件大小
	std::vector<std::string> mHasUpdateFileList;				// 已更新的文件列表
	size_t mHasRecordCount;										// 已记录的文件数量
};

#endif	// _RESOURCE_UPDATE_H_