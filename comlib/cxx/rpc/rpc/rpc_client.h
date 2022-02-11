#pragma once
#include <atomic>
#include <future>

#include "nsocket/payload.h"
#include "nsocket/tcp/tcp_client.h"
#include "rpc_msg.hpp"

namespace rpc
{
using REG_HANDLER = std::function<void(const ErrorCode& code)>;
using CALL_HANDLER = std::function<std::vector<unsigned char>(const std::string& callId, int proc, const std::vector<unsigned char>& data)>;
using REPLY_FUNC = std::function<void(const std::vector<unsigned char>& data, const ErrorCode& code)>;

/**
 * @brief RPC客户端
 */
class Client
{
public:
    /**
     * @brief 构造函数
     * @param id 调用者ID
     * @param brokerHost 代理服务地址
     * @param brokerPort 代理服务端口
     * @param certFile 证书文件, 例如: client.crt
     * @param privateKeyFile 私钥文件, 例如: client.key
     * @param privateKeyFilePwd 私钥文件密码, 例如: 123456
     */
#if (1 == ENABLE_NSOCKET_OPENSSL)
    Client(const std::string& id, const std::string& brokerHost, int brokerPort, const std::string& certFile = "",
           const std::string& privateKeyFile = "", const std::string& privateKeyFilePwd = "");
#else
    Client(const std::string& id, const std::string& brokerHost, int brokerPort);
#endif

    /**
     * @brief 设置注册回调
     * @param handler 注册回调
     */
    void setRegHandler(const REG_HANDLER& handler);

    /**
     * @brief 设置调用回调
     * @param handler 调用回调, 注意: 不要在回调中做耗时阻塞操作
     */
    void setCallHandler(const CALL_HANDLER& handler);

    /**
     * @brief 运行(进入循环, 阻塞和占用调用线程)
     * @param async 是否异步连接(选填), 默认异步
     * @param retryTime 重试时间(选填)
     */
    void run(bool async = true, std::chrono::steady_clock::duration retryTime = std::chrono::steady_clock::duration::zero());

    /**
     * @brief 调用指定客户端接口
     * @param replyer 应答者ID
     * @param proc 程序ID
     * @param data 数据
     * @param replyData [输出]应答数据(选填)
     * @param timeout 超时时间(选填)
     * @return 错误码
     */
    rpc::ErrorCode call(const std::string& replyer, int proc, const std::vector<unsigned char>& data, std::vector<unsigned char>& replyData,
                        const std::chrono::steady_clock::duration& timeout = std::chrono::steady_clock::duration::zero());

private:
    /**
     * @brief 处理连接结果
     */
    void handleConnection(const boost::system::error_code& code, bool async);

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
    void reqRegister(bool async);

private:
    std::shared_ptr<nsocket::Payload> m_payload; /* 负载 */
    std::shared_ptr<nsocket::TcpClient> m_tcpClient; /* 客户端 */
    REG_HANDLER m_regHandler; /* 注册回调句柄 */
    CALL_HANDLER m_callHandler; /* 调用回调句柄 */
    std::mutex m_mutex;
    std::map<long long, std::weak_ptr<std::promise<msg_reply>>> m_resultMap; /* 应答表 */
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
