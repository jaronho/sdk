#include "response.h"

#include "base64/base64.h"
#include "sha1/sha1.h"

namespace nsocket
{
namespace ws
{
static const std::string FIRST_LINE = "HTTP/1.1 101 Switching Protocols";

int Response::parse(const unsigned char* data, int length, const std::string& secWebSocketKey, const HEAD_CALLBACK& headCb)
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
        case ParseStep::firstline: /* 解析首行 */
            if ((used = parseFirstLine(remainData, remainLen)) <= 0)
            {
                return 0;
            }
            break;
        case ParseStep::header: /* 解析头部 */
            if ((used = parseHeader(remainData, remainLen, secWebSocketKey, headCb)) <= 0)
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

bool Response::isParseEnd()
{
    return (ParseStep::frame == m_parseStep);
}

void Response::create(Response resp, const std::string& secWebSocketKey, std::vector<unsigned char>& data)
{
    static const std::string CRLF = "\r\n";
    static const std::string SEP = ": ";
    data.clear();
    data.insert(data.end(), FIRST_LINE.begin(), FIRST_LINE.end());
    data.insert(data.end(), CRLF.begin(), CRLF.end());
    auto iter = resp.headers.find("Upgrade");
    if (resp.headers.end() == iter)
    {
        resp.headers.insert(std::make_pair("Upgrade", "websocket"));
    }
    iter = resp.headers.find("Connection");
    if (resp.headers.end() == iter)
    {
        resp.headers.insert(std::make_pair("Connection", "Upgrade"));
    }
    iter = resp.headers.find("Sec-WebSocket-Accept");
    if (resp.headers.end() == iter)
    {
        resp.headers.insert(std::make_pair("Sec-WebSocket-Accept", calcSecWebSocketAccept(secWebSocketKey)));
    }
    for (auto iter = resp.headers.begin(); resp.headers.end() != iter; ++iter)
    {
        data.insert(data.end(), iter->first.begin(), iter->first.end());
        data.insert(data.end(), SEP.begin(), SEP.end());
        data.insert(data.end(), iter->second.begin(), iter->second.end());
        data.insert(data.end(), CRLF.begin(), CRLF.end());
    }
    data.insert(data.end(), CRLF.begin(), CRLF.end());
}

int Response::parseFirstLine(const unsigned char* data, int length)
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
                if (FIRST_LINE == m_firstLine)
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
                m_firstLine.push_back(ch);
            }
            else
            {
                return 0;
            }
        }
    }
    return used;
}

int Response::parseHeader(const unsigned char* data, int length, const std::string& secWebSocketKey, const HEAD_CALLBACK& headCb)
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
                if (!checkHeader(secWebSocketKey))
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

bool Response::checkHeader(const std::string& secWebSocketKey)
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
    iter = headers.find("Sec-WebSocket-Accept");
    if (headers.end() == iter || iter->second.empty())
    {
        return false;
    }
    if (calcSecWebSocketAccept(secWebSocketKey) != iter->second)
    {
        return false;
    }
    return true;
}

void Response::clearTmp()
{
    m_tmpKeyFlag = true;
    m_tmpKey.clear();
    m_tmpValue.clear();
}

std::string Response::calcSecWebSocketAccept(const std::string& secWebSocketKey)
{
    /* 算法: accept = base64(sha1(key + MAGIC)) */
    static const std::string MAGIC = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::string accept;
    std::string str = secWebSocketKey + MAGIC;
    unsigned char digest[20];
    Sha1::sign((const unsigned char*)str.c_str(), str.size(), digest);
    unsigned char* out;
    unsigned int len = Base64::encode(digest, sizeof(digest), &out);
    if (out && len > 0)
    {
        accept = (char*)out;
        free(out);
    }
    return accept;
}
} // namespace ws
} // namespace nsocket
