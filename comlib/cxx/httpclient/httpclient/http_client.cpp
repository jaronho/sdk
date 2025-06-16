#include "http_client.h"

#include <stdexcept>

namespace http
{
static threading::ExecutorPtr s_workers = nullptr; /* 网络线程池 */
static threading::ExecutorPtr s_respExecutor = nullptr; /* 响应回认执行器 */
static ResponseExecutorHook s_respExecutorHook = nullptr; /* 响应回调执行器钩子 */

void HttpClient::start(size_t threadCount, const threading::ExecutorPtr& respExecutor, const ResponseExecutorHook& respExecutorHook)
{
    if (!s_workers)
    {
        s_workers = threading::ThreadProxy::createAsioExecutor("http", std::max<size_t>(1U, threadCount));
    }
    s_respExecutor = respExecutor;
    s_respExecutorHook = respExecutorHook;
}

void HttpClient::stop()
{
    if (s_workers)
    {
        s_workers.reset();
    }
    if (s_respExecutor)
    {
        s_respExecutor.reset();
    }
    if (s_respExecutorHook)
    {
        s_respExecutorHook = nullptr;
    }
}

curlex::SimpleRequestPtr HttpClient::makeSimpleRequest(const std::string& url, unsigned int localPort)
{
    return std::make_shared<curlex::SimpleRequest>(url, localPort);
}

curlex::Ssl1WayRequestPtr HttpClient::makeSsl1WayRequest(const std::string& caFile, const std::string& url, unsigned int localPort)
{
    return std::make_shared<curlex::Ssl1WayRequest>(caFile, url, localPort);
}

curlex::Ssl2WayRequestPtr HttpClient::makeSsl2WayRequest(const curlex::FileFormat& fileFmt, const std::string& certFile,
                                                         const std::string& privateKeyFile, const std::string& privateKeyFilePwd,
                                                         const std::string& url, unsigned int localPort)
{
    return std::make_shared<curlex::Ssl2WayRequest>(fileFmt, certFile, privateKeyFile, privateKeyFilePwd, url, localPort);
}

curlex::UserpwdRequestPtr HttpClient::makeUserpwdRequest(const std::string& username, const std::string& password, const std::string& url,
                                                         unsigned int localPort)
{
    return std::make_shared<curlex::UserpwdRequest>(username, password, url, localPort);
}

curlex::RawRequestDataPtr HttpClient::makeRawData(const char* bytes, size_t count, bool chunk)
{
    return std::make_shared<curlex::RawRequestData>(bytes, count, chunk);
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
    auto name = "http.easy_delete|" + req->getUrl();
    threading::ThreadProxy::async(
        name,
        [name, req, funcSet, respCb]() {
            curlex::Response resp;
            curlex::curlDelete(req, funcSet, resp);
            handleResp(name, resp, respCb);
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
    auto name = "http.easy_get|" + req->getUrl();
    threading::ThreadProxy::async(
        name,
        [name, req, funcSet, respCb]() {
            curlex::Response resp;
            curlex::curlGet(req, funcSet, resp);
            handleResp(name, resp, respCb);
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
    auto name = "http.easy_put|" + req->getUrl();
    threading::ThreadProxy::async(
        name,
        [name, req, funcSet, respCb]() {
            curlex::Response resp;
            curlex::curlPut(req, funcSet, resp);
            handleResp(name, resp, respCb);
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
    auto name = "http.easy_post|" + req->getUrl();
    threading::ThreadProxy::async(
        name,
        [name, req, funcSet, respCb]() {
            curlex::Response resp;
            curlex::curlPost(req, funcSet, resp);
            handleResp(name, resp, respCb);
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
    auto name = "http.easy_download|" + req->getUrl();
    threading::ThreadProxy::async(
        name,
        [name, req, filename, recover, funcSet, respCb]() {
            curlex::Response resp;
            curlex::curlDownload(req, filename, recover, funcSet, resp);
            handleResp(name, resp, respCb);
        },
        s_workers);
}

void HttpClient::handleResp(const std::string& name, const curlex::Response& resp, const ResponseCallback& respCb)
{
    if (!respCb)
    {
        return;
    }
    if (s_respExecutor)
    {
        s_respExecutor->post(name, [name, resp, respCb]() {
            if (respCb)
            {
                if (s_respExecutorHook)
                {
                    s_respExecutorHook(name, [resp, respCb] {
                        if (respCb)
                        {
                            respCb(resp);
                        }
                    });
                }
                else
                {
                    respCb(resp);
                }
            }
        });
    }
    else
    {
        respCb(resp);
    }
}
} // namespace http
