#include "http_status_code.h"

#include <unordered_map>

namespace nsocket
{
/**
 * @brief HTTP状态码描述表
 */
const std::unordered_map<HttpStatusCode, std::string>& http_status_code_strings()
{
    static const std::unordered_map<HttpStatusCode, std::string> s_statusCodeMap = {
        {HttpStatusCode::unknown, ""},
        {HttpStatusCode::information_continue, "100 Continue"},
        {HttpStatusCode::information_switching_protocols, "101 Switching Protocols"},
        {HttpStatusCode::information_processing, "102 Processing"},
        {HttpStatusCode::success_ok, "200 OK"},
        {HttpStatusCode::success_created, "201 Created"},
        {HttpStatusCode::success_accepted, "202 Accepted"},
        {HttpStatusCode::success_non_authoritative_information, "203 Non-Authoritative Information"},
        {HttpStatusCode::success_no_content, "204 No Content"},
        {HttpStatusCode::success_reset_content, "205 Reset Content"},
        {HttpStatusCode::success_partial_content, "206 Partial Content"},
        {HttpStatusCode::success_multi_status, "207 Multi-Status"},
        {HttpStatusCode::success_already_reported, "208 Already Reported"},
        {HttpStatusCode::success_im_used, "226 IM Used"},
        {HttpStatusCode::redirection_multiple_choices, "300 Multiple Choices"},
        {HttpStatusCode::redirection_moved_permanently, "301 Moved Permanently"},
        {HttpStatusCode::redirection_found, "302 Found"},
        {HttpStatusCode::redirection_see_other, "303 See Other"},
        {HttpStatusCode::redirection_not_modified, "304 Not Modified"},
        {HttpStatusCode::redirection_use_proxy, "305 Use Proxy"},
        {HttpStatusCode::redirection_switch_proxy, "306 Switch Proxy"},
        {HttpStatusCode::redirection_temporary_redirect, "307 Temporary Redirect"},
        {HttpStatusCode::redirection_permanent_redirect, "308 Permanent Redirect"},
        {HttpStatusCode::client_error_bad_request, "400 Bad Request"},
        {HttpStatusCode::client_error_unauthorized, "401 Unauthorized"},
        {HttpStatusCode::client_error_payment_required, "402 Payment Required"},
        {HttpStatusCode::client_error_forbidden, "403 Forbidden"},
        {HttpStatusCode::client_error_not_found, "404 Not Found"},
        {HttpStatusCode::client_error_method_not_allowed, "405 Method Not Allowed"},
        {HttpStatusCode::client_error_not_acceptable, "406 Not Acceptable"},
        {HttpStatusCode::client_error_proxy_authentication_required, "407 Proxy Authentication Required"},
        {HttpStatusCode::client_error_request_timeout, "408 Request Timeout"},
        {HttpStatusCode::client_error_conflict, "409 Conflict"},
        {HttpStatusCode::client_error_gone, "410 Gone"},
        {HttpStatusCode::client_error_length_required, "411 Length Required"},
        {HttpStatusCode::client_error_precondition_failed, "412 Precondition Failed"},
        {HttpStatusCode::client_error_payload_too_large, "413 Payload Too Large"},
        {HttpStatusCode::client_error_uri_too_long, "414 URI Too Long"},
        {HttpStatusCode::client_error_unsupported_media_type, "415 Unsupported Media Type"},
        {HttpStatusCode::client_error_range_not_satisfiable, "416 Range Not Satisfiable"},
        {HttpStatusCode::client_error_expectation_failed, "417 Expectation Failed"},
        {HttpStatusCode::client_error_im_a_teapot, "418 I'm a teapot"},
        {HttpStatusCode::client_error_misdirection_required, "421 Misdirected Request"},
        {HttpStatusCode::client_error_unprocessable_entity, "422 Unprocessable Entity"},
        {HttpStatusCode::client_error_locked, "423 Locked"},
        {HttpStatusCode::client_error_failed_dependency, "424 Failed Dependency"},
        {HttpStatusCode::client_error_upgrade_required, "426 Upgrade Required"},
        {HttpStatusCode::client_error_precondition_required, "428 Precondition Required"},
        {HttpStatusCode::client_error_too_many_requests, "429 Too Many Requests"},
        {HttpStatusCode::client_error_request_header_fields_too_large, "431 Request Header Fields Too Large"},
        {HttpStatusCode::client_error_unavailable_for_legal_reasons, "451 Unavailable For Legal Reasons"},
        {HttpStatusCode::server_error_internal_server_error, "500 Internal Server Error"},
        {HttpStatusCode::server_error_not_implemented, "501 Not Implemented"},
        {HttpStatusCode::server_error_bad_gateway, "502 Bad Gateway"},
        {HttpStatusCode::server_error_service_unavailable, "503 Service Unavailable"},
        {HttpStatusCode::server_error_gateway_timeout, "504 Gateway Timeout"},
        {HttpStatusCode::server_error_http_version_not_supported, "505 HTTP Version Not Supported"},
        {HttpStatusCode::server_error_variant_also_negotiates, "506 Variant Also Negotiates"},
        {HttpStatusCode::server_error_insufficient_storage, "507 Insufficient Storage"},
        {HttpStatusCode::server_error_loop_detected, "508 Loop Detected"},
        {HttpStatusCode::server_error_not_extended, "510 Not Extended"},
        {HttpStatusCode::server_error_network_authentication_required, "511 Network Authentication Required"}};
    return s_statusCodeMap;
}

HttpStatusCode http_status_code(const std::string& desc)
{
    class StringToStatusCode : public std::unordered_map<std::string, HttpStatusCode>
    {
    public:
        StringToStatusCode()
        {
            for (auto& item : http_status_code_strings())
            {
                emplace(item.second, item.first);
            }
        }
    };
    static StringToStatusCode s_statusCodeMap;
    auto iter = s_statusCodeMap.find(desc);
    if (s_statusCodeMap.end() == iter)
    {
        return HttpStatusCode::unknown;
    }
    return iter->second;
}

std::string http_status_desc(const HttpStatusCode& code)
{
    auto iter = http_status_code_strings().find(code);
    if (http_status_code_strings().end() == iter)
    {
        return std::string();
    }
    return iter->second;
}
} // namespace nsocket
