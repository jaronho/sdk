/**********************************************************************
* Author:	jaron.ho
* Date:		2014-03-08
* Brief:	CURLEx
**********************************************************************/
#ifndef _CURL_EX_H_
#define _CURL_EX_H_

#include <string.h>
#include <string>
#include <vector>
#include <map>
#include "curl/curl.h"

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
    CURLEx(const std::string& sslCaFilename = "");
    ~CURLEx(void);

public:
    template <class T>
    CURLcode setOption(CURLoption option, T data) {
        if (!mCurl) {
            return CURLE_FAILED_INIT;
        }
        return curl_easy_setopt(mCurl, option, data);
    }

public:
    /* set cookie file */
    bool setCookieFile(const std::string& cookieFilename = "");

    /* set timeout for connect */
    bool setConnectTimeout(int connectTimeout = 30);

    /* set timeout for read */
    bool setTimeout(int timeout = 60);

    /* set full url for request */
    bool setURL(const std::string& url);

    /* set headers */
    bool setHeaders(const std::map<std::string, std::string>& headers);

    /* set post fields and field size */
    bool setPostFields(const unsigned char* fields, unsigned int fieldsize);

    /* set header function */
    bool setHeaderFunction(CURLEx_callback func, void* userdata);

    /* set write function */
    bool setWriteFunction(CURLEx_callback func, void* userdata);

    /* set read function */
    bool setReadFunction(CURLEx_callback func, void* userdata);

    /* set process function */
    bool setProgressFunction(CURLEx_progress func, void* userdata);

    /*--------------------- multipart/formdata block ---------------------*/
    bool addFormContent(const char* name, const unsigned char* content, unsigned int length = 0, const char* type = NULL);

    bool addFormFile(const char* name, const char* filename, const char* type = NULL);
    /*--------------------------------------------------------------------*/

    /* called at last */
    bool perform(int* curlCode = NULL, int* responseCode = NULL, std::string* errorBuffer = NULL);

private:
    /* init CURL options */
    bool initialize(const std::string& sslCaFilename = "");

private:
    static unsigned int sObjCount;			/* object count in program */
    CURL* mCurl;							/* instance of curl */
    struct curl_slist* mHeaders;			/* keeps custom header data */
    struct curl_httppost* mHttpPost;		/* needed when multipart/formdata request */
    struct curl_httppost* mLastPost;		/* needed when multipart/formdata request */
    char mErrorBuffer[CURL_ERROR_SIZE];		/* error buffer */
};

/*
* struct of curl request
*/
class CurlRequest {
public:
    class Form {
    public:
        Form(void) : value(NULL), length(0) {}
        ~Form(void) {
            if (value) {
                delete []value;
                value = NULL;
            }
        }
        bool setValue(const unsigned char* value, unsigned int length, bool isText) {
            if (this->value) {
                free(this->value);
                this->value = NULL;
            }
            this->length = 0;
            if (0 == length && !isText) {
                return false;
            }
            if (value) {
                this->value = (unsigned char*)malloc(sizeof(unsigned char) * (isText ? length + 1 : length));
                memset(this->value, 0, length);
                memcpy(this->value, value, length);
                if (isText) {
                    *(this->value + length) = '\0';
                }
                this->length = length;
            }
            return true;
        }
    public:
        std::string name;
        CURLformoption option;
        unsigned char* value;
        unsigned int length;
        std::string type;           /* e.g. "text/html","image/jpeg","image/png" */
    };

public:
    CurlRequest(void) : mData(NULL), mDataSize(0) {
        clear();
    }

    ~CurlRequest(void) {
        clear();
    }

    void clear(void) {
        sslcafilename = "";
        cookiefilename = "";
        connecttimeout = 30;
        timeout = 60;
        url = "";
        headers.clear();
        if (mData) {
            delete []mData;
            mData = NULL;
        }
        mDataSize = 0;
        std::map<std::string, Form*>::iterator iter = mForms.begin();
        for (; mForms.end() != iter; ++iter) {
            delete iter->second;
        }
        mForms.clear();
    }

    bool setData(const unsigned char* data, unsigned int length, bool isText) {
        if (mData) {
            free(mData);
            mData = NULL;
        }
        mDataSize = 0;
        if (0 == length && !isText) {
            return false;
        }
        if (data) {
            mData = (unsigned char*)malloc(sizeof(unsigned char) * (isText ? length + 1 : length));
            memset(mData, 0, length);
            memcpy(mData, data, length);
            *(mData + length) = '\0';
            mDataSize = length;
        }
        return true;
    }

    const unsigned char* getData(void) {
        return mData;
    }

    unsigned int getDataSize(void) {
        return mDataSize;
    }

    bool addBuffer(const std::string& name, const unsigned char* content, unsigned int length, const std::string& type = "") {
        if (name.empty() || !content || 0 == length) {
            return false;
        }
        Form* f = new Form();
        f->name = name;
        f->option = CURLFORM_COPYCONTENTS;
        f->setValue(content, length, false);
        f->type = type;
        mForms[name] = f;
        return true;
    }

    bool addText(const std::string& name, const std::string& text, const std::string& type = "") {
        if (name.empty()) {
            return false;
        }
        Form* f = new Form();
        f->name = name;
        f->option = CURLFORM_COPYCONTENTS;
        f->setValue((const unsigned char*)text.c_str(), text.length(), true);
        f->type = type;
        mForms[name] = f;
        return true;
    }

    bool addFile(const std::string& name, const std::string& filename, const std::string& type = "") {
        if (name.empty() || filename.empty()) {
            return false;
        }
        FILE* fp = fopen(filename.c_str(), "r");
        if (!fp) {
            return false;
        }
        fclose(fp);
        Form* f = new Form();
        f->name = name;
        f->option = CURLFORM_FILE;
        f->setValue((const unsigned char*)filename.c_str(), filename.length(), true);
        f->type = type;
        mForms[name] = f;
        return true;
    }

    const std::map<std::string, Form*>& getForms(void) {
        return mForms;
    }

public:
    std::string sslcafilename;                      /* ssl CA filename */
    std::string cookiefilename;                     /* cookie filename */
    int connecttimeout;                             /* connect timeout */
    int timeout;                                    /* read timeout */
    std::string url;                                /* request url */
    std::map<std::string, std::string> headers;     /* headers */

private:
    unsigned char* mData;                           /* request data, support binary data */
    unsigned int mDataSize;                         /* request data size */
    std::map<std::string, Form*> mForms;            /* request form */
};

/*
* curl request interface
*/
bool curlGet(CurlRequest& request, CURLEx_callback headerFunc, void* headerStream, CURLEx_callback bodyFunc, void* bodyStream, int* curlCode = NULL, int* responseCode = NULL, std::string* errorBuffer = NULL);

bool curlPost(CurlRequest& request, CURLEx_callback headerFunc, void* headerStream, CURLEx_callback bodyFunc, void* bodyStream, int* curlCode = NULL, int* responseCode = NULL, std::string* errorBuffer = NULL);

bool curlPostForm(CurlRequest& request, CURLEx_callback headerFunc, void* headerStream, CURLEx_callback bodyFunc, void* bodyStream, int* curlCode = NULL, int* responseCode = NULL, std::string* errorBuffer = NULL);

bool curlPut(CurlRequest& request, CURLEx_callback headerFunc, void* headerStream, CURLEx_callback bodyFunc, void* bodyStream, int* curlCode = NULL, int* responseCode = NULL, std::string* errorBuffer = NULL);

bool curlDelete(CurlRequest& request, CURLEx_callback headerFunc, void* headerStream, CURLEx_callback bodyFunc, void* bodyStream, int* curlCode = NULL, int* responseCode = NULL, std::string* errorBuffer = NULL);

#endif	/* _CURL_EX_H_ */

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
    if (!fp) {
		return "";
	}
	CURLEx curl;
	if (false == curl.initialize()) {
		return "";
	}
    int curlCode = -1;
	int responseCode = -1;
	std::string errorBuffer = "";
	std::string responseStr = "";
	curl.setURL(uploadUrl);
	curl.addFormContent("acct", account);
	curl.addFormFile("image", localPath);
	curl.setWriteFunction(uploadWriteFunc, &responseStr);
    bool ret = curl.perform(&curlCode, &responseCode, &errorBuffer);
    if (!ret || 200 != responseCode) {
		return "";
	}
	Json* root= Json_create(responseStr.c_str());
    if (!root) {
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
    if (!fp) {                       // 创建保存路劲失败
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
    bool ret = curlObj.perform(curlCode, responseCode, buffer);
	fclose(fp);
    if (!ret || 200 != *responseCode) {
        return 4;   // 文件下载失败
    }
    return 0;   // 文件下载成功
}
*/
