/**********************************************************************
* Author:	jaron.ho
* Date:		2015-12-01
* Brief:	×ÊÔ´ÏÂÔØ
**********************************************************************/
#include "ResourceDownload.h"
//------------------------------------------------------------------------
ResourceDownload::ResourceDownload(void) {
	mFileDownload.setListener(this);
}
//------------------------------------------------------------------------
ResourceDownload::~ResourceDownload(void) {
}
//------------------------------------------------------------------------
void ResourceDownload::onError(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer) {
	if (FDC_DOWNLOAD_ERROR == code) {
		if (mErrorCB) {
			mErrorCB(fileURL, curlCode, responseCode, buffer);
		}
	}
}
//------------------------------------------------------------------------
void ResourceDownload::onProgress(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer, double totalToDownload, double nowDownloaded) {
	if (FDC_FILE_PROGRESS == code) {
		if (mProgressCB) {
			mProgressCB(fileURL, totalToDownload, nowDownloaded);
		}
	} else if (FDC_LIST_PROGRESS == code) {
		if (mTotalProgressCB) {
			mTotalProgressCB(fileURL, (int)totalToDownload, (int)nowDownloaded);
		}
	}
}
//------------------------------------------------------------------------
void ResourceDownload::onSuccess(FileDownloadCode code, const std::string& fileURL, int curlCode, int responseCode, const std::string& buffer) {
	if (FDC_FILE_SUCCESS == code) {
		if (mSuccessCB) {
			mSuccessCB(fileURL);
		}
	} else if (FDC_LIST_SUCCESS == code) {
		if (mTotalSuccessCB) {
			mTotalSuccessCB();
		}
	}
}
//------------------------------------------------------------------------
unsigned int ResourceDownload::getId(void) {
	return mFileDownload.getId();
}
//------------------------------------------------------------------------
void ResourceDownload::listen(void) {
	mFileDownload.listenMessage();
}
//------------------------------------------------------------------------
void ResourceDownload::setDownloadPath(const std::string& path) {
	if (path.empty()) {
		return;
	}
	mFileDownload.setStoragePath(path);
}
//------------------------------------------------------------------------
void ResourceDownload::setTimeout(unsigned int connectTimeout, unsigned int downloadTimeout) {
	mFileDownload.setConnectTimeout(connectTimeout);
	mFileDownload.setDownloadTimeout(downloadTimeout);
}
//------------------------------------------------------------------------
void ResourceDownload::setErrorCB(const RDErrorCB& callback) {
	if (!mFileDownload.isDownloading()) {
		mErrorCB = callback;
	}
}
//------------------------------------------------------------------------
void ResourceDownload::setProgressCB(const RDProgressCB& callback) {
	if (!mFileDownload.isDownloading()) {
		mProgressCB = callback;
	}
}
//------------------------------------------------------------------------
void ResourceDownload::setSuccessCB(const RDSuccessCB& callback) {
	if (!mFileDownload.isDownloading()) {
		mSuccessCB = callback;
	}
}
//------------------------------------------------------------------------
void ResourceDownload::setTotalProgressCB(const RDTotalProgressCB& callback) {
	if (!mFileDownload.isDownloading()) {
		mTotalProgressCB = callback;
	}
}
//------------------------------------------------------------------------
void ResourceDownload::setTotalSuccessCB(const RDTotalSuccessCB& callback) {
	if (!mFileDownload.isDownloading()) {
		mTotalSuccessCB = callback;
	}
}
//------------------------------------------------------------------------
bool ResourceDownload::excute(const std::vector<std::string>& fileUrlVec, const std::string& cacheSuffix /*= ""*/) {
	if (fileUrlVec.empty()) {
		return false;
	}
	return mFileDownload.download(fileUrlVec, cacheSuffix);
}
//------------------------------------------------------------------------
bool ResourceDownload::isDownloading(void) {
	return mFileDownload.isDownloading();
}
//------------------------------------------------------------------------