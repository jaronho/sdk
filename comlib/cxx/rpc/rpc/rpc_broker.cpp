#include "rpc_broker.h"

namespace rpc
{
class Broker::Client
{
public:
    using MSG_HANDLER = std::function<void(MsgType type, utilitiy::ByteArray& ba)>;

public:
    /**
     * @brief 构造函数
     */
    Client(const std::weak_ptr<nsocket::TcpConnection>& wpConn, const std::string& host, int port)
        : m_wpConn(wpConn), m_host(host), m_port(port)
    {
        m_payload = std::make_shared<nsocket::Payload>(msg_base::maxsize());
    }

    /**
     * @brief 设置消息回调, 参数: type-消息类型, ba-消息字节流
     */
    void setMsgHandler(const MSG_HANDLER& handler)
    {
        m_msgHandler = handler;
    }

    /**
     * @brief 获取客户端地址
     * @return 客户端地址
     */
    std::string getHost() const
    {
        return m_host;
    }

    /**
     * @brief 获取客户端端口
     * @return 客户端端口
     */
    int getPort() const
    {
        return m_port;
    }

    /**
     * @brief 获取客户端ID
     * @return 客户端ID
     */
    std::string getId() const
    {
        return m_id;
    }

    /**
     * @brief 设置客户端ID
     * @param id 客户端ID
     */
    void setId(const std::string& id)
    {
        m_id = id;
    }

    /**
     * @brief 发送消息
     * @param msg 消息
     * @param callback 回调, 参数: ret-true(成功)/false(失败)
     */
    void send(msg_base* msg, const std::function<void(bool ret)>& callback = nullptr)
    {
        const auto conn = m_wpConn.lock();
        if (conn)
        {
            utilitiy::ByteArray ba;
            msg->encode(ba);
            std::vector<unsigned char> data;
            nsocket::Payload::pack(ba.getBuffer(), ba.getCurrentSize(), data);
            conn->sendAsync(data, [&, callback](const boost::system::error_code& code, std::size_t length) {
                const auto conn = m_wpConn.lock();
                if (conn)
                {
                    auto point = conn->getRemoteEndpoint();
                    std::string clientHost = point.address().to_string().c_str();
                    int clientPort = (int)point.port();
                    if (code)
                    {
                        printf(">>>>>>>>>> send [%s:%d] fail, %d, %s\n", clientHost.c_str(), clientPort, code.value(),
                               code.message().c_str());
                        if (callback)
                        {
                            callback(false);
                        }
                        conn->close();
                    }
                    else
                    {
                        printf(">>>>>>>>>> send [%s:%d] ok, length: %d\n", clientHost.c_str(), clientPort, (int)length);
                        if (callback)
                        {
                            callback(true);
                        }
                    }
                }
            });
        }
        else
        {
            if (callback)
            {
                callback(false);
            }
        }
    }

    /**
     * @brief 处理接收到的数据
     * @param data 数据
     */
    void handleRecv(const std::vector<unsigned char>& data)
    {
        m_payload->unpack(
            data,
            [&](const std::vector<unsigned char>& body) {
                utilitiy::ByteArray ba;
                ba.setBuffer(body.data(), body.size());
                /* 解析消息类型 */
                MsgType type = (MsgType)ba.readInt();
                /* 处理消息 */
                if (m_msgHandler)
                {
                    m_msgHandler(type, ba);
                }
            },
            [&](const std::vector<unsigned char>& data) {});
    }

private:
    std::shared_ptr<nsocket::Payload> m_payload; /* 负载 */
    std::weak_ptr<nsocket::TcpConnection> m_wpConn; /* 连接 */
    MSG_HANDLER m_msgHandler; /* 消息句柄 */
    std::string m_host; /* 主机地址 */
    int m_port; /* 主机端口 */
    std::string m_id; /* 客户端ID */
};

#if (1 == ENABLE_NSOCKET_OPENSSL)
Broker::Broker(const std::string& name, size_t threadCount, const std::string& serverHost, int serverPort, const std::string& certFile,
               const std::string& privateKeyFile, const std::string& privateKeyFilePwd)
#else
Broker::Broker(const std::string& name, size_t threadCount, const std::string& serverHost, int serverPort)
#endif
{
    m_tcpServer = std::make_shared<nsocket::TcpServer>(name, threadCount, serverHost, serverPort);
    m_tcpServer->setNewConnectionCallback([&](const std::weak_ptr<nsocket::TcpConnection>& wpConn) { handleNewConnection(wpConn); });
    m_tcpServer->setConnectionDataCallback([&](const std::weak_ptr<nsocket::TcpConnection>& wpConn,
                                               const std::vector<unsigned char>& data) { handleRecvConnectionData(wpConn, data); });
    m_tcpServer->setConnectionCloseCallback([&](int64_t sid, const boost::asio::ip::tcp::endpoint& point,
                                                const boost::system::error_code& code) { handleConnectionClose(point, code); });
#if (1 == ENABLE_NSOCKET_OPENSSL)
    m_certFile = certFile;
    m_privateKeyFile = privateKeyFile;
    m_privateKeyFilePwd = privateKeyFilePwd;
#endif
}

void Broker::run()
{
    /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
    try
    {
#if (1 == ENABLE_NSOCKET_OPENSSL)
        m_tcpServer->run(nsocket::TcpServer::getSslContext(m_certFile, m_privateKeyFile, m_privateKeyFilePwd));
#else
        m_tcpServer->run();
#endif
    }
    catch (const std::exception& e)
    {
        printf("******************** execption: %s\n", e.what());
    }
    catch (...)
    {
        printf("******************** execption: unknown\n");
    }
}

void Broker::handleNewConnection(const std::weak_ptr<nsocket::TcpConnection>& wpConn)
{
    const auto conn = wpConn.lock();
    if (conn)
    {
        auto point = conn->getRemoteEndpoint();
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        /* 信息打印 */
        printf("++++++++++++++++++++++++++++++ new connection [%s:%d]\n", clientHost.c_str(), clientPort);
        /* 逻辑处理 */
        std::lock_guard<std::mutex> locker(m_mutexClientMap);
        if (m_clientMap.end() == m_clientMap.find(point))
        {
            auto client = std::make_shared<Broker::Client>(wpConn, clientHost, clientPort);
            client->setMsgHandler([&, client](MsgType type, utilitiy::ByteArray& ba) { handleClientMsg(client, type, ba); });
            m_clientMap.insert(std::make_pair(point, client));
        }
    }
}

void Broker::handleRecvConnectionData(const std::weak_ptr<nsocket::TcpConnection>& wpConn, const std::vector<unsigned char>& data)
{
    const auto conn = wpConn.lock();
    if (conn)
    {
        auto point = conn->getRemoteEndpoint();
        /* 信息打印 */
        {
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            printf("<<<<<<<<<<<<<<<<<<< recv data [%s:%d], length: %d\n", clientHost.c_str(), clientPort, (int)data.size());
            /* 以十六进制格式打印数据 */
            //printf("<<<<<<<<<< [hex format]\n");
            for (size_t i = 0; i < data.size(); ++i)
            {
                printf("%02X ", data[i]);
            }
            //printf("\n");
            /* 以字符串格式打印数据 */
            //printf("<<<<<<<<<< [string format]\n");
            //std::string str(data.begin(), data.end());
            //printf("%s", str.c_str());
            printf("\n");
        }
        /* 逻辑处理 */
        std::shared_ptr<Broker::Client> client = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutexClientMap);
            auto iter = m_clientMap.find(point);
            if (m_clientMap.end() != iter)
            {
                client = iter->second;
            }
        }
        if (client)
        {
            client->handleRecv(data);
        }
    }
}

void Broker::handleConnectionClose(const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code)
{
    /* 信息打印 */
    {
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        if (code)
        {
            printf("------------------------------ connection closed [%s:%d] fail, %d, %s\n", clientHost.c_str(), clientPort, code.value(),
                   code.message().c_str());
        }
        else
        {
            printf("------------------------------ connection closed [%s:%d]\n", clientHost.c_str(), clientPort);
        }
    }
    /* 逻辑处理 */
    std::lock_guard<std::mutex> locker(m_mutexClientMap);
    auto iter = m_clientMap.find(point);
    if (m_clientMap.end() != iter)
    {
        m_clientMap.erase(iter);
    }
}

void Broker::handleClientMsg(const std::shared_ptr<Broker::Client> client, const MsgType& type, utilitiy::ByteArray& ba)
{
    switch (type)
    {
    case MsgType::HEARTBEAT: {
        printf("<<<<< [HEARTBEAT] [%s:%d], client id: %s\n", client->getHost().c_str(), client->getPort(), client->getId().c_str());
    }
    break;
    case MsgType::REGISTER: {
        msg_register reg;
        reg.decode(ba);
        printf("<<<<< [REGISTER], client id: %s\n", reg.self_id.c_str());
        /* 设置客户端ID */
        bool isNewId = true;
        {
            std::lock_guard<std::mutex> locker(m_mutexClientMap);
            for (auto iter = m_clientMap.begin(); m_clientMap.end() != iter; ++iter)
            {
                if (iter->second->getId() == reg.self_id) /* ID重复 */
                {
                    isNewId = false;
                    break;
                }
            }
        }
        if (isNewId)
        {
            client->setId(reg.self_id);
        }
        else /* ID重复 */
        {
            printf("********** register repeat **********\n");
        }
        msg_register_result resp;
        resp.code = isNewId ? ErrorCode::OK : ErrorCode::REGISTER_REPEAT;
        client->send(&resp);
    }
    break;
    case MsgType::CALL: {
        msg_call mc;
        mc.decode(ba);
        printf("<<<<< [CALL], seq id: %lld, caller: %s, replyer: %s, proc: %d\n", mc.seq_id, mc.caller.c_str(), mc.replyer.c_str(),
               mc.proc);
        /* 查找应答者 */
        std::shared_ptr<Broker::Client> replyerClient = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutexClientMap);
            for (auto iter = m_clientMap.begin(); m_clientMap.end() != iter; ++iter)
            {
                if (iter->second->getId() == mc.replyer)
                {
                    replyerClient = iter->second;
                    break;
                }
            }
        }
        if (replyerClient)
        {
            /* 通知应答者 */
            replyerClient->send(&mc, [&, client, seqId = mc.seq_id, callId = mc.caller, replyer = mc.replyer, proc = mc.proc](bool ret) {
                if (ret)
                {
                    /* 等待应答 */
                    std::lock_guard<std::mutex> locker(m_mutexCallMap);
                    if (m_callMap.end() == m_callMap.find(seqId))
                    {
                        m_callMap.insert(std::make_pair(seqId, client));
                    }
                }
                else
                {
                    printf("********** call failed **********\n");
                    /* 通知调用方 */
                    msg_reply mr;
                    mr.seq_id = seqId;
                    mr.caller = callId;
                    mr.replyer = replyer;
                    mr.proc = mc.proc;
                    mr.code = ErrorCode::CALL_REPLYER_FAILED;
                    client->send(&mr);
                }
            });
        }
        else
        {
            printf("********** target unfound **********\n");
            /* 通知调用方 */
            msg_reply mr;
            mr.seq_id = mc.seq_id;
            mr.caller = mc.caller;
            mr.replyer = mc.replyer;
            mr.proc = mc.proc;
            mr.code = ErrorCode::REPLYER_NOT_FOUND;
            client->send(&mr);
        }
    }
    break;
    case MsgType::REPLY: {
        msg_reply mr;
        mr.decode(ba);
        printf("<<<<< [REPLY], seq id: %lld, caller: %s, replyer: %s, proc: %d\n", mr.seq_id, mr.caller.c_str(), mr.replyer.c_str(),
               mr.proc);
        /* 通知调用方 */
        std::weak_ptr<Broker::Client> wpCallerClient;
        {
            std::lock_guard<std::mutex> locker(m_mutexCallMap);
            auto iter = m_callMap.find(mr.seq_id);
            if (m_callMap.end() != iter)
            {
                wpCallerClient = iter->second;
                m_callMap.erase(iter);
            }
        }
        auto callerClient = wpCallerClient.lock();
        if (callerClient)
        {
            callerClient->send(&mr);
        }
    }
    break;
    default: {
        printf("********** msg [%d], unknown type **********\n", (int)type);
    }
    break;
    }
} // namespace rpc
} // namespace rpc
