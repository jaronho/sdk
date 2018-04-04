/**********************************************************************
* Author:	jaron.ho
* Date:		2014-06-04
* Brief:	http 客户端
**********************************************************************/
#include "HttpClient.h"
//------------------------------------------------------------------------
static bool sIsRunning = false;								// 是否在运行
static std::list<HttpObject*>* sReuqestList = NULL;			// 请求列表
static std::list<HttpObject*>* sResponseList = NULL;		// 响应列表
static std::mutex sRequestListMutex;						// 请求列表互斥
static std::mutex sResponseListMutex;						// 响应列表互斥
static std::condition_variable_any sSleepCondition;			// 条件变量
//------------------------------------------------------------------------
static void createThreadSemphore(void) {
    if (sIsRunning || sReuqestList || sResponseList) {
        return;
    }
    sIsRunning = true;
    sReuqestList = new std::list<HttpObject*>();
    sResponseList = new std::list<HttpObject*>();
}
//------------------------------------------------------------------------
static void destroyThreadSemphore(void) {
    sIsRunning = false;
    if (NULL == sReuqestList || NULL == sResponseList) {
        return;
    }
    // 清除请求列表
    sRequestListMutex.lock();
    if (sReuqestList) {
        sReuqestList->clear();
        delete sReuqestList;
        sReuqestList = NULL;
    }
    sRequestListMutex.unlock();
    // 清除响应列表
    sResponseListMutex.lock();
    if (sResponseList) {
        sResponseList->clear();
        delete sResponseList;
        sResponseList = NULL;
    }
    sResponseListMutex.unlock();
}
//------------------------------------------------------------------------
static void sendRequest(HttpObject* obj) {
    if (NULL == obj) {
        return;
    }
    if (!sIsRunning) {
        delete obj;
        return;
    }
    sRequestListMutex.lock();
    sReuqestList->push_back(obj);
    sRequestListMutex.unlock();
    sSleepCondition.notify_one();
}
//------------------------------------------------------------------------
static void recvResponse(void) {
    if (!sIsRunning) {
        return;
    }
    HttpObject* responseObj = NULL;
    sResponseListMutex.lock();
    if (sResponseList && sResponseList->size() > 0) {
        responseObj = *(sResponseList->begin());
        sResponseList->pop_front();
    }
    sResponseListMutex.unlock();
    if (NULL == responseObj) {
        return;
    }
    if (responseObj->callback) {
        std::string responseheader(responseObj->responseheader.begin(), responseObj->responseheader.end());
        std::string responsebody(responseObj->responsebody.begin(), responseObj->responsebody.end());
        responseObj->callback(responseObj->success, responseObj->curlcode, responseObj->responsecode, responseObj->errorbuffer, responseheader, responsebody);
    }
    delete responseObj;
}
//------------------------------------------------------------------------
static unsigned int headerFunc(void* ptr, unsigned int size, unsigned int nmemb, void* stream) {
    std::vector<char>* recvBuffer = (std::vector<char>*)stream;
    unsigned int sizes = size * nmemb;
    recvBuffer->insert(recvBuffer->end(), (char*)ptr, (char*)ptr + sizes);
    return sizes;
}
//------------------------------------------------------------------------
static unsigned int bodyFunc(void* ptr, unsigned int size, unsigned int nmemb, void* stream) {
    std::vector<char>* recvBuffer = (std::vector<char>*)stream;
    unsigned int sizes = size * nmemb;
    recvBuffer->insert(recvBuffer->end(), (char*)ptr, (char*)ptr + sizes);
    return sizes;
}
//------------------------------------------------------------------------
static void* httpNetworkThread() {
    while (sIsRunning) {
        HttpObject* requestObj = NULL;
        sRequestListMutex.lock();
        if (sReuqestList && sReuqestList->size() > 0) {
            requestObj = *(sReuqestList->begin());
            sReuqestList->pop_front();
        }
        sRequestListMutex.unlock();
        if (NULL == requestObj) {
            std::lock_guard<std::mutex> lock(sRequestListMutex);
            sSleepCondition.wait(sRequestListMutex);
            continue;
        }
        std::transform(requestObj->requesttype.begin(), requestObj->requesttype.end(), requestObj->requesttype.begin(), ::toupper);
        if ("GET" == requestObj->requesttype) {
            requestObj->success = curlGet(requestObj->requestdata, headerFunc, &(requestObj->responseheader), bodyFunc, &(requestObj->responsebody), &(requestObj->curlcode), &(requestObj->responsecode), &(requestObj->errorbuffer));
        } else if ("POST" == requestObj->requesttype) {
            requestObj->success = curlPost(requestObj->requestdata, headerFunc, &(requestObj->responseheader), bodyFunc, &(requestObj->responsebody), &(requestObj->curlcode), &(requestObj->responsecode), &(requestObj->errorbuffer));
        } else if ("PUT" == requestObj->requesttype) {
            requestObj->success = curlPut(requestObj->requestdata, headerFunc, &(requestObj->responseheader), bodyFunc, &(requestObj->responsebody), &(requestObj->curlcode), &(requestObj->responsecode), &(requestObj->errorbuffer));
        } else if ("DELETE" == requestObj->requesttype) {
            requestObj->success = curlDelete(requestObj->requestdata, headerFunc, &(requestObj->responseheader), bodyFunc, &(requestObj->responsebody), &(requestObj->curlcode), &(requestObj->responsecode), &(requestObj->errorbuffer));
        } else {
            continue;
        }
        if (requestObj->syncresponse) {
            if (requestObj->callback) {
                std::string responseheader(requestObj->responseheader.begin(), requestObj->responseheader.end());
                std::string responsebody(requestObj->responsebody.begin(), requestObj->responsebody.end());
                requestObj->callback(requestObj->success, requestObj->curlcode, requestObj->responsecode, requestObj->errorbuffer, responseheader, responsebody);
            }
            delete requestObj;
        } else {
            sResponseListMutex.lock();
            sResponseList->push_back(requestObj);
            sResponseListMutex.unlock();
        }
    }
    return 0;
}
//------------------------------------------------------------------------
static HttpClient* mInstance = NULL;
//------------------------------------------------------------------------
HttpClient* HttpClient::getInstance(void) {
    if (NULL == mInstance) {
        mInstance = new HttpClient();
        std::thread httpThread(httpNetworkThread);
        httpThread.detach();
        createThreadSemphore();
    }
    return mInstance;
}
//------------------------------------------------------------------------
void HttpClient::destroyInstance(void) {
    if (mInstance) {
        destroyThreadSemphore();
        delete mInstance;
        mInstance = NULL;
    }
}
//------------------------------------------------------------------------
void HttpClient::send(HttpObject* obj) {
    sendRequest(obj);
}
//------------------------------------------------------------------------
void HttpClient::receive(void) {
    recvResponse();
}
//------------------------------------------------------------------------
void HttpClient::get(const std::string& url, HTTP_CALLBACK callback /*= 0*/) {
    HttpObject* obj = new HttpObject();
    obj->syncresponse = true;
    obj->tag = "";
    obj->requesttype = "GET";
    obj->requestdata.url = url;
    obj->requestdata.connecttimeout = 30;
    obj->requestdata.timeout = 60;
    obj->callback = callback;
    sendRequest(obj);
}
//------------------------------------------------------------------------
void HttpClient::post(const std::string& url, const char* data, HTTP_CALLBACK callback /*= 0*/) {
    HttpObject* obj = new HttpObject();
    obj->syncresponse = true;
    obj->tag = "";
    obj->requesttype = "POST";
    obj->requestdata.url = url;
    obj->requestdata.setData(data, NULL == data ? 0 : strlen(data));
    obj->requestdata.connecttimeout = 30;
    obj->requestdata.timeout = 60;
    obj->callback = callback;
    sendRequest(obj);
}
//------------------------------------------------------------------------
