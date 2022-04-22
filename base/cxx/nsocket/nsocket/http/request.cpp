#include "request.h"

#include <math.h>
#include <string.h>
#include <vector>

namespace nsocket
{
namespace http
{
static const std::vector<Method> METHOD_LIST = {Method::CONNECT, Method::DELETE, Method::GET,     Method::HEAD, Method::PATCH,
                                                Method::POST,    Method::PUT,    Method::OPTIONS, Method::TRACE};
static const std::vector<std::string> VERSION_NAMES = {"HTTP/0.9", "HTTP/1.0", "HTTP/1.1", "HTTP/2"};

std::string method_desc(const Method& method)
{
    switch (method)
    {
    case Method::CONNECT:
        return "CONNECT";
    case Method::DELETE:
        return "DELETE";
    case Method::GET:
        return "GET";
    case Method::HEAD:
        return "HEAD";
    case Method::PATCH:
        return "PATCH";
    case Method::POST:
        return "POST";
    case Method::PUT:
        return "PUT";
    case Method::OPTIONS:
        return "OPTIONS";
    case Method::TRACE:
        return "TRACE";
    }
    return std::to_string((int)method);
}

std::string url_encode(const std::string& src)
{
    static const char* HEX_CHARS = "0123456789ABCDEF";
    std::string dst;
    dst.reserve(src.size()); /* 设置最小长度 */
    for (const auto& ch : src)
    {
        if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || strchr("-_.~!*'();:@&=+$,/?#[]", ch))
        {
            dst += ch;
        }
        else
        {
            dst += "%";
            dst.push_back(HEX_CHARS[(unsigned char)ch >> 4]);
            dst.push_back(HEX_CHARS[(unsigned char)ch & 15]);
        }
    }
    return dst;
}

std::string url_decode(const std::string& src)
{
    std::string dst;
    dst.reserve(src.size() / 3 + (src.size() % 3)); /* 设置最小长度 */
    for (size_t i = 0; i < src.size(); ++i)
    {
        const auto& ch = src[i];
        if ('%' == ch && i + 2 < src.size())
        {
            auto hex = src.substr(i + 1, 2);
            auto decodedCh = (char)std::strtol(hex.c_str(), nullptr, 16);
            dst += decodedCh;
            i += 2;
        }
        else if ('+' == ch)
        {
            dst += ' ';
        }
        else
        {
            dst += ch;
        }
    }
    return dst;
}

/**
 * @breif 十六进制转十进制
 */
static unsigned int hex2dec(const std::string& hexStr)
{
    unsigned int dec = 0;
    size_t len = hexStr.size();
    size_t start = 0;
    if (len >= 3)
    {
        const char& ch2 = hexStr.at(1);
        if ('x' == ch2 || 'X' == ch2)
        {
            if ('0' != hexStr.at(0))
            {
                return 0;
            }
            start = 2;
        }
        else if ((ch2 < '0' || ch2 > '9') && (ch2 < 'A' || ch2 > 'F') && (ch2 < 'a' || ch2 > 'f'))
        {
            return 0;
        }
    }
    for (size_t i = start; i < len; ++i)
    {
        const char& ch = hexStr.at(i);
        if (ch >= '0' && ch <= '9')
        {
            dec += ((unsigned int)(ch)-48) * (unsigned int)(pow(16, len - i - 1));
        }
        else if (ch >= 'A' && ch <= 'F')
        {
            dec += ((unsigned int)(ch)-55) * (unsigned int)(pow(16, len - i - 1));
        }
        else if (ch >= 'a' && ch <= 'f')
        {
            dec += ((unsigned int)(ch)-87) * (unsigned int)(pow(16, len - i - 1));
        }
        else
        {
            return 0;
        }
    }
    return dec;
}

int Request::parse(const unsigned char* data, int length, const HEAD_CALLBACK& headCb, const CONTENT_CALLBACK& contentCb,
                   const FINISH_CALLBACK& finishCb)
{
    if (!data || length <= 0)
    {
        return 0;
    }
    int totalUsed = 0;
    while (totalUsed < length)
    {
        const unsigned char* remainData = data + totalUsed;
        int remainLen = length - totalUsed;
        int used = 0;
        switch (m_parseStep)
        {
        case ParseStep::method: /* 解析方法 */
            if ((used = parseMethod(remainData, remainLen)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::uri: /* 解析URI */
            if ((used = parseUri(remainData, remainLen)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::queries: /* 解析参数 */
            if ((used = parseQueries(remainData, remainLen)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::version: /* 解析版本 */
            if ((used = parseVersion(remainData, remainLen)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::header: /* 解析头部 */
            if ((used = parseHeader(remainData, remainLen, headCb, finishCb)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::chunk: /* 解析分块内容 */
            if ((used = parseChunk(remainData, remainLen, contentCb, finishCb)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::unchunk: /* 解析非分块内容 */
            if ((used = parseUnchunk(remainData, remainLen, contentCb, finishCb)) <= 0)
            {
                return 0;
            }
            break;
        }
        totalUsed += used;
    }
    return totalUsed;
}

bool Request::isTransferEncodingChunked()
{
    return m_transferEncodingChunked;
}

std::string Request::getContentType()
{
    return m_contentType;
}

size_t Request::getContentLength()
{
    return m_contentLength;
}

void Request::reset()
{
    method.clear();
    uri.clear();
    queries.clear();
    version.clear();
    headers.clear();
    m_sepFlag = SepFlag::none;
    m_parseStep = ParseStep::method;
    m_tmpKeyFlag = true;
    m_tmpKey.clear();
    m_tmpValue.clear();
    m_transferEncodingChunked = false;
    m_chunkParseFlag = true;
    m_chunkSizeHex.clear();
    m_chunkSize = 0;
    m_chunkReceived = 0;
    m_contentType.clear();
    m_contentLength = 0;
    m_contentReceived = 0;
}

int Request::parseMethod(const unsigned char* data, int length)
{
    int used = 0;
    for (; used < length; ++used)
    {
        const auto& ch = data[used];
        if (' ' == ch)
        {
            if (checkMethod())
            {
                m_parseStep = ParseStep::uri;
                return (used + 1);
            }
            return 0;
        }
        else
        {
            method.push_back(ch);
            if (method.size() > maxMethodLength()) /* 方法名的长度不合法 */
            {
                return 0;
            }
        }
    }
    return used;
}

int Request::parseUri(const unsigned char* data, int length)
{
    int used = 0;
    for (; used < length; ++used)
    {
        const auto& ch = data[used];
        if ('?' == ch)
        {
            m_parseStep = ParseStep::queries;
            return (used + 1);
        }
        else if (' ' == ch)
        {
            m_parseStep = ParseStep::version;
            return (used + 1);
        }
        else
        {
            if (uri.empty() && '/' != ch) /* 第1个字符必须为'/' */
            {
                return 0;
            }
            uri.push_back(ch);
        }
    }
    return used;
}

int Request::parseQueries(const unsigned char* data, int length)
{
    int used = 0;
    for (; used < length; ++used)
    {
        const auto& ch = data[used];
        if (' ' == ch)
        {
            if (!m_tmpKey.empty())
            {
                queries.insert(std::make_pair(url_decode(m_tmpKey), url_decode(m_tmpValue)));
            }
            clearTmp();
            m_parseStep = ParseStep::version;
            return (used + 1);
        }
        else
        {
            if ('=' == ch)
            {
                if (m_tmpKey.empty())
                {
                    return 0;
                }
                if (m_tmpKeyFlag)
                {
                    m_tmpKeyFlag = false;
                }
                else
                {
                    m_tmpValue.push_back(ch);
                }
            }
            else if ('&' == ch)
            {
                if (m_tmpKey.empty())
                {
                    return 0;
                }
                queries.insert(std::make_pair(url_decode(m_tmpKey), url_decode(m_tmpValue)));
                clearTmp();
            }
            else
            {
                if (m_tmpKeyFlag)
                {
                    m_tmpKey.push_back(ch);
                }
                else
                {
                    m_tmpValue.push_back(ch);
                }
            }
        }
    }
    return used;
}

int Request::parseVersion(const unsigned char* data, int length)
{
    int used = 0;
    for (; used < length; ++used)
    {
        const auto& ch = data[used];
        if ('\r' == ch)
        {
            if (SepFlag::none == m_sepFlag)
            {
                m_sepFlag = SepFlag::r;
            }
            else
            {
                return 0;
            }
        }
        else if ('\n' == ch)
        {
            if (SepFlag::r == m_sepFlag)
            {
                if (checkVersion())
                {
                    m_sepFlag = SepFlag::none;
                    m_parseStep = ParseStep::header;
                    return (used + 1);
                }
            }
            return 0;
        }
        else
        {
            if (SepFlag::none == m_sepFlag)
            {
                version.push_back(ch);
                if (version.size() > maxVersionLength()) /* 版本号的长度不合法 */
                {
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
    }
    return used;
}

int Request::parseHeader(const unsigned char* data, int length, const HEAD_CALLBACK& headCb, const FINISH_CALLBACK& finishCb)
{
    int used = 0;
    for (; used < length; ++used)
    {
        const auto& ch = data[used];
        if ('\r' == ch)
        {
            if (SepFlag::none == m_sepFlag)
            {
                m_sepFlag = SepFlag::r;
            }
            else if (SepFlag::rn == m_sepFlag)
            {
                m_sepFlag = SepFlag::rnr;
            }
            else
            {
                return 0;
            }
        }
        else if ('\n' == ch)
        {
            if (SepFlag::r == m_sepFlag)
            {
                if (used + 1 < length && '\r' == data[used + 1]) /* 下一个也是'\r' */
                {
                    m_sepFlag = SepFlag::rn;
                }
                else
                {
                    if (m_tmpKey.empty())
                    {
                        return 0;
                    }
                    headers.insert(std::make_pair(m_tmpKey, m_tmpValue));
                    parseContentTypeAndLength();
                    clearTmp();
                    m_sepFlag = SepFlag::none;
                }
            }
            else if (SepFlag::rnr == m_sepFlag)
            {
                if (!m_tmpKey.empty())
                {
                    headers.insert(std::make_pair(m_tmpKey, m_tmpValue));
                    parseContentTypeAndLength();
                }
                clearTmp();
                m_sepFlag = SepFlag::none;

                if (headCb)
                {
                    headCb();
                }
                if (m_transferEncodingChunked) /* 数据分块 */
                {
                    m_parseStep = ParseStep::chunk;
                }
                else /* 数据非分块 */
                {
                    m_parseStep = ParseStep::unchunk;
                    if (0 == m_contentLength) /* 无内容 */
                    {
                        if (finishCb)
                        {
                            finishCb();
                        }
                        reset();
                    }
                }
                return (used + 1);
            }
            else
            {
                return 0;
            }
        }
        else if (':' == ch)
        {
            if (m_tmpKey.empty())
            {
                return 0;
            }
            if (m_tmpKeyFlag)
            {
                m_tmpKeyFlag = false;
            }
            else
            {
                m_tmpValue.push_back(ch);
            }
        }
        else
        {
            switch (m_sepFlag)
            {
            case SepFlag::none:
                if (' ' != ch)
                {
                    if (m_tmpKeyFlag)
                    {
                        m_tmpKey.push_back(ch);
                    }
                    else
                    {
                        m_tmpValue.push_back(ch);
                    }
                }
                break;
            default:
                return 0;
            }
        }
    }
    return used;
}

void Request::parseContentTypeAndLength()
{
    if (case_insensitive_equal("Transfer-Encoding", m_tmpKey)) /* 内容分块 */
    {
        if (case_insensitive_equal("chunked", m_tmpValue))
        {
            m_transferEncodingChunked = true;
            m_contentLength = 0; /* 此时忽略`Content-Length`的值 */
        }
    }
    else if (case_insensitive_equal("Content-Type", m_tmpKey)) /* 内容类型 */
    {
        m_contentType = m_tmpValue;
    }
    else if (case_insensitive_equal("Content-Length", m_tmpKey)) /* 内容长度 */
    {
        if (!m_transferEncodingChunked)
        {
            try
            {
                m_contentLength = atoll(m_tmpValue.c_str());
            }
            catch (...)
            {
                m_contentLength = 0;
            }
        }
    }
}

int Request::parseChunk(const unsigned char* data, int length, const CONTENT_CALLBACK& contentCb, const FINISH_CALLBACK& finishCb)
{
    int used = 0;
    if (m_chunkParseFlag) /* 解析分块头 */
    {
        for (; used < length; ++used)
        {
            const auto& ch = data[used];
            if ('\r' == ch)
            {
                if (SepFlag::none == m_sepFlag)
                {
                    m_sepFlag = SepFlag::r;
                }
                else if (SepFlag::rn == m_sepFlag)
                {
                    m_sepFlag = SepFlag::rnr;
                }
                else
                {
                    return 0;
                }
            }
            else if ('\n' == ch)
            {
                if (SepFlag::r == m_sepFlag)
                {
                    m_chunkSize = hex2dec(m_chunkSizeHex);
                    m_chunkSizeHex.clear();
                    m_contentLength += m_chunkSize;
                    if (0 == m_chunkSize)
                    {
                        m_sepFlag = SepFlag::rn;
                    }
                    else
                    {
                        m_sepFlag = SepFlag::none;
                        m_chunkParseFlag = false;
                        return (used + 1);
                    }
                }
                else if (SepFlag::rnr == m_sepFlag) /* 所有分块结束 */
                {
                    if (finishCb)
                    {
                        finishCb();
                    }
                    reset();
                    return (used + 1);
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                m_chunkSizeHex.push_back(ch);
            }
        }
    }
    else /* 解析分块数据 */
    {
        if (m_chunkReceived < m_chunkSize)
        {
            used = length;
            if (m_chunkReceived + used > m_chunkSize)
            {
                used = m_chunkSize - m_chunkReceived;
            }
            if (contentCb)
            {
                contentCb(m_contentReceived, data, used);
            }
            m_chunkReceived += used;
            m_contentReceived += used;
        }
        else /* 单个分块数据都已接收完毕 */
        {
            for (; used < length; ++used)
            {
                const auto& ch = data[used];
                if ('\r' == ch)
                {
                    if (SepFlag::none == m_sepFlag)
                    {
                        m_sepFlag = SepFlag::r;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else if ('\n' == ch)
                {
                    if (SepFlag::r == m_sepFlag)
                    {
                        m_sepFlag = SepFlag::none;
                        m_chunkSize = 0;
                        m_chunkReceived = 0;
                        m_chunkParseFlag = true;
                        return (used + 1);
                    }
                    else
                    {
                        return 0;
                    }
                }
            }
        }
    }
    return used;
}

int Request::parseUnchunk(const unsigned char* data, int length, const CONTENT_CALLBACK& contentCb, const FINISH_CALLBACK& finishCb)
{
    int used = length;
    if (m_contentReceived + used > m_contentLength)
    {
        used = m_contentLength - m_contentReceived;
    }
    if (contentCb)
    {
        contentCb(m_contentReceived, data, used);
    }
    m_contentReceived += used;
    if (m_contentReceived == m_contentLength) /* 数据都已接收完毕 */
    {
        if (finishCb)
        {
            finishCb();
        }
        reset();
    }
    return used;
}

int Request::maxMethodLength()
{
    static int s_maxLength = 0;
    if (0 == s_maxLength)
    {
        for (size_t i = 0; i < METHOD_LIST.size(); ++i)
        {
            auto methodName = method_desc(METHOD_LIST[i]);
            if (s_maxLength < methodName.size())
            {
                s_maxLength = methodName.size();
            }
        }
    }
    return s_maxLength;
}

bool Request::checkMethod()
{
    for (size_t i = 0; i < METHOD_LIST.size(); ++i)
    {
        if (case_insensitive_equal(method_desc(METHOD_LIST[i]), method))
        {
            return true;
        }
    }
    return false;
}

int Request::maxVersionLength()
{
    static int s_maxLength = 0;
    if (0 == s_maxLength)
    {
        for (size_t i = 0; i < VERSION_NAMES.size(); ++i)
        {
            if (s_maxLength < VERSION_NAMES[i].size())
            {
                s_maxLength = VERSION_NAMES[i].size();
            }
        }
    }
    return s_maxLength;
}

bool Request::checkVersion()
{
    for (size_t i = 0; i < VERSION_NAMES.size(); ++i)
    {
        if (case_insensitive_equal(VERSION_NAMES[i], version))
        {
            return true;
        }
    }
    return false;
}

void Request::clearTmp()
{
    m_tmpKeyFlag = true;
    m_tmpKey.clear();
    m_tmpValue.clear();
}
} // namespace http
} // namespace nsocket
