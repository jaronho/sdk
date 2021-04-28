/**********************************************************************
* Author:	jaron.ho
* Date:		2014-02-08
* Brief:	文件下载
**********************************************************************/
#include "FileDownload.h"
#ifdef WIN32
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

static unsigned int sFileDownloadObjectCount = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// 文件下载帮助类
////////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------
FileDownloadHelper::FileDownloadHelper(void)
{
	mMessageList = new std::list<FileDownloadMessage*>();
}
//------------------------------------------------------------------------
FileDownloadHelper::~FileDownloadHelper(void)
{
	mMessageQueueMutex.lock();
	if (mMessageList)
	{
		std::list<FileDownloadMessage*>::iterator iter = mMessageList->begin();
		for (; mMessageList->end() != iter; ++iter)
		{
			if (*iter)
			{
				delete (*iter);
				(*iter) = NULL;
			}
		}
		mMessageList->clear();
		delete mMessageList;
		mMessageList = NULL;
	}
	mMessageQueueMutex.unlock();
}
//------------------------------------------------------------------------
void FileDownloadHelper::sendMessage(FileDownloadMessage* msg)
{
	mMessageQueueMutex.lock();
	mMessageList->push_back(msg);
	mMessageQueueMutex.unlock();
}
//------------------------------------------------------------------------
void FileDownloadHelper::recvMessage(void)
{
	FileDownloadMessage* msg = NULL;
	mMessageQueueMutex.lock();
	if (mMessageList->size() > 0)
	{
		msg = *(mMessageList->begin());
		mMessageList->pop_front();
	}
	mMessageQueueMutex.unlock();
	if (NULL == msg)
	{
		return;
	}
	// dispatch message
	if (msg->listener)
	{
		switch (msg->code)
		{
		case FDC_CREATE_FILE_FAILED:
		case FDC_INIT_CURL_FAILED:
		case FDC_DOWNLOAD_ERROR:
			msg->listener->onError(msg->code, msg->fileurl, msg->curlcode, msg->responsecode, msg->buffer);
			break;
		case FDC_FILE_PROGRESS:
		case FDC_LIST_PROGRESS:
			msg->listener->onProgress(msg->code, msg->fileurl, msg->curlcode, msg->responsecode, msg->buffer, msg->totalcount, msg->nowcount);
			break;
		case FDC_FILE_SUCCESS:
		case FDC_LIST_SUCCESS:
			msg->listener->onSuccess(msg->code, msg->fileurl, msg->curlcode, msg->responsecode, msg->buffer);
			break;
		default:	// FDC_NONE
			break;
		}
	}
	delete msg;
	msg = NULL;
}
//------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// 文件下载
////////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------
FileDownload::FileDownload(void)
:mId(++sFileDownloadObjectCount)
,mListener(NULL)
,mConnectTimeout(0)
,mDownloadTimeout(0)
,mCurrDownloadCurlCode(-1)
,mCurrDownloadResponseCode(-1)
,mIsDownloading(false)
{
	mHelper = new FileDownloadHelper();
}
//------------------------------------------------------------------------
FileDownload::~FileDownload(void)
{
	mFileUrlVec.clear();
	if (mHelper)
	{
		delete mHelper;
		mHelper = NULL;
	}
}
//------------------------------------------------------------------------
unsigned int FileDownload::getId(void)
{
	return mId;
}
//------------------------------------------------------------------------
void FileDownload::listenMessage(void)
{
	mHelper->recvMessage();
}
//------------------------------------------------------------------------
void FileDownload::setListener(FileDownloadListener* listener)
{
	mListener = listener;
}
//------------------------------------------------------------------------
void FileDownload::setConnectTimeout(unsigned int timeout)
{
	mConnectTimeout = timeout;
}
//------------------------------------------------------------------------
void FileDownload::setDownloadTimeout(unsigned int timeout)
{
	mDownloadTimeout = timeout;
}
//------------------------------------------------------------------------
void FileDownload::setStoragePath(const std::string& storagePath)
{
	const std::string& tempStoragePath = replaceString(storagePath, "\\", "/");
	createDir(tempStoragePath);
	mStoragePath = tempStoragePath;
}
//------------------------------------------------------------------------
std::string FileDownload::getStoragePath(void)
{
	return mStoragePath;
}
//------------------------------------------------------------------------
void* downloadProcessFunc(void* ptr)
{
	FileDownload* self = (FileDownload*)ptr;
	// 全部下载成功
	if (self && self->downloadImpl())
	{
		FileDownloadMessage *msg = new FileDownloadMessage();
		msg->code = FDC_LIST_SUCCESS;
		msg->fileurl = self->mCurrDownloadFileURL;
		msg->curlcode = self->mCurrDownloadCurlCode;
		msg->responsecode =self-> mCurrDownloadResponseCode;
		msg->buffer = self->mCurrDownloadBuffer;
		msg->listener = self->mListener;
		self->mHelper->sendMessage(msg);
	}
	return NULL;
}
//------------------------------------------------------------------------
bool FileDownload::download(const std::vector<std::string>& fileUrlVec)
{
	if (fileUrlVec.empty() || mIsDownloading)
	{
		return false;
	}
	mIsDownloading = true;
	mFileUrlVec = fileUrlVec;
	// 开辟新线程处理文件下载
	std::thread downThread(downloadProcessFunc, this);
	downThread.detach();		// 设置线程分离
	return true;
}
//------------------------------------------------------------------------
bool FileDownload::isDownloading(void)
{
	return mIsDownloading;
}
//------------------------------------------------------------------------
void FileDownload::downloadFileProgress(double totalToDownload, double nowDownloaded)
{
	if (!mHelper)
	{
		return;
	}
	FileDownloadMessage *msg = new FileDownloadMessage();
	msg->code = FDC_FILE_PROGRESS;
	msg->fileurl = mCurrDownloadFileURL;
	msg->curlcode = mCurrDownloadCurlCode;
	msg->responsecode = mCurrDownloadResponseCode;
	msg->buffer = mCurrDownloadBuffer;
	msg->totalcount = totalToDownload;
	msg->nowcount = nowDownloaded;
	msg->listener = mListener;
	mHelper->sendMessage(msg);
}
//------------------------------------------------------------------------
std::string FileDownload::replaceString(std::string str, const std::string& src, const std::string& dest)
{
	if (str.empty() || src.empty() || dest.empty())
	{
		return str;
	}
	std::string::size_type pos = 0;
	while (std::string::npos != (pos = str.find(src, pos)))
	{
		str.replace(pos, src.size(), dest);
		pos += dest.size();
	}
	return str;
}
//------------------------------------------------------------------------
bool FileDownload::createDir(const std::string& dirName)
{
#ifdef WIN32
	return 0 == _mkdir(dirName.c_str());
#else
	return 0 == mkdir(dirName.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
#endif
}
//------------------------------------------------------------------------
void FileDownload::removeDir(const std::string& dirName)
{
#ifdef WIN32
	struct _finddata_t fileData;
	int handle = _findfirst((dirName + "/*.*").c_str(), &fileData);
	if (-1 == handle || _A_SUBDIR != fileData.attrib)
	{
		return;
	}
	while (0 == _findnext(handle, &fileData))
	{
		if (0 == strcmp(fileData.name, ".") || 0 == strcmp(fileData.name, ".."))
		{
			continue;
		}
		std::string subName = dirName + "/" + fileData.name;
		if (_A_SUBDIR == fileData.attrib)
		{
			removeDir(subName);
		}
		else
		{
			remove(subName.c_str());
		}
	}
	_findclose(handle);
	_rmdir(dirName.c_str());
#else
	DIR *dir = opendir(dirName.c_str());
	if (NULL == dir)
	{
		return;
	}
	struct dirent *dirp = NULL;
	while (NULL != (dirp = readdir(dir)))
	{
		if (0 == strcmp(dirp->d_name, ".") || 0 == strcmp(dirp->d_name, ".."))
		{
			continue;
		}
		std::string subName = dirName + "/" + dirp->d_name;
		DIR *subDir = opendir(subName.c_str());
		if (NULL == subDir)
		{
			remove(subName.c_str());
		}
		else
		{
			closedir(subDir);
			subDir = NULL;
			removeDir(subName);
		}
	}
	closedir(dir);
	dir = NULL;
	dirp = NULL;
	rmdir(dirName.c_str());
#endif
}
//------------------------------------------------------------------------
std::string FileDownload::getFileName(const std::string& fullFileName)
{
	std::string fileName = fullFileName;
	size_t pos = fullFileName.find_last_of("/\\:");
	if (pos < fullFileName.size())
	{
		fileName = fullFileName.c_str() + pos + 1;
	}
	return fileName;
}
//------------------------------------------------------------------------
bool FileDownload::downloadImpl()
{
	bool allDownload = true;	// 是否全部下载成功
	// 遍历下载文件列表
	unsigned int totalCount = mFileUrlVec.size();
	for (unsigned int i=0; i<totalCount; ++i)
	{
		const std::string &fileURL = replaceString(mFileUrlVec[i], "\\", "/");
		if (downloadImpl(fileURL))	// 文件下载成功
		{
			FileDownloadMessage* msg = new FileDownloadMessage();
			msg->code = FDC_LIST_PROGRESS;
			msg->fileurl = fileURL;
			msg->curlcode = mCurrDownloadCurlCode;
			msg->responsecode = mCurrDownloadResponseCode;
			msg->buffer = mCurrDownloadBuffer;
			msg->totalcount = totalCount;
			msg->nowcount = i + 1;
			msg->listener = mListener;
			mHelper->sendMessage(msg);
		}
		else						// 文件下载失败
		{
			allDownload = false;
			break;
		}
	}
	mIsDownloading = false;
	return allDownload;
}
//------------------------------------------------------------------------
bool FileDownload::downloadImpl(const std::string& fileURL)
{
	mCurrDownloadFileURL = fileURL;
	int res = downloadFile(mStoragePath, getFileName(fileURL), fileURL, mConnectTimeout, mDownloadTimeout, this, &mCurrDownloadCurlCode, &mCurrDownloadResponseCode, &mCurrDownloadBuffer);
	FileDownloadMessage* msg = new FileDownloadMessage();
	switch (res)
	{
	case 0: msg->code = FDC_FILE_SUCCESS; break;
	case 1: case 2: msg->code = FDC_CREATE_FILE_FAILED; break;
	case 3: msg->code = FDC_INIT_CURL_FAILED; break;
	case 4: msg->code = FDC_DOWNLOAD_ERROR; break;
	}
	msg->fileurl = mCurrDownloadFileURL;
	msg->curlcode = mCurrDownloadCurlCode;
	msg->responsecode = mCurrDownloadResponseCode;
	msg->buffer = mCurrDownloadBuffer;
	msg->listener = mListener;
	mHelper->sendMessage(msg);
	return 0 == res;
}
//------------------------------------------------------------------------
static unsigned int downloadFileWriteFunc(void* ptr, unsigned int size, unsigned int number, void* userdata)
{
	FILE* fp = (FILE*)userdata;
	unsigned int written = fwrite(ptr, size, number, fp);
	return written;
}
//------------------------------------------------------------------------
static int downloadFileProgressFunc(void* ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded)
{
	FileDownload* self = (FileDownload*)ptr;
	if (self)
	{
		self->downloadFileProgress(totalToDownload, nowDownloaded);
	}
	return 0;
}
//------------------------------------------------------------------------
int FileDownload::downloadFile(const std::string& savePath, const std::string& saveName, const std::string& fileURL, unsigned int connectTimeout, unsigned int downloadTimeout, FileDownload* self, int* curlCode, int* responseCode, std::string* buffer)
{
	if (curlCode)
	{
		*curlCode = -1;
	}
	if (responseCode)
	{
		*responseCode = -1;
	}
	// 参数检测
	if (savePath.empty() || saveName.empty() || fileURL.empty())
	{
		if (buffer)
		{
			*buffer = "parameters is error.";
		}
		return 1;
	}
	// 创建下载的文件保存路径
	std::string backslash = ('/' != savePath.at(savePath.size() - 1) && '\\' != savePath.at(savePath.size() - 1)) ? "/" : "";
	std::string fullFilePath = savePath + backslash + saveName;
	FILE* fp = fopen(fullFilePath.c_str(), "wb");
	if (NULL == fp)				// 创建保存路劲失败
	{
		if (buffer)
		{
			*buffer = "create file failed.";
		}
		return 2;
	}
	// 下载文件核心部分
	CURLEx curlObj;
	if (!curlObj.initialize())	// 初始curl失败
	{
		if (buffer)
		{
			*buffer = "curl initialize failed.";
		}
		return 3;
	}
	curlObj.setURL(fileURL);
	curlObj.setConnectTimeout(connectTimeout);
	curlObj.setTimeout(downloadTimeout);
	curlObj.setWriteFunction(downloadFileWriteFunc, fp);
	curlObj.setProgressFunction(downloadFileProgressFunc, self);
	bool res = curlObj.perform(curlCode, responseCode, buffer);
	fclose(fp);
	return res ? 0 : 4;			// 0-文件下载成功;4-文件下载失败
}
//------------------------------------------------------------------------