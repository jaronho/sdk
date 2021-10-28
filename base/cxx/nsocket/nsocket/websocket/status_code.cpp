#include "status_code.h"

#include <unordered_map>

namespace nsocket
{
namespace ws
{
/**
 * @brief WebSocket关闭状态码描述表
 */
const std::unordered_map<CloseCode, std::string>& close_code_strings()
{
    static const std::unordered_map<CloseCode, std::string> s_closeCodeMap = {
        {CloseCode::reserve, "0 Reserve"},
        {CloseCode::close_normal, "1000 Close Normal"},
        {CloseCode::close_going_away, "1001 Close Going Away"},
        {CloseCode::close_protocol_error, "1002 Close Protocol Error"},
        {CloseCode::close_unsupported, "1003 Close Unsupported"},
        {CloseCode::close_no_status, "1005 Close No Status"},
        {CloseCode::close_abnormal, "1006 Close Abnormal"},
        {CloseCode::unsupported_data, "1007 Unsupported Data"},
        {CloseCode::policy_violation, "1008 Policy Violation"},
        {CloseCode::close_too_large, "1009 Close Too Large"},
        {CloseCode::missing_extension, "1010 Missing Extension"},
        {CloseCode::internal_error, "1011 Internal Error"},
        {CloseCode::service_restart, "1012 Service Restart"},
        {CloseCode::try_again_later, "1013 Try Again Later"},
        {CloseCode::tls_handshake, "1015 TLS Handshake"}};
    return s_closeCodeMap;
}

CloseCode close_code(const std::string& desc)
{
    class StringToCloseCode : public std::unordered_map<std::string, CloseCode>
    {
    public:
        StringToCloseCode()
        {
            for (auto& item : close_code_strings())
            {
                emplace(item.second, item.first);
            }
        }
    };
    static StringToCloseCode s_closeCodeMap;
    auto iter = s_closeCodeMap.find(desc);
    if (s_closeCodeMap.end() == iter)
    {
        return CloseCode::reserve;
    }
    return iter->second;
}

std::string close_desc(const CloseCode& code)
{
    auto iter = close_code_strings().find(code);
    if (close_code_strings().end() == iter)
    {
        return std::string();
    }
    return iter->second;
}
} // namespace ws
} // namespace nsocket
