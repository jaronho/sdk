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
	mHeaders = NULL;
    mHttpPost = NULL;
    mLastPost = NULL;
}
//------------------------------------------------------------------------
CURLEx::~CURLEx(void) {
	if (mCurl) {
		curl_easy_cleanup(mCurl);
		mCurl = NULL;
	}
    if (mHeaders) {
        curl_slist_free_all(mHeaders);
        mHeaders = NULL;
    }
    if (mHttpPost) {
        curl_formfree(mHttpPost);
        mHttpPost = NULL;
    }
    mLastPost = NULL;
	--sObjCount;
	if (0 == sObjCount) {
		curl_global_cleanup();
	}
}
//------------------------------------------------------------------------
bool CURLEx::initialize(const std::string& sslCaFileName /*= ""*/) {
    memset(mErrorBuffer, 0, CURL_ERROR_SIZE);
    if (CURLE_OK != setOption(CURLOPT_ERRORBUFFER, mErrorBuffer)) {
		return false;
	}
    if (sslCaFileName.empty()) {
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
        if (CURLE_OK != setOption(CURLOPT_CAINFO, sslCaFileName.c_str())) {
			return false;
		}
    }
    return true;//CURLE_OK == setOption(CURLOPT_NOSIGNAL, 1L);
}
//------------------------------------------------------------------------
bool CURLEx::setCookieFile(const std::string& cookieFile /*= ""*/) {
    if (!cookieFile.empty()) {
        if (CURLE_OK != setOption(CURLOPT_COOKIEFILE, cookieFile.c_str())) {
			return false;
        }
        if (CURLE_OK != setOption(CURLOPT_COOKIEJAR, cookieFile.c_str())) {
			return false;
		}
	}
	return true;
}
//------------------------------------------------------------------------
bool CURLEx::setConnectTimeout(int connectTimeout /*= 30*/) {
    return CURLE_OK == setOption(CURLOPT_CONNECTTIMEOUT, connectTimeout);
}
//------------------------------------------------------------------------
bool CURLEx::setTimeout(int timeout /*= 60*/) {
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
bool CURLEx::setHeaders(const std::vector<std::string>& headers) {
	if (headers.empty()) {
		return false;
	}
	// append custom headers one by one
	std::vector<std::string>::const_iterator iter = headers.begin();
    for (; headers.end() != iter; ++iter) {
		// the second parameter must use type: const char*
		mHeaders = curl_slist_append(mHeaders, iter->c_str());
	}
    // set custom headers for curl
    return CURLE_OK == setOption(CURLOPT_HTTPHEADER, mHeaders);
}
//------------------------------------------------------------------------
bool CURLEx::setPostFields(const char* fields, unsigned int fieldsize) {
	if (NULL == fields || 0 == fieldsize) {
		return false;
    }
    if (CURLE_OK != setOption(CURLOPT_POSTFIELDS, fields)) {
		return false;
    }
    return CURLE_OK == setOption(CURLOPT_POSTFIELDSIZE, fieldsize);
}
//------------------------------------------------------------------------
bool CURLEx::setHeaderFunction(CURLEx_callback func, void* userdata) {
	if (NULL == func || NULL == userdata) {
		return false;
    }
    if (CURLE_OK != setOption(CURLOPT_HEADERFUNCTION, func)) {
		return false;
    }
    return CURLE_OK == setOption(CURLOPT_HEADERDATA, userdata);
}
//------------------------------------------------------------------------
bool CURLEx::setWriteFunction(CURLEx_callback func, void* userdata) {
	if (NULL == func || NULL == userdata) {
		return false;
    }
    if (CURLE_OK != setOption(CURLOPT_WRITEFUNCTION, func)) {
		return false;
    }
    return CURLE_OK == setOption(CURLOPT_WRITEDATA, userdata);
}
//------------------------------------------------------------------------
bool CURLEx::setReadFunction(CURLEx_callback func, void* userdata) {
	if (NULL == func || NULL == userdata) {
		return false;
    }
    if (CURLE_OK != setOption(CURLOPT_READFUNCTION, func)) {
		return false;
    }
    return CURLE_OK == setOption(CURLOPT_READDATA, userdata);
}
//------------------------------------------------------------------------
bool CURLEx::setProgressFunction(CURLEx_progress func, void* userdata) {
	if (NULL == func || NULL == userdata) {
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
bool CURLEx::addForm(const char* name, CURLformoption option, const char* value, const char* type /*= NULL*/) {
    if (NULL == mCurl) {
        return false;
    }
    if (NULL == name || 0 == strlen(name) || NULL == value) {
		return false;
    }
    if (NULL == type || 0 == strlen(type)) {
        return CURL_FORMADD_OK == curl_formadd(&mHttpPost, &mLastPost, CURLFORM_COPYNAME, name, option, value, CURLFORM_END);
    }
    return CURL_FORMADD_OK == curl_formadd(&mHttpPost, &mLastPost, CURLFORM_COPYNAME, name, option, value, CURLFORM_CONTENTTYPE, type, CURLFORM_END);
}
//------------------------------------------------------------------------
bool CURLEx::addFormContent(const std::string& name, const std::string& content, const std::string& type /*= ""*/) {
    return addForm(name.c_str(), CURLFORM_COPYCONTENTS, content.c_str(), type.c_str());
}
//------------------------------------------------------------------------
bool CURLEx::addFormFile(const std::string& name, const std::string& file, const std::string& type /*= ""*/) {
    return addForm(name.c_str(), CURLFORM_FILE, file.c_str(), type.c_str());
}
//------------------------------------------------------------------------
bool CURLEx::perform(int* curlCode, int* responseCode, std::string* errorBuffer) {
    if (NULL == mCurl) {
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
	if (NULL != curlCode) {
		*curlCode = (int)code;
	}
	if (NULL != errorBuffer) {
		*errorBuffer = mErrorBuffer;
	}
    if (NULL != responseCode) {
		CURLcode code = curl_easy_getinfo(mCurl, CURLINFO_RESPONSE_CODE, responseCode);
        if (CURLE_OK != code || 200 != *responseCode) {
            if (mHttpPost) {
                curl_formfree(mHttpPost);
                mHttpPost = NULL;
            }
            mLastPost = NULL;
			return false;
        }
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
    if (NULL == pCurl || request.url.empty() || NULL == headerFunc || NULL == headerStream || NULL == bodyFunc || NULL == bodyStream) {
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
bool curlPostForm(CurlRequestForm& requestForm, CURLEx_callback headerFunc, void* headerStream, CURLEx_callback bodyFunc, void* bodyStream, int* curlCode, int* responseCode, std::string* errorBuffer) {
    CURLEx curlObj;
    if (!curlReuqestConfigure(&curlObj, requestForm, headerFunc, headerStream, bodyFunc, bodyStream)) {
        return false;
    }
    if (CURLE_OK != curlObj.setOption(CURLOPT_FOLLOWLOCATION, 1)) {
        return false;
    }
    requestForm.transform(&curlObj);
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
