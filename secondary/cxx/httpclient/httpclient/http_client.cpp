#include "http_client.h"

#include <assert.h>

namespace http
{
threading::ExecutorPtr HttpClient::s_workers = nullptr;
std::mutex HttpClient::s_respMutex;
std::list<std::shared_ptr<HttpClient::RespParam>> HttpClient::s_respList;

void HttpClient::start(size_t threadCount)
{
    if (!s_workers)
    {
        s_workers = threading::ThreadProxy::createAsioExecutor("http", threadCount > 0 ? threadCount : 1);
    }
}

void HttpClient::stop()
{
    if (s_workers)
    {
        s_workers.reset();
    }
}

void HttpClient::runOnce()
{
    std::unique_lock<std::mutex> locker(s_respMutex);
    if (s_respList.empty())
    {
        return;
    }
    std::shared_ptr<RespParam> param = *(s_respList.begin());
    s_respList.pop_front();
    locker.unlock();
    if (param->respCb)
    {
        param->respCb(param->resp);
    }
}

curlex::SimpleRequestPtr HttpClient::makeSimpleRequest(const std::string& url)
{
    return std::make_shared<curlex::SimpleRequest>(url);
}

curlex::CafileRequestPtr HttpClient::makeCafileRequest(const std::string& sslCaFilename, const std::string& url)
{
    return std::make_shared<curlex::CafileRequest>(sslCaFilename, url);
}

curlex::UserpwdRequestPtr HttpClient::makeUserpwdRequest(const std::string& username, const std::string& password, const std::string& url)
{
    return std::make_shared<curlex::UserpwdRequest>(username, password, url);
}

curlex::RawRequestDataPtr HttpClient::makeRawData(const unsigned char* bytes, size_t count)
{
    return std::make_shared<curlex::RawRequestData>(bytes, count);
}

curlex::FormRequestDataPtr HttpClient::makeFormData(const std::string& data)
{
    return std::make_shared<curlex::FormRequestData>(data);
}

curlex::MultipartFormRequestDataPtr HttpClient::makeMultipartFormData()
{
    return std::make_shared<curlex::MultipartFormRequestData>();
}

void HttpClient::easyDelete(const curlex::RequestPtr& req, const curlex::FuncSet& funcSet, const ResponseCallback& respCb)
{
    assert(s_workers);
    s_workers->post("http.simple_delete", [req, funcSet, respCb]() {
        curlex::Response resp;
        curlex::curlDelete(req, funcSet, resp);
        insertRespList(resp, respCb);
    });
}

void HttpClient::easyGet(const curlex::RequestPtr& req, const curlex::FuncSet& funcSet, const ResponseCallback& respCb)
{
    assert(s_workers);
    s_workers->post("http.simple_get", [req, funcSet, respCb]() {
        curlex::Response resp;
        curlex::curlGet(req, funcSet, resp);
        insertRespList(resp, respCb);
    });
}

void HttpClient::easyPut(const curlex::RequestPtr& req, const curlex::FuncSet& funcSet, const ResponseCallback& respCb)
{
    assert(s_workers);
    s_workers->post("http.simple_put", [req, funcSet, respCb]() {
        curlex::Response resp;
        curlex::curlPut(req, funcSet, resp);
        insertRespList(resp, respCb);
    });
}

void HttpClient::easyPost(const curlex::RequestPtr& req, const curlex::FuncSet& funcSet, const ResponseCallback& respCb)
{
    assert(s_workers);
    s_workers->post("http.simple_post", [req, funcSet, respCb]() {
        curlex::Response resp;
        curlex::curlPost(req, funcSet, resp);
        insertRespList(resp, respCb);
    });
}

void HttpClient::easyDownload(const curlex::RequestPtr& req, const std::string& filename, bool recover, const curlex::FuncSet& funcSet,
                              const ResponseCallback& respCb)
{
    assert(s_workers);
    s_workers->post("http.simple_download", [req, filename, recover, funcSet, respCb]() {
        curlex::Response resp;
        curlex::curlDownload(req, filename, recover, funcSet, resp);
        insertRespList(resp, respCb);
    });
}

void HttpClient::insertRespList(const curlex::Response& resp, const ResponseCallback& respCb)
{
    std::unique_lock<std::mutex> locker(s_respMutex);
    std::shared_ptr<RespParam> param = std::make_shared<RespParam>();
    param->resp = resp;
    param->respCb = respCb;
    s_respList.push_back(param);
}
} // namespace http
