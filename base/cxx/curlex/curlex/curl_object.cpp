#include "curl_object.h"

#include <chrono>
#include <mutex>
#include <string.h>

namespace curlex
{
std::mutex g_mutexObjectCount;
int g_objectCount = 0; /* 全局对象个数 */

/**
 * @brief Curl对象被创建
 */
void onCurlObjectCreate()
{
    std::lock_guard<std::mutex> locker(g_mutexObjectCount);
    if (0 == g_objectCount)
    {
        auto code = curl_global_init(CURL_GLOBAL_ALL);
        if (CURLE_OK != code)
        {
            perror("curl_global_init failed!");
            return;
        }
    }
    g_objectCount += 1;
}

/**
 * @brief Curl对象被销毁
 */
void onCurlObejctDestroy()
{
    std::lock_guard<std::mutex> locker(g_mutexObjectCount);
    g_objectCount -= 1;
    if (g_objectCount <= 0)
    {
        g_objectCount = 0;
        curl_global_cleanup();
    }
}

/**
 * @brief 读取要发送到服务器的数据并写入缓冲区
 * @param buffer 数据要写入的缓冲区
 * @param size 数据单位大小(字节)
 * @param number 数据单位个数
 * @param userdata 用户数据
 * @return 实际处理数据大小
 */
size_t onSendDataFunc(void* buffer, size_t size, size_t number, void* userdata)
{
    auto obj = static_cast<CurlObject::SendObject*>(userdata);
    if (!obj)
    {
        return 0;
    }
    return obj->read(buffer, size * number);
}

/**
 * @brief 收到服务器下发的数据
 * @param buffer 收到的数据流缓冲区
 * @param size 每单位的大小
 * @param number 总共几个单位
 * @param userdata 用户数据
 * @return 真正写的字节长度
 */
size_t onRecvDataFunc(void* buffer, size_t size, size_t number, void* userdata)
{
    auto func = static_cast<CurlRecvFunc*>(userdata);
    if (!func)
    {
        return 0;
    }
    return (*func)(buffer, size * number);
}

/**
 * @brief 接收服务器的响应头数据并解析
 * @param buffer 收到的头部数据缓冲区
 * @param size 数据单位大小(字节)
 * @param number 数据单位个数
 * @param userdata 用户数据
 * @return 实际处理的数据大小
 */
size_t onResponseHeaderFunc(void* buffer, size_t size, size_t number, void* userdata)
{
    auto headerMap = static_cast<std::map<std::string, std::string>*>(userdata);
    if (!headerMap)
    {
        return 0;
    }
    auto count = size * number;
    std::string header(static_cast<char*>(buffer), count);
    auto flagPos = header.find(':');
    if (std::string::npos != flagPos)
    {
        auto key = header.substr(0, flagPos);
        auto endPos = header.find("\r\n");
        std::string value;
        if (std::string::npos != endPos && flagPos + 2 < endPos)
        {
            value = header.substr(flagPos + 2, endPos - flagPos - 2);
        }
        headerMap->insert(std::make_pair(key, value));
    }
    return count;
}

/**
 * @brief 进度函数
 * @param userdata 用户数据
 * @param totalDownload 需要下载的总大小
 * @param nowDownloaded 当前已下载的大小
 * @param totalUpload 需要上传的总大小
 * @param nowUploaded 当前已上传的大小
 * @return 0-继续, 1-停止
 */
int onProgressFunc(void* userdata, int64_t totalDownload, int64_t nowDownloaded, int64_t totalUpload, int64_t nowUploaded)
{
    auto progressObject = static_cast<CurlObject::ProgressObject*>(userdata);
    if (!progressObject)
    {
        return 0;
    }
    double uploadSpeed = 0, downloadSpeed = 0;
    if (progressObject->curl)
    {
        curl_easy_getinfo(progressObject->curl, CURLINFO_SPEED_UPLOAD, &uploadSpeed);
        curl_easy_getinfo(progressObject->curl, CURLINFO_SPEED_DOWNLOAD, &downloadSpeed);
    }
    if (!progressObject->func)
    {
        return 0;
    }
    if (progressObject->func(nowUploaded, totalUpload, uploadSpeed, nowDownloaded, totalDownload, downloadSpeed))
    {
        return 1; /* 停止, curl码将为42, 错误信息为"Callback aborted" */
    }
    return 0;
}

/**
 * @brief 调试函数
 * @param handle curl指针
 * @param type 信息类型
 * @param data 信息内容
 * @param size 内容大小
 * @param userdata 用户数据
 * @return 0
 */
int onDebugFunc(CURL* handle, curl_infotype type, char* data, size_t size, void* userdata)
{
    auto func = static_cast<CurlDebugFunc*>(userdata);
    if (func)
    {
        (*func)(type, std::string(data, size));
    }
    return 0;
}

bool CurlObject::SendObject::reset(const std::string& data)
{
    if (data.empty())
    {
        return false;
    }
    m_data = data;
    m_length = data.size();
    m_readed = 0;
    return true;
}

size_t CurlObject::SendObject::read(void* dest, size_t count)
{
    auto left = m_length - m_readed;
    if (!dest || count <= 0 || left <= 0)
    {
        return 0;
    }
    auto total = std::min<size_t>(count, left);
    memcpy(dest, m_data.c_str() + m_readed, total);
    m_readed += total;
    return total;
}

CurlObject::CurlObject()
{
    onCurlObjectCreate();
    m_curl = curl_easy_init();
    if (!m_curl)
    {
        perror("curl_easy_init failed!");
        return;
    }
    if (!initialize())
    {
        curl_easy_cleanup(m_curl);
        m_curl = nullptr;
        perror("curl initialize failed!");
    }
}

CurlObject::CurlObject(const std::string& caFile)
{
    onCurlObjectCreate();
    m_curl = curl_easy_init();
    if (!m_curl)
    {
        perror("curl_easy_init failed!");
        return;
    }
    if (!initialize(caFile))
    {
        curl_easy_cleanup(m_curl);
        m_curl = nullptr;
        perror("curl initialize failed!");
    }
}

CurlObject::CurlObject(const std::string& certFile, const std::string& privateKeyFile, const std::string& privateKeyFilePwd)
{
    onCurlObjectCreate();
    m_curl = curl_easy_init();
    if (!m_curl)
    {
        perror("curl_easy_init failed!");
        return;
    }
    if (!initialize(certFile, privateKeyFile, privateKeyFilePwd))
    {
        curl_easy_cleanup(m_curl);
        m_curl = nullptr;
        perror("curl initialize failed!");
    }
}

CurlObject::CurlObject(const std::string& user, const std::string& password)
{
    onCurlObjectCreate();
    m_curl = curl_easy_init();
    if (!m_curl)
    {
        perror("curl_easy_init failed!");
        return;
    }
    if (!initialize(user, password))
    {
        curl_easy_cleanup(m_curl);
        m_curl = nullptr;
        perror("curl initialize failed!");
    }
}

CurlObject::~CurlObject(void)
{
    if (m_httpPost)
    {
        curl_formfree(m_httpPost);
        m_httpPost = nullptr;
    }
    m_lastPost = nullptr;
    if (m_headers)
    {
        curl_slist_free_all(m_headers);
        m_headers = nullptr;
    }
    if (m_curl)
    {
        curl_easy_cleanup(m_curl);
        m_curl = nullptr;
    }
    onCurlObejctDestroy();
}

bool CurlObject::initialize()
{
    memset(m_errorBuffer, 0, CURL_ERROR_SIZE);
    auto code = setOption(CURLOPT_ERRORBUFFER, m_errorBuffer);
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_SSL_VERIFYPEER, 0L);
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_SSL_VERIFYHOST, 0L);
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_NOSIGNAL, 1L);
    return CURLE_OK == code;
}

bool CurlObject::initialize(const std::string& caFile)
{
    memset(m_errorBuffer, 0, CURL_ERROR_SIZE);
    auto code = setOption(CURLOPT_ERRORBUFFER, m_errorBuffer);
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_SSL_VERIFYPEER, 0L); /* 设置不验证服务器证书有效性 */
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_SSL_VERIFYHOST, 0L); /* 设置不检验证书中的主机名和你访问的主机名是否一致 */
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_CAINFO, caFile.c_str());
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_NOSIGNAL, 1L);
    return CURLE_OK == code;
}

bool CurlObject::initialize(const std::string& certFile, const std::string& privateKeyFile, const std::string& privateKeyFilePwd)
{
    memset(m_errorBuffer, 0, CURL_ERROR_SIZE);
    auto code = setOption(CURLOPT_ERRORBUFFER, m_errorBuffer);
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_SSL_VERIFYPEER, 0L); /* 设置不验证服务器证书有效性 */
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_SSL_VERIFYHOST, 0L); /* 设置不检验证书中的主机名和你访问的主机名是否一致 */
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_SSLCERTTYPE, "PEM");
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_SSLCERT, certFile.c_str());
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_SSLKEYTYPE, "PEM");
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_SSLKEY, privateKeyFile.c_str());
    if (CURLE_OK != code)
    {
        return false;
    }
    if (!privateKeyFilePwd.empty())
    {
        code = setOption(CURLOPT_KEYPASSWD, privateKeyFilePwd.c_str());
        if (CURLE_OK != code)
        {
            return false;
        }
    }
    code = setOption(CURLOPT_NOSIGNAL, 1L);
    return CURLE_OK == code;
}

bool CurlObject::initialize(const std::string& user, const std::string& password)
{
    memset(m_errorBuffer, 0, CURL_ERROR_SIZE);
    auto code = setOption(CURLOPT_ERRORBUFFER, m_errorBuffer);
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_SSL_VERIFYPEER, 0L);
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_SSL_VERIFYHOST, 0L);
    if (CURLE_OK != code)
    {
        return false;
    }
    auto userpwd = user + ":" + password;
    code = setOption(CURLOPT_USERPWD, userpwd.c_str());
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_NOSIGNAL, 1L);
    return CURLE_OK == code;
}

bool CurlObject::isValid(void) const
{
    return m_curl ? true : false;
}

bool CurlObject::setUrl(const std::string& url)
{
    if (url.empty())
    {
        return false;
    }
    /* the second parameter must use type: const char* */
    auto code = setOption(CURLOPT_URL, url.c_str());
    return CURLE_OK == code;
}

bool CurlObject::setEnableRedirect(int maxRedirects)
{
    auto code = setOption(CURLOPT_FOLLOWLOCATION, 1L);
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_MAXREDIRS, maxRedirects);
    return CURLE_OK == code;
}

bool CurlObject::setConnectTimeout(size_t seconds)
{
    auto code = setOption(CURLOPT_CONNECTTIMEOUT, seconds);
    return CURLE_OK == code;
}

bool CurlObject::setTimeout(size_t seconds)
{
    auto code = setOption(CURLOPT_TIMEOUT, seconds);
    return CURLE_OK == code;
}

bool CurlObject::setKeepAlive(size_t idle, size_t interval)
{
    auto code = setOption(CURLOPT_TCP_KEEPALIVE, 1L);
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_TCP_KEEPIDLE, idle);
    if (CURLE_OK != code)
    {
        return false;
    }
    code = setOption(CURLOPT_TCP_KEEPINTVL, interval);
    if (CURLE_OK != code)
    {
        return false;
    }
    return true;
}

bool CurlObject::setHeaders(const std::map<std::string, std::string>& headers)
{
    if (headers.empty())
    {
        return false;
    }
    /* append custom headers one by one */
    auto iter = headers.begin();
    for (; headers.end() != iter; ++iter)
    {
        /* the second parameter must use type: const char* */
        m_headers = curl_slist_append(m_headers, (iter->first + ":" + iter->second).c_str());
    }
    return true;
}

bool CurlObject::setCookieFile(const std::string& filename)
{
    if (!filename.empty())
    {
        auto code = setOption(CURLOPT_COOKIEFILE, filename.c_str());
        if (CURLE_OK != code)
        {
            return false;
        }
        code = setOption(CURLOPT_COOKIEJAR, filename.c_str());
        if (CURLE_OK != code)
        {
            return false;
        }
    }
    return true;
}

bool CurlObject::setRecvFunc(const CurlRecvFunc& func)
{
    if (!func)
    {
        return false;
    }
    m_recvFunc = func;
    return true;
}

bool CurlObject::setProgressFunc(CurlProgressFunc func)
{
    if (!func)
    {
        return false;
    }
    m_progressObject.curl = m_curl;
    m_progressObject.func = func;
    return true;
}

bool CurlObject::setDebugFunc(const CurlDebugFunc& func)
{
    if (!func)
    {
        return false;
    }
    m_debugFunc = func;
    return true;
}

bool CurlObject::setResumeOffset(int64_t index)
{
    if (index <= 0)
    {
        return false;
    }
    auto code = setOption(CURLOPT_RESUME_FROM, index);
    return CURLE_OK == code;
}

bool CurlObject::setRawData(const std::vector<char>& bytes)
{
    if (bytes.empty())
    {
        return false;
    }
    if (!m_sendObject.reset(std::string(bytes.begin(), bytes.end())))
    {
        return false;
    }
    auto code = setOption(CURLOPT_INFILESIZE_LARGE, bytes.size());
    if (CURLE_OK != code)
    {
        return false;
    }
    m_headers = curl_slist_append(m_headers, "Content-Type:application/octet-stream");
    return true;
}

bool CurlObject::setFormData(const std::map<std::string, std::string>& fieldMap)
{
    if (fieldMap.empty())
    {
        return false;
    }
    std::string data;
    for (auto iter = fieldMap.begin(); fieldMap.end() != iter; ++iter)
    {
        if (!data.empty())
        {
            data.append("&");
        }
        data.append(iter->first).append("=").append(iter->second);
    }
    if (!m_sendObject.reset(data))
    {
        return false;
    }
    auto code = setOption(CURLOPT_INFILESIZE_LARGE, data.size());
    if (CURLE_OK != code)
    {
        return false;
    }
    return true;
}

bool CurlObject::addMultipartFormText(const std::string& fieldName, const std::string& text, const std::string& contentType)
{
    if (!m_curl)
    {
        return false;
    }
    if (fieldName.empty())
    {
        return false;
    }
    if (contentType.empty())
    {
        auto code = curl_formadd(&m_httpPost, &m_lastPost, CURLFORM_COPYNAME, fieldName.c_str(), CURLFORM_COPYCONTENTS, text.c_str(),
                                 CURLFORM_CONTENTSLENGTH, text.size(), CURLFORM_END);
        return CURL_FORMADD_OK == code;
    }
    auto code = curl_formadd(&m_httpPost, &m_lastPost, CURLFORM_COPYNAME, fieldName.c_str(), CURLFORM_COPYCONTENTS, text.c_str(),
                             CURLFORM_CONTENTSLENGTH, text.size(), CURLFORM_CONTENTTYPE, contentType.c_str(), CURLFORM_END);
    return CURL_FORMADD_OK == code;
}

bool CurlObject::addMultipartFormFile(const std::string& fieldName, const std::string& filename)
{
    if (!m_curl)
    {
        return false;
    }
    if (fieldName.empty() || filename.empty())
    {
        return false;
    }
    auto code = curl_formadd(&m_httpPost, &m_lastPost, CURLFORM_COPYNAME, fieldName.c_str(), CURLFORM_FILE, filename.c_str(), CURLFORM_END);
    return CURL_FORMADD_OK == code;
}

bool CurlObject::perform(std::string& localIp, unsigned int& localPort, std::string& remoteIp, unsigned int& remotePort, int& curlCode,
                         std::string& errorDesc, int& respCode, std::map<std::string, std::string>& respHeaders, int& respElapsed)
{
    localIp.clear();
    localPort = 0;
    remoteIp.clear();
    remotePort = 0;
    curlCode = -1;
    errorDesc.clear();
    respCode = -1;
    respHeaders.clear();
    respElapsed = 0;
    auto beg = std::chrono::steady_clock::now();
    if (!m_curl)
    {
        respElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg).count();
        return false;
    }
    CURLcode code;
    do
    {
        code = setOption(CURLOPT_READFUNCTION, onSendDataFunc);
        if (CURLE_OK != code)
        {
            break;
        }
        code = setOption(CURLOPT_READDATA, &m_sendObject);
        if (CURLE_OK != code)
        {
            break;
        }
        code = setOption(CURLOPT_WRITEFUNCTION, onRecvDataFunc);
        if (CURLE_OK != code)
        {
            break;
        }
        if (m_recvFunc)
        {
            code = setOption(CURLOPT_WRITEDATA, &m_recvFunc);
            if (CURLE_OK != code)
            {
                break;
            }
        }
        code = setOption(CURLOPT_HEADERFUNCTION, onResponseHeaderFunc);
        if (CURLE_OK != code)
        {
            break;
        }
        code = setOption(CURLOPT_HEADERDATA, &respHeaders);
        if (CURLE_OK != code)
        {
            break;
        }
        code = setOption(CURLOPT_NOPROGRESS, 0L);
        if (CURLE_OK != code)
        {
            break;
        }
        code = setOption(CURLOPT_XFERINFOFUNCTION, onProgressFunc);
        if (CURLE_OK != code)
        {
            break;
        }
        code = setOption(CURLOPT_XFERINFODATA, &m_progressObject);
        if (CURLE_OK != code)
        {
            break;
        }
        if (m_debugFunc)
        {
            code = setOption(CURLOPT_VERBOSE, 1L);
            if (CURLE_OK != code)
            {
                break;
            }
            code = setOption(CURLOPT_DEBUGFUNCTION, onDebugFunc);
            if (CURLE_OK != code)
            {
                break;
            }
            code = setOption(CURLOPT_DEBUGDATA, &m_debugFunc);
            if (CURLE_OK != code)
            {
                break;
            }
        }
        if (m_headers)
        {
            code = setOption(CURLOPT_HTTPHEADER, m_headers);
            if (CURLE_OK != code)
            {
                break;
            }
        }
        if (m_httpPost)
        {
            code = setOption(CURLOPT_HTTPPOST, m_httpPost);
            if (CURLE_OK != code)
            {
                curl_formfree(m_httpPost);
                m_httpPost = nullptr;
                m_lastPost = nullptr;
                break;
            }
        }
    } while (0);
    if (CURLE_OK != code)
    {
        curlCode = static_cast<int>(code);
        errorDesc = m_errorBuffer;
        respElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg).count();
        return false;
    }
    code = curl_easy_perform(m_curl);
    char* szLocalIp = nullptr;
    if (CURLE_OK == curl_easy_getinfo(m_curl, CURLINFO_LOCAL_IP, &szLocalIp) && szLocalIp)
    {
        localIp = szLocalIp;
    }
    curl_easy_getinfo(m_curl, CURLINFO_LOCAL_PORT, &localPort);
    char* szRemoteIp = nullptr;
    if (CURLE_OK == curl_easy_getinfo(m_curl, CURLINFO_PRIMARY_IP, &szRemoteIp) && szRemoteIp)
    {
        remoteIp = szRemoteIp;
    }
    curl_easy_getinfo(m_curl, CURLINFO_PRIMARY_PORT, &remotePort);
    curlCode = static_cast<int>(code);
    errorDesc = m_errorBuffer;
    curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &respCode);
    if (m_httpPost)
    {
        curl_formfree(m_httpPost);
        m_httpPost = nullptr;
    }
    m_lastPost = nullptr;
    respElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg).count();
    return CURLE_OK == code;
}
} // namespace curlex
