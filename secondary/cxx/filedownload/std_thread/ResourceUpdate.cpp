/**********************************************************************
* Author:	jaron.ho
* Date:		2014-02-14
* Brief:	资源更新
**********************************************************************/
#include "ResourceUpdate.h"
#include "../MD5/MD5.h"
#include <stdlib.h>

static unsigned int sObjectCount = 0;
static std::map<unsigned int, bool> sObjectMessageQueueFlag;
//------------------------------------------------------------------------
ResourceUpdate::ResourceUpdate(ResourceUpdateListener* listener /*= NULL*/)
: mId(0)
, mMessageList(NULL)
, mTotalUpdateSize(0)
, mUpdateStatus(RUOS_CAN_CHECK_VERSION)
, mDeepCheckUpdate(true)
{
	mId = sObjectCount++;
	mListener = listener;
	mIsInnerCreatedListener = false;
	if (NULL == mListener)
	{
		mListener = new ResourceUpdateListener();
		mIsInnerCreatedListener = true;
	}
	mFileDownload.setListener(this);
	createMessageQueue();
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
}
//------------------------------------------------------------------------
void ResourceUpdate::createMessageQueue(void)
{
	sObjectMessageQueueFlag.insert(std::make_pair(mId, true));
	mMessageList = new std::list<RUPthreadType>();
}
//------------------------------------------------------------------------
void ResourceUpdate::destroyMessageQueue(void)
{
	sObjectMessageQueueFlag.erase(mId);
	mMessageQueueMutex.lock();
	if (mMessageList)
	{
		mMessageList->clear();
		delete mMessageList;
		mMessageList = NULL;
	}
	mMessageQueueMutex.unlock();
}
//------------------------------------------------------------------------
void ResourceUpdate::sendMessage(RUPthreadType msg)
{
	if (false == sObjectMessageQueueFlag[mId])
	{
		return;
	}
	mMessageQueueMutex.lock();
	mMessageList->push_back(msg);
	mMessageQueueMutex.unlock();
}
//------------------------------------------------------------------------
void ResourceUpdate::recvMessage(void)
{
	if (false == sObjectMessageQueueFlag[mId])
	{
		return;
	}
	RUPthreadType msg = RAUPT_NONE;
	mMessageQueueMutex.lock();
	if (mMessageList->size() > 0)
	{
		msg = *(mMessageList->begin());
		mMessageList->pop_front();
	}
	mMessageQueueMutex.unlock();
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
void ResourceUpdate::onProgress(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer, double totalToDownload, double nowDownloaded)
{
	if (fileURL == mURL + mCheckVersionFile || fileURL == mURL + mCheckMd5File)
	{
		return;
	}
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
void ResourceUpdate::onSuccess(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer)
{
	if (FDC_LIST_SUCCESS == code)
	{
		if (fileURL == mURL + mCheckVersionFile)
		{
			handleVersionCheckFileDownloaded(true, fileURL, curlCode, responseCode, buffer);		// 版本校验文件下载成功
		}
		else if (fileURL == mURL + mCheckMd5File)
		{
			if (parseFileInfo(mDownloadDir + mCheckVersionFile, mCheckInfoVec))
			{
				handleMd5CheckFileDownloaded(true, fileURL, curlCode, responseCode, buffer);			// MD5列表校验文件下载成功
			}
			else
			{
				onError(FDC_LIST_ERROR, fileURL, curlCode, responseCode, "md5 check file format is error.");
			}
		}
		else
		{
			mUpdateStatus = RUOS_CAN_CHECK_VERSION;
			handleUpdateListDownloaded();						// 更新列表全部下载成功(删除废弃的资源)
		}
	}
	else
	{
		if (fileURL != mURL + mCheckVersionFile && fileURL != mURL + mCheckMd5File)
		{
			if (checkMd5Value(fileURL))
			{
				mDownloadedVec.push_back(fileURL);
				mListener->onSuccess(fileURL);
			}
			else
			{
				onError(FDC_FILE_ERROR, fileURL, curlCode, responseCode, "file md5 value is error.");
			}
		}
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::onError(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer)
{
	mUpdateStatus = RUOS_CAN_CHECK_VERSION;
	if (FDC_DOWNLOAD_ERROR == code)
	{
		if (fileURL == mURL + mCheckVersionFile)
		{
			handleVersionCheckFileDownloaded(false, fileURL, curlCode, responseCode, buffer);	// 版本校验文件下载失败
		}
		else if (fileURL == mURL + mCheckMd5File)
		{
			handleMd5CheckFileDownloaded(false, fileURL, curlCode, responseCode, buffer);		// MD5列表校验文件下载失败
		}
		else
		{
			mListener->onError(fileURL, curlCode, responseCode, buffer);
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
void ResourceUpdate::setTimeout(unsigned int connectTimeout, unsigned int downloadTimeout)
{
	mFileDownload.setConnectTimeout(connectTimeout);
	mFileDownload.setDownloadTimeout(downloadTimeout);
}
//------------------------------------------------------------------------
bool ResourceUpdate::setNative(const std::string& path,const std::string& nativeVersionFile, const std::string& nativeMd5File)
{
	if (path.empty() || nativeVersionFile.empty() || nativeMd5File.empty())
	{
		return false;
	}
	mFileDownload.setStoragePath(path);
	mDownloadDir = path;
	mNativeVersionFile = nativeVersionFile;
	mNativeMd5File = nativeMd5File;
	unsigned long length = 0;
	char* buffer = getFileData(path + nativeVersionFile, &length);
	if (buffer)
	{
		mCurrVersion = buffer;
		mNewVersion = mCurrVersion;
		delete buffer;
		buffer = NULL;
	}
	parseFileInfo(mDownloadDir + mNativeMd5File, mNativeInfoVec);
	return true;
}
//------------------------------------------------------------------------
bool ResourceUpdate::checkVersion(const std::string& url, const std::string& checkVersionFile, const std::string& checkMd5File)
{
	if (mDownloadDir.empty() || url.empty() || checkVersionFile.empty() || checkMd5File.empty())
	{
		return false;
	}
	if (RUOS_CAN_CHECK_VERSION != mUpdateStatus)
	{
		return false;
	}
	mUpdateStatus = RUOS_IN_CHECK_VERSION;
	mURL = url;
	mCheckVersionFile = checkVersionFile;
	mCheckMd5File = checkMd5File;
	// 下载版本验证文件
	std::vector<std::string> urlVec;
	urlVec.push_back(url + checkVersionFile);
	return mFileDownload.download(urlVec);
}
//------------------------------------------------------------------------
void ResourceUpdate::handleVersionCheckFileDownloaded(bool downloaded, const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer)
{
	if (downloaded)
	{
		unsigned long length = 0;
		char *buffer = getFileData(mDownloadDir + mCheckVersionFile, &length);
		if (buffer)
		{
			mNewVersion = buffer;
			delete buffer;
			buffer = NULL;
		}
		if (mNewVersion == mCurrVersion)	// 版本一样
		{
			removeCheckFile();
			mListener->onNoNewVersion(mCurrVersion);
		}
		else								// 版本不一样
		{
			mUpdateStatus = RUOS_CAN_CHECK_UPDATE;
			mListener->onNewVersion(mCurrVersion, mNewVersion);
		}
	}
	else
	{
		removeCheckFile();
		mListener->onCheckVersionFailed(fileURL, curlCode, responseCode, errorBuffer);
	}
}
//------------------------------------------------------------------------
bool ResourceUpdate::checkUpdate(bool deepCheck /*= true*/)
{
	if (RUOS_CAN_CHECK_UPDATE != mUpdateStatus)
	{
		return false;
	}
	mUpdateStatus = RUOS_IN_CHECK_UPDATE;
	mDeepCheckUpdate = deepCheck;
	// 下载校验MD5列表文件
	std::vector<std::string> urlVec;
	urlVec.push_back(mURL + mCheckMd5File);
	return mFileDownload.download(urlVec);
}
//------------------------------------------------------------------------
void ResourceUpdate::handleMd5CheckFileDownloaded(bool downloaded, const std::string& fileURL, int curlCode, int responseCode, const std::string& errorBuffer)
{
	if (downloaded)
	{
		createUpdatePthread();
	}
	else
	{
		removeCheckFile();
		mListener->onCheckUpdateListFailed(fileURL, curlCode, responseCode, errorBuffer);
	}
}
//------------------------------------------------------------------------
bool ResourceUpdate::startUpdate(void)
{
	if (RUOS_CAN_UPDATE != mUpdateStatus)
	{
		return false;
	}
	mUpdateStatus = RUOS_IN_UPDATE;
	std::vector<std::string> urlList;
	for (size_t i=0; i<mFileList.size(); ++i)
	{
		urlList.push_back(mURL + mFileList[i]);
	}
	return mFileDownload.download(urlList);
}
//------------------------------------------------------------------------
void* updateProcessFunc(void* ptr)
{
	ResourceUpdate* self = (ResourceUpdate*)ptr;
	self->updateImpl();
	return NULL;
}
//------------------------------------------------------------------------
void ResourceUpdate::updateImpl(void)
{
	mFileList.clear();
	mTotalUpdateSize = parseUpdateFileList(mFileList);
	if (mFileList.empty())		// 没有更新列表
	{
		if (!mDeepCheckUpdate)	// 非深度检查,保存新版本号
		{
			writeDataToFile(mNewVersion.c_str(), mNewVersion.size(), mDownloadDir + mNativeVersionFile);
		}
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
	std::thread updateThread(updateProcessFunc, this);
	updateThread.detach();
}
//------------------------------------------------------------------------
void* removeProcessFunc(void* ptr)
{
	ResourceUpdate* self = (ResourceUpdate*)ptr;
	self->removeImpl();
	return NULL;
}
//------------------------------------------------------------------------
void ResourceUpdate::removeImpl(void)
{
	// step1:删除资源文件
	std::vector<std::string> fileList;
	parseRemoveFileList(fileList);
	for (size_t i=0; i<fileList.size(); ++i)
	{
		std::string filePath = mDownloadDir + fileList[i];
		remove(filePath.c_str());
	}
	// step2:覆盖旧的MD5列表文件
	unsigned long length = 0;
	char *buffer = getFileData(mDownloadDir + mCheckMd5File, &length);
	if (buffer)
	{
		writeDataToFile(buffer, length, mDownloadDir + mNativeMd5File);
		delete buffer;
		buffer = NULL;
	}
	// step3:保存版本号
	writeDataToFile(mNewVersion.c_str(), mNewVersion.size(), mDownloadDir + mNativeVersionFile);
	removeCheckFile();
	sendMessage(RAUPT_UPDATE_LIST_DOWNLOADED);
}
//------------------------------------------------------------------------
void ResourceUpdate::handleUpdateListDownloaded(void)
{
	std::thread removeThread(removeProcessFunc, this);
	removeThread.detach();
}
//------------------------------------------------------------------------
void* recordProcessFunc(void* ptr)
{
	ResourceUpdate* self = (ResourceUpdate*)ptr;
	self->recordImpl();
	return NULL;
}
//------------------------------------------------------------------------
void ResourceUpdate::recordImpl(void)
{
	// 更新本地信息
	std::vector<ResourceUpdateInfo> downloadedVec;
	for (size_t i=0; i<mDownloadedVec.size(); ++i)
	{
		const std::string fileName = FileDownload::getFileName(mDownloadedVec[i]);
		std::map<std::string, ResourceUpdateInfo>::iterator checkIter = mCheckInfoVec.find(fileName);
		if (mCheckInfoVec.end() == checkIter)
		{
			continue;
		}
		std::map<std::string, ResourceUpdateInfo>::iterator nativeIter = mNativeInfoVec.find(fileName);
		if (mNativeInfoVec.end() == nativeIter)
		{
			mNativeInfoVec.insert(std::make_pair(fileName, checkIter->second));
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
	std::map<std::string, ResourceUpdateInfo>::iterator nativeIter = mNativeInfoVec.begin();
	for (; mNativeInfoVec.end() != nativeIter; ++nativeIter)
	{
		if (mNativeInfoVec.begin() != nativeIter)
		{
			str += newLineString();
		}
		char filesizeBuf[32];
		sprintf(filesizeBuf, "%ld", nativeIter->second.filesize);
		str += nativeIter->second.md5value + "," + nativeIter->second.filename + "," + filesizeBuf;
	}
	str += "\0";
	writeDataToFile(str.c_str(), str.size(), mDownloadDir + mNativeMd5File);
}
//------------------------------------------------------------------------
bool ResourceUpdate::record(void)
{
	if (mDownloadedVec.empty())
	{
		return false;
	}
	std::thread recordThread(recordProcessFunc, this);
	recordThread.detach();
	return true;
}
//------------------------------------------------------------------------
void ResourceUpdate::removeCheckFile(void)
{
	remove((mDownloadDir + mCheckVersionFile).c_str());
	remove((mDownloadDir + mCheckMd5File).c_str());
}
//------------------------------------------------------------------------
bool ResourceUpdate::checkMd5Value(const std::string& fileURL)
{
	std::string fileName = FileDownload::getFileName(fileURL);
	std::map<std::string, ResourceUpdateInfo>::iterator checkIter = mCheckInfoVec.find(fileName);
	if (mCheckInfoVec.end() == checkIter)
	{
		return false;
	}
	unsigned long fileLength = 0;
	char* fileData = (char*)getFileData(mDownloadDir + fileName, &fileLength);
	if (NULL == fileData)
	{
		return false;
	}
	std::string md5Value = MD5_sign((unsigned char*)fileData, fileLength);
	delete fileData;
	fileData = NULL;
	return md5Value == checkIter->second.md5value;
}
//------------------------------------------------------------------------
long ResourceUpdate::parseUpdateFileList(std::vector<std::string>& updateVec)
{
	long totalUpdateSize = 0;
	// 检查需要更新的资源
	std::map<std::string, ResourceUpdateInfo>::iterator checkIter = mCheckInfoVec.begin();
	for (; mCheckInfoVec.end() != checkIter; ++checkIter)
	{
		std::map<std::string, ResourceUpdateInfo>::iterator nativeIter = mNativeInfoVec.find(checkIter->first);
		if (mNativeInfoVec.end() == nativeIter || nativeIter->second.filename != checkIter->second.filename || nativeIter->second.md5value != checkIter->second.md5value)
		{
			updateVec.push_back(checkIter->second.filename);
			totalUpdateSize += checkIter->second.filesize;
		}
	}
	return totalUpdateSize;
}
//------------------------------------------------------------------------
void ResourceUpdate::parseRemoveFileList(std::vector<std::string>& removeVec)
{
	// 检查需要删除的资源
	std::map<std::string, ResourceUpdateInfo>::iterator nativeIter = mNativeInfoVec.begin();
	for (; mNativeInfoVec.end() != nativeIter; ++nativeIter)
	{
		std::map<std::string, ResourceUpdateInfo>::iterator checkIter = mCheckInfoVec.find(nativeIter->first);
		if (mCheckInfoVec.end() == checkIter)
		{
			removeVec.push_back(nativeIter->first);
		}
	}
}
//------------------------------------------------------------------------
bool ResourceUpdate::parseFileInfo(const std::string& filename, std::map<std::string, ResourceUpdateInfo>& infoVec)
{
	const std::vector<std::string> &lineVec = getFileDataEx(filename);
	for (size_t i=0; i<lineVec.size(); ++i)
	{
		const std::vector<std::string> &line = splitString(lineVec[i], ",");
		if (3 != line.size() || line[0].empty() || line[1].empty() || line[2].empty())
		{
			infoVec.clear();
			return false;
		}
		ResourceUpdateInfo info;
		info.md5value = line[0];
		info.filename = line[1];
		info.filesize = atol(line[2].c_str());
		infoVec.insert(std::make_pair(FileDownload::getFileName(info.filename), info));
	}
	return true;
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
	{
		return result;
	}
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
	FILE* fp = fopen(file.c_str(), "r");
	if (NULL == fp)
	{
		return false;
	}
	fclose(fp);
	fp = NULL;
	return true;
}
//--------------------------------------------------------------------------
char* ResourceUpdate::getFileData(const std::string& file, unsigned long* fileSize)
{
	FILE* fp = fopen(file.c_str(), "rb");
	if (NULL == fp)
	{
		return NULL;
	}
    fseek(fp, 0, SEEK_END);
    *fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* buffer = new char[*fileSize + 1];
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
	char* fileData = getFileData(file, &fileSize);
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
	{
		return false;
	}
	FILE* fp = fopen(file.c_str(), "wb");
	if (NULL == fp)
	{
		return false;
	}
	fwrite(data, dataSize, sizeof(char), fp);
    fclose(fp);
	return true;
}
//------------------------------------------------------------------------