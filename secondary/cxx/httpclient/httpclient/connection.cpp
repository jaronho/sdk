#include "connection.h"

namespace http
{
Connection::Connection(const std::string& url)
{
    m_req = HttpClient::makeSimpleRequest(url);
    setStopFunc();
}

Connection::Connection(const std::string& sslCaFilename, const std::string& url)
{
    m_req = HttpClient::makeCafileRequest(sslCaFilename, url);
    setStopFunc();
}

Connection::Connection(const std::string& username, const std::string& password, const std::string& url)
{
    m_req = HttpClient::makeUserpwdRequest(username, password, url);
    setStopFunc();
}

void Connection::terminate()
{
    m_stop.store(1);
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

void Connection::setSendProgressFunc(const std::function<void(int64_t now, int64_t total, double speed)>& func)
{
    m_funcSet.sendProgressFunc = func;
}

void Connection::setRecvProgressFunc(const std::function<void(int64_t now, int64_t total, double speed)>& func)
{
    m_funcSet.recvProgressFunc = func;
}

bool Connection::setRawData(const unsigned char* bytes, size_t count)
{
    if (m_data)
    {
        if (curlex::RequestData::Type::RAW != m_data->getType())
        {
            return false;
        }
        return true;
    }
    m_data = HttpClient::makeRawData(bytes, count);
    return true;
}

bool Connection::setFormData(const std::string& data)
{
    if (m_data)
    {
        if (curlex::RequestData::Type::FORM != m_data->getType())
        {
            return false;
        }
        return true;
    }
    m_data = HttpClient::makeFormData(data);
    return true;
}

bool Connection::appendContentText(const std::string& key, const std::string& value, const std::string& contentType)
{
    if (m_data)
    {
        if (curlex::RequestData::Type::MULTIPART_FORM != m_data->getType())
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
        if (curlex::RequestData::Type::MULTIPART_FORM != m_data->getType())
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

void Connection::doDelete(const ResponseCallback& respCb)
{
    m_req->setData(m_data);
    HttpClient::easyDelete(m_req, m_funcSet, respCb);
}

void Connection::doGet(const ResponseCallback& respCb)
{
    m_req->setData(m_data);
    HttpClient::easyGet(m_req, m_funcSet, respCb);
}

void Connection::doPut(const ResponseCallback& respCb)
{
    m_req->setData(m_data);
    HttpClient::easyPut(m_req, m_funcSet, respCb);
}

void Connection::doPost(const ResponseCallback& respCb)
{
    m_req->setData(m_data);
    HttpClient::easyPost(m_req, m_funcSet, respCb);
}

void Connection::doDownload(const std::string& filename, bool recover, const ResponseCallback& respCb)
{
    m_req->setData(m_data);
    HttpClient::easyDownload(m_req, filename, recover, m_funcSet, respCb);
}

void Connection::setStopFunc()
{
    m_funcSet.isStopFunc = [&]() { return 1 == m_stop.load(); };
}
} // namespace http
