#include "curl_request.h"

namespace curlex
{
Request::Request(const std::string& url, unsigned int localPort) : m_url(url), m_localPort(localPort) {}

std::string Request::getUrl() const
{
    return m_url;
}

unsigned int Request::getLocalPort() const
{
    return m_localPort;
}

bool Request::isEnableRedirect() const
{
    return m_redirect;
}

int Request::getMaxRedirects() const
{
    return m_maxRedirects;
}

void Request::setEnableRedirect(int maxRedirects)
{
    m_redirect = true;
    m_maxRedirects = maxRedirects;
}

int Request::getTimeout() const
{
    return m_timeout;
}

void Request::setTimeout(size_t seconds)
{
    m_timeout = static_cast<int>(seconds);
}

int Request::getConnectTimeout() const
{
    return m_connectTimeout;
}

void Request::setConnectTimeout(size_t seconds)
{
    m_connectTimeout = static_cast<int>(seconds);
}

void Request::getLowSpeedTimeout(int& limit, int& timeout) const
{
    limit = m_lowSpeedLimit;
    timeout = m_lowSpeedTimeout;
}

void Request::setLowSpeedTimeout(size_t limit, size_t seconds)
{
    m_lowSpeedLimit = static_cast<int>(limit);
    m_lowSpeedTimeout = static_cast<int>(seconds);
}

bool Request::isKeepAlive() const
{
    return m_keepAlive;
}

size_t Request::getKeepAliveIdle() const
{
    return m_keepAliveIdle;
}

size_t Request::getKeepAliveInterval() const
{
    return m_keepAliveInterval;
}

void Request::setKeepAlive(size_t idle, size_t interval)
{
    m_keepAlive = true;
    m_keepAliveIdle = idle;
    m_keepAliveInterval = interval;
}

size_t Request::getMaxSendSpeed() const
{
    return m_maxSendSpeed;
}

size_t Request::getMaxRecvSpeed() const
{
    return m_maxRecvSpeed;
}

void Request::setLimitSpeed(size_t sendSpeed, size_t recvSpeed)
{
    m_maxSendSpeed = sendSpeed;
    m_maxRecvSpeed = recvSpeed;
}

std::map<std::string, std::string> Request::getHeaders() const
{
    return m_headers;
}

void Request::setHeaders(const std::map<std::string, std::string>& headers)
{
    m_headers = headers;
}

void Request::appendHeader(const std::string& key, const std::string& value)
{
    if (key.empty())
    {
        return;
    }
    m_headers[key] = value;
}

std::string Request::getCookieFile() const
{
    return m_cookieFilename;
}

void Request::setCookieFile(const std::string& filename)
{
    m_cookieFilename = filename;
}

RequestDataPtr Request::getData() const
{
    return m_data;
}

void Request::setData(const RequestDataPtr& data)
{
    m_data = data;
}

SimpleRequest::SimpleRequest(const std::string& url, unsigned int localPort) : Request(url, localPort) {}

Request::Type SimpleRequest::getType() const
{
    return Type::simple;
}

Ssl1WayRequest::Ssl1WayRequest(const std::string& caFile, const std::string& url, unsigned int localPort)
    : Request(url, localPort), m_caFile(caFile)
{
}

Request::Type Ssl1WayRequest::getType() const
{
    return Type::ssl_1way;
}

std::string Ssl1WayRequest::getCaFile() const
{
    return m_caFile;
}

Ssl2WayRequest::Ssl2WayRequest(const FileFormat& fileFmt, const std::string& certFile, const std::string& privateKeyFile,
                               const std::string& privateKeyFilePwd, const std::string& url, unsigned int localPort)
    : Request(url, localPort)
    , m_fileFormat(fileFmt)
    , m_certFile(certFile)
    , m_privateKeyFile(privateKeyFile)
    , m_privateKeyFilePwd(privateKeyFilePwd)
{
}

Request::Type Ssl2WayRequest::getType() const
{
    return Type::ssl_2way;
}

FileFormat Ssl2WayRequest::getFileFormat() const
{
    return m_fileFormat;
}

std::string Ssl2WayRequest::getCertFile() const
{
    return m_certFile;
}

std::string Ssl2WayRequest::getPrivateKeyFile() const
{
    return m_privateKeyFile;
}

std::string Ssl2WayRequest::getPrivateKeyFilePwd() const
{
    return m_privateKeyFilePwd;
}

UserpwdRequest::UserpwdRequest(const std::string& username, const std::string& password, const std::string& url, unsigned int localPort)
    : Request(url, localPort), m_username(username), m_password(password)
{
}

Request::Type UserpwdRequest::getType() const
{
    return Type::user_pwd;
}

std::string UserpwdRequest::getUsername() const
{
    return m_username;
}

std::string UserpwdRequest::getPassword() const
{
    return m_password;
}
} // namespace curlex
