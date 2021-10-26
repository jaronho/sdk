#include "response.h"

namespace nsocket
{
namespace http
{
void Response::create(std::vector<unsigned char>& data)
{
    static const std::string CRLF = "\r\n\r\n";
    static const std::string SEP = ": ";
    data.clear();
    data.insert(data.end(), version.begin(), version.end());
    data.push_back(' ');
    auto statusStr = status_desc(statusCode);
    data.insert(data.end(), statusStr.begin(), statusStr.end());
    data.insert(data.end(), CRLF.begin(), CRLF.end());
    for (auto iter = headers.begin(); headers.end() != iter; ++iter)
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
