/**********************************************************************
* Author:	jaron.ho
* Date:		2015-12-01
* Brief:	资源下载
**********************************************************************/
#ifndef _RESOURCE_DOWNLOAD_H_
#define _RESOURCE_DOWNLOAD_H_

#include <string>
#include <vector>
#include <functional>
#include "FileDownload.h"

// 资源下载回调类型定义
typedef std::function<void(const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer)>	RDErrorCB;			// 单个文件下载出错回调定义
typedef std::function<void(const std::string& fileURL, double totalSize, double currSize)>								RDProgressCB;		// 单个文件下载进度回调定义
typedef std::function<void(const std::string& fileURL)>																	RDSuccessCB;		// 单个文件下载完成回调定义
typedef std::function<void(const std::string& fileURL, int totalCount, int currCount)>									RDTotalProgressCB;	// 文件列表下载进度回调定义
typedef std::function<void(void)>																						RDTotalSuccessCB;	// 文件列表下载完成回调定义

class ResourceDownload : public FileDownloadListener {
public:
	ResourceDownload(void);
	~ResourceDownload(void);

	// 下载失败
	virtual void onError(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer);

	// 下载进度
	virtual void onProgress(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer, double totalToDownload, double nowDownloaded);

	// 下载成功
	virtual void onSuccess(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer);

public:
	// 获取id
	unsigned int getId(void);

	// 每帧监听
	void listen(void);

	// 设置超时
	void setTimeout(unsigned int connectTimeout, unsigned int downloadTimeout);

	// 设置回调(单个文件下载出错)
	void setErrorCB(const RDErrorCB& callback);

	// 设置回调(单个文件下载进度)
	void setProgressCB(const RDProgressCB& callback);

	// 设置回调(单个文件下载完成)
	void setSuccessCB(const RDSuccessCB& callback);

	// 设置回调(文件列表下载进度)
	void setTotalProgressCB(const RDTotalProgressCB& callback);

	// 设置回调(文件列表下载完成)
	void setTotalSuccessCB(const RDTotalSuccessCB& callback);

	// 设置下载路径
	void setDownloadPath(const std::string& path);

	// 执行下载
	bool excute(const std::vector<std::string>& fileUrlVec, const std::string& cacheSuffix = "");

	// 是否在下载中
	bool isDownloading(void);

private:
	FileDownload mFileDownload;							// 文件下载对象
	RDErrorCB mErrorCB;									// 单个文件下载出错回调
	RDProgressCB mProgressCB;							// 单个文件下载进度回调
	RDSuccessCB mSuccessCB;								// 单个文件下载完成回调
	RDTotalProgressCB mTotalProgressCB;					// 文件列表下载进度回调
	RDTotalSuccessCB mTotalSuccessCB;					// 文件列表下载完成回调
};

#endif	// _RESOURCE_DOWNLOAD_H_