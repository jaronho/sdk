#pragma once
#include <memory>
#include <string>

#include "../tcp/tcp_server.h"
#include "request.h"

namespace nsocket
{
namespace ws
{
/**
 * @brief WebSocket会话
 */
class Session
{
public:
    std::weak_ptr<TcpSession> wpTcpSession; /* TCP会话 */
    std::shared_ptr<Request> req; /* 请求 */
};
using SESSION_PTR = std::shared_ptr<Session>;
} // namespace ws
} // namespace nsocket
