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
	mPost = NULL;
}
//------------------------------------------------------------------------
CURLEx::~CURLEx(void) {
	if (mCurl) {
		curl_easy_cleanup(mCurl);
		mCurl = NULL;
	}
	// free the linked list for header data
	if (mHeaders) {
		curl_slist_free_all(mHeaders);
		mHeaders = NULL;
	}
	if (mPost) {
		curl_formfree(mPost);
		mPost = NULL;
	}
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
    return CURLE_OK == setOption(CURLOPT_NOSIGNAL, 1L);
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
bool CURLEx::addForm(curl_forms forms[], unsigned int length) {
	// the final element in the array must be a CURLFORM_END
	if (CURLFORM_END != forms[length-1].option) {
		return false;
    }
    return CURL_FORMADD_OK == curl_formadd(&mPost, NULL, CURLFORM_ARRAY, forms, CURLFORM_END);
}
//------------------------------------------------------------------------
bool CURLEx::addForm(const std::string& name, CURLformoption option, const std::string& value, const std::string& type) {
	if (name.empty() || value.empty()) {
		return false;
	}
	if (type.empty()) {
		curl_forms formVec[3];
		formVec[0].option = CURLFORM_COPYNAME;
		formVec[0].value = name.c_str();
		formVec[1].option = option;
		formVec[1].value = value.c_str();
		formVec[2].option = CURLFORM_END;
		return addForm(formVec, 3);
	} else {
		curl_forms formVec[4];
		formVec[0].option = CURLFORM_COPYNAME;
		formVec[0].value = name.c_str();
		formVec[1].option = option;
		formVec[1].value = value.c_str();
		formVec[2].option = CURLFORM_CONTENTTYPE;
		formVec[2].value = type.c_str();
		formVec[3].option = CURLFORM_END;
		return addForm(formVec, 4);
	}
	return false;
}
//------------------------------------------------------------------------
bool CURLEx::addFormContent(const std::string& name, const std::string& content, const std::string& type) {
	return addForm(name, CURLFORM_COPYCONTENTS, content, type);
}
//------------------------------------------------------------------------
bool CURLEx::addFormFile(const std::string& name, const std::string& file, const std::string& type) {
	// multipart/formdata reuqest, needn't read file into memory
	return addForm(name, CURLFORM_FILE, file, type);
}
//------------------------------------------------------------------------
bool CURLEx::setHttpPost(void) {
    if (NULL == mPost) {
        return false;
    }
    return CURLE_OK == setOption(CURLOPT_HTTPPOST, mPost);
}
//------------------------------------------------------------------------
bool CURLEx::perform(int* curlCode, int* responseCode, std::string* errorBuffer) {
    if (NULL == mCurl) {
		return false;
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
			return false;
        }
    }
	return CURLE_OK == code;
}
//------------------------------------------------------------------------
bool curlReuqestConfigure(CURLEx* pCurl, CurlRequest& request, CURLEx_callback headerFunc, void* headerStream, CURLEx_callback responseFunc, void* responseStream) {
    if (NULL == pCurl || request.url.empty() || NULL == headerFunc || NULL == headerStream || NULL == responseFunc || NULL == responseStream) {
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
    if (!pCurl->setWriteFunction(responseFunc, responseStream)) {
		return false;
	}
	return true;
}
//------------------------------------------------------------------------
bool curlGet(CurlRequest& request, CURLEx_callback headerFunc, void* headerStream, CURLEx_callback responseFunc, void* responseStream, int* curlCode, int* responseCode, std::string* errorBuffer) {
	CURLEx curlObj;
    if (!curlReuqestConfigure(&curlObj, request, headerFunc, headerStream, responseFunc, responseStream)) {
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
bool curlPost(CurlRequest& request, CURLEx_callback headerFunc, void* headerStream, CURLEx_callback responseFunc, void* responseStream, int* curlCode, int* responseCode, std::string* errorBuffer) {
	CURLEx curlObj;
    if (!curlReuqestConfigure(&curlObj, request, headerFunc, headerStream, responseFunc, responseStream)) {
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
bool curlPut(CurlRequest& request, CURLEx_callback headerFunc, void* headerStream, CURLEx_callback responseFunc, void* responseStream, int* curlCode, int* responseCode, std::string* errorBuffer) {
	CURLEx curlObj;
    if (!curlReuqestConfigure(&curlObj, request, headerFunc, headerStream, responseFunc, responseStream)) {
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
bool curlDelete(CurlRequest& request, CURLEx_callback headerFunc, void* headerStream, CURLEx_callback responseFunc, void* responseStream, int* curlCode, int* responseCode, std::string* errorBuffer) {
	CURLEx curlObj;
    if (!curlReuqestConfigure(&curlObj, request, headerFunc, headerStream, responseFunc, responseStream)) {
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
