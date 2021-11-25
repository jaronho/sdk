#include "status_code.h"

#include <unordered_map>

namespace nsocket
{
namespace http
{
/**
 * @brief HTTP状态码描述表
 */
const std::unordered_map<int, std::string>& status_code_strings()
{
    static const std::unordered_map<int, std::string> s_statusCodeMap{
        {(int)StatusCode::unknown, ""},
        {(int)StatusCode::information_continue, "100 Continue"},
        {(int)StatusCode::information_switching_protocols, "101 Switching Protocols"},
        {(int)StatusCode::information_processing, "102 Processing"},
        {(int)StatusCode::success_ok, "200 OK"},
        {(int)StatusCode::success_created, "201 Created"},
        {(int)StatusCode::success_accepted, "202 Accepted"},
        {(int)StatusCode::success_non_authoritative_information, "203 Non-Authoritative Information"},
        {(int)StatusCode::success_no_content, "204 No Content"},
        {(int)StatusCode::success_reset_content, "205 Reset Content"},
        {(int)StatusCode::success_partial_content, "206 Partial Content"},
        {(int)StatusCode::success_multi_status, "207 Multi-Status"},
        {(int)StatusCode::success_already_reported, "208 Already Reported"},
        {(int)StatusCode::success_im_used, "226 IM Used"},
        {(int)StatusCode::redirection_multiple_choices, "300 Multiple Choices"},
        {(int)StatusCode::redirection_moved_permanently, "301 Moved Permanently"},
        {(int)StatusCode::redirection_found, "302 Found"},
        {(int)StatusCode::redirection_see_other, "303 See Other"},
        {(int)StatusCode::redirection_not_modified, "304 Not Modified"},
        {(int)StatusCode::redirection_use_proxy, "305 Use Proxy"},
        {(int)StatusCode::redirection_switch_proxy, "306 Switch Proxy"},
        {(int)StatusCode::redirection_temporary_redirect, "307 Temporary Redirect"},
        {(int)StatusCode::redirection_permanent_redirect, "308 Permanent Redirect"},
        {(int)StatusCode::client_error_bad_request, "400 Bad Request"},
        {(int)StatusCode::client_error_unauthorized, "401 Unauthorized"},
        {(int)StatusCode::client_error_payment_required, "402 Payment Required"},
        {(int)StatusCode::client_error_forbidden, "403 Forbidden"},
        {(int)StatusCode::client_error_not_found, "404 Not Found"},
        {(int)StatusCode::client_error_method_not_allowed, "405 Method Not Allowed"},
        {(int)StatusCode::client_error_not_acceptable, "406 Not Acceptable"},
        {(int)StatusCode::client_error_proxy_authentication_required, "407 Proxy Authentication Required"},
        {(int)StatusCode::client_error_request_timeout, "408 Request Timeout"},
        {(int)StatusCode::client_error_conflict, "409 Conflict"},
        {(int)StatusCode::client_error_gone, "410 Gone"},
        {(int)StatusCode::client_error_length_required, "411 Length Required"},
        {(int)StatusCode::client_error_precondition_failed, "412 Precondition Failed"},
        {(int)StatusCode::client_error_payload_too_large, "413 Payload Too Large"},
        {(int)StatusCode::client_error_uri_too_long, "414 URI Too Long"},
        {(int)StatusCode::client_error_unsupported_media_type, "415 Unsupported Media Type"},
        {(int)StatusCode::client_error_range_not_satisfiable, "416 Range Not Satisfiable"},
        {(int)StatusCode::client_error_expectation_failed, "417 Expectation Failed"},
        {(int)StatusCode::client_error_im_a_teapot, "418 I'm a teapot"},
        {(int)StatusCode::client_error_misdirection_required, "421 Misdirected Request"},
        {(int)StatusCode::client_error_unprocessable_entity, "422 Unprocessable Entity"},
        {(int)StatusCode::client_error_locked, "423 Locked"},
        {(int)StatusCode::client_error_failed_dependency, "424 Failed Dependency"},
        {(int)StatusCode::client_error_upgrade_required, "426 Upgrade Required"},
        {(int)StatusCode::client_error_precondition_required, "428 Precondition Required"},
        {(int)StatusCode::client_error_too_many_requests, "429 Too Many Requests"},
        {(int)StatusCode::client_error_request_header_fields_too_large, "431 Request Header Fields Too Large"},
        {(int)StatusCode::client_error_unavailable_for_legal_reasons, "451 Unavailable For Legal Reasons"},
        {(int)StatusCode::server_error_internal_server_error, "500 Internal Server Error"},
        {(int)StatusCode::server_error_not_implemented, "501 Not Implemented"},
        {(int)StatusCode::server_error_bad_gateway, "502 Bad Gateway"},
        {(int)StatusCode::server_error_service_unavailable, "503 Service Unavailable"},
        {(int)StatusCode::server_error_gateway_timeout, "504 Gateway Timeout"},
        {(int)StatusCode::server_error_http_version_not_supported, "505 HTTP Version Not Supported"},
        {(int)StatusCode::server_error_variant_also_negotiates, "506 Variant Also Negotiates"},
        {(int)StatusCode::server_error_insufficient_storage, "507 Insufficient Storage"},
        {(int)StatusCode::server_error_loop_detected, "508 Loop Detected"},
        {(int)StatusCode::server_error_not_extended, "510 Not Extended"},
        {(int)StatusCode::server_error_network_authentication_required, "511 Network Authentication Required"}};
    return s_statusCodeMap;
}

StatusCode status_code(const std::string& desc)
{
    class StringToStatusCode : public std::unordered_map<std::string, int>
    {
    public:
        StringToStatusCode()
        {
            for (auto& item : status_code_strings())
            {
                emplace(item.second, item.first);
            }
        }
    };
    static StringToStatusCode s_statusCodeMap;
    auto iter = s_statusCodeMap.find(desc);
    if (s_statusCodeMap.end() == iter)
    {
        return StatusCode::unknown;
    }
    return (StatusCode)iter->second;
}

std::string status_desc(const StatusCode& code)
{
    auto iter = status_code_strings().find((int)code);
    if (status_code_strings().end() == iter)
    {
        return std::string();
    }
    return iter->second;
}
} // namespace http
} // namespace nsocket
