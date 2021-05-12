#include "curl_request.h"

namespace curlex
{
Request::Request(const std::string& url) : m_url(url) {}

std::string Request::getUrl()
{
    return m_url;
}

bool Request::isEnableRedirect()
{
    return m_redirect;
}

void Request::setEnableRedirect()
{
    m_redirect = true;
}

int Request::getConnectTimeout()
{
    return m_connectTimeout;
}

void Request::setConnectTimeout(size_t seconds)
{
    m_connectTimeout = static_cast<int>(seconds);
}

int Request::getTimeout()
{
    return m_timeout;
}

void Request::setTimeout(size_t seconds)
{
    m_timeout = static_cast<int>(seconds);
}

bool Request::isKeepAlive()
{
    return m_keepAlive;
}

size_t Request::getKeepAliveIdle()
{
    return m_keepAliveIdle;
}

size_t Request::getKeepAliveInterval()
{
    return m_keepAliveInterval;
}

void Request::setKeepAlive(size_t idle, size_t interval)
{
    m_keepAlive = true;
    m_keepAliveIdle = idle;
    m_keepAliveInterval = interval;
}

std::map<std::string, std::string> Request::getHeaders()
{
    return m_headers;
}

void Request::setHeaders(const std::map<std::string, std::string>& headers)
{
    m_headers = headers;
}

std::string Request::getCookieFile()
{
    return m_cookieFilename;
}

void Request::setCookieFile(const std::string& filename)
{
    m_cookieFilename = filename;
}

RequestDataPtr Request::getData()
{
    return m_data;
}

void Request::setData(const RequestDataPtr& data)
{
    m_data = data;
}

SimpleRequest::SimpleRequest(const std::string& url) : Request(url) {}

Request::Type SimpleRequest::getType()
{
    return Type::SIMPLE;
}

CafileRequest::CafileRequest(const std::string& sslCaFilename, const std::string& url) : Request(url), m_sslCaFilename(sslCaFilename) {}

Request::Type CafileRequest::getType()
{
    return Type::CAFILE;
}

std::string CafileRequest::getCaFilename()
{
    return m_sslCaFilename;
}

UserpwdRequest::UserpwdRequest(const std::string& username, const std::string& password, const std::string& url)
    : Request(url), m_username(username), m_password(password)
{
}

Request::Type UserpwdRequest::getType()
{
    return Type::USERPWD;
}

std::string UserpwdRequest::getUsername()
{
    return m_username;
}

std::string UserpwdRequest::getPassword()
{
    return m_password;
}
} // namespace curlex
