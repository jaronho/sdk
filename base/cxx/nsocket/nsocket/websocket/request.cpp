#include "request.h"

#include <random>

#include "base64/base64.h"

namespace nsocket
{
namespace ws
{
static const std::vector<std::string> METHOD_NAMES = {"GET"};
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

int Request::parse(const unsigned char* data, int length, const HEAD_CALLBACK& headCb)
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
            if ((used = parseHeader(remainData, remainLen, headCb)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::frame:
            return totalUsed;
        }
        totalUsed += used;
    }
    return totalUsed;
}

bool Request::isParseEnd()
{
    return (ParseStep::frame == m_parseStep);
}

int Request::getSecWebSocketVersion()
{
    return m_secWebSocketVersion;
}

std::string Request::getSecWebSocketKey()
{
    return m_secWebSocketKey;
}

void Request::create(Request req, const std::string& hostPort, std::string& secWebSocketKey, std::vector<unsigned char>& data)
{
    static const std::string CRLF = "\r\n";
    static const std::string SEP = ": ";
    secWebSocketKey.clear();
    data.clear();
    std::string firstLine;
    firstLine += "GET " + (req.uri.empty() ? "/" : req.uri);
    if (!req.queries.empty())
    {
        firstLine += "?";
        bool firstQuery = true;
        for (auto iter = req.queries.begin(); req.queries.end() != iter; ++iter)
        {
            if (firstQuery)
            {
                firstQuery = false;
            }
            else
            {
                firstLine += "&";
            }
            firstLine += iter->first + "=" + iter->second;
        }
    }
    firstLine += " HTTP/1.1";
    data.insert(data.end(), firstLine.begin(), firstLine.end());
    data.insert(data.end(), CRLF.begin(), CRLF.end());
    auto iter = req.headers.find("Upgrade");
    if (req.headers.end() == iter)
    {
        req.headers.insert(std::make_pair("Upgrade", "websocket"));
    }
    iter = req.headers.find("Connection");
    if (req.headers.end() == iter)
    {
        req.headers.insert(std::make_pair("Connection", "Upgrade"));
    }
    iter = req.headers.find("Sec-WebSocket-Version");
    if (req.headers.end() == iter)
    {
        req.headers.insert(std::make_pair("Sec-WebSocket-Version", "13"));
    }
    iter = req.headers.find("Sec-WebSocket-Key");
    if (req.headers.end() == iter)
    {
        secWebSocketKey = calcSecWebSocketKey();
        req.headers.insert(std::make_pair("Sec-WebSocket-Key", secWebSocketKey));
    }
    else
    {
        secWebSocketKey = iter->second;
    }
    iter = req.headers.find("Host");
    if (req.headers.end() == iter && !hostPort.empty())
    {
        req.headers.insert(std::make_pair("Host", hostPort));
    }
    for (auto iter = req.headers.begin(); req.headers.end() != iter; ++iter)
    {
        data.insert(data.end(), iter->first.begin(), iter->first.end());
        data.insert(data.end(), SEP.begin(), SEP.end());
        data.insert(data.end(), iter->second.begin(), iter->second.end());
        data.insert(data.end(), CRLF.begin(), CRLF.end());
    }
    data.insert(data.end(), CRLF.begin(), CRLF.end());
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
                queries.insert(std::make_pair(m_tmpKey, percentDecode(m_tmpValue)));
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

int Request::parseHeader(const unsigned char* data, int length, const HEAD_CALLBACK& headCb)
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
                    clearTmp();
                    m_sepFlag = SepFlag::none;
                }
            }
            else if (SepFlag::rnr == m_sepFlag)
            {
                if (!m_tmpKey.empty())
                {
                    headers.insert(std::make_pair(m_tmpKey, m_tmpValue));
                }
                if (!checkHeader())
                {
                    return 0;
                }
                clearTmp();
                m_sepFlag = SepFlag::none;
                m_parseStep = ParseStep::frame;
                if (headCb)
                {
                    headCb();
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

bool Request::checkHeader()
{
    auto iter = headers.find("Connection");
    if (headers.end() == iter || iter->second.empty() || !case_insensitive_contains(iter->second, "Upgrade"))
    {
        return false;
    }
    iter = headers.find("Upgrade");
    if (headers.end() == iter || !case_insensitive_equal("websocket", iter->second))
    {
        return false;
    }
    iter = headers.find("Sec-WebSocket-Version");
    if (headers.end() == iter || iter->second.empty())
    {
        return false;
    }
    try
    {
        m_secWebSocketVersion = atoi(iter->second.c_str());
    }
    catch (...)
    {
        m_secWebSocketVersion = 0;
    }
    iter = headers.find("Sec-WebSocket-Key");
    if (headers.end() == iter || iter->second.empty())
    {
        return false;
    }
    m_secWebSocketKey = iter->second;
    return true;
}

void Request::clearTmp()
{
    m_tmpKeyFlag = true;
    m_tmpKey.clear();
    m_tmpValue.clear();
}

std::string Request::calcSecWebSocketKey()
{
    std::string nonce;
    nonce.reserve(16);
    std::uniform_int_distribution<unsigned short> dist(0, 255);
    std::random_device rd;
    for (int i = 0; i < 16; ++i)
    {
        nonce += (char)(dist(rd));
    }
    std::string secWebSocketKey;
    unsigned char* out;
    unsigned int len = Base64::encode((const unsigned char*)nonce.c_str(), nonce.size(), &out);
    if (out && len > 0)
    {
        secWebSocketKey = (char*)out;
        free(out);
    }
    return secWebSocketKey;
}
} // namespace ws
} // namespace nsocket
