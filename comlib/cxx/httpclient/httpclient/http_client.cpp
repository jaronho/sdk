#include "http_client.h"

#include <chrono>
#include <list>
#include <mutex>
#include <stdexcept>

#include "threading/thread_proxy.hpp"

namespace http
{
/**
 * @brief 响应参数, 用于跨线程传输
 */
struct RespParam
{
    curlex::Response resp;
    ResponseCallback respCb = nullptr;
};

static threading::ExecutorPtr s_workers = nullptr; /* 网络线程池 */
static std::mutex s_respMutex;
static std::list<std::shared_ptr<RespParam>> s_respList; /* 响应列表 */
static std::mutex s_stateCallbackMutex;
static ResponseProcessNormalStateCallback s_finishedStateCallback = nullptr; /* 响应处理结束状态回调 */
static ResponseProcessExceptionStateCallback s_exceptionStateCallback = nullptr; /* 响应处理异常状态回调 */

void HttpClient::setResponseProcessFinishedStateCallback(const ResponseProcessNormalStateCallback& stateCb)
{
    std::lock_guard<std::mutex> locker(s_stateCallbackMutex);
    s_finishedStateCallback = stateCb;
}

void HttpClient::setResponseProcessExceptionStateCallback(const ResponseProcessExceptionStateCallback& stateCb)
{
    std::lock_guard<std::mutex> locker(s_stateCallbackMutex);
    s_exceptionStateCallback = stateCb;
}

void HttpClient::start(size_t threadCount)
{
    if (!s_workers)
    {
        s_workers = threading::ThreadProxy::createAsioExecutor("http", std::max<size_t>(1U, threadCount));
    }
}

void HttpClient::stop()
{
    if (s_workers)
    {
        s_workers.reset();
    }
    std::lock_guard<std::mutex> locker(s_respMutex);
    s_respList.clear();
}

void HttpClient::runOnce()
{
    std::shared_ptr<RespParam> param = nullptr;
    {
        std::lock_guard<std::mutex> locker(s_respMutex);
        if (s_respList.empty())
        {
            return;
        }
        param = *(s_respList.begin());
        s_respList.pop_front();
    }
    if (param && param->respCb)
    {
        try
        {
            auto beg = std::chrono::steady_clock::now();
            param->respCb(param->resp);
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg).count();
            ResponseProcessNormalStateCallback finishedStateCallback = nullptr;
            {
                std::lock_guard<std::mutex> locker(s_stateCallbackMutex);
                finishedStateCallback = s_finishedStateCallback;
            }
            if (finishedStateCallback)
            {
                finishedStateCallback(param->resp.url, elapsed);
            }
        }
        catch (const std::exception& e)
        {
            ResponseProcessExceptionStateCallback exceptionStateCallback = nullptr;
            {
                std::lock_guard<std::mutex> locker(s_stateCallbackMutex);
                exceptionStateCallback = s_exceptionStateCallback;
            }
            if (exceptionStateCallback)
            {
                exceptionStateCallback(param->resp.url, e.what());
            }
        }
        catch (...)
        {
            ResponseProcessExceptionStateCallback exceptionStateCallback = nullptr;
            {
                std::lock_guard<std::mutex> locker(s_stateCallbackMutex);
                exceptionStateCallback = s_exceptionStateCallback;
            }
            if (exceptionStateCallback)
            {
                exceptionStateCallback(param->resp.url, "unknown exception");
            }
        }
    }
}

curlex::SimpleRequestPtr HttpClient::makeSimpleRequest(const std::string& url)
{
    return std::make_shared<curlex::SimpleRequest>(url);
}

curlex::Ssl1WayRequestPtr HttpClient::makeSsl1WayRequest(const std::string& caFile, const std::string& url)
{
    return std::make_shared<curlex::Ssl1WayRequest>(caFile, url);
}

curlex::Ssl2WayRequestPtr HttpClient::makeSsl2WayRequest(const std::string& certFile, const std::string& privateKeyFile,
                                                         const std::string& privateKeyFilePwd, const std::string& url)
{
    return std::make_shared<curlex::Ssl2WayRequest>(certFile, privateKeyFile, privateKeyFilePwd, url);
}

curlex::UserpwdRequestPtr HttpClient::makeUserpwdRequest(const std::string& username, const std::string& password, const std::string& url)
{
    return std::make_shared<curlex::UserpwdRequest>(username, password, url);
}

curlex::RawRequestDataPtr HttpClient::makeRawData(const char* bytes, size_t count)
{
    return std::make_shared<curlex::RawRequestData>(bytes, count);
}

curlex::FormRequestDataPtr HttpClient::makeFormData(const std::map<std::string, std::string>& fieldMap)
{
    return std::make_shared<curlex::FormRequestData>(fieldMap);
}

curlex::MultipartFormRequestDataPtr HttpClient::makeMultipartFormData()
{
    return std::make_shared<curlex::MultipartFormRequestData>();
}

void HttpClient::easyDelete(const curlex::RequestPtr& req, const curlex::FuncSet& funcSet, const ResponseCallback& respCb)
{
    if (!s_workers)
    {
        throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__
                               + "] var 's_workers' is null");
    }
    threading::ThreadProxy::async(
        "http.easy_delete|" + req->getUrl(),
        [req, funcSet, respCb]() {
            curlex::Response resp;
            curlex::curlDelete(req, funcSet, resp);
            insertRespList(resp, respCb);
        },
        s_workers);
}

void HttpClient::easyGet(const curlex::RequestPtr& req, const curlex::FuncSet& funcSet, const ResponseCallback& respCb)
{
    if (!s_workers)
    {
        throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__
                               + "] var 's_workers' is null");
    }
    threading::ThreadProxy::async(
        "http.easy_get|" + req->getUrl(),
        [req, funcSet, respCb]() {
            curlex::Response resp;
            curlex::curlGet(req, funcSet, resp);
            insertRespList(resp, respCb);
        },
        s_workers);
}

void HttpClient::easyPut(const curlex::RequestPtr& req, const curlex::FuncSet& funcSet, const ResponseCallback& respCb)
{
    if (!s_workers)
    {
        throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__
                               + "] var 's_workers' is null");
    }
    threading::ThreadProxy::async(
        "http.easy_put|" + req->getUrl(),
        [req, funcSet, respCb]() {
            curlex::Response resp;
            curlex::curlPut(req, funcSet, resp);
            insertRespList(resp, respCb);
        },
        s_workers);
}

void HttpClient::easyPost(const curlex::RequestPtr& req, const curlex::FuncSet& funcSet, const ResponseCallback& respCb)
{
    if (!s_workers)
    {
        throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__
                               + "] var 's_workers' is null");
    }
    threading::ThreadProxy::async(
        "http.easy_post|" + req->getUrl(),
        [req, funcSet, respCb]() {
            curlex::Response resp;
            curlex::curlPost(req, funcSet, resp);
            insertRespList(resp, respCb);
        },
        s_workers);
}

void HttpClient::easyDownload(const curlex::RequestPtr& req, const std::string& filename, bool recover, const curlex::FuncSet& funcSet,
                              const ResponseCallback& respCb)
{
    if (!s_workers)
    {
        throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__
                               + "] var 's_workers' is null");
    }
    threading::ThreadProxy::async(
        "http.easy_download|" + req->getUrl(),
        [req, filename, recover, funcSet, respCb]() {
            curlex::Response resp;
            curlex::curlDownload(req, filename, recover, funcSet, resp);
            insertRespList(resp, respCb);
        },
        s_workers);
}

void HttpClient::insertRespList(const curlex::Response& resp, const ResponseCallback& respCb)
{
    std::shared_ptr<RespParam> param = std::make_shared<RespParam>();
    param->resp = resp;
    param->respCb = respCb;
    std::lock_guard<std::mutex> locker(s_respMutex);
    s_respList.emplace_back(param);
}
} // namespace http
