/**********************************************************************
* Author:	jaron.ho
* Date:		2014-06-04
* Brief:	http 客户端
**********************************************************************/
#include "HttpClient.h"

//------------------------------------------------------------------------
static bool sRunning = false;								// 是否在运行http

static std::list<HttpObject*> *sReuqestList = NULL;			// 请求列表
static std::list<HttpObject*> *sResponseList = NULL;		// 响应列表

static pthread_mutex_t sRequestListMutex;					// 请求列表互斥
static pthread_mutex_t sResponseListMutex;					// 响应列表互斥

static pthread_mutex_t sSleepMutex;
static pthread_cond_t sSleepCondition;

static pthread_t sHttpNetworkThread;						// http处理线程
//------------------------------------------------------------------------
static void destroyThreadSemphore(void)
{
	if (NULL == sReuqestList || NULL == sResponseList)
		return;

	// 清除请求列表
	pthread_mutex_lock(&sRequestListMutex);
	if (sReuqestList)
	{
		sReuqestList->clear();
		delete sReuqestList;
		sReuqestList = NULL;
	}
	pthread_mutex_unlock(&sRequestListMutex);
	// 清除响应列表
	pthread_mutex_lock(&sResponseListMutex);
	if (sResponseList)
	{
		sResponseList->clear();
		delete sResponseList;
		sResponseList = NULL;
	}
	pthread_mutex_unlock(&sResponseListMutex);
	// 清除互斥
	pthread_mutex_destroy(&sRequestListMutex);
	pthread_mutex_destroy(&sResponseListMutex);

	pthread_mutex_destroy(&sSleepMutex);
	pthread_cond_destroy(&sSleepCondition);

    pthread_exit(NULL);
}
//------------------------------------------------------------------------
static void sendRequest(HttpObject* obj)
{
	if (NULL == obj || !sRunning)
		return;

	pthread_mutex_lock(&sRequestListMutex);
	sReuqestList->push_back(obj);
    pthread_mutex_unlock(&sRequestListMutex);
    
    pthread_cond_signal(&sSleepCondition);
}
//------------------------------------------------------------------------
static void recvResponse(void)
{
	if (!sRunning)
		return;

	HttpObject *responseObj = NULL;
	pthread_mutex_lock(&sResponseListMutex);
	if (sResponseList && sResponseList->size() > 0)
	{
		responseObj = *(sResponseList->begin());
		sResponseList->pop_front();
	}
	pthread_mutex_unlock(&sResponseListMutex);

	if (NULL == responseObj)
		return;

	responseObj->response();
	delete responseObj;
	responseObj = NULL;
}
//------------------------------------------------------------------------
static unsigned int writeDataFunc(void* ptr, unsigned int size, unsigned int nmemb, void* stream)
{
	std::vector<char> *recvBuffer = (std::vector<char>*)stream;
	unsigned int sizes = size * nmemb;
	recvBuffer->insert(recvBuffer->end(), (char*)ptr, (char*)ptr+sizes);
	return sizes;
}
//------------------------------------------------------------------------
static unsigned int writeHeaderDataFunc(void* ptr, unsigned int size, unsigned int nmemb, void* stream)
{
	std::vector<char> *recvBuffer = (std::vector<char>*)stream;
	unsigned int sizes = size * nmemb;
	recvBuffer->insert(recvBuffer->end(), (char*)ptr, (char*)ptr+sizes);
	return sizes;
}
//------------------------------------------------------------------------
static void* httpNetworkThread(void *data)
{
	while (true)
	{
		if (!sRunning)
			break;

		HttpObject *requestObj = NULL;
		pthread_mutex_lock(&sRequestListMutex);
		if (sReuqestList && sReuqestList->size() > 0)
		{
			requestObj = *(sReuqestList->begin());
			sReuqestList->pop_front();
		}
		pthread_mutex_unlock(&sRequestListMutex);

		if (NULL == requestObj)
		{
			pthread_cond_wait(&sSleepCondition, &sSleepMutex);
			continue;
		}

		switch (requestObj->requesttype)
		{
		case HRT_GET:
			requestObj->success = httpGet(requestObj->requestdata, writeDataFunc, &(requestObj->responsedata), writeHeaderDataFunc, &(requestObj->responseheader), &(requestObj->curlcode), &(requestObj->responsecode), &(requestObj->errorbuffer));
			break;
		case HRT_POST:
			requestObj->success = httpPost(requestObj->requestdata, writeDataFunc, &(requestObj->responsedata), writeHeaderDataFunc, &(requestObj->responseheader), &(requestObj->curlcode), &(requestObj->responsecode), &(requestObj->errorbuffer));
			break;
		case HRT_PUT:
			requestObj->success = httpPut(requestObj->requestdata, writeDataFunc, &(requestObj->responsedata), writeHeaderDataFunc, &(requestObj->responseheader), &(requestObj->curlcode), &(requestObj->responsecode), &(requestObj->errorbuffer));
			break;
		case HRT_DELETE:
			requestObj->success = httpDelete(requestObj->requestdata, writeDataFunc, &(requestObj->responsedata), writeHeaderDataFunc, &(requestObj->responseheader), &(requestObj->curlcode), &(requestObj->responsecode), &(requestObj->errorbuffer));
			break;
		}

		pthread_mutex_lock(&sResponseListMutex);
		sResponseList->push_back(requestObj);
		pthread_mutex_unlock(&sResponseListMutex);
	}
	destroyThreadSemphore();
	return 0;
}
//------------------------------------------------------------------------
static void createThreadSemphore(void)
{
	if (sReuqestList || sResponseList)
		return;

	sReuqestList = new std::list<HttpObject*>();
	sResponseList = new std::list<HttpObject*>();

	pthread_mutex_init(&sRequestListMutex, NULL);
	pthread_mutex_init(&sResponseListMutex, NULL);
        
	pthread_mutex_init(&sSleepMutex, NULL);
	pthread_cond_init(&sSleepCondition, NULL);

	pthread_create(&sHttpNetworkThread, NULL, httpNetworkThread, NULL);
	pthread_detach(sHttpNetworkThread);
}
//------------------------------------------------------------------------
static HttpClient *mInstance = NULL;
//------------------------------------------------------------------------
HttpClient* HttpClient::getInstance(void)
{
	if (NULL == mInstance)
	{
		mInstance = new HttpClient();
		createThreadSemphore();
		sRunning = true;
	}
	return mInstance;
}
//------------------------------------------------------------------------
void HttpClient::destroyInstance(void)
{
	if (mInstance)
	{
		sRunning = false;
		delete mInstance;
		mInstance = NULL;
	}
}
//------------------------------------------------------------------------
void HttpClient::send(HttpObject* obj)
{
	sendRequest(obj);
}
//------------------------------------------------------------------------
void HttpClient::receive(void)
{
	recvResponse();
}
//------------------------------------------------------------------------
