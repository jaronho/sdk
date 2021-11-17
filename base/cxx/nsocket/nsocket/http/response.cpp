#include "response.h"

namespace nsocket
{
namespace http
{
void Response::create(Response resp, std::vector<unsigned char>& data)
{
    static const std::string CRLF = "\r\n";
    static const std::string SEP = ": ";
    data.clear();
    if (resp.version.empty())
    {
        resp.version = "HTTP/1.1";
    }
    data.insert(data.end(), resp.version.begin(), resp.version.end());
    data.push_back(' ');
    auto statusStr = status_desc(resp.statusCode);
    data.insert(data.end(), statusStr.begin(), statusStr.end());
    data.insert(data.end(), CRLF.begin(), CRLF.end());
    if (resp.headers.end() == resp.headers.find("Content-Length"))
    {
        resp.headers.insert(std::make_pair("Content-Length", "0"));
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
} // namespace http
} // namespace nsocket
