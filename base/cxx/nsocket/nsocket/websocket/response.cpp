#include "response.h"

#include "base64/base64.h"
#include "sha1/sha1.h"

namespace nsocket
{
namespace ws
{
void Response::create(std::vector<unsigned char>& data, const std::string& secWebSocketKey)
{
    static const std::string FIRST_LINE = "HTTP/1.1 101 Switching Protocols";
    static const std::string CRLF = "\r\n";
    static const std::string SEP = ": ";
    data.clear();
    data.insert(data.end(), FIRST_LINE.begin(), FIRST_LINE.end());
    data.insert(data.end(), CRLF.begin(), CRLF.end());
    auto iter = headers.find("Upgrade");
    if (headers.end() == iter)
    {
        headers.insert(std::make_pair("Upgrade", "websocket"));
    }
    iter = headers.find("Connection");
    if (headers.end() == iter)
    {
        headers.insert(std::make_pair("Connection", "Upgrade"));
    }
    iter = headers.find("Sec-WebSocket-Accept");
    if (headers.end() == iter)
    {
        headers.insert(std::make_pair("Sec-WebSocket-Accept", calcSecWebSocketAccept(secWebSocketKey)));
    }
    for (auto iter = headers.begin(); headers.end() != iter; ++iter)
    {
        data.insert(data.end(), iter->first.begin(), iter->first.end());
        data.insert(data.end(), SEP.begin(), SEP.end());
        data.insert(data.end(), iter->second.begin(), iter->second.end());
        data.insert(data.end(), CRLF.begin(), CRLF.end());
    }
    data.insert(data.end(), CRLF.begin(), CRLF.end());
}

std::string Response::calcSecWebSocketAccept(const std::string& secWebSocketKey)
{
    /* 算法: accept = base64(sha1(key + MAGIC)) */
    static const std::string MAGIC = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::string accept;
    std::string str = secWebSocketKey + MAGIC;
    unsigned char digest[20];
    sha1Sign((const unsigned char*)str.c_str(), str.size(), digest);
    unsigned char* out;
    unsigned int len = base64Encode(digest, sizeof(digest), &out);
    if (out && len > 0)
    {
        accept = (char*)out;
        free(out);
    }
    return accept;
}
} // namespace ws
} // namespace nsocket
