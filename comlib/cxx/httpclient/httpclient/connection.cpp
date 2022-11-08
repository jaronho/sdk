#include "connection.h"

#include <chrono>

namespace http
{
Connection::Connection(const std::string& url)
{
    m_req = HttpClient::makeSimpleRequest(url);
}

Connection::Connection(const std::string& caFile, const std::string& url)
{
    m_req = HttpClient::makeSsl1WayRequest(caFile, url);
}

Connection::Connection(const curlex::FileFormat& fileFmt, const std::string& certFile, const std::string& privateKeyFile,
                       const std::string& privateKeyFilePwd, const std::string& url)
{
    m_req = HttpClient::makeSsl2WayRequest(fileFmt, certFile, privateKeyFile, privateKeyFilePwd, url);
}

Connection::Connection(const std::string& username, const std::string& password, const std::string& url)
{
    m_req = HttpClient::makeUserpwdRequest(username, password, url);
}

void Connection::setConnectTimeout(size_t seconds)
{
    m_req->setConnectTimeout(seconds);
}

void Connection::setTimeout(size_t seconds)
{
    m_req->setTimeout(seconds);
}

void Connection::appendHeader(const std::string& key, const std::string& value)
{
    m_req->appendHeader(key, value);
}

void Connection::setStopFunc(const std::function<bool()>& func)
{
    m_funcSet.isStopFunc = func;
}

void Connection::setSendProgressFunc(const std::function<void(int64_t now, int64_t total, double speed)>& func)
{
    m_funcSet.sendProgressFunc = func;
}

void Connection::setRecvProgressFunc(const std::function<void(int64_t now, int64_t total, double speed)>& func)
{
    m_funcSet.recvProgressFunc = func;
}

bool Connection::setRawData(const char* bytes, size_t count)
{
    if (m_data)
    {
        if (curlex::RequestData::Type::raw != m_data->getType())
        {
            return false;
        }
        return true;
    }
    m_data = HttpClient::makeRawData(bytes, count);
    return true;
}

bool Connection::setFormData(const std::map<std::string, std::string>& fieldMap)
{
    if (m_data)
    {
        if (curlex::RequestData::Type::form != m_data->getType())
        {
            return false;
        }
        return true;
    }
    m_data = HttpClient::makeFormData(fieldMap);
    return true;
}

bool Connection::appendContentText(const std::string& key, const std::string& value, const std::string& contentType)
{
    if (m_data)
    {
        if (curlex::RequestData::Type::multipart_form != m_data->getType())
        {
            return false;
        }
    }
    else
    {
        m_data = HttpClient::makeMultipartFormData();
    }
    auto data = std::dynamic_pointer_cast<curlex::MultipartFormRequestData>(m_data);
    data->addText(key, value, contentType);
    return true;
}

bool Connection::appendContentFile(const std::string& key, const std::string& filename)
{
    if (m_data)
    {
        if (curlex::RequestData::Type::multipart_form != m_data->getType())
        {
            return false;
        }
    }
    else
    {
        m_data = HttpClient::makeMultipartFormData();
    }
    auto data = std::dynamic_pointer_cast<curlex::MultipartFormRequestData>(m_data);
    data->addFile(key, filename);
    return true;
}

std::string Connection::toString()
{
    std::string str;
    /* URL */
    str.append(m_req->getUrl()).append("\n");
    /* 请求首部字段 */
    auto headers = m_req->getHeaders();
    for (auto iter = headers.begin(); headers.end() != iter; ++iter)
    {
        str.append(iter->first).append(": ").append(iter->second).append("\n");
    }
    /* 内容实体 */
    if (m_data)
    {
        str.append("\n");
        str.append(m_data->toString());
    }
    return str;
}

void Connection::doDelete(const ResponseCallback& respCb, bool asyncOp)
{
    m_req->setData(m_data);
    if (asyncOp)
    {
        HttpClient::easyDelete(m_req, m_funcSet, respCb);
    }
    else
    {
        curlex::Response resp;
        curlex::curlDelete(m_req, m_funcSet, resp);
        if (respCb)
        {
            try
            {
                auto beg = std::chrono::steady_clock::now();
                respCb(resp);
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg).count();
                HttpClient::onResponseProcessFinishedState(resp.url, elapsed);
            }
            catch (const std::exception& e)
            {
                HttpClient::onResponseProcessExceptionStateCallback(resp.url, e.what());
            }
            catch (...)
            {
                HttpClient::onResponseProcessExceptionStateCallback(resp.url, "unknown exception");
            }
        }
    }
}

void Connection::doGet(const ResponseCallback& respCb, bool asyncOp)
{
    m_req->setData(m_data);
    if (asyncOp)
    {
        HttpClient::easyGet(m_req, m_funcSet, respCb);
    }
    else
    {
        curlex::Response resp;
        curlex::curlGet(m_req, m_funcSet, resp);
        if (respCb)
        {
            try
            {
                auto beg = std::chrono::steady_clock::now();
                respCb(resp);
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg).count();
                HttpClient::onResponseProcessFinishedState(resp.url, elapsed);
            }
            catch (const std::exception& e)
            {
                HttpClient::onResponseProcessExceptionStateCallback(resp.url, e.what());
            }
            catch (...)
            {
                HttpClient::onResponseProcessExceptionStateCallback(resp.url, "unknown exception");
            }
        }
    }
}

void Connection::doPut(const ResponseCallback& respCb, bool asyncOp)
{
    m_req->setData(m_data);
    if (asyncOp)
    {
        HttpClient::easyPut(m_req, m_funcSet, respCb);
    }
    else
    {
        curlex::Response resp;
        curlex::curlPut(m_req, m_funcSet, resp);
        if (respCb)
        {
            try
            {
                auto beg = std::chrono::steady_clock::now();
                respCb(resp);
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg).count();
                HttpClient::onResponseProcessFinishedState(resp.url, elapsed);
            }
            catch (const std::exception& e)
            {
                HttpClient::onResponseProcessExceptionStateCallback(resp.url, e.what());
            }
            catch (...)
            {
                HttpClient::onResponseProcessExceptionStateCallback(resp.url, "unknown exception");
            }
        }
    }
}

void Connection::doPost(const ResponseCallback& respCb, bool asyncOp)
{
    m_req->setData(m_data);
    if (asyncOp)
    {
        HttpClient::easyPost(m_req, m_funcSet, respCb);
    }
    else
    {
        curlex::Response resp;
        curlex::curlPost(m_req, m_funcSet, resp);
        if (respCb)
        {
            try
            {
                auto beg = std::chrono::steady_clock::now();
                respCb(resp);
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg).count();
                HttpClient::onResponseProcessFinishedState(resp.url, elapsed);
            }
            catch (const std::exception& e)
            {
                HttpClient::onResponseProcessExceptionStateCallback(resp.url, e.what());
            }
            catch (...)
            {
                HttpClient::onResponseProcessExceptionStateCallback(resp.url, "unknown exception");
            }
        }
    }
}

void Connection::doDownload(const std::string& filename, bool recover, const ResponseCallback& respCb, bool asyncOp)
{
    m_req->setData(m_data);
    if (asyncOp)
    {
        HttpClient::easyDownload(m_req, filename, recover, m_funcSet, respCb);
    }
    else
    {
        curlex::Response resp;
        curlex::curlDownload(m_req, filename, recover, m_funcSet, resp);
        if (respCb)
        {
            try
            {
                auto beg = std::chrono::steady_clock::now();
                respCb(resp);
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg).count();
                HttpClient::onResponseProcessFinishedState(resp.url, elapsed);
            }
            catch (const std::exception& e)
            {
                HttpClient::onResponseProcessExceptionStateCallback(resp.url, e.what());
            }
            catch (...)
            {
                HttpClient::onResponseProcessExceptionStateCallback(resp.url, "unknown exception");
            }
        }
    }
}
} // namespace http
