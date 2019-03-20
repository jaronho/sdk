/**********************************************************************
* Author:	jaron.ho
* Date:		2014-03-08
* Brief:	CURLEx
**********************************************************************/
#include "CURLEx.h"

/*
* interface implemented of CURLEx
*/
unsigned int CURLEx::sObjCount = 0;
//------------------------------------------------------------------------
CURLEx::CURLEx(void) {
    if (0 == sObjCount) {
        curl_global_init(CURL_GLOBAL_ALL);
    }
    ++sObjCount;
    memset(mErrorBuffer, 0, CURL_ERROR_SIZE);
    mCurl = curl_easy_init();
    curl_easy_setopt(mCurl, CURLOPT_FORBID_REUSE, 1L);
    mHeaders = NULL;
    mHttpPost = NULL;
    mLastPost = NULL;
}
//------------------------------------------------------------------------
CURLEx::~CURLEx(void) {
    if (mHttpPost) {
        curl_formfree(mHttpPost);
        mHttpPost = NULL;
    }
    mLastPost = NULL;
    if (mHeaders) {
        curl_slist_free_all(mHeaders);
        mHeaders = NULL;
    }
    if (mCurl) {
        curl_easy_cleanup(mCurl);
        mCurl = NULL;
    }
    --sObjCount;
    if (0 == sObjCount) {
        curl_global_cleanup();
    }
}
//------------------------------------------------------------------------
bool CURLEx::initialize(const std::string& sslCaFilename) {
    memset(mErrorBuffer, 0, CURL_ERROR_SIZE);
    if (CURLE_OK != setOption(CURLOPT_ERRORBUFFER, mErrorBuffer)) {
        return false;
    }
    if (sslCaFilename.empty()) {
        if (CURLE_OK != setOption(CURLOPT_SSL_VERIFYPEER, 0L)) {
            return false;
        }
        if (CURLE_OK != setOption(CURLOPT_SSL_VERIFYHOST, 0L)) {
            return false;
        }
    } else {
        if (CURLE_OK != setOption(CURLOPT_SSL_VERIFYPEER, 1L)) {
            return false;
        }
        if (CURLE_OK != setOption(CURLOPT_SSL_VERIFYHOST, 2L)) {
            return false;
        }
        if (CURLE_OK != setOption(CURLOPT_CAINFO, sslCaFilename.c_str())) {
            return false;
        }
    }
    return CURLE_OK == setOption(CURLOPT_NOSIGNAL, 1L);
}
//------------------------------------------------------------------------
bool CURLEx::setCookieFile(const std::string& cookieFilename) {
    if (!cookieFilename.empty()) {
        if (CURLE_OK != setOption(CURLOPT_COOKIEFILE, cookieFilename.c_str())) {
            return false;
        }
        if (CURLE_OK != setOption(CURLOPT_COOKIEJAR, cookieFilename.c_str())) {
            return false;
        }
    }
    return true;
}
//------------------------------------------------------------------------
bool CURLEx::setConnectTimeout(int connectTimeout) {
    return CURLE_OK == setOption(CURLOPT_CONNECTTIMEOUT, connectTimeout);
}
//------------------------------------------------------------------------
bool CURLEx::setTimeout(int timeout) {
    return CURLE_OK == setOption(CURLOPT_TIMEOUT, timeout);
}
//------------------------------------------------------------------------
bool CURLEx::setURL(const std::string& url) {
    if (url.empty()) {
        return false;
    }
    // the second parameter must use type: const char*
    return CURLE_OK == setOption(CURLOPT_URL, url.c_str());
}
//------------------------------------------------------------------------
bool CURLEx::setHeaders(const std::map<std::string, std::string>& headers) {
    if (headers.empty()) {
        return false;
    }
    // append custom headers one by one
    std::map<std::string, std::string>::const_iterator iter = headers.begin();
    for (; headers.end() != iter; ++iter) {
        // the second parameter must use type: const char*
        mHeaders = curl_slist_append(mHeaders, (iter->first + ":" + iter->second).c_str());
    }
    // set custom headers for curl
    return CURLE_OK == setOption(CURLOPT_HTTPHEADER, mHeaders);
}
//------------------------------------------------------------------------
bool CURLEx::setPostFields(const unsigned char* fields, unsigned int fieldsize) {
    if (CURLE_OK != setOption(CURLOPT_POSTFIELDS, fields)) {
        return false;
    }
    return CURLE_OK == setOption(CURLOPT_POSTFIELDSIZE, fieldsize);
}
//------------------------------------------------------------------------
bool CURLEx::setHeaderFunction(CURLEx_callback func, void* userdata) {
    if (!func || !userdata) {
        return false;
    }
    if (CURLE_OK != setOption(CURLOPT_HEADERFUNCTION, func)) {
        return false;
    }
    return CURLE_OK == setOption(CURLOPT_HEADERDATA, userdata);
}
//------------------------------------------------------------------------
bool CURLEx::setWriteFunction(CURLEx_callback func, void* userdata) {
    if (!func || !userdata) {
        return false;
    }
    if (CURLE_OK != setOption(CURLOPT_WRITEFUNCTION, func)) {
        return false;
    }
    return CURLE_OK == setOption(CURLOPT_WRITEDATA, userdata);
}
//------------------------------------------------------------------------
bool CURLEx::setReadFunction(CURLEx_callback func, void* userdata) {
    if (!func || !userdata) {
        return false;
    }
    if (CURLE_OK != setOption(CURLOPT_READFUNCTION, func)) {
        return false;
    }
    return CURLE_OK == setOption(CURLOPT_READDATA, userdata);
}
//------------------------------------------------------------------------
bool CURLEx::setProgressFunction(CURLEx_progress func, void* userdata) {
    if (!func || !userdata) {
        return false;
    }
    if (CURLE_OK != setOption(CURLOPT_NOPROGRESS, false)) {
        return false;
    }
    if (CURLE_OK != setOption(CURLOPT_PROGRESSFUNCTION, func)) {
        return false;
    }
    return CURLE_OK == setOption(CURLOPT_PROGRESSDATA, userdata);
}
//------------------------------------------------------------------------
bool CURLEx::addFormContent(const char* name, const unsigned char* content, unsigned int length, const char* type) {
    if (!mCurl) {
        return false;
    }
    if (!name || 0 == strlen(name) || !content) {
        return false;
    }
    if (!type || 0 == strlen(type)) {
        return CURL_FORMADD_OK == curl_formadd(&mHttpPost, &mLastPost, CURLFORM_COPYNAME, name, CURLFORM_COPYCONTENTS, content, CURLFORM_CONTENTSLENGTH, length, CURLFORM_END);
    }
    return CURL_FORMADD_OK == curl_formadd(&mHttpPost, &mLastPost, CURLFORM_COPYNAME, name, CURLFORM_COPYCONTENTS, content, CURLFORM_CONTENTSLENGTH, length, CURLFORM_CONTENTTYPE, type, CURLFORM_END);
}
//------------------------------------------------------------------------
bool CURLEx::addFormFile(const char* name, const char* filename, const char* type) {
    if (!mCurl) {
        return false;
    }
    if (!name || 0 == strlen(name) || !filename || 0 == strlen(filename)) {
        return false;
    }
    if (!type || 0 == strlen(type)) {
        return CURL_FORMADD_OK == curl_formadd(&mHttpPost, &mLastPost, CURLFORM_COPYNAME, name, CURLFORM_FILE, filename, CURLFORM_END);
    }
    return CURL_FORMADD_OK == curl_formadd(&mHttpPost, &mLastPost, CURLFORM_COPYNAME, name, CURLFORM_FILE, filename, CURLFORM_CONTENTTYPE, type, CURLFORM_END);
}
//------------------------------------------------------------------------
bool CURLEx::perform(int* curlCode, int* responseCode, std::string* errorBuffer) {
    if (!mCurl) {
        return false;
    }
    if (mHttpPost) {
        if (CURLE_OK != setOption(CURLOPT_HTTPPOST, mHttpPost)) {
            curl_formfree(mHttpPost);
            mHttpPost = NULL;
            mLastPost = NULL;
            return false;
        }
    }
    CURLcode code = curl_easy_perform(mCurl);
    if (curlCode) {
        *curlCode = (int)code;
    }
    if (errorBuffer) {
        *errorBuffer = mErrorBuffer;
    }
    if (responseCode) {
        curl_easy_getinfo(mCurl, CURLINFO_RESPONSE_CODE, responseCode);
    }
    if (mHttpPost) {
        curl_formfree(mHttpPost);
        mHttpPost = NULL;
    }
    mLastPost = NULL;
    return CURLE_OK == code;
}
//------------------------------------------------------------------------
bool curlReuqestConfigure(CURLEx* pCurl, const CurlRequest& request, CURLEx_callback headerFunc, void* headerStream, CURLEx_callback bodyFunc, void* bodyStream) {
    if (!pCurl || request.url.empty() || !headerFunc || !headerStream || !bodyFunc || !bodyStream) {
        return false;
    }
    if (!pCurl->initialize(request.sslcafilename)) {
        return false;
    }
    if (!pCurl->setCookieFile(request.cookiefilename)) {
        return false;
    }
    pCurl->setOption(CURLOPT_CONNECTTIMEOUT, request.connecttimeout);
    pCurl->setOption(CURLOPT_TIMEOUT, request.timeout);
    if (!pCurl->setURL(request.url)) {
        return false;
    }
    pCurl->setHeaders(request.headers);
    if (!pCurl->setHeaderFunction(headerFunc, headerStream)) {
        return false;
    }
    if (!pCurl->setWriteFunction(bodyFunc, bodyStream)) {
        return false;
    }
    return true;
}
//------------------------------------------------------------------------
bool curlGet(CurlRequest& request, CURLEx_callback headerFunc, void* headerStream, CURLEx_callback bodyFunc, void* bodyStream, int* curlCode, int* responseCode, std::string* errorBuffer) {
    CURLEx curlObj;
    if (!curlReuqestConfigure(&curlObj, request, headerFunc, headerStream, bodyFunc, bodyStream)) {
        return false;
    }
    if (CURLE_OK != curlObj.setOption(CURLOPT_FOLLOWLOCATION, 1)) {
        return false;
    }
    if (CURLE_OK != curlObj.setOption(CURLOPT_POST, 0)) {
        return false;
    }
    return curlObj.perform(curlCode, responseCode, errorBuffer);
}
//------------------------------------------------------------------------
bool curlPost(CurlRequest& request, CURLEx_callback headerFunc, void* headerStream, CURLEx_callback bodyFunc, void* bodyStream, int* curlCode, int* responseCode, std::string* errorBuffer) {
    CURLEx curlObj;
    if (!curlReuqestConfigure(&curlObj, request, headerFunc, headerStream, bodyFunc, bodyStream)) {
        return false;
    }
    if (CURLE_OK != curlObj.setOption(CURLOPT_FOLLOWLOCATION, 1)) {
        return false;
    }
    if (CURLE_OK != curlObj.setOption(CURLOPT_POST, 1)) {
        return false;
    }
    curlObj.setPostFields(request.getData(), request.getDataSize());
    return curlObj.perform(curlCode, responseCode, errorBuffer);
}
//------------------------------------------------------------------------
bool curlPostForm(CurlRequest& request, CURLEx_callback headerFunc, void* headerStream, CURLEx_callback bodyFunc, void* bodyStream, int* curlCode, int* responseCode, std::string* errorBuffer) {
    CURLEx curlObj;
    if (!curlReuqestConfigure(&curlObj, request, headerFunc, headerStream, bodyFunc, bodyStream)) {
        return false;
    }
    if (CURLE_OK != curlObj.setOption(CURLOPT_FOLLOWLOCATION, 1)) {
        return false;
    }
    if (CURLE_OK != curlObj.setOption(CURLOPT_POST, 1)) {
        return false;
    }
    if (request.getForms().empty()) {
        curlObj.setPostFields(request.getData(), request.getDataSize());
    } else {
        std::map<std::string, CurlRequest::Form*>::const_iterator formIter = request.getForms().begin();
        for (; request.getForms().end() != formIter; ++formIter) {
            const CurlRequest::Form* f = formIter->second;
            if (CURLFORM_COPYCONTENTS == f->option) {
                if (!curlObj.addFormContent(f->name.c_str(), f->value, f->length, f->type.c_str())) {
                    return false;
                }
            } else if (CURLFORM_FILE == f->option) {
                if (!curlObj.addFormFile(f->name.c_str(), (const char*)f->value, f->type.c_str())) {
                    return false;
                }
            }
        }
    }
    return curlObj.perform(curlCode, responseCode, errorBuffer);
}
//------------------------------------------------------------------------
bool curlPut(CurlRequest& request, CURLEx_callback headerFunc, void* headerStream, CURLEx_callback bodyFunc, void* bodyStream, int* curlCode, int* responseCode, std::string* errorBuffer) {
    CURLEx curlObj;
    if (!curlReuqestConfigure(&curlObj, request, headerFunc, headerStream, bodyFunc, bodyStream)) {
        return false;
    }
    if (CURLE_OK != curlObj.setOption(CURLOPT_FOLLOWLOCATION, 1)) {
        return false;
    }
    if (CURLE_OK != curlObj.setOption(CURLOPT_CUSTOMREQUEST, "PUT")) {
        return false;
    }
    curlObj.setPostFields(request.getData(), request.getDataSize());
    return curlObj.perform(curlCode, responseCode, errorBuffer);
}
//------------------------------------------------------------------------
bool curlDelete(CurlRequest& request, CURLEx_callback headerFunc, void* headerStream, CURLEx_callback bodyFunc, void* bodyStream, int* curlCode, int* responseCode, std::string* errorBuffer) {
    CURLEx curlObj;
    if (!curlReuqestConfigure(&curlObj, request, headerFunc, headerStream, bodyFunc, bodyStream)) {
        return false;
    }
    if (CURLE_OK != curlObj.setOption(CURLOPT_FOLLOWLOCATION, 1)) {
        return false;
    }
    if (CURLE_OK != curlObj.setOption(CURLOPT_CUSTOMREQUEST, "DELETE")) {
        return false;
    }
    curlObj.setPostFields(request.getData(), request.getDataSize());
    return curlObj.perform(curlCode, responseCode, errorBuffer);
}
//------------------------------------------------------------------------
