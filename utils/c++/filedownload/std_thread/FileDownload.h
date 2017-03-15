/**********************************************************************
* Author:	jaron.ho
* Date:		2014-02-08
* Brief:	文件下载
**********************************************************************/
#ifndef _FILE_DOWNLOAD_H_
#define _FILE_DOWNLOAD_H_

#include <string>
#include <vector>
#include <list>
#include <thread>
#include <mutex>
#include "CURLEx.h"

// 文件下载代码
enum FileDownloadCode
{
	FDC_NONE = 0,
	// 初始操作
	FDC_CREATE_FILE_FAILED,			// 文件创建失败
	FDC_INIT_CURL_FAILED,			// 初始curl失败
	FDC_DOWNLOAD_ERROR,				// 文件下载出错
	// 下载单个文件
	FDC_FILE_PROGRESS,				// 单个文件下载进度
	FDC_FILE_SUCCESS,				// 单个文件下载成功
	// 下载文件列表
	FDC_LIST_PROGRESS,				// 文件列表下载进度
	FDC_LIST_SUCCESS,				// 文件列表下载成功
};

// 文件下载监听（需要外部继承实现）
class FileDownloadListener
{
public:
	// 下载失败
	virtual void onError(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer) {}

	// 下载进度
	virtual void onProgress(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer, double totalToDownload, double nowDownloaded) {}

	// 下载成功
	virtual void onSuccess(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer) {}
};

// 文件下载消息结构（内部调用）
struct FileDownloadMessage
{
	FileDownloadMessage(void) : code(FDC_NONE), curlcode(0), responsecode(0), totalcount(0.0f), nowcount(0.0f), listener(NULL) {}

	FileDownloadCode code;					// 下载标识
	std::string fileurl;					// 文件url
	int curlcode;							// curl code
	int responsecode;						// http code
	std::string buffer;						// 日志
	double totalcount;						// 总下载数
	double nowcount;						// 已下载数
	FileDownloadListener* listener;			// 下载监听
};

// 文件下载帮助类（内部调用）
class FileDownloadHelper
{
public:
	FileDownloadHelper(void);
	~FileDownloadHelper(void);

public:
	// 发送消息
	void sendMessage(FileDownloadMessage* msg);

	// 接收消息
	void recvMessage(void);

private:
	std::list<FileDownloadMessage*>* mMessageList;		// 消息队列
	std::mutex mMessageQueueMutex;						// 消息队列互斥
};

// 文件下载
class FileDownload
{
public:
	FileDownload(void);
	~FileDownload(void);

public:
	// 获取id
	unsigned int getId(void);

	// 监听消息队列（每帧监听）
	void listenMessage(void);

	// 设置监听
	void setListener(FileDownloadListener* listener);

	// 设置连接超时
	void setConnectTimeout(unsigned int timeout = 0);

	// 设置下载超时
	void setDownloadTimeout(unsigned int timeout = 0);

	// 设置文件保存路径
	void setStoragePath(const std::string& storagePath);

	// 获取文件保存路径
	std::string getStoragePath(void);

	// 下载文件列表
	bool download(const std::vector<std::string>& fileUrlVec);

	// 是否在下载中
	bool isDownloading(void);

	// 文件下载处理函数（内部调用）
	friend void* downloadProcessFunc(void* ptr);

	// 文件下载进度（内部调用）
	void downloadFileProgress(double totalToDownload, double nowDownloaded);

public:
	// 字符串替换
	static std::string replaceString(std::string str, const std::string& src, const std::string& dest);

	// 创建目录
	static bool createDir(const std::string& dirName);

	// 删除目录
	static void removeDir(const std::string& dirName);

	// 截取文件名:D:\temp\test.txt,结果为test.txt
	static std::string getFileName(const std::string& fullFileName);

protected:
	// 下载文件列表
	bool downloadImpl(void);

	// 下载单个文件
	bool downloadImpl(const std::string& fileURL);

	// 单个文件下载核心处理函数
	int downloadFile(const std::string& savePath, const std::string& saveName, const std::string& fileURL, unsigned int connectTimeout = 0, unsigned int downloadTimeout = 0, FileDownload* self = NULL, int* curlCode = NULL, int* responseCode = NULL, std::string* buffer = NULL);

protected:
	unsigned int mId;							// 唯一id
	FileDownloadHelper* mHelper;				// 消息传递
	FileDownloadListener* mListener;			// 监听下载事件
	unsigned int mConnectTimeout;				// 连接超时
	unsigned int mDownloadTimeout;				// 下载超时
	std::vector<std::string> mFileUrlVec;		// 文件下载列表
	std::string mCurrDownloadFileURL;			// 当前下载的文件资源
	int mCurrDownloadCurlCode;					// 当前文件下载的curl code
	int mCurrDownloadResponseCode;				// 当前文件下载的http code
	std::string mCurrDownloadBuffer;			// 当前文件下载的日志
	std::string mStoragePath;					// 文件保存路径
	bool mIsDownloading;						// 是否在下载中
};

#endif	// _FILE_DOWNLOAD_H_