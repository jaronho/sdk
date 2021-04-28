/**********************************************************************
* Author:	jaron.ho
* Date:		2014-02-14
* Brief:	资源更新
**********************************************************************/
#include "ResourceUpdate.h"
#include <stdlib.h>
//------------------------------------------------------------------------
void* updateListPrepareProcessFunc(void* ptr);
void* updateListEndedProcessFunc(void* ptr);
void* recordProcessFunc(void* ptr);
static std::map<unsigned int, bool> sObjectMessageQueueFlag;
//------------------------------------------------------------------------
ResourceUpdate::ResourceUpdate(void)
:mCacheSuffix("")
,mIsRemoveInvalidFile(true)
,mUpdateFileSize(0)
,mHasRecordCount(0) {
	mFileDownload.setListener(this);
	createMessageQueue();
}
//------------------------------------------------------------------------
ResourceUpdate::~ResourceUpdate(void) {
	for (size_t i=0; i<mCheckMd5FileList.size(); ++i) {
		remove((mDIR + mCheckMd5FileList[i]).c_str());
	}
	destroyMessageQueue();
}
//------------------------------------------------------------------------
void ResourceUpdate::createMessageQueue(void) {
	sObjectMessageQueueFlag.insert(std::make_pair(mFileDownload.getId(), true));
	mMessageList = new std::list<RUThreadType>();
}
//------------------------------------------------------------------------
void ResourceUpdate::destroyMessageQueue(void) {
	sObjectMessageQueueFlag.erase(mFileDownload.getId());
	mMessageQueueMutex.lock();
	if (mMessageList) {
		mMessageList->clear();
		delete mMessageList;
		mMessageList = NULL;
	}
	mMessageQueueMutex.unlock();
}
//------------------------------------------------------------------------
void ResourceUpdate::sendMessage(RUThreadType msg) {
	if (!mMessageList || !sObjectMessageQueueFlag[mFileDownload.getId()]) {
		return;
	}
	mMessageQueueMutex.lock();
	mMessageList->push_back(msg);
	mMessageQueueMutex.unlock();
}
//------------------------------------------------------------------------
void ResourceUpdate::recvMessage(void) {
	if (!mMessageList || !sObjectMessageQueueFlag[mFileDownload.getId()]) {
		return;
	}
	mMessageQueueMutex.lock();
	while (mMessageList->size() > 0) {
		RUThreadType msg = *(mMessageList->begin());
		mMessageList->pop_front();
		switch (msg) {
		case RUTT_UPDATE_LIST_NOT_FOUND: {
				if (mUpdateListNotFoundCB) {
					mUpdateListNotFoundCB();
				}
			}
			break;
		case RUTT_UPDATE_LIST: {
				if (mUpdateListCB) {
					mUpdateListCB(mUpdateFileList.size(), mUpdateFileSize);
				}
			}
			break;
		case RUTT_UPDATE_LIST_ENDED: {
				if (mTotalSuccessCB) {
					mTotalSuccessCB();
				}
			}
			break;
		default:
			break;
		}
	}
	mMessageQueueMutex.unlock();
}
//------------------------------------------------------------------------
void ResourceUpdate::onError(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer) {
	if (FDC_DOWNLOAD_ERROR == code) {
		if (isCheckMd5File(fileURL)) {
			if (mUpdateListErrorCB) {
				mUpdateListErrorCB(fileURL, curlCode, responseCode, buffer);
			}
		} else {
			if (mErrorCB) {
				mErrorCB(fileURL, curlCode, responseCode, buffer);
			}
		}
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::onProgress(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer, double totalToDownload, double nowDownloaded) {
	if (FDC_FILE_PROGRESS == code) {
		if (!isCheckMd5File(fileURL)) {
			if (mProgressCB) {
				mProgressCB(fileURL, totalToDownload, nowDownloaded);
			}
		}
	} else if (FDC_LIST_PROGRESS == code) {
		if (!isCheckMd5File(fileURL)) {
			if (mTotalProgressCB) {
				mTotalProgressCB(fileURL, (int)totalToDownload, (int)nowDownloaded);
			}
		}
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::onSuccess(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer) {
	if (FDC_FILE_SUCCESS == code) {
		if (isCheckMd5File(fileURL)) {
			if (!parseFileInfo(mDIR + FileDownload::stripFileInfo(fileURL)[1], mCheckInfoVec)) {
				onError(FDC_DOWNLOAD_ERROR, fileURL, curlCode, responseCode, "ERROR_FILE_FORMAT");
			}
		} else {
			if (checkMd5Value(fileURL)) {
				mHasUpdateFileList.push_back(fileURL);
				if (mSuccessCB) {
					mSuccessCB(fileURL);
				}
			} else {
				onError(FDC_DOWNLOAD_ERROR, fileURL, curlCode, responseCode, "ERROR_FILE_MD5");
			}
		}
	} else if (FDC_LIST_SUCCESS == code) {
		if (isCheckMd5File(fileURL)) {
			handleUpdateListPrepare();
		} else {
			handleUpdateListEnded();
		}
	}
}
//------------------------------------------------------------------------
unsigned int ResourceUpdate::getId(void) {
	return mFileDownload.getId();
}
//------------------------------------------------------------------------
void ResourceUpdate::listen(void) {
	mFileDownload.listenMessage();
	recvMessage();
}
//------------------------------------------------------------------------
void ResourceUpdate::setTimeout(unsigned int connectTimeout, unsigned int downloadTimeout) {
	mFileDownload.setConnectTimeout(connectTimeout);
	mFileDownload.setDownloadTimeout(downloadTimeout);
}
//------------------------------------------------------------------------
void ResourceUpdate::setFileMd5CheckFunc(const RUMd5CheckFunc& func) {
	mFileMd5CheckFunc = func;
}
//------------------------------------------------------------------------
void ResourceUpdate::setUpdateListErrorCB(const RUUpdateListErrorCB& callback) {
	if (!mFileDownload.isDownloading()) {
		mUpdateListErrorCB = callback;
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::setUpdateListNotFoundCB(const RUUpdateListNotFoundCB& callback) {
	if (!mFileDownload.isDownloading()) {
		mUpdateListNotFoundCB = callback;
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::setUpdateListCB(const RUUpdateListCB& callback) {
	if (!mFileDownload.isDownloading()) {
		mUpdateListCB = callback;
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::setErrorCB(const RUErrorCB& callback) {
	if (!mFileDownload.isDownloading()) {
		mErrorCB = callback;
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::setProgressCB(const RUProgressCB& callback) {
	if (!mFileDownload.isDownloading()) {
		mProgressCB = callback;
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::setSuccessCB(const RUSuccessCB& callback) {
	if (!mFileDownload.isDownloading()) {
		mSuccessCB = callback;
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::setTotalProgressCB(const RUTotalProgressCB& callback) {
	if (!mFileDownload.isDownloading()) {
		mTotalProgressCB = callback;
	}
}
//------------------------------------------------------------------------
void ResourceUpdate::setTotalSuccessCB(const RUTotalSuccessCB& callback) {
	if (!mFileDownload.isDownloading()) {
		mTotalSuccessCB = callback;
	}
}
//------------------------------------------------------------------------
bool ResourceUpdate::setNative(const std::string& path, const std::string& nativeMd5File) {
	if (path.empty() || nativeMd5File.empty() || mFileDownload.isDownloading()) {
		return false;
	}
	std::string backslash = ('/' != path.at(path.size() - 1) && '\\' != path.at(path.size() - 1)) ? "/" : "";
	mFileDownload.setStoragePath(path + backslash);
	mDIR = path + backslash;
	mNativeMd5File = nativeMd5File;
	mNativeInfoVec.clear();
	parseFileInfo(mDIR + mNativeMd5File, mNativeInfoVec);
	return true;
}
//------------------------------------------------------------------------
bool ResourceUpdate::checkUpdate(const std::string& url, const std::vector<std::string>& checkMd5FileList) {
	if (mDIR.empty() || url.empty() || checkMd5FileList.empty() || mFileDownload.isDownloading()) {
		return false;
	}
	std::string backslash = '/' != url.at(url.size() - 1) ? "/" : "";
	std::vector<std::string> checkMd5FileUrlList;
	for (size_t i=0; i<checkMd5FileList.size(); ++i) {
		if (checkMd5FileList[i].length() > 0) {
			checkMd5FileUrlList.push_back(url + backslash + checkMd5FileList[i]);
		}
	}
	if (checkMd5FileUrlList.empty()) {
		return false;
	}
	mURL = url + backslash;
	for (size_t i=0; i<mCheckMd5FileList.size(); ++i) {
		remove((mDIR + mCheckMd5FileList[i]).c_str());
	}
	mCheckMd5FileList = checkMd5FileList;
	mCheckInfoVec.clear();
	return mFileDownload.download(checkMd5FileUrlList, "");
}
//------------------------------------------------------------------------
void ResourceUpdate::handleUpdateListPrepare(void) {
	std::thread updateListPrepareThread(updateListPrepareProcessFunc, this);
	updateListPrepareThread.detach();
}
//------------------------------------------------------------------------
void* updateListPrepareProcessFunc(void* ptr) {
	ResourceUpdate* self = (ResourceUpdate*)ptr;
	self->updateListPrepareImpl();
	return NULL;
}
//------------------------------------------------------------------------
void ResourceUpdate::updateListPrepareImpl(void) {
	mUpdateFileList.clear();
	mUpdateFileSize = parseUpdateFileList(mUpdateFileList);
	mHasUpdateFileList.clear();
	mHasRecordCount = 0;
	if (mUpdateFileList.empty()) {
		sendMessage(RUTT_UPDATE_LIST_NOT_FOUND);
	} else {
		sendMessage(RUTT_UPDATE_LIST);
	}
}
//------------------------------------------------------------------------
bool ResourceUpdate::startUpdate(const std::string& cacheSuffix /*= ""*/, bool removeInvalidFileFlag /*= true*/) {
	if (mURL.empty() || mUpdateFileList.empty() || mFileDownload.isDownloading()) {
		return false;
	}
	mCacheSuffix = cacheSuffix;
	mIsRemoveInvalidFile = removeInvalidFileFlag;
	std::vector<std::string> fileUrlList;
	for (size_t i=0; i<mUpdateFileList.size(); ++i) {
		fileUrlList.push_back(mURL + mUpdateFileList[i]);
	}
	return mFileDownload.download(fileUrlList, cacheSuffix);
}
//------------------------------------------------------------------------
bool ResourceUpdate::isDownloading(void) {
	return mFileDownload.isDownloading();
}
//------------------------------------------------------------------------
void ResourceUpdate::handleUpdateListEnded(void) {
	std::thread updateListEndedThread(updateListEndedProcessFunc, this);
	updateListEndedThread.detach();
}
//------------------------------------------------------------------------
void* updateListEndedProcessFunc(void* ptr) {
	ResourceUpdate* self = (ResourceUpdate*)ptr;
	self->updateListEndedImpl();
	return NULL;
}
//------------------------------------------------------------------------
void ResourceUpdate::updateListEndedImpl(void) {
	std::map<std::string, ResourceUpdateInfo> infoVec;
	if (mIsRemoveInvalidFile) {
		std::vector<std::string> invalidFileList;
		parseInvalidFileList(invalidFileList);
		for (size_t i=0; i<invalidFileList.size(); ++i)
		{
			remove((mDIR + invalidFileList[i]).c_str());
		}
		infoVec = mCheckInfoVec;
	} else {
		infoVec = mNativeInfoVec;
		std::map<std::string, ResourceUpdateInfo>::iterator checkIter = mCheckInfoVec.begin();
		for (; mCheckInfoVec.end() != checkIter; ++checkIter) {
			infoVec.insert(std::make_pair(checkIter->first, checkIter->second));
		}
	}
	// save native md5 info
	saveUpdateFileList(infoVec, mDIR + mNativeMd5File);
	mHasRecordCount = mHasUpdateFileList.size();
	sendMessage(RUTT_UPDATE_LIST_ENDED);
}
//------------------------------------------------------------------------
bool ResourceUpdate::record(bool immediatelyFlag /*= true*/) {
	if (mHasRecordCount >= mHasUpdateFileList.size()) {
		return false;
	}
	if (immediatelyFlag) {
		recordImpl();
	} else {
		std::thread recordThread(recordProcessFunc, this);
		recordThread.detach();
	}
	return true;
}
//------------------------------------------------------------------------
void* recordProcessFunc(void* ptr) {
	ResourceUpdate* self = (ResourceUpdate*)ptr;
	self->recordImpl();
	return NULL;
}
//------------------------------------------------------------------------
void ResourceUpdate::recordImpl(void) {
	// step1: update native md5 info
	std::vector<ResourceUpdateInfo> downloadedVec;
	for (size_t i=mHasRecordCount; i<mHasUpdateFileList.size(); ++i, ++mHasRecordCount) {
		const std::string fileName = FileDownload::stripFileInfo(mHasUpdateFileList[i])[1];
		std::map<std::string, ResourceUpdateInfo>::iterator checkIter = mCheckInfoVec.find(fileName);
		if (mCheckInfoVec.end() == checkIter) {
			continue;
		}
		std::map<std::string, ResourceUpdateInfo>::iterator nativeIter = mNativeInfoVec.find(fileName);
		if (mNativeInfoVec.end() == nativeIter) {
			mNativeInfoVec.insert(std::make_pair(fileName, checkIter->second));
		} else {
			nativeIter->second.md5value = checkIter->second.md5value;
			nativeIter->second.filename = checkIter->second.filename;
			nativeIter->second.filesize = checkIter->second.filesize;
		}
	}
	// step2: save native md5 info
	saveUpdateFileList(mNativeInfoVec, mDIR + mNativeMd5File);
}
//------------------------------------------------------------------------
bool ResourceUpdate::checkMd5Value(const std::string& fileURL) {
	std::string fileName = FileDownload::stripFileInfo(fileURL)[1];
	std::map<std::string, ResourceUpdateInfo>::iterator checkIter = mCheckInfoVec.find(fileName);
	if (mCheckInfoVec.end() == checkIter) {
		return false;
	}
	if (mFileMd5CheckFunc) {
		std::string md5value = mFileMd5CheckFunc(mDIR + fileName + mCacheSuffix);
		return md5value == checkIter->second.md5value;
	}
	return true;
}
//------------------------------------------------------------------------
bool ResourceUpdate::isCheckMd5File(const std::string& fileURL) {
	if (fileURL.empty() || mCheckMd5FileList.empty()) {
		return false;
	}
	for (size_t i=0; i<mCheckMd5FileList.size(); ++i) {
		if (fileURL == mURL + mCheckMd5FileList[i]) {
			return true;
		}
	}
	return false;
}
//------------------------------------------------------------------------
void ResourceUpdate::saveUpdateFileList(const std::map<std::string, ResourceUpdateInfo>& infoVec, const std::string& fileName) {
	std::string str = "";
	std::map<std::string, ResourceUpdateInfo>::const_iterator iter = infoVec.begin();
	for (; infoVec.end() != iter; ++iter) {
		if (infoVec.begin() != iter) {
			str += newLineString();
		}
		char filesizeBuf[32];
		sprintf(filesizeBuf, "%ld", iter->second.filesize);
		str += iter->second.md5value + "," + iter->second.filename + "," + filesizeBuf;
	}
	str += "\0";
	writeDataToFile(str.c_str(), str.size(), fileName);
}
//------------------------------------------------------------------------
long ResourceUpdate::parseUpdateFileList(std::vector<std::string>& updateVec) {
	long totalUpdateSize = 0;
	std::map<std::string, ResourceUpdateInfo>::iterator checkIter = mCheckInfoVec.begin();
	for (; mCheckInfoVec.end() != checkIter; ++checkIter) {
		std::map<std::string, ResourceUpdateInfo>::iterator nativeIter = mNativeInfoVec.find(checkIter->first);
		if (mNativeInfoVec.end() == nativeIter || nativeIter->second.filename != checkIter->second.filename || nativeIter->second.md5value != checkIter->second.md5value) {
			updateVec.push_back(checkIter->second.filename);
			totalUpdateSize += checkIter->second.filesize;
		}
	}
	return totalUpdateSize;
}
//------------------------------------------------------------------------
void ResourceUpdate::parseInvalidFileList(std::vector<std::string>& invalidVec) {
	std::map<std::string, ResourceUpdateInfo>::iterator nativeIter = mNativeInfoVec.begin();
	for (; mNativeInfoVec.end() != nativeIter; ++nativeIter) {
		std::map<std::string, ResourceUpdateInfo>::iterator checkIter = mCheckInfoVec.find(nativeIter->first);
		if (mCheckInfoVec.end() == checkIter) {
			invalidVec.push_back(nativeIter->first);
		}
	}
}
//------------------------------------------------------------------------
bool ResourceUpdate::parseFileInfo(const std::string& filename, std::map<std::string, ResourceUpdateInfo>& infoVec) {
	const std::vector<std::string>& lineVec = getFileDataEx(filename);
	for (size_t i=0; i<lineVec.size(); ++i) {
		const std::vector<std::string> &line = splitString(lineVec[i], ",");
		if (3 != line.size() || line[0].empty() || line[1].empty() || line[2].empty()) {
			infoVec.clear();
			return false;
		}
		ResourceUpdateInfo info;
		info.md5value = line[0];
		info.filename = line[1];
		info.filesize = atol(line[2].c_str());
		infoVec.insert(std::make_pair(FileDownload::stripFileInfo(info.filename)[1], info));
	}
	return true;
}
//--------------------------------------------------------------------------
std::string ResourceUpdate::newLineString(void) {
	return std::string("\r\n");
}
//--------------------------------------------------------------------------
std::vector<std::string> ResourceUpdate::splitString(std::string str, const std::string& pattern) {
	std::vector<std::string> result;
	if (0 == str.compare("") || 0 == pattern.compare("")) {
		return result;
	}
	str += pattern;
	std::string::size_type pos;
	for (size_t i=0; i<str.size(); ++i) {
		pos = str.find(pattern, i);
		if (pos < str.size()) {
			result.push_back(str.substr(i, pos - i));
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}
//--------------------------------------------------------------------------
char* ResourceUpdate::getFileData(const std::string& file, unsigned long* fileSize) {
	FILE* fp = fopen(file.c_str(), "rb");
	if (NULL == fp) {
		return NULL;
	}
    fseek(fp, 0, SEEK_END);
    *fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* buffer = new char[*fileSize + 1];
    *fileSize = fread(buffer, sizeof(char), *fileSize, fp);
	*(buffer + *fileSize) = '\0';
    fclose(fp);
	return buffer;
}
//--------------------------------------------------------------------------
std::vector<std::string> ResourceUpdate::getFileDataEx(const std::string& file) {
	std::string fileString = "";
	unsigned long fileSize;
	char* fileData = getFileData(file, &fileSize);
	if (fileData) {
		fileString = fileData;
		delete fileData;
		fileData = NULL;
	}
	return splitString(fileString, newLineString());
}
//------------------------------------------------------------------------
bool ResourceUpdate::writeDataToFile(const char* data, unsigned long dataSize, const std::string& file) {
	if (NULL == data) {
		return false;
	}
	FILE* fp = fopen(file.c_str(), "wb");
	if (NULL == fp) {
		return false;
	}
	fwrite(data, dataSize, sizeof(char), fp);
    fclose(fp);
	return true;
}
//------------------------------------------------------------------------