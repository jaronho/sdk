#pragma once
#include "nsocket/payload.h"
#include "nsocket/tcp/tcp_server.h"
#include "rpc_msg.hpp"

namespace rpc
{
/**
 * @brief RPC代理
 */
class Broker
{
private:
    /**
     * @brief 客户端连接
     */
    class Client
    {
    public:
        using MSG_HANDLER = std::function<void(MsgType type, utilitiy::ByteArray& ba)>;

    public:
        /**
         * @brief 构造函数
         */
        Client(const nsocket::TCP_CONN_SEND_HANDLER& sendHandler) : m_sendHandler(sendHandler)
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
         * @param callback 回调
         */
        void send(msg_base* msg, const std::function<void(bool ret)>& callback = nullptr)
        {
            if (m_sendHandler)
            {
                utilitiy::ByteArray ba;
                msg->encode(ba);
                std::vector<unsigned char> data;
                nsocket::Payload::pack(ba.getBuffer(), ba.getCurrentSize(), data);
                m_sendHandler(data, [&, callback](const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code,
                                                  std::size_t length) {
                    std::string clientHost = point.address().to_string().c_str();
                    int clientPort = (int)point.port();
                    if (code)
                    {
                        printf(">>>>>>>>>> on send [%s:%d] fail, %d, %s\n", clientHost.c_str(), clientPort, code.value(),
                               code.message().c_str());
                        if (callback)
                        {
                            callback(false);
                        }
                    }
                    else
                    {
                        printf(">>>>>>>>>> on send [%s:%d] ok, length: %d\n", clientHost.c_str(), clientPort, (int)length);
                        if (callback)
                        {
                            callback(true);
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
        nsocket::TCP_CONN_SEND_HANDLER m_sendHandler; /* 发送句柄 */
        MSG_HANDLER m_msgHandler; /* 消息句柄 */
        std::string m_id; /* 客户端ID */
    };

public:
    /**
     * @brief 构造函数
     */
#if (1 == ENABLE_NSOCKET_OPENSSL)
    Broker(const std::string& serverHost, int serverPort, const std::string& certFile = "", const std::string& privateKeyFile = "",
           const std::string& privateKeyFilePwd = "")
#else
    Broker(const std::string& serverHost, int serverPort)
#endif
    {
        m_tcpServer = std::make_shared<nsocket::TcpServer>(serverHost, serverPort);
        m_tcpServer->setNewConnectionCallback(
            [&](int64_t sid, const boost::asio::ip::tcp::endpoint& point, const nsocket::TCP_CONN_SEND_HANDLER& sendHandler,
                const nsocket::TCP_CONN_CLOSE_HANDLER& closeHandler) { handleNewConnection(point, sendHandler); });
        m_tcpServer->setConnectionDataCallback(
            [&](int64_t sid, const boost::asio::ip::tcp::endpoint& point, const std::vector<unsigned char>& data,
                const nsocket::TCP_CONN_SEND_HANDLER& sendHandler,
                const nsocket::TCP_CONN_CLOSE_HANDLER& closeHandler) { handleRecvConnectionData(point, data, sendHandler); });
        m_tcpServer->setConnectionCloseCallback([&](int64_t sid, const boost::asio::ip::tcp::endpoint& point,
                                                    const boost::system::error_code& code) { handleConnectionClose(point, code); });
#if (1 == ENABLE_NSOCKET_OPENSSL)
        m_certFile = certFile;
        m_privateKeyFile = privateKeyFile;
        m_privateKeyFilePwd = privateKeyFilePwd;
#endif
    }

    /**
     * @brief 运行
     */
    void run()
    {
        /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
        try
        {
#if (1 == ENABLE_NSOCKET_OPENSSL)
            auto sslContext = nsocket::TcpServer::getSslContext(m_certFile, m_privateKeyFile, m_privateKeyFilePwd);
            m_tcpServer->run(sslContext);
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

private:
    /**
     * @brief 处理新连接
     */
    void handleNewConnection(const boost::asio::ip::tcp::endpoint& point, const nsocket::TCP_CONN_SEND_HANDLER& sendHandler)
    {
        /* 信息打印 */
        {
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            printf("++++++++++++++++++++++++++++++ on new connection [%s:%d]\n", clientHost.c_str(), clientPort);
        }
        /* 逻辑处理 */
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        auto iter = m_clientMap.find(point);
        if (m_clientMap.end() == iter)
        {
            auto client = std::make_shared<Client>(sendHandler);
            client->setMsgHandler([&, client](MsgType type, utilitiy::ByteArray& ba) { handleClientMsg(client, type, ba); });
            m_clientMap.insert(std::make_pair(point, client));
        }
    }

    /**
     * @brief 处理接收到连接数据
     */
    void handleRecvConnectionData(const boost::asio::ip::tcp::endpoint& point, const std::vector<unsigned char>& data,
                                  const nsocket::TCP_CONN_SEND_HANDLER& sendHandler)
    {
        /* 信息打印 */
        {
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            printf("<<<<<<<<<<<<<<<<<<< on recv data [%s:%d], length: %d\n", clientHost.c_str(), clientPort, (int)data.size());
            /* 以十六进制格式打印数据 */
            printf("<<<<<<<<<< [hex format]\n");
            for (size_t i = 0; i < data.size(); ++i)
            {
                printf("%02X ", data[i]);
            }
            printf("\n");
            /* 以字符串格式打印数据 */
            printf("<<<<<<<<<< [string format]\n");
            std::string str(data.begin(), data.end());
            printf("%s", str.c_str());
            printf("\n");
        }
        /* 逻辑处理 */
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        auto iter = m_clientMap.find(point);
        if (m_clientMap.end() != iter)
        {
            iter->second->handleRecv(data);
        }
    }

    /**
     * @brief 处理连接关闭
     */
    void handleConnectionClose(const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code)
    {
        /* 信息打印 */
        {
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            if (code)
            {
                printf("------------------------------ on connection closed [%s:%d] fail, %d, %s\n", clientHost.c_str(), clientPort,
                       code.value(), code.message().c_str());
            }
            else
            {
                printf("------------------------------ on connection closed [%s:%d]\n", clientHost.c_str(), clientPort);
            }
        }
        /* 逻辑处理 */
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        auto iter = m_clientMap.find(point);
        if (m_clientMap.end() != iter)
        {
            m_clientMap.erase(iter);
        }
    }

    /**
     * @brief 处理客户端消息
     */
    void handleClientMsg(const std::shared_ptr<Client> client, const MsgType& type, utilitiy::ByteArray& ba)
    {
        switch (type)
        {
        case MsgType::REQ_REGISTER: {
            msg_req_register req;
            req.decode(ba);
            printf("<<<<< msg [REQ_REGISTER], self id: %s\n", req.self_id.c_str());
            /* 设置客户端ID */
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            bool isNewId = true;
            for (auto iter = m_clientMap.begin(); m_clientMap.end() != iter; ++iter)
            {
                if (iter->second->getId() == req.self_id) /* ID重复 */
                {
                    isNewId = false;
                    break;
                }
            }
            if (isNewId)
            {
                client->setId(req.self_id);
            }
            else /* ID重复 */
            {
                printf("********** client id already exist **********\n");
            }
            msg_notify_register_result resp;
            resp.ok = isNewId ? true : false;
            resp.desc = isNewId ? "success" : "already exist id";
            client->send(&resp);
        }
        break;
        case MsgType::REQ_SEND_DATA: {
            msg_req_send_data req;
            req.decode(ba);
            printf("<<<<< msg [REQ_SEND_DATA], seq id: %lld, src id: %s, target id: %s, data length: %d\n", req.seq_id,
                   client->getId().c_str(), req.target_id.c_str(), (int)req.data.size());
            /* 转发数据到其他客户端 */
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            for (auto iter = m_clientMap.begin(); m_clientMap.end() != iter; ++iter)
            {
                auto target = iter->second;
                if (target->getId() == req.target_id)
                {
                    msg_notify_recv_data msg;
                    msg.src_id = client->getId();
                    msg.data = std::move(req.data);
                    target->send(&msg, [&, client, seqId = req.seq_id, targetId = req.target_id](bool ret) {
                        msg_req_send_data_result resp;
                        resp.seq_id = seqId;
                        resp.target_id = targetId;
                        resp.ok = ret;
                        resp.desc = ret ? "success" : "send failed";
                        client->send(&resp);
                    });
                    return;
                }
            }
            printf("********** target unfound **********\n");
            msg_req_send_data_result resp;
            resp.seq_id = req.seq_id;
            resp.target_id = req.target_id;
            resp.ok = false;
            resp.desc = "target unfound";
            client->send(&resp);
        }
        break;
        default: {
            printf("********** msg [%d], unknown type **********\n", (int)type);
        }
        break;
        }
    }

private:
    std::shared_ptr<nsocket::TcpServer> m_tcpServer; /* 服务器 */
    std::string m_certFile;
    std::string m_privateKeyFile;
    std::string m_privateKeyFilePwd;
    std::recursive_mutex m_mutex;
    std::map<boost::asio::ip::tcp::endpoint, std::shared_ptr<Client>> m_clientMap; /* 已连接的客户端表 */
};
} // namespace rpc
