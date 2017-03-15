/**********************************************************************
* Author:	jaron.ho
* Date:		2014-02-14
* Brief:	资源更新
**********************************************************************/
#include "ResourceUpdate.h"
#include <stdlib.h>

static unsigned int sObjectCount = 0;
static std::map<unsigned int, bool> sObjectMessageQueueFlag;
//------------------------------------------------------------------------
ResourceUpdate::ResourceUpdate(const std::string& downloadDir, const std::string& versionNativeFile, const std::string& md5NativeFile, ResourceUpdateListener* listener /*= NULL*/)
: mId(0)
, mMessageList(NULL)
, mUpdatePthread(NULL)
, mRemovePthread(NULL)
, mRecordPthread(NULL)
, mDownloadDir(downloadDir)
, mVersionNativeFile(versionNativeFile)
, mMd5NativeFile(md5NativeFile)
, mTotalUpdateSize(0)
, mUpdateStatus(RUOS_CAN_CHECK_VERSION)
{
	mId = sObjectCount++;
	mListener = listener;
	mIsInnerCreatedListener = false;
	if (NULL == mListener)
	{
		mListener = new ResourceUpdateListener();
		mIsInnerCreatedListener = true;
	}
	// 设置资源下载路径
	mFileDownload.setStoragePath(downloadDir);
	mFileDownload.setListener(this);
	// 
	createMessageQueue();
	// 其他初始操作
	unsigned long length = 0;
	char *buffer = getFileData(downloadDir + versionNativeFile, &length);
	if (buffer)
	{
		mCurVersion = buffer;
		mNewVersion = mCurVersion;
		delete buffer;
		buffer = NULL;
	}
}
//------------------------------------------------------------------------
ResourceUpdate::~ResourceUpdate(void)
{
	destroyMessageQueue();
	if (mIsInnerCreatedListener)
	{
		delete mListener;
		mListener = NULL;
	}
	if (mUpdatePthread)
	{
		delete mUpdatePthread;
		mUpdatePthread = NULL;
	}
	if (mRemovePthread)
	{
		delete mRemovePthread;
		mRemovePthread = NULL;
	}
	if (mRecordPthread)
	{
		delete mRecordPthread;
		mRecordPthread = NULL;
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::setTimeout(unsigned int connectTimeout, unsigned int downloadTimeout)
{
	mFileDownload.setConnectTimeout(connectTimeout);
	mFileDownload.setDownloadTimeout(downloadTimeout);
}
//------------------------------------------------------------------------
void ResourceUpdate::createMessageQueue(void)
{
	sObjectMessageQueueFlag.insert(std::make_pair(mId, true));
	mMessageList = new std::list<RUPthreadType>();
	pthread_mutex_init(&mMessageQueueMutex, NULL);
}
//------------------------------------------------------------------------
void ResourceUpdate::destroyMessageQueue(void)
{
	sObjectMessageQueueFlag.erase(mId);
	pthread_mutex_lock(&mMessageQueueMutex);
	if (mMessageList)
	{
		mMessageList->clear();
		delete mMessageList;
		mMessageList = NULL;
	}
	pthread_mutex_unlock(&mMessageQueueMutex);
	pthread_mutex_destroy(&mMessageQueueMutex);
}
//------------------------------------------------------------------------
void ResourceUpdate::sendMessage(RUPthreadType msg)
{
	if (false == sObjectMessageQueueFlag[mId])
		return;

	pthread_mutex_lock(&mMessageQueueMutex);
	mMessageList->push_back(msg);
	pthread_mutex_unlock(&mMessageQueueMutex);
}
//------------------------------------------------------------------------
void ResourceUpdate::recvMessage(void)
{
	if (false == sObjectMessageQueueFlag[mId])
		return;

	RUPthreadType msg = RAUPT_NONE;
	pthread_mutex_lock(&mMessageQueueMutex);
	if (mMessageList->size() > 0)
	{
		msg = *(mMessageList->begin());
		mMessageList->pop_front();
	}
	pthread_mutex_unlock(&mMessageQueueMutex);
	switch (msg)
	{
	case RAUPT_UPDATE_LIST:
		{
			mListener->onUpdateList(mFileList.size(), mTotalUpdateSize);
		}
		break;
	case RAUPT_NO_UPDATE_LIST:
		{
			mListener->onNoUpdateList();
		}
		break;
	case RAUPT_UPDATE_LIST_DOWNLOADED:
		{
			mListener->onTotalSuccess();
		}
		break;
	default:
		break;
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::onProgress(FileDownloadCode code, const std::string& fileURL, const std::string& buffer, double totalToDownload, double nowDownloaded)
{
	if (fileURL == mURL + mVersionCheckFile || fileURL == mURL + mMd5CheckFile)
		return;
	
	if (FDC_FILE_PROGRESS == code)
	{
		mListener->onPogress(fileURL, totalToDownload, nowDownloaded);
	}
	else if (FDC_LIST_PROGRESS == code)
	{
		mListener->onTotalProgress(fileURL, (int)totalToDownload, (int)nowDownloaded);
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::onSuccess(FileDownloadCode code, const std::string& fileURL, const std::string& buffer)
{
	if (FDC_LIST_SUCCESS == code)
	{
		if (fileURL == mURL + mVersionCheckFile)
		{
			handleVersionCheckFileDownloaded(true, buffer);		// 版本校验文件下载成功
		}
		else if (fileURL == mURL + mMd5CheckFile)
		{
			handleMd5CheckFileDownloaded(true, buffer);			// MD5列表校验文件下载成功
		}
		else
		{
			mUpdateStatus = RUOS_CAN_CHECK_VERSION;
			handleUpdateListDownloaded();						// 更新列表全部下载成功(删除废弃的资源)
		}
	}
	else
	{
		if (fileURL != mURL + mVersionCheckFile && fileURL != mURL + mMd5CheckFile)
		{
			mDownloadedVec.push_back(fileURL);
			mListener->onSuccess(fileURL);
		}
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::onError(FileDownloadCode code, const std::string& fileURL, const std::string& buffer)
{
	mUpdateStatus = RUOS_CAN_CHECK_VERSION;
	if (FDC_LIST_ERROR == code)
	{
		if (fileURL == mURL + mVersionCheckFile)
		{
			handleVersionCheckFileDownloaded(false, buffer);	// 版本校验文件下载失败
		}
		else if (fileURL == mURL + mMd5CheckFile)
		{
			handleMd5CheckFileDownloaded(false, buffer);		// MD5列表校验文件下载失败
		}
	}
	else
	{
		if (fileURL != mURL + mVersionCheckFile && fileURL != mURL + mMd5CheckFile)
		{
			mListener->onError(fileURL, buffer);
		}
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::listen(void)
{
	mFileDownload.listenMessage();
	recvMessage();
}
//------------------------------------------------------------------------
void ResourceUpdate::checkVersion(const std::string& url, const std::string& versionCheckFile, const std::string& md5CheckFile)
{
	if (url.empty() || versionCheckFile.empty() || md5CheckFile.empty())
		return;

	if (RUOS_CAN_CHECK_VERSION != mUpdateStatus)
		return;

	mUpdateStatus = RUOS_IN_CHECK_VERSION;
	mURL = url;
	mVersionCheckFile = versionCheckFile;
	mMd5CheckFile = md5CheckFile;
	// 下载版本验证文件
	std::vector<std::string> urlVec;
	urlVec.push_back(url + versionCheckFile);
	mFileDownload.download(urlVec);
}
//------------------------------------------------------------------------
void ResourceUpdate::handleVersionCheckFileDownloaded(bool downloaded, const std::string& errorBuffer)
{
	if (downloaded)
	{
		unsigned long length = 0;
		char *buffer = getFileData(mDownloadDir + mVersionCheckFile, &length);
		if (buffer)
		{
			mNewVersion = buffer;
			delete buffer;
			buffer = NULL;
		}
		if (mNewVersion == mCurVersion)		// 版本一样
		{
			removeCheckFile();
			mListener->onNoNewVersion(mCurVersion);
		}
		else								// 版本不一样
		{
			mUpdateStatus = RUOS_CAN_CHECK_UPDATE;
			mListener->onNewVersion(mCurVersion, mNewVersion);
		}
	}
	else
	{
		removeCheckFile();
		mListener->onCheckVersionFailed(errorBuffer);
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::checkUpdate(void)
{
	if (RUOS_CAN_CHECK_UPDATE != mUpdateStatus)
		return;

	mUpdateStatus = RUOS_IN_CHECK_UPDATE;
	// 下载校验MD5列表文件
	std::vector<std::string> urlVec;
	urlVec.push_back(mURL + mMd5CheckFile);
	mFileDownload.download(urlVec);
}
//------------------------------------------------------------------------
void ResourceUpdate::handleMd5CheckFileDownloaded(bool downloaded, const std::string& errorBuffer)
{
	if (downloaded)
	{
		createUpdatePthread();
	}
	else
	{
		removeCheckFile();
		mListener->onCheckUpdateListFailed(errorBuffer);
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::startUpdate(void)
{
	if (RUOS_CAN_UPDATE != mUpdateStatus)
		return;

	mUpdateStatus = RUOS_IN_UPDATE;
	std::vector<std::string> urlList;
	for (size_t i=0; i<mFileList.size(); ++i)
	{
		urlList.push_back(mURL + mFileList[i]);
	}
	mFileDownload.download(urlList);
}
//------------------------------------------------------------------------
void* updateProcessFunc(void* ptr)
{
	ResourceUpdate *self = (ResourceUpdate*)ptr;
	if (self->mUpdatePthread)
	{
		delete self->mUpdatePthread;
		self->mUpdatePthread = NULL;
	}
	self->updateImpl();
	return NULL;
}
//------------------------------------------------------------------------
void ResourceUpdate::updateImpl(void)
{
	mFileList.clear();
	mTotalUpdateSize = parseUpdateFileList(mDownloadDir, mMd5CheckFile, mMd5NativeFile, mFileList);
	if (mFileList.empty())		// 没有更新列表
	{
		writeDataToFile(mNewVersion.c_str(), mNewVersion.size(), mDownloadDir + mVersionNativeFile);
		removeCheckFile();
		sendMessage(RAUPT_NO_UPDATE_LIST);
	}
	else						// 有更新列表
	{
		mUpdateStatus = RUOS_CAN_UPDATE;
		sendMessage(RAUPT_UPDATE_LIST);
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::createUpdatePthread(void)
{
	if (mUpdatePthread)
		return;

	// 开辟新线程处理文件下载
	mUpdatePthread = new pthread_t();
	pthread_attr_t attr;
	memset(&attr, 0, sizeof(pthread_attr_t));
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);	// 设置线程分离
	pthread_create(mUpdatePthread, &attr, updateProcessFunc, this);
}
//------------------------------------------------------------------------
void* removeProcessFunc(void* ptr)
{
	ResourceUpdate *self = (ResourceUpdate*)ptr;
	if (self->mRemovePthread)
	{
		delete self->mRemovePthread;
		self->mRemovePthread = NULL;
	}
	self->removeImpl();
	return NULL;
}
//------------------------------------------------------------------------
void ResourceUpdate::removeImpl(void)
{
	// step1:删除资源文件
	std::vector<std::string> fileList;
	parseRemoveFileList(mDownloadDir, mMd5CheckFile, mMd5NativeFile, fileList);
	for (size_t i=0; i<fileList.size(); ++i)
	{
		std::string filePath = mDownloadDir + fileList[i];
		remove(filePath.c_str());
	}
	// step2:覆盖旧的MD5列表文件
	unsigned long length = 0;
	char *buffer = getFileData(mDownloadDir + mMd5CheckFile, &length);
	if (buffer)
	{
		writeDataToFile(buffer, length, mDownloadDir + mMd5NativeFile);
		delete buffer;
		buffer = NULL;
	}
	// step3:保存版本号
	writeDataToFile(mNewVersion.c_str(), mNewVersion.size(), mDownloadDir + mVersionNativeFile);
	removeCheckFile();
	sendMessage(RAUPT_UPDATE_LIST_DOWNLOADED);
}
//------------------------------------------------------------------------
void ResourceUpdate::handleUpdateListDownloaded(void)
{
	if (mRemovePthread)
		return;

	// 开辟新线程处理文件下载
	mRemovePthread = new pthread_t();
	pthread_attr_t attr;
	memset(&attr, 0, sizeof(pthread_attr_t));
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);	// 设置线程分离
	pthread_create(mRemovePthread, &attr, removeProcessFunc, this);
}
//------------------------------------------------------------------------
void* recordProcessFunc(void* ptr)
{
	ResourceUpdate *self = (ResourceUpdate*)ptr;
	if (self->mRecordPthread)
	{
		delete self->mRecordPthread;
		self->mRecordPthread = NULL;
	}
	self->recordImpl();
	return NULL;
}
//------------------------------------------------------------------------
void ResourceUpdate::recordImpl(void)
{
	// 校验文件信息
	std::map<std::string, ResourceUpdateInfo> checkInfoVec;
	parseFileInfo(mDownloadDir + mMd5CheckFile, checkInfoVec);
	// 本地文件信息
	std::map<std::string, ResourceUpdateInfo> nativeInfoVec;
	parseFileInfo(mDownloadDir + mMd5NativeFile, nativeInfoVec);
	// 更新本地信息
	std::vector<ResourceUpdateInfo> downloadedVec;
	for (size_t i=0; i<mDownloadedVec.size(); ++i)
	{
		const std::string fileName = FileDownload::getFileName(mDownloadedVec[i]);
		std::map<std::string, ResourceUpdateInfo>::iterator checkIter = checkInfoVec.find(fileName);
		if (checkInfoVec.end() == checkIter)
			continue;
		
		bool find = false;
		std::map<std::string, ResourceUpdateInfo>::iterator nativeIter = nativeInfoVec.find(fileName);
		if (nativeInfoVec.end() == nativeIter)
		{
			nativeInfoVec.insert(std::make_pair(fileName, checkIter->second));
		}
		else
		{
			nativeIter->second.md5value = checkIter->second.md5value;
			nativeIter->second.filename = checkIter->second.filename;
			nativeIter->second.filesize = checkIter->second.filesize;
		}
	}
	mDownloadedVec.clear();
	// 保存到本地文件
	std::string str = "";
	std::map<std::string, ResourceUpdateInfo>::iterator nativeIter = nativeInfoVec.begin();
	for (nativeIter; nativeInfoVec.end() != nativeIter; ++nativeIter)
	{
		if (nativeInfoVec.begin() != nativeIter)
		{
			str += newLineString();
		}
		char filesizeBuf[32];
		sprintf(filesizeBuf, "%ld", nativeIter->second.filesize);
		str += nativeIter->second.md5value + "," + nativeIter->second.filename + "," + filesizeBuf;
	}
	str += "\0";
	writeDataToFile(str.c_str(), str.size(), mDownloadDir + mMd5NativeFile);
}
//------------------------------------------------------------------------
void ResourceUpdate::record(void)
{
	if (mRecordPthread || mDownloadedVec.empty())
		return;

	// 开辟新线程处理文件下载
	mRecordPthread = new pthread_t();
	pthread_attr_t attr;
	memset(&attr, 0, sizeof(pthread_attr_t));
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);	// 设置线程分离
	pthread_create(mRecordPthread, &attr, recordProcessFunc, this);
}
//------------------------------------------------------------------------
void ResourceUpdate::removeCheckFile(void)
{
	remove((mDownloadDir + mVersionCheckFile).c_str());
	remove((mDownloadDir + mMd5CheckFile).c_str());
}
//------------------------------------------------------------------------
long ResourceUpdate::parseUpdateFileList(const std::string& path, const std::string& checkFileName, const std::string& nativeFileName, std::vector<std::string>& updateVec)
{
	long totalUpdateSize = 0;
	// 校验文件信息
	std::map<std::string, ResourceUpdateInfo> checkInfoVec;
	parseFileInfo(path + checkFileName, checkInfoVec);
	// 本地文件信息
	std::map<std::string, ResourceUpdateInfo> nativeInfoVec;
	parseFileInfo(path + nativeFileName, nativeInfoVec);
	// 检查需要更新的资源
	std::map<std::string, ResourceUpdateInfo>::iterator checkIter = checkInfoVec.begin();
	for (checkIter; checkInfoVec.end() != checkIter; ++checkIter)
	{
		std::map<std::string, ResourceUpdateInfo>::iterator nativeIter = nativeInfoVec.find(checkIter->first);
		if (nativeInfoVec.end() == nativeIter || nativeIter->second.filename != checkIter->second.filename || nativeIter->second.md5value != checkIter->second.md5value)
		{
			updateVec.push_back(checkIter->second.filename);
			totalUpdateSize += checkIter->second.filesize;
		}
	}
	return totalUpdateSize;
}
//------------------------------------------------------------------------
void ResourceUpdate::parseRemoveFileList(const std::string& path, const std::string& checkFileName, const std::string& nativeFileName, std::vector<std::string>& removeVec)
{
	// 校验文件信息
	std::map<std::string, ResourceUpdateInfo> checkInfoVec;
	parseFileInfo(path + checkFileName, checkInfoVec);
	// 本地文件信息
	std::map<std::string, ResourceUpdateInfo> nativeInfoVec;
	parseFileInfo(path + nativeFileName, nativeInfoVec);
	// 检查需要删除的资源
	std::map<std::string, ResourceUpdateInfo>::iterator nativeIter = nativeInfoVec.begin();
	for (nativeIter; nativeInfoVec.end() != nativeIter; ++nativeIter)
	{
		std::map<std::string, ResourceUpdateInfo>::iterator checkIter = checkInfoVec.find(nativeIter->first);
		if (checkInfoVec.end() == checkIter)
		{
			removeVec.push_back(nativeIter->first);
		}
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::parseFileInfo(const std::string& filename, std::map<std::string, ResourceUpdateInfo>& infoVec)
{
	const std::vector<std::string> &lineVec = getFileDataEx(filename);
	for (size_t i=0; i<lineVec.size(); ++i)
	{
		const std::vector<std::string> &line = splitString(lineVec[i], ",");
		ResourceUpdateInfo info;
		info.md5value = line[0];
		info.filename = line[1];
		info.filesize = atol(line[2].c_str());
		infoVec.insert(std::make_pair(FileDownload::getFileName(info.filename), info));
	}
}
//--------------------------------------------------------------------------
std::string ResourceUpdate::newLineString(void)
{
	return std::string("\r\n");
}
//--------------------------------------------------------------------------
std::vector<std::string> ResourceUpdate::splitString(std::string str, const std::string& pattern)
{
	std::vector<std::string> result;
	if (0 == str.compare("") || 0 == pattern.compare(""))
		return result;

	str += pattern;	// 扩展字符串以方便操作
	std::string::size_type pos;
	for (size_t i=0; i<str.size(); ++i)
	{
		pos = str.find(pattern, i);
		if (pos < str.size())
		{
			result.push_back(str.substr(i, pos - i));
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}
//--------------------------------------------------------------------------
bool ResourceUpdate::existFile(const std::string& file)
{
	FILE *fp = fopen(file.c_str(), "r");
	if (NULL == fp)
		return false;

	fclose(fp);
	fp = NULL;
	return true;
}
//--------------------------------------------------------------------------
char* ResourceUpdate::getFileData(const std::string& file, unsigned long* fileSize)
{
	FILE *fp = fopen(file.c_str(), "rb");
	if (NULL == fp)
		return NULL;

    fseek(fp, 0, SEEK_END);
    *fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buffer = new char[*fileSize + 1];
    *fileSize = fread(buffer, sizeof(char), *fileSize, fp);
	*(buffer + *fileSize) = '\0';	// 要设置文件末尾结束符
    fclose(fp);
	return buffer;
}
//--------------------------------------------------------------------------
std::vector<std::string> ResourceUpdate::getFileDataEx(const std::string& file)
{
	std::string fileString = "";
	unsigned long fileSize;
	char *fileData = getFileData(file, &fileSize);
	if (fileData)
	{
		fileString = fileData;
		delete fileData;
		fileData = NULL;
	}
	return splitString(fileString, newLineString());
}
//------------------------------------------------------------------------
bool ResourceUpdate::writeDataToFile(const char* data, unsigned long dataSize, const std::string& file)
{
	if (NULL == data)
		return false;
	
	FILE *fp = fopen(file.c_str(), "wb");
	if (NULL == fp)
		return false;
	
	fwrite(data, dataSize, sizeof(char), fp);
    fclose(fp);
	return true;
}
//------------------------------------------------------------------------

