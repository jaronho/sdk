/**********************************************************************
* Author:	jaron.ho
* Date:		2014-03-08
* Brief:	CURLEx
**********************************************************************/
#ifndef _CURL_EX_H_
#define _CURL_EX_H_

#include <string>
#include <vector>
#include <curl/curl.h>

/*
* write/read callback defined
*/
typedef unsigned int (*CURLEx_callback)(void* buffer, unsigned int size, unsigned int number, void* userdata);

/*
* progress callback defined
*/
typedef int (*CURLEx_progress)(void* ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded);

/*
* packaging for CURL, make the use of interface more convenience
*/
class CURLEx {
public:
	CURLEx(void);
	~CURLEx(void);

public:
	template <class T>
    CURLcode setOption(CURLoption option, T data) {
		if (NULL == mCurl) {
			return CURLE_FAILED_INIT;
		}
		CURLcode code = curl_easy_setopt(mCurl, option, data);
        return code;
    }

public:
	// init CURL options
	bool initialize(const std::string& sslCaFileName = "");

	// set cookie file
	bool setCookieFile(const std::string& cookieFile = "");

	// set timeout for connect
	bool setConnectTimeout(int timeout);

	// set timeout for read
	bool setTimeout(int timeout);

	// set full url to get/put
	bool setURL(const std::string& url);

	// set headers
	bool setHeaders(const std::vector<std::string>& headers);

	// set post fields and field size
	bool setPostFields(const char* fields, unsigned int fieldsize);

	bool setHeadFunction(CURLEx_callback func, void* userdata);

	bool setWriteFunction(CURLEx_callback func, void* userdata);

	bool setReadFunctioin(CURLEx_callback func, void* userdata);

	bool setProgressFunction(CURLEx_progress func, void* userdata);

	//------------------------ multipart/formdata block ------------------------
	bool addForm(curl_forms forms[], unsigned int length);

	bool addForm(const std::string& name, CURLformoption option, const std::string& value, const std::string& type = "");

	bool addFormContent(const std::string& name, const std::string& content, const std::string& type = "");

	bool addFormFile(const std::string& name, const std::string& file, const std::string& type = "");

	// called when multipart/formdata, after call addForm
	bool setHttpPost(void);
	//--------------------------------------------------------------------

	// called at last
	bool perform(int* curlCode = NULL, int* responseCode = NULL, std::string* errorBuffer = NULL);

private:
	char mErrorBuffer[CURL_ERROR_SIZE];		// error buffer
	CURL* mCurl;							// instance of curl
    curl_slist* mHeaders;					// keeps custom header data
	curl_httppost* mPost;					// needed when multipart/formdata request
	static unsigned int sObjCount;			// object count in program
};

/*
* struct of curl request
*/
class CurlRequest {
public:
	CurlRequest(void) : connecttimeout(30), timeout(60) {}

	void setData(const char* buffer, unsigned int len) {
		if (NULL != buffer && len > 0) {
			data.assign(buffer, buffer + len);
		}
	}

	char* getData(void) {
		if (data.size() > 0) {
			return &(data.front());
		}
		return NULL;
	}

	unsigned int getDataSize(void) {
		return data.size();
	}

public:
	std::string sslcafilename;				// ssl CA file name
	std::string cookiefilename;				// cookie file name
	std::string url;						// request url
	std::vector<std::string> headers;		// header
	int connecttimeout;						// connect timeout
	int timeout;							// read timeout

private:
	std::vector<char> data;					// request data, support binary data
};

/*
* curl request interface
*/
bool curlGet(CurlRequest& request, CURLEx_callback writeFunc, void* writeStream, CURLEx_callback headerCallback, void* headerStream, int* curlCode = NULL, int* responseCode = NULL, std::string* errorBuffer = NULL);

bool curlPost(CurlRequest& request, CURLEx_callback writeFunc, void* writeStream, CURLEx_callback headerCallback, void* headerStream, int* curlCode = NULL, int* responseCode = NULL, std::string* errorBuffer = NULL);

bool curlPut(CurlRequest& request, CURLEx_callback writeFunc, void* writeStream, CURLEx_callback headerCallback, void* headerStream, int* curlCode = NULL, int* responseCode = NULL, std::string* errorBuffer = NULL);

bool curlDelete(CurlRequest& request, CURLEx_callback writeFunc, void* writeStream, CURLEx_callback headerCallback, void* headerStream, int* curlCode = NULL, int* responseCode = NULL, std::string* errorBuffer = NULL);

#endif	// _CURL_EX_H_

/*
************************************************** sample_01

static unsigned int uploadWriteFunc(void* buffer, unsigned int size, unsigned int number, void* userdata) {
	std::string* responseStr = (std::string*)userdata;
	unsigned int length = size * number;
	responseStr->append((char*)buffer, length);
	return length;
}

std::string Screenshot::uploadScreenshotImage(std::string uploadUrl, std::string account, std::string localPath) {
	if (uploadUrl.empty() || account.empty() || localPath.empty()) {
		return "";
	}
	FILE* fp = fopen(localPath.c_str(), "rb");
	if (NULL == fp) {
		return "";
	}
	CURLEx curl;
	if (false == curl.initialize()) {
		return "";
	}
	int responseCode = -1;
	std::string errorBuffer = "";
	std::string responseStr = "";
	curl.setURL(uploadUrl);
	curl.addFormContent("acct", account);
	curl.addFormFile("image", localPath);
	curl.setHttpPost();
	curl.setWriteFunction(uploadWriteFunc, &responseStr);
	bool res = curl.perform(&responseCode, &errorBuffer);
	if (false == res) {
		return "";
	}
	Json* root= Json_create(responseStr.c_str());
	if (NULL == root) {
		return "";
	}
	unsigned int status= Json_getItem(root, "status")->valueint;
	if (1 == status) {
		return "";
	}
	std::string imageURL = Json_getItem(root, "image_path")->valuestring;
	mUrlPath = imageURL;
	return imageURL;
}
*/

/*
************************************************** sample_02

static unsigned int downloadFileWriteFunc(void* ptr, unsigned int size, unsigned int number, void* userdata) {
	FILE* fp = (FILE*)userdata;
	unsigned int written = fwrite(ptr, size, number, fp);
	return written;
}

static int downloadFileProgressFunc(void* ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded) {
	FileDownload* self = (FileDownload*)ptr;
	if (self) {
		self->downloadFileProgress(totalToDownload, nowDownloaded);
	}
	return 0;
}

int FileDownload::downloadFile(const std::string& savePath, const std::string& saveName, const std::string& fileURL, unsigned int connectTimeout, unsigned int downloadTimeout, FileDownload* self, int* curlCode, int* responseCode, std::string* buffer) {
	if (curlCode) {
		*curlCode = -1;
	}
	if (responseCode) {
		*responseCode = -1;
	}
	// 参数检测
	if (savePath.empty() || saveName.empty() || fileURL.empty()) {
		if (buffer) {
			*buffer = "parameters is error.";
		}
		return 1;
	}
	// 创建下载的文件保存路径
	std::string backslash = ('/' != savePath.at(savePath.size() - 1) && '\\' != savePath.at(savePath.size() - 1)) ? "/" : "";
	std::string fullFilePath = savePath + backslash + saveName;
	FILE* fp = fopen(fullFilePath.c_str(), "wb");
	if (NULL == fp) {				// 创建保存路劲失败
		if (buffer) {
			*buffer = "create file failed.";
		}
		return 2;
	}
	// 下载文件核心部分
	static CURLEx curlObj;
	if (!curlObj.initialize()) {	// 初始curl失败
		if (buffer) {
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
*/