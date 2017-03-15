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
	CURLcode code;
	memset(mErrorBuffer, 0, CURL_ERROR_SIZE);
	code = setOption(CURLOPT_ERRORBUFFER, mErrorBuffer);
	if (CURLE_OK != code) {
		return false;
	}
	if (sslCaFileName.empty()) {
		code = setOption(CURLOPT_SSL_VERIFYPEER, 0L);
		if (CURLE_OK != code) {
			return false;
		}
		code = setOption(CURLOPT_SSL_VERIFYHOST, 0L);
		if (CURLE_OK != code) {
			return false;
		}
	} else {
		code = setOption(CURLOPT_SSL_VERIFYPEER, 1L);
		if (CURLE_OK != code) {
			return false;
		}
		code = setOption(CURLOPT_SSL_VERIFYHOST, 2L);
		if (CURLE_OK != code) {
			return false;
		}
		code = setOption(CURLOPT_CAINFO, sslCaFileName.c_str());
		if (CURLE_OK != code) {
			return false;
		}
	}
	code = setOption(CURLOPT_NOSIGNAL, 1L);
	return CURLE_OK == code;
}
//------------------------------------------------------------------------
bool CURLEx::setCookieFile(const std::string& cookieFile /*= ""*/) {
	if (!cookieFile.empty()) {
		CURLcode code = setOption(CURLOPT_COOKIEFILE, cookieFile.c_str());
		if (CURLE_OK != code) {
			return false;
		}
		code = setOption(CURLOPT_COOKIEJAR, cookieFile.c_str());
		if (CURLE_OK != code) {
			return false;
		}
	}
	return true;
}
//------------------------------------------------------------------------
bool CURLEx::setConnectTimeout(int timeout) {
	CURLcode code = setOption(CURLOPT_CONNECTTIMEOUT, timeout);
	return CURLE_OK == code;
}
//------------------------------------------------------------------------
bool CURLEx::setTimeout(int timeout) {
	CURLcode code = setOption(CURLOPT_TIMEOUT, timeout);
	return CURLE_OK == code;
}
//------------------------------------------------------------------------
bool CURLEx::setURL(const std::string& url) {
	if (url.empty()) {
		return false;
	}
	// the second parameter must use type: const char*
	CURLcode code = setOption(CURLOPT_URL, url.c_str());
	return CURLE_OK == code;
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
	CURLcode code = setOption(CURLOPT_HTTPHEADER, mHeaders);
	return CURLE_OK == code;
}
//------------------------------------------------------------------------
bool CURLEx::setPostFields(const char* fields, unsigned int fieldsize) {
	if (NULL == fields || 0 == fieldsize) {
		return false;
	}
	CURLcode code;
	code = setOption(CURLOPT_POSTFIELDS, fields);
	if (CURLE_OK != code) {
		return false;
	}
	code = setOption(CURLOPT_POSTFIELDSIZE, fieldsize);
	return CURLE_OK == code;
}
//------------------------------------------------------------------------
bool CURLEx::setHttpPost(void) {
	if (NULL == mPost) {
		return false;
	}
	CURLcode code = setOption(CURLOPT_HTTPPOST, mPost);
	return CURLE_OK == code;
}
//------------------------------------------------------------------------
bool CURLEx::setHeadFunction(CURLEx_callback func, void* userdata) {
	if (NULL == func || NULL == userdata) {
		return false;
	}
	CURLcode code;
	code = setOption(CURLOPT_HEADERFUNCTION, func);
	if (CURLE_OK != code) {
		return false;
	}
	code = setOption(CURLOPT_HEADERDATA, userdata);
	return CURLE_OK == code;
}
//------------------------------------------------------------------------
bool CURLEx::setWriteFunction(CURLEx_callback func, void* userdata) {
	if (NULL == func || NULL == userdata) {
		return false;
	}
	CURLcode code;
	code = setOption(CURLOPT_WRITEFUNCTION, func);
	if (CURLE_OK != code) {
		return false;
	}
	code = setOption(CURLOPT_WRITEDATA, userdata);
	return CURLE_OK == code;
}
//------------------------------------------------------------------------
bool CURLEx::setReadFunctioin(CURLEx_callback func, void* userdata) {
	if (NULL == func || NULL == userdata) {
		return false;
	}
	CURLcode code;
	code = setOption(CURLOPT_READFUNCTION, func);
	if (CURLE_OK != code) {
		return false;
	}
	code = setOption(CURLOPT_READDATA, userdata);
	return CURLE_OK == code;
}
//------------------------------------------------------------------------
bool CURLEx::setProgressFunction(CURLEx_progress func, void* userdata) {
	if (NULL == func || NULL == userdata) {
		return false;
	}
	CURLcode code;
	code = setOption(CURLOPT_NOPROGRESS, false);
	if (CURLE_OK != code) {
		return false;
	}
	code = setOption(CURLOPT_PROGRESSFUNCTION, func);
	if (CURLE_OK != code) {
		return false;
	}
	code = setOption(CURLOPT_PROGRESSDATA, userdata);
	return CURLE_OK == code;
}
//------------------------------------------------------------------------
bool CURLEx::addForm(curl_forms forms[], unsigned int length) {
	// the final element in the array must be a CURLFORM_END
	if (CURLFORM_END != forms[length-1].option) {
		return false;
	}
	CURLFORMcode code = curl_formadd(&mPost, NULL, CURLFORM_ARRAY, forms, CURLFORM_END);
	return CURL_FORMADD_OK == code;
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
bool CURLEx::perform(int* curlCode, int *responseCode, std::string* errorBuffer) {
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
bool curlReuqestConfigure(CURLEx* pCurl, CurlRequest& request, CURLEx_callback writeFunc, void* writeStream, CURLEx_callback headerCallback, void* headerStream) {
	if (NULL == pCurl || request.url.empty() || NULL == writeFunc || NULL == writeStream || NULL == headerCallback || NULL == headerStream) {
		return false;
	}
	if (!pCurl->initialize(request.sslcafilename)) {
		return false;
	}
	if (!pCurl->setCookieFile(request.cookiefilename)) {
		return false;
	}
	if (!pCurl->setURL(request.url)) {
		return false;
	}
	pCurl->setHeaders(request.headers);
	pCurl->setOption(CURLOPT_CONNECTTIMEOUT, request.connecttimeout);
	pCurl->setOption(CURLOPT_TIMEOUT, request.timeout);
	if (!pCurl->setWriteFunction(writeFunc, writeStream)) {
		return false;
	}
	if (!pCurl->setHeadFunction(headerCallback, headerStream)) {
		return false;
	}
	return true;
}
//------------------------------------------------------------------------
bool curlGet(CurlRequest& request, CURLEx_callback writeFunc, void* writeStream, CURLEx_callback headerCallback, void* headerStream, int* curlCode, int* responseCode, std::string* errorBuffer) {
	CURLEx curlObj;
	if (!curlReuqestConfigure(&curlObj, request, writeFunc, writeStream, headerCallback, headerStream)) {
		return false;
	}
	if (!curlObj.setOption(CURLOPT_FOLLOWLOCATION, true)) {
		return false;
	}
	return curlObj.perform(curlCode, responseCode, errorBuffer);
}
//------------------------------------------------------------------------
bool curlPost(CurlRequest& request, CURLEx_callback writeFunc, void* writeStream, CURLEx_callback headerCallback, void* headerStream, int* curlCode, int* responseCode, std::string* errorBuffer) {
	CURLEx curlObj;
	if (!curlReuqestConfigure(&curlObj, request, writeFunc, writeStream, headerCallback, headerStream)) {
		return false;
	}
	if (!curlObj.setOption(CURLOPT_POST, 1)) {
		return false;
	}
	if (!curlObj.setPostFields(request.getData(), request.getDataSize())) {
		return false;
	}
	return curlObj.perform(curlCode, responseCode, errorBuffer);
}
//------------------------------------------------------------------------
bool curlPut(CurlRequest& request, CURLEx_callback writeFunc, void* writeStream, CURLEx_callback headerCallback, void* headerStream, int* curlCode, int* responseCode, std::string* errorBuffer) {
	CURLEx curlObj;
	if (!curlReuqestConfigure(&curlObj, request, writeFunc, writeStream, headerCallback, headerStream)) {
		return false;
	}
	if (!curlObj.setOption(CURLOPT_CUSTOMREQUEST, "PUT")) {
		return false;
	}
	if (!curlObj.setPostFields(request.getData(), request.getDataSize())) {
		return false;
	}
	return curlObj.perform(curlCode, responseCode, errorBuffer);
}
//------------------------------------------------------------------------
bool curlDelete(CurlRequest& request, CURLEx_callback writeFunc, void* writeStream, CURLEx_callback headerCallback, void* headerStream, int* curlCode, int* responseCode, std::string* errorBuffer) {
	CURLEx curlObj;
	if (!curlReuqestConfigure(&curlObj, request, writeFunc, writeStream, headerCallback, headerStream)) {
		return false;
	}
	if (!curlObj.setOption(CURLOPT_CUSTOMREQUEST, "DELETE")) {
		return false;
	}
	if (!curlObj.setOption(CURLOPT_FOLLOWLOCATION, true)) {
		return false;
	}
	return curlObj.perform(curlCode, responseCode, errorBuffer);
}
//------------------------------------------------------------------------