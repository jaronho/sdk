/**********************************************************************
* Author:	jaron.ho
* Date:		2014-06-04
* Brief:	http client
**********************************************************************/
#include "HttpClient.h"
//------------------------------------------------------------------------
static unsigned int bodyFunc(void* ptr, unsigned int size, unsigned int nmemb, void* stream) {
    std::vector<char>* recvBuffer = (std::vector<char>*)stream;
    unsigned int sizes = size * nmemb;
    recvBuffer->insert(recvBuffer->end(), (char*)ptr, (char*)ptr + sizes);
    return sizes;
}
//------------------------------------------------------------------------
static void sendHttpObject(HttpObject* obj) {
    if (!obj) {
        return;
    }
    std::transform(obj->requesttype.begin(), obj->requesttype.end(), obj->requesttype.begin(), ::toupper);
    if ("GET" == obj->requesttype) {
        obj->success = curlGet(*obj, &(obj->responseheader), bodyFunc, &(obj->responsebody), NULL, NULL, &(obj->curlcode), &(obj->responsecode), &(obj->errorbuffer));
    } else if ("POST" == obj->requesttype) {
        obj->success = curlPost(*obj, &(obj->responseheader), bodyFunc, &(obj->responsebody), NULL, NULL, &(obj->curlcode), &(obj->responsecode), &(obj->errorbuffer));
    } else if ("POST_FORM" == obj->requesttype) {
        obj->success = curlPostForm(*obj, &(obj->responseheader), bodyFunc, &(obj->responsebody), NULL, NULL, &(obj->curlcode), &(obj->responsecode), &(obj->errorbuffer));
    } else if ("PUT" == obj->requesttype) {
        obj->success = curlPut(*obj, &(obj->responseheader), bodyFunc, &(obj->responsebody), NULL, NULL, &(obj->curlcode), &(obj->responsecode), &(obj->errorbuffer));
    } else if ("DELETE" == obj->requesttype) {
        obj->success = curlDelete(*obj, &(obj->responseheader), bodyFunc, &(obj->responsebody), NULL, NULL, &(obj->curlcode), &(obj->responsecode), &(obj->errorbuffer));
    } else {
        delete obj;
        return;
    }
    if (obj->callback) {
        std::string responseheader(obj->responseheader.begin(), obj->responseheader.end());
        std::string responsebody(obj->responsebody.begin(), obj->responsebody.end());
        obj->callback(obj->success, obj->curlcode, obj->responsecode, obj->errorbuffer, responseheader, responsebody, obj->param);
    }
    delete obj;
}
//------------------------------------------------------------------------
static bool sIsRunning = false;								// 是否在运行
static std::mutex sRequestListMutex;						// 请求列表互斥
static std::mutex sResponseListMutex;						// 响应列表互斥
static std::list<HttpObject*>* sReuqestList = NULL;			// 请求列表
static std::list<HttpObject*>* sResponseList = NULL;		// 响应列表
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
    if (!sReuqestList || !sResponseList) {
        return;
    }
    sRequestListMutex.lock();
    if (sReuqestList) {
        sReuqestList->clear();
        delete sReuqestList;
        sReuqestList = NULL;
    }
    sRequestListMutex.unlock();
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
    if (!obj) {
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
    HttpObject* obj = NULL;
    sResponseListMutex.lock();
    if (sResponseList && sResponseList->size() > 0) {
        obj = *(sResponseList->begin());
        sResponseList->pop_front();
    }
    sResponseListMutex.unlock();
    if (!obj) {
        return;
    }
    if (obj->callback) {
        std::string responseheader(obj->responseheader.begin(), obj->responseheader.end());
        std::string responsebody(obj->responsebody.begin(), obj->responsebody.end());
        obj->callback(obj->success, obj->curlcode, obj->responsecode, obj->errorbuffer, responseheader, responsebody, obj->param);
    }
    delete obj;
}
//------------------------------------------------------------------------
static void httpNetworkThread() {
    while (sIsRunning) {
        HttpObject* obj = NULL;
        sRequestListMutex.lock();
        if (sReuqestList && sReuqestList->size() > 0) {
            obj = *(sReuqestList->begin());
            sReuqestList->pop_front();
        }
        sRequestListMutex.unlock();
        if (!obj) {
            std::lock_guard<std::mutex> lock(sRequestListMutex);
            sSleepCondition.wait(sRequestListMutex);
            continue;
        }
        std::transform(obj->requesttype.begin(), obj->requesttype.end(), obj->requesttype.begin(), ::toupper);
        if ("GET" == obj->requesttype) {
            obj->success = curlGet(*obj, &(obj->responseheader), bodyFunc, &(obj->responsebody), NULL, NULL, &(obj->curlcode), &(obj->responsecode), &(obj->errorbuffer));
        } else if ("POST" == obj->requesttype) {
            obj->success = curlPost(*obj, &(obj->responseheader), bodyFunc, &(obj->responsebody), NULL, NULL, &(obj->curlcode), &(obj->responsecode), &(obj->errorbuffer));
        } else if ("POST_FORM" == obj->requesttype) {
            obj->success = curlPostForm(*obj, &(obj->responseheader), bodyFunc, &(obj->responsebody), NULL, NULL, &(obj->curlcode), &(obj->responsecode), &(obj->errorbuffer));
        } else if ("PUT" == obj->requesttype) {
            obj->success = curlPut(*obj, &(obj->responseheader), bodyFunc, &(obj->responsebody), NULL, NULL, &(obj->curlcode), &(obj->responsecode), &(obj->errorbuffer));
        } else if ("DELETE" == obj->requesttype) {
            obj->success = curlDelete(*obj, &(obj->responseheader), bodyFunc, &(obj->responsebody), NULL, NULL, &(obj->curlcode), &(obj->responsecode), &(obj->errorbuffer));
        } else {
            delete obj;
            continue;
        }
        sResponseListMutex.lock();
        sResponseList->push_back(obj);
        sResponseListMutex.unlock();
    }
}
//------------------------------------------------------------------------
static HttpClient* mInstance = NULL;
//------------------------------------------------------------------------
HttpClient* HttpClient::getInstance(void) {
    if (!mInstance) {
        createThreadSemphore();
        mInstance = new HttpClient();
        std::thread httpThread(httpNetworkThread);
        httpThread.detach();
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
void HttpClient::asyncListen(void) {
    recvResponse();
}
//------------------------------------------------------------------------
void HttpClient::get(const std::string& url,
                     const std::map<std::string, std::string>* headers,
                     HTTP_REQUEST_CALLBACK callback,
                     void* param,
                     int connecttimeout,
                     int timeout,
                     bool async) {
    HttpObject* obj = new HttpObject();
    obj->connecttimeout = connecttimeout;
    obj->timeout = timeout;
    obj->url = url;
    if (headers) {
        std::map<std::string, std::string>::const_iterator iter = headers->begin();
        for (; headers->end() != iter; ++iter) {
            obj->headers[iter->first] = iter->second;
        }
    }
    obj->requesttype = "GET";
    obj->callback = callback;
    obj->param = param;
    if (async) {
        sendRequest(obj);
    } else {
        sendHttpObject(obj);
    }
}
//------------------------------------------------------------------------
void HttpClient::post(const std::string& url,
                      const std::map<std::string, std::string>* headers,
                      const unsigned char* data,
                      unsigned int dataLength,
                      HTTP_REQUEST_CALLBACK callback,
                      void* param,
                      int connecttimeout,
                      int timeout,
                      bool async) {
    HttpObject* obj = new HttpObject();
    obj->connecttimeout = connecttimeout;
    obj->timeout = timeout;
    obj->url = url;
    if (headers) {
        std::map<std::string, std::string>::const_iterator iter = headers->begin();
        for (; headers->end() != iter; ++iter) {
            obj->headers[iter->first] = iter->second;
        }
    }
    obj->setData(data, dataLength, true);
    obj->requesttype = "POST";
    obj->callback = callback;
    obj->param = param;
    if (async) {
        sendRequest(obj);
    } else {
        sendHttpObject(obj);
    }
}
//------------------------------------------------------------------------
void HttpClient::postForm(const std::string& url,
                          const std::map<std::string, std::string>* headers,
                          const std::map<std::string, std::string>* contents,
                          const std::map<std::string, std::string>* filenames,
                          HTTP_REQUEST_CALLBACK callback,
                          void* param,
                          int connecttimeout,
                          int timeout,
                          bool async) {
    HttpObject* obj = new HttpObject();
    obj->connecttimeout = connecttimeout;
    obj->timeout = timeout;
    obj->url = url;
    if (headers) {
        std::map<std::string, std::string>::const_iterator iter = headers->begin();
        for (; headers->end() != iter; ++iter) {
            obj->headers[iter->first] = iter->second;
        }
    }
    if (contents) {
        std::map<std::string, std::string>::const_iterator contentIter = contents->begin();
        for (; contents->end() != contentIter; ++contentIter) {
            obj->addText(contentIter->first, contentIter->second);
        }
    }
    if (filenames) {
        std::map<std::string, std::string>::const_iterator fileIter = filenames->begin();
        for (; filenames->end() != fileIter; ++fileIter) {
            obj->addFile(fileIter->first, fileIter->second);
        }
    }
    obj->requesttype = "POST_FORM";
    obj->callback = callback;
    obj->param = param;
    if (async) {
        sendRequest(obj);
    } else {
        sendHttpObject(obj);
    }
}
//------------------------------------------------------------------------
