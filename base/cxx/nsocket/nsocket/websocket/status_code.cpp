#include "status_code.h"

#include <unordered_map>

namespace nsocket
{
namespace ws
{
/**
 * @brief WebSocket关闭状态码描述表
 */
const std::unordered_map<int, std::string>& close_code_strings()
{
    static const std::unordered_map<int, std::string> s_closeCodeMap = {{(int)CloseCode::reserve, "0 Reserve"},
                                                                        {(int)CloseCode::close_normal, "1000 Close Normal"},
                                                                        {(int)CloseCode::close_going_away, "1001 Close Going Away"},
                                                                        {(int)CloseCode::close_protocol_error, "1002 Close Protocol Error"},
                                                                        {(int)CloseCode::close_unsupported, "1003 Close Unsupported"},
                                                                        {(int)CloseCode::close_no_status, "1005 Close No Status"},
                                                                        {(int)CloseCode::close_abnormal, "1006 Close Abnormal"},
                                                                        {(int)CloseCode::unsupported_data, "1007 Unsupported Data"},
                                                                        {(int)CloseCode::policy_violation, "1008 Policy Violation"},
                                                                        {(int)CloseCode::close_too_large, "1009 Close Too Large"},
                                                                        {(int)CloseCode::missing_extension, "1010 Missing Extension"},
                                                                        {(int)CloseCode::internal_error, "1011 Internal Error"},
                                                                        {(int)CloseCode::service_restart, "1012 Service Restart"},
                                                                        {(int)CloseCode::try_again_later, "1013 Try Again Later"},
                                                                        {(int)CloseCode::tls_handshake, "1015 TLS Handshake"}};
    return s_closeCodeMap;
}

CloseCode close_code(const std::string& desc)
{
    class StringToCloseCode : public std::unordered_map<std::string, int>
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
    return (CloseCode)iter->second;
}

std::string close_desc(const CloseCode& code)
{
    auto iter = close_code_strings().find((int)code);
    if (close_code_strings().end() == iter)
    {
        return std::string();
    }
    return iter->second;
}
} // namespace ws
} // namespace nsocket
