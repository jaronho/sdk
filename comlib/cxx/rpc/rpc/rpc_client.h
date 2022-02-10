#pragma once
#include <atomic>

#include "nsocket/payload.h"
#include "nsocket/tcp/tcp_client.h"
#include "rpc_msg.hpp"

namespace rpc
{
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
     * @brief 设置注册回调, 参数: ret-成功/失败
     */
    void setRegHandler(const std::function<void(bool ret)>& handler);

    /**
     * @brief 设置消息回调, 参数: srcId-消息发送方ID, handler-处理器
     */
    void setMsgHandler(const std::function<void(const std::string& srcId, const std::vector<unsigned char>& data)>& handler);

    /**
     * @brief 运行
     */
    void run();

    /**
     * @brief 发送数据到指定客户端
     * @param targetId 接收方ID
     * @param data 数据
     * @param onSendCb 发送回调
     */
    void send(const std::string& targetId, const std::vector<unsigned char>& data,
              const std::function<void(const std::string& targetId, bool ret)>& onSendCb);

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
    std::function<void(bool ret)> m_regHandler; /* 注册句柄 */
    std::function<void(const std::string& srcId, const std::vector<unsigned char>& data)> m_msgHandler; /* 消息句柄 */
    std::map<long long, std::function<void(const std::string& targetId, bool ret)>> m_sendCbMap; /* 发送回调表 */
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
