#pragma once
#include <memory>
#include <string>

#include "../tcp/tcp_server.h"
#include "frame.h"
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
    void sendText(const std::string& text, bool isFin = true);

    void sendBytes(const std::vector<unsigned char>& bytes, bool isFin = true);

    void sendClose(const CloseCode& code = CloseCode::close_normal);

    void sendPing();

    void sendPong();

public:
    std::weak_ptr<TcpSession> wpTcpSession; /* TCP会话 */
    std::shared_ptr<Request> req; /* 请求 */
    std::shared_ptr<Frame> frame; /* 数据帧 */
};
using SESSION_PTR = std::shared_ptr<Session>;
} // namespace ws
} // namespace nsocket
