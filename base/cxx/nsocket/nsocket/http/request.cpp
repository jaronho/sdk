#include "request.h"

#include <vector>

namespace nsocket
{
namespace http
{
static const std::vector<std::string> METHOD_NAMES = {"GET", "HEAD", "POST", "PUT", "DELETE", "TRACE", "OPTIONS", "CONNECT", "PATCH"};
static const std::vector<std::string> VERSION_NAMES = {"HTTP/0.9", "HTTP/1.0", "HTTP/1.1", "HTTP/2"};

/**
 * @breif 百分号编码, URL中传递带%的参数时，那么%就需要进行转义或编码以防止解析URL时造成歧义
 */
static std::string percentEncode(const std::string& value) noexcept
{
    static const char* HEX_CHARS = "0123456789ABCDEF";
    std::string result;
    result.reserve(value.size()); /* 设置最小长度 */
    for (const auto& ch : value)
    {
        if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || '-' == ch || '.' == ch || '_' == ch
            || '~' == ch)
        {
            result += ch;
        }
        else
        {
            result += "%" + HEX_CHARS[(unsigned char)ch >> 4] + HEX_CHARS[(unsigned char)ch & 15];
        }
    }
    return result;
}

/**
 * @breif 百分号解码, URL中传递带%的参数时，那么%就需要进行转义或编码以防止解析URL时造成歧义
 */
static std::string percentDecode(const std::string& value) noexcept
{
    std::string result;
    result.reserve(value.size() / 3 + (value.size() % 3)); /* 设置最小长度 */
    for (size_t i = 0; i < value.size(); ++i)
    {
        const auto& ch = value[i];
        if ('%' == ch && i + 2 < value.size())
        {
            auto hex = value.substr(i + 1, 2);
            auto decodedCh = (char)std::strtol(hex.c_str(), nullptr, 16);
            result += decodedCh;
            i += 2;
        }
        else if ('+' == ch)
        {
            result += ' ';
        }
        else
        {
            result += ch;
        }
    }
    return result;
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
        case ParseStep::METHOD: /* 解析方法 */
            if ((used = parseMethod(remainData, remainLen)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::URI: /* 解析URI */
            if ((used = parseUri(remainData, remainLen)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::QUERIES: /* 解析参数 */
            if ((used = parseQueries(remainData, remainLen)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::VERSION: /* 解析版本 */
            if ((used = parseVersion(remainData, remainLen)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::HEADER: /* 解析头部 */
            if ((used = parseHeader(remainData, remainLen, headCb, finishCb)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::CONTENT: /* 解析内容 */
            if ((used = parseContent(remainData, remainLen, contentCb, finishCb)) <= 0)
            {
                return 0;
            }
            break;
        }
        totalUsed += used;
    }
    return totalUsed;
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
    m_sepFlag = SepFlag::NONE;
    m_parseStep = ParseStep::METHOD;
    m_tmpKeyFlag = true;
    m_tmpKey.clear();
    m_tmpValue.clear();
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
                m_parseStep = ParseStep::URI;
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
            m_parseStep = ParseStep::QUERIES;
            return (used + 1);
        }
        else if (' ' == ch)
        {
            m_parseStep = ParseStep::VERSION;
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
                queries.insert(std::make_pair(m_tmpKey, percentDecode(m_tmpValue)));
            }
            clearTmp();
            m_parseStep = ParseStep::VERSION;
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
                queries.insert(std::make_pair(m_tmpKey, percentDecode(m_tmpValue)));
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
            if (SepFlag::NONE == m_sepFlag)
            {
                m_sepFlag = SepFlag::R;
            }
            else
            {
                return 0;
            }
        }
        else if ('\n' == ch)
        {
            if (SepFlag::R == m_sepFlag)
            {
                if (checkVersion())
                {
                    m_sepFlag = SepFlag::NONE;
                    m_parseStep = ParseStep::HEADER;
                    return (used + 1);
                }
            }
            return 0;
        }
        else
        {
            if (SepFlag::NONE == m_sepFlag)
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
            if (SepFlag::NONE == m_sepFlag)
            {
                m_sepFlag = SepFlag::R;
            }
            else if (SepFlag::RN == m_sepFlag)
            {
                m_sepFlag = SepFlag::RNR;
            }
            else
            {
                return 0;
            }
        }
        else if ('\n' == ch)
        {
            if (SepFlag::R == m_sepFlag)
            {
                if (used + 1 < length && '\r' == data[used + 1]) /* 下一个也是'\r' */
                {
                    m_sepFlag = SepFlag::RN;
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
                    m_sepFlag = SepFlag::NONE;
                }
            }
            else if (SepFlag::RNR == m_sepFlag)
            {
                if (!m_tmpKey.empty())
                {
                    headers.insert(std::make_pair(m_tmpKey, m_tmpValue));
                    parseContentTypeAndLength();
                }
                clearTmp();
                m_sepFlag = SepFlag::NONE;
                m_parseStep = ParseStep::CONTENT;
                if (headCb)
                {
                    headCb();
                }
                if (0 == m_contentLength)
                {
                    if (finishCb)
                    {
                        finishCb();
                    }
                    reset();
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
            case SepFlag::NONE:
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
    if (case_insensitive_equal("Content-Type", m_tmpKey)) /* 内容类型 */
    {
        m_contentType = m_tmpValue;
    }
    else if (case_insensitive_equal("Content-Length", m_tmpKey)) /* 内容长度 */
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

int Request::parseContent(const unsigned char* data, int length, const CONTENT_CALLBACK& contentCb, const FINISH_CALLBACK& finishCb)
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
        for (size_t i = 0; i < METHOD_NAMES.size(); ++i)
        {
            if (s_maxLength < METHOD_NAMES[i].size())
            {
                s_maxLength = METHOD_NAMES[i].size();
            }
        }
    }
    return s_maxLength;
}

bool Request::checkMethod()
{
    for (size_t i = 0; i < METHOD_NAMES.size(); ++i)
    {
        if (case_insensitive_equal(METHOD_NAMES[i], method))
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