#include "connection.h"

#include <chrono>

namespace http
{
Connection::Connection(const std::string& url, unsigned int localPort)
{
    m_req = HttpClient::makeSimpleRequest(url, localPort);
}

Connection::Connection(const std::string& caFile, const std::string& url, unsigned int localPort)
{
    m_req = HttpClient::makeSsl1WayRequest(caFile, url, localPort);
}

Connection::Connection(const curlex::FileFormat& fileFmt, const std::string& certFile, const std::string& privateKeyFile,
                       const std::string& privateKeyFilePwd, const std::string& url, unsigned int localPort)
{
    m_req = HttpClient::makeSsl2WayRequest(fileFmt, certFile, privateKeyFile, privateKeyFilePwd, url, localPort);
}

Connection::Connection(const std::string& username, const std::string& password, const std::string& url, unsigned int localPort)
{
    m_req = HttpClient::makeUserpwdRequest(username, password, url, localPort);
}

void Connection::setTimeout(size_t seconds)
{
    m_req->setTimeout(seconds);
}

void Connection::setConnectTimeout(size_t seconds)
{
    m_req->setConnectTimeout(seconds);
}

void Connection::setLowSpeedTimeout(size_t limit, size_t seconds)
{
    m_req->setLowSpeedTimeout(limit, seconds);
}

void Connection::setLimitSpeed(size_t sendSpeed, size_t recvSpeed)
{
    m_req->setLimitSpeed(sendSpeed, recvSpeed);
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

bool Connection::setRawData(const char* bytes, size_t count, bool chunk)
{
    if (m_data)
    {
        if (curlex::RequestData::Type::raw != m_data->getType())
        {
            return false;
        }
        return true;
    }
    m_data = HttpClient::makeRawData(bytes, count, chunk);
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
        HttpClient::easyDelete(m_req, m_funcSet, [req = m_req, respCb](const curlex::Response& resp) {
            if (req)
            {
                req->setData(nullptr); /* 清空数据 */
            }
            if (respCb)
            {
                respCb(resp);
            }
        });
    }
    else
    {
        curlex::Response resp;
        curlex::curlDelete(m_req, m_funcSet, resp);
        /* 清空数据 */
        m_data.reset();
        m_req->setData(nullptr);
        if (respCb)
        {
            respCb(resp);
        }
    }
}

void Connection::doGet(const ResponseCallback& respCb, bool asyncOp)
{
    m_req->setData(m_data);
    if (asyncOp)
    {
        HttpClient::easyGet(m_req, m_funcSet, [req = m_req, respCb](const curlex::Response& resp) {
            if (req)
            {
                req->setData(nullptr); /* 清空数据 */
            }
            if (respCb)
            {
                respCb(resp);
            }
        });
    }
    else
    {
        curlex::Response resp;
        curlex::curlGet(m_req, m_funcSet, resp);
        /* 清空数据 */
        m_data.reset();
        m_req->setData(nullptr);
        if (respCb)
        {
            respCb(resp);
        }
    }
}

void Connection::doPut(const ResponseCallback& respCb, bool asyncOp)
{
    m_req->setData(m_data);
    if (asyncOp)
    {
        HttpClient::easyPut(m_req, m_funcSet, [req = m_req, respCb](const curlex::Response& resp) {
            if (req)
            {
                req->setData(nullptr); /* 清空数据 */
            }
            if (respCb)
            {
                respCb(resp);
            }
        });
    }
    else
    {
        curlex::Response resp;
        curlex::curlPut(m_req, m_funcSet, resp);
        /* 清空数据 */
        m_data.reset();
        m_req->setData(nullptr);
        if (respCb)
        {
            respCb(resp);
        }
    }
}

void Connection::doPost(const ResponseCallback& respCb, bool asyncOp)
{
    m_req->setData(m_data);
    if (asyncOp)
    {
        HttpClient::easyPost(m_req, m_funcSet, [req = m_req, respCb](const curlex::Response& resp) {
            if (req)
            {
                req->setData(nullptr); /* 清空数据 */
            }
            if (respCb)
            {
                respCb(resp);
            }
        });
    }
    else
    {
        curlex::Response resp;
        curlex::curlPost(m_req, m_funcSet, resp);
        /* 清空数据 */
        m_data.reset();
        m_req->setData(nullptr);
        if (respCb)
        {
            respCb(resp);
        }
    }
}

void Connection::doDownload(const std::string& filename, bool recover, const ResponseCallback& respCb, bool asyncOp)
{
    m_req->setData(m_data);
    if (asyncOp)
    {
        HttpClient::easyDownload(m_req, filename, recover, m_funcSet, [req = m_req, respCb](const curlex::Response& resp) {
            if (req)
            {
                req->setData(nullptr); /* 清空数据 */
            }
            if (respCb)
            {
                respCb(resp);
            }
        });
    }
    else
    {
        curlex::Response resp;
        curlex::curlDownload(m_req, filename, recover, m_funcSet, resp);
        /* 清空数据 */
        m_data.reset();
        m_req->setData(nullptr);
        if (respCb)
        {
            respCb(resp);
        }
    }
}
} // namespace http
