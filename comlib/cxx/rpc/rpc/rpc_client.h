#pragma once
#include <atomic>

#include "nsocket/payload.h"
#include "nsocket/tcp/tcp_client.h"
#include "rpc_msg.hpp"

namespace rpc
{
using REG_HANDLER = std::function<void(const ErrorCode& code)>;
using CALL_HANDLER = std::function<std::vector<unsigned char>(const std::string& callId, const std::vector<unsigned char>& data)>;
using REPLY_FUNC = std::function<void(const std::vector<unsigned char>& data, const ErrorCode& code)>;

/**
 * @brief RPC客户端
 */
class Client
{
public:
    /**
     * @brief 构造函数
     */
#if (1 == ENABLE_NSOCKET_OPENSSL)
    Client(const std::string& id, const std::string& brokerHost, int brokerPort, const std::string& certFile = "",
           const std::string& privateKeyFile = "", const std::string& privateKeyFilePwd = "");
#else
    Client(const std::string& id, const std::string& brokerHost, int brokerPort);
#endif

    /**
     * @brief 设置注册回调
     */
    void setRegHandler(const REG_HANDLER& handler);

    /**
     * @brief 设置调用回调
     */
    void setCallHandler(const CALL_HANDLER& handler);

    /**
     * @brief 运行
     */
    void run();

    /**
     * @brief 调用指定客户端接口
     * @param replyId 应答方ID
     * @param data 数据
     * @param replyFunc 应答函数
     */
    void call(const std::string& replyId, const std::vector<unsigned char>& data, const REPLY_FUNC& replyFunc);

private:
    /**
     * @brief 处理连接结果
     */
    void handleConnection(const boost::system::error_code& code);

    /**
     * @brief 处理数据接收
     */
    void handleRecvData(const std::vector<unsigned char>& data);

    /**
     * @brief 处理消息
     */
    void handleMsg(const MsgType& type, utilitiy::ByteArray& ba);

    /**
     * @brief 请求注册
     */
    void reqRegister();

private:
    std::shared_ptr<nsocket::Payload> m_payload; /* 负载 */
    std::shared_ptr<nsocket::TcpClient> m_tcpClient; /* 客户端 */
    REG_HANDLER m_regHandler; /* 注册回调句柄 */
    CALL_HANDLER m_callHandler; /* 调用回调句柄 */
    std::map<long long, REPLY_FUNC> m_replyFuncMap; /* 应答函数表 */
    std::string m_id; /* 客户端ID */
    std::string m_brokerHost; /* broker地址 */
    int m_serverPort; /* broker端口 */
    std::string m_certFile;
    std::string m_privateKeyFile;
    std::string m_privateKeyFilePwd;
    std::atomic_bool m_running; /* 是否运行中 */
    std::atomic_bool m_registered; /* 是否已向broker注册 */
};
} // namespace rpc
