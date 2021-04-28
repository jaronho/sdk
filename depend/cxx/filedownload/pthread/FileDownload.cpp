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
#include <sys/stat.h>
#include <sys/types.h>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// 文件下载帮助类
////////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------
FileDownloadHelper::FileDownloadHelper(void)
{
	mMessageList = new std::list<FileDownloadMessage*>();
	pthread_mutex_init(&mMessageQueueMutex, NULL);
}
//------------------------------------------------------------------------
FileDownloadHelper::~FileDownloadHelper(void)
{
	pthread_mutex_lock(&mMessageQueueMutex);
	if (mMessageList)
	{
		std::list<FileDownloadMessage*>::iterator iter = mMessageList->begin();
		for (iter; mMessageList->end() != iter; ++iter)
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
	pthread_mutex_unlock(&mMessageQueueMutex);
	pthread_mutex_destroy(&mMessageQueueMutex);
}
//------------------------------------------------------------------------
void FileDownloadHelper::sendMessage(FileDownloadMessage* msg)
{
	pthread_mutex_lock(&mMessageQueueMutex);
	mMessageList->push_back(msg);
	pthread_mutex_unlock(&mMessageQueueMutex);
}
//------------------------------------------------------------------------
void FileDownloadHelper::recvMessage(void)
{
	FileDownloadMessage *msg = NULL;
	pthread_mutex_lock(&mMessageQueueMutex);
	if (mMessageList->size() > 0)
	{
		msg = *(mMessageList->begin());
		mMessageList->pop_front();
	}
	pthread_mutex_unlock(&mMessageQueueMutex);
	if (NULL == msg)
		return;

	// dispatch message
	if (msg->listener)
	{
		switch (msg->code)
		{
		case FDC_CREATE_FILE_FAILED:
		case FDC_INIT_CURL_FAILED:
		case FDC_FILE_ERROR:
		case FDC_LIST_ERROR:
			msg->listener->onError(msg->code, msg->fileurl, msg->buffer);
			break;
		case FDC_FILE_PROGRESS:
		case FDC_LIST_PROGRESS:
			msg->listener->onProgress(msg->code, msg->fileurl, msg->buffer, msg->totalcount, msg->nowcount);
			break;
		case FDC_FILE_SUCCESS:
		case FDC_LIST_SUCCESS:
			msg->listener->onSuccess(msg->code, msg->fileurl, msg->buffer);
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
:mPthread(NULL)
,mListener(NULL)
,mConnectTimeout(0)
,mDownloadTimeout(0)
,mInDownload(false)
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
	if (mPthread)
	{
		delete mPthread;
		mPthread = NULL;
	}
}
//------------------------------------------------------------------------
void FileDownload::setListener(FileDownloadListener* listener)
{
	mListener = listener;
}
//------------------------------------------------------------------------
void FileDownload::listenMessage(void)
{
	mHelper->recvMessage();
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
void* downloadProcessFunc(void* ptr)
{
	FileDownload *self = (FileDownload*)ptr;
	if (self->mPthread)
	{
		delete self->mPthread;
		self->mPthread = NULL;
	}
	FileDownloadMessage *msg = new FileDownloadMessage();
	// 全部下载成功
	if (self->downloadImpl())
	{
		msg->code = FDC_LIST_SUCCESS;
	}
	else
	{
		msg->code = FDC_LIST_ERROR;
	}
	msg->fileurl = self->mCurDownloadFileUrl;
	msg->buffer = self->mCurDownloadBuffer;
	msg->listener = self->mListener;
	self->mHelper->sendMessage(msg);
	return NULL;
}
//------------------------------------------------------------------------
void FileDownload::download(const std::vector<std::string>& fileUrlVec)
{
	if (mPthread || fileUrlVec.empty() || mInDownload)
		return;

	mInDownload = true;
	mFileUrlVec = fileUrlVec;
	// 开辟新线程处理文件下载
	mPthread = new pthread_t();
	pthread_attr_t attr;
	memset(&attr, 0, sizeof(pthread_attr_t));
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);	// 设置线程分离
	pthread_create(mPthread, &attr, downloadProcessFunc, this);
}
//------------------------------------------------------------------------
bool FileDownload::inDownload(void)
{
	return mInDownload;
}
//------------------------------------------------------------------------
void FileDownload::downloadFileProgress(double totalToDownload, double nowDownloaded)
{
	FileDownloadMessage *msg = new FileDownloadMessage();
	msg->code = FDC_FILE_PROGRESS;
	msg->fileurl = mCurDownloadFileUrl;
	msg->buffer = mCurDownloadBuffer;
	msg->totalcount = totalToDownload;
	msg->nowcount = nowDownloaded;
	msg->listener = mListener;
	mHelper->sendMessage(msg);
}
//------------------------------------------------------------------------
std::string FileDownload::replaceString(std::string str, const std::string& src, const std::string& dest)
{
	if (str.empty() || src.empty() || dest.empty())
		return str;

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
	return 0 == mkdir(dirName.c_str());
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
		return;

	while (0 == _findnext(handle, &fileData))
	{
		if (0 == strcmp(fileData.name, ".") || 0 == strcmp(fileData.name, ".."))
			continue;

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
	rmdir(dirName.c_str());
#else
	DIR *dir = opendir(dirName.c_str());
	if (NULL == dir)
		return;

	struct dirent *dirp = NULL;
	while (NULL != (dirp = readdir(dir)))
	{
		if (0 == strcmp(dirp->d_name, ".") || 0 == strcmp(dirp->d_name, ".."))
			continue;

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
		if (downloadImpl(fileURL))	// 单个文件下载成功
		{
			FileDownloadMessage *msg = new FileDownloadMessage();
			msg->code = FDC_LIST_PROGRESS;
			msg->fileurl = fileURL;
			msg->buffer = mCurDownloadBuffer;
			msg->totalcount = totalCount;
			msg->nowcount = i + 1;
			msg->listener = mListener;
			mHelper->sendMessage(msg);
		}
		else						// 某个文件下载失败
		{
			allDownload = false;
			break;
		}
	}
	mInDownload = false;
	return allDownload;
}
//------------------------------------------------------------------------
bool FileDownload::downloadImpl(const std::string& fileURL)
{
	mCurDownloadFileUrl = fileURL;
	mCurDownloadBuffer = "";
	int res = downloadFile(mStoragePath, getFileName(fileURL), fileURL, mConnectTimeout, mDownloadTimeout, &mCurDownloadBuffer, this);
	FileDownloadMessage *msg = new FileDownloadMessage();
	switch (res)
	{
	case 0: msg->code = FDC_FILE_SUCCESS; break;
	case 1: msg->code = FDC_CREATE_FILE_FAILED; break;
	case 2: msg->code = FDC_INIT_CURL_FAILED; break;
	case 3: msg->code = FDC_FILE_ERROR; break;
	}
	msg->fileurl = fileURL;
	msg->buffer = mCurDownloadBuffer;
	msg->listener = mListener;
	mHelper->sendMessage(msg);
	return 0 == res;
}
//------------------------------------------------------------------------
static unsigned int downloadFileWriteFunc(void* ptr, unsigned int size, unsigned int number, void* userdata)
{
	FILE *fp = (FILE*)userdata;
	unsigned int written = fwrite(ptr, size, number, fp);
	return written;
}
//------------------------------------------------------------------------
static int downloadFileProgressFunc(void* ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded)
{
	FileDownload *self = (FileDownload*)ptr;
	if (self)
	{
		self->downloadFileProgress(totalToDownload, nowDownloaded);
	}
	return 0;
}
//------------------------------------------------------------------------
int FileDownload::downloadFile(const std::string& savePath, const std::string& saveName, const std::string& fileURL, unsigned int connectTimeout, unsigned int downloadTimeout, std::string* buffer, FileDownload* self)
{
	char buf[64];
	int curlCode = -1, responseCode = -1;
	// 参数检测
	if (savePath.empty() || saveName.empty() || fileURL.empty())
	{
		if (buffer)
		{
			*buffer = "parameters is error.";
			sprintf(buf, "\",\"curl_code\":\"%d\",\"response_code\":\"%d\",\"error\":\"", curlCode, responseCode);
			*buffer = "{\"url\":\"" + fileURL + buf + *buffer + "\",\"file_path\":\"" + savePath + "\",\"file_name\":\"" + saveName + "\"}";
		}
		return 1;
	}
	// 创建下载的文件保存路径
	std::string backslash = ('/' != savePath.at(savePath.size() - 1) && '\\' != savePath.at(savePath.size() - 1)) ? "/" : "";
	std::string fullFilePath = savePath + backslash + saveName;
	FILE *fp = fopen(fullFilePath.c_str(), "wb");
	if (NULL == fp)				// 创建保存路劲失败
	{
		if (buffer)
		{
			*buffer = "create file failed.";
			sprintf(buf, "\",\"curl_code\":\"%d\",\"response_code\":\"%d\",\"error\":\"", curlCode, responseCode);
			*buffer = "{\"url\":\"" + fileURL + buf + *buffer + "\",\"file_path\":\"" + savePath + "\",\"file_name\":\"" + saveName + "\"}";
		}
		return 1;
	}
	// 下载文件核心部分
	CURLEx curlObj;
	if (!curlObj.initialize())	// 初始curl失败
	{
		if (buffer)
		{
			*buffer = "curl initialize failed.";
			sprintf(buf, "\",\"curl_code\":\"%d\",\"response_code\":\"%d\",\"error\":\"", curlCode, responseCode);
			*buffer = "{\"url\":\"" + fileURL + buf + *buffer + "\",\"file_path\":\"" + savePath + "\",\"file_name\":\"" + saveName + "\"}";
		}
		return 2;
	}
	curlObj.setURL(fileURL);
	curlObj.setWriteFunction(downloadFileWriteFunc, fp);
	curlObj.setProgressFunction(downloadFileProgressFunc, self);
	curlObj.setConnectTimeout(connectTimeout);
	curlObj.setTimeout(downloadTimeout);
	bool res = curlObj.perform(&curlCode, &responseCode, buffer);
	fclose(fp);
	if (buffer)
	{
		sprintf(buf, "\",\"curl_code\":\"%d\",\"response_code\":\"%d\",\"error\":\"", curlCode, responseCode);
		*buffer = "{\"url\":\"" + fileURL + buf + *buffer + "\",\"file_path\":\"" + savePath + "\",\"file_name\":\"" + saveName + "\"}";
	}
	return res ? 0 : 3;			// 0-文件下载成功;3-文件下载失败
}
//------------------------------------------------------------------------
