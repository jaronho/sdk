#include "response.h"

namespace nsocket
{
namespace http
{
void Response::pack(Response resp, std::vector<unsigned char>& data)
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
        resp.headers.insert(std::make_pair("Content-Length", std::to_string(resp.body.size())));
    }
    for (auto iter = resp.headers.begin(); resp.headers.end() != iter; ++iter)
    {
        data.insert(data.end(), iter->first.begin(), iter->first.end());
        data.insert(data.end(), SEP.begin(), SEP.end());
        data.insert(data.end(), iter->second.begin(), iter->second.end());
        data.insert(data.end(), CRLF.begin(), CRLF.end());
    }
    data.insert(data.end(), CRLF.begin(), CRLF.end());
    data.insert(data.end(), resp.body.begin(), resp.body.end());
}

RESPONSE_PTR makeResponse200()
{
    auto resp = std::make_shared<nsocket::http::Response>();
    resp->statusCode = StatusCode::success_ok;
    return resp;
}

RESPONSE_PTR makeResponse404()
{
    auto resp = std::make_shared<nsocket::http::Response>();
    resp->statusCode = StatusCode::client_error_not_found;
    return resp;
}

RESPONSE_PTR makeResponse405()
{
    auto resp = std::make_shared<nsocket::http::Response>();
    resp->statusCode = StatusCode::client_error_method_not_allowed;
    return resp;
}
} // namespace http
} // namespace nsocket
