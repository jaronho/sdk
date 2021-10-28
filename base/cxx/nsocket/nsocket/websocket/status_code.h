#pragma once
#include <string>

namespace nsocket
{
namespace ws
{
/**
 * @brief WebSocket关闭状态码类型
 */
enum class CloseCode
{
    reserve = 0, /* 0-999, 保留段, 未使用 */
    close_normal = 1000, /* 正常关闭, 无论为何目的而创建, 该连接都已成功完成任务 */
    close_going_away = 1001, /* 终端离开, 可能因为服务端错误, 也可能因为浏览器正从打开连接的页面跳转离开 */
    close_protocol_error = 1002, /* 由于协议错误而中断连接 */
    close_unsupported = 1003, /* 由于接收到不允许的数据类型而断开连接(如仅接收文本数据的终端接收到了二进制数据) */
    /* 1004, 保留, 其意义可能会在未来定义 */
    close_no_status = 1005, /* 保留, 表示没有收到预期的状态码 */
    close_abnormal = 1006, /* 保留, 用于期望收到状态码时连接非正常关闭(也就是说, 没有发送关闭帧) */
    unsupported_data = 1007, /* 由于收到了格式不符的数据而断开连接(如文本消息中包含了非UTF-8数据) */
    policy_violation = 1008, /* 由于收到不符合约定的数据而断开连接, 这是一个通用状态码, 用于不适合使用1003和1009状态码的场景 */
    close_too_large = 1009, /* 由于收到过大的数据帧而断开连接 */
    missing_extension = 1010, /* 客户端期望服务器商定一个或多个拓展, 但服务器没有处理, 因此客户端断开连接 */
    internal_error = 1011, /* 客户端由于遇到没有预料的情况阻止其完成请求, 因此服务端断开连接 */
    service_restart = 1012, /* 服务器由于重启而断开连接 */
    try_again_later = 1013, /* 服务器由于临时原因断开连接, 如服务器过载因此断开一部分客户端连接 */
    /* 1014, 由 WebSocket 标准保留以便未来使用 */
    tls_handshake = 1015, /* 保留, 表示连接由于无法完成TLS握手而关闭(例如无法验证服务器证书) */
    /* 1016-1999, 由 WebSocket 标准保留以便未来使用 */
    /* 2000-2999, 由 WebSocket 拓展保留使用 */
    /* 3000-3999, 可以由库或框架使用. 不应由应用使用. 可以在IANA注册, 先到先得 */
    /* 4000-4999, 可以由应用使用 */
};

/**
 * @brief 获取WebSocket关闭状态码
 * @param desc 状态描述
 * @return 状态码
 */
CloseCode close_code(const std::string& desc);

/**
 * @brief 获取WebSocket关闭状态描述
 * @param code 状态码
 * @return 状态描述
 */
std::string close_desc(const CloseCode& code);
} // namespace ws
} // namespace nsocket
