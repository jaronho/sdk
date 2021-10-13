#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

#include "../../nsocket/tcp/tcp_server.h"
#include "../net_packet.hpp"

class ClientInfo;

std::recursive_mutex g_mutex;
std::map<boost::asio::ip::tcp::endpoint, std::shared_ptr<ClientInfo>> g_clientMap; /* �ͻ�������ӳ��� */

/**
 * @brief �ͻ�����Ϣ 
 */
class ClientInfo
{
public:
    ClientInfo(const nsocket::TCP_CONN_SEND_HANDLER& handler) : m_sendHandler(handler), m_pktBodyLen(0) {}

    void handleSend(const std::vector<unsigned char>& data)
    {
        std::vector<unsigned char> buffer;
        /* ��װ��ͷ, ��ͷ4���ֽ�, ��Ű��峤��(���) */
        size_t pktBodyLen = data.size();
        buffer.emplace_back((pktBodyLen >> 24) & 0xFF);
        buffer.emplace_back((pktBodyLen >> 16) & 0xFF);
        buffer.emplace_back((pktBodyLen >> 8) & 0xFF);
        buffer.emplace_back((pktBodyLen >> 0) & 0xFF);
        buffer.insert(buffer.end(), data.begin(), data.end());
        m_sendHandler(buffer, [&](const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code, std::size_t length) {
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            if (code)
            {
                printf("---------- on send [%s:%d] fail, %d, %s\n", clientHost.c_str(), clientPort, code.value(), code.message().c_str());
            }
            else
            {
                printf("---------- on send [%s:%d] ok, length: %d\n", clientHost.c_str(), clientPort, (int)length);
            }
        });
    }

    void handleRecv(const std::vector<unsigned char>& data)
    {
        m_recvBuffer.insert(m_recvBuffer.end(), data.begin(), data.end());
        while (m_recvBuffer.size() > 0)
        {
            if (0 == m_pktBodyLen) /* ������ͷ */
            {
                if (m_recvBuffer.size() >= 4)
                {
                    /* ��ͷ4���ֽ�, ��Ű��峤��(���) */
                    unsigned int pktHead = 0;
                    pktHead += (unsigned int)m_recvBuffer[0] << 24;
                    pktHead += (unsigned int)m_recvBuffer[1] << 16;
                    pktHead += (unsigned int)m_recvBuffer[2] << 8;
                    pktHead += (unsigned int)m_recvBuffer[3];
                    if (pktHead > 10 * 1024 * 1024) /* ���峤�ȴ���10M��Ϊ�Ǵ���� */
                    {
                        printf("+ protocol illegal\n");
                        m_recvBuffer.clear();
                        break;
                    }
                    m_recvBuffer.erase(m_recvBuffer.begin(), m_recvBuffer.begin() + 4);
                    m_pktBodyLen = pktHead;
                }
                else
                {
                    break;
                }
            }
            else /* �������� */
            {
                unsigned int needBodyLen = m_pktBodyLen - m_pktBody.size();
                if (needBodyLen > m_recvBuffer.size()) /* �������� */
                {
                    m_pktBody.insert(m_pktBody.end(), m_recvBuffer.begin(), m_recvBuffer.end());
                    m_recvBuffer.clear();
                }
                else /* ������ */
                {
                    m_pktBody.insert(m_pktBody.end(), m_recvBuffer.begin(), m_recvBuffer.begin() + needBodyLen);
                    m_recvBuffer.erase(m_recvBuffer.begin(), m_recvBuffer.begin() + needBodyLen);
                    /* ������ */
                    if (m_pktBody.size() >= 4)
                    {
                        /* ������Ϣ����(С��) */
                        unsigned int msgType = 0;
                        msgType += m_pktBody[0];
                        msgType += m_pktBody[1];
                        msgType += m_pktBody[2];
                        msgType += m_pktBody[3];
                        m_pktBody.erase(m_pktBody.begin(), m_pktBody.begin() + 4);
                        switch (msgType)
                        {
                        case MSG_REQ_SET_SELF_ID: {
                            req_set_self_id req;
                            req.decode(m_pktBody);
                            handleMsgReqSetSelfId(req);
                        }
                        break;
                        case MSG_REQ_SEND_DATA: {
                            req_send_data req;
                            req.decode(m_pktBody);
                            handleMsgReqSendData(req);
                        }
                        break;
                        default:
                            printf("+ can't handle unknown net msg type [%u]\n", msgType);
                            break;
                        }
                    }
                    /* ���ð� */
                    m_pktBodyLen = 0;
                    m_pktBody.clear();
                }
            }
        }
    }

private:
    void handleMsgReqSetSelfId(const req_set_self_id& req)
    {
        m_id = req.self_id;
        printf("+ handle msg req_set_self_id, self_id: %s\n", req.self_id.c_str());
    }

    void handleMsgReqSendData(const req_send_data& req)
    {
        std::lock_guard<std::recursive_mutex> locker(g_mutex);
        for (auto iter = g_clientMap.begin(); g_clientMap.end() != iter; ++iter)
        {
            if (req.target_id == iter->second->m_id)
            {
                printf("+ handle msg req_send_data, target_id: %s find\n", req.target_id.c_str());
                notify_recv_data resp;
                resp.src_id = m_id;
                resp.data = req.data;
                iter->second->handleSend(resp.encode());
                return;
            }
        }
        printf("+ handle msg req_send_data, target_id: %s not find\n", req.target_id.c_str());
    }

private:
    std::string m_id; /* �ͻ���ID */
    nsocket::TCP_CONN_SEND_HANDLER m_sendHandler; /* �ͻ��˷��;�� */
    std::vector<unsigned char> m_recvBuffer; /* �ͻ��˽��ջ����� */
    unsigned int m_pktBodyLen; /* ��Ҫ���յĿͻ��˰����ܳ��� */
    std::vector<unsigned char> m_pktBody; /* �ѽ��յĿͻ��˰������� */
};

int main(int argc, char* argv[])
{
    printf("***********************************************************************************************************\n");
    printf("** This is TCP server                                                                                    **\n");
    printf("** Options:                                                                                              **\n");
    printf("**                                                                                                       **\n");
    printf("** [-s]                   server address, default: 127.0.0.1                                             **\n");
    printf("** [-p]                   server port, default: 4335                                                     **\n");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("** [-cf]                  specify certificate file. e.g. server.crt                                      **\n");
    printf("** [-pkf]                 specify private key file, e.g. server.key                                      **\n");
    printf("** [-pkp]                 specify private key file password, e.g. qq123456                               **\n");
#endif
    printf("** [-t]                   specify type, [ack, broker]                                                    **\n");
    printf("**                                                                                                       **\n");
    printf("***********************************************************************************************************\n");
    printf("\n");
    std::string serverHost;
    int serverPort = 0;
    std::string certFile;
    std::string privateKeyFile;
    std::string privateKeyFilePwd;
    std::string type;
    for (int i = 1; i < argc;)
    {
        const char* key = argv[i];
        if (0 == strcmp(key, "-s")) /* ��������ַ */
        {
            ++i;
            if (i < argc)
            {
                serverHost = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-p")) /* �������˿� */
        {
            ++i;
            if (i < argc)
            {
                serverPort = atoi(argv[i]);
                ++i;
            }
        }
#if (1 == ENABLE_NSOCKET_OPENSSL)
        else if (0 == strcmp(key, "-cf")) /* ֤���ļ� */
        {
            ++i;
            if (i < argc)
            {
                certFile = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pkf")) /* ˽Կ�ļ� */
        {
            ++i;
            if (i < argc)
            {
                privateKeyFile = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pkp")) /* ˽Կ�ļ����� */
        {
            ++i;
            if (i < argc)
            {
                privateKeyFilePwd = argv[i];
                ++i;
            }
        }
#endif
        else if (0 == strcmp(key, "-t")) /* ����: ack(�ͻ��˷�ʲô�ͷ���ʲô), broker(����, ���ڽ��̼�ͨ��) */
        {
            ++i;
            if (i < argc)
            {
                type = argv[i];
                ++i;
            }
        }
        else
        {
            ++i;
        }
    }
    if (serverHost.empty())
    {
        serverHost = "127.0.0.1";
    }
    if (serverPort <= 0)
    {
        serverPort = 4335;
    }
    printf("server: %s:%d\n", serverHost.c_str(), serverPort);
    auto server = std::make_shared<nsocket::TcpServer>(serverHost, serverPort);
    /* ���������ӻص� */
    server->setNewConnectionCallback([&](const boost::asio::ip::tcp::endpoint& point, const nsocket::TCP_CONN_SEND_HANDLER& sendHandler) {
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        printf("============================== on new connection [%s:%d]\n", clientHost.c_str(), clientPort);
        std::lock_guard<std::recursive_mutex> locker(g_mutex);
        auto iter = g_clientMap.find(point);
        if (g_clientMap.end() == iter)
        {
            g_clientMap.insert(std::make_pair(point, std::make_shared<ClientInfo>(sendHandler)));
        }
    });
    /* ���ý����������ݻص� */
    server->setRecvConnectionDataCallback([&, type](const boost::asio::ip::tcp::endpoint& point, const std::vector<unsigned char>& data,
                                                    const nsocket::TCP_CONN_SEND_HANDLER& sendHandler) {
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        printf("++++++++++ on recv data [%s:%d], length: %d\n", clientHost.c_str(), clientPort, (int)data.size());
        /* ��ʮ�����Ƹ�ʽ��ӡ���� */
        printf("+++++ [hex format]\n");
        for (size_t i = 0; i < data.size(); ++i)
        {
            printf("%02X ", data[i]);
        }
        printf("\n");
        /* ���ַ�����ʽ��ӡ���� */
        printf("+++++ [string format]\n");
        std::string str(data.begin(), data.end());
        printf("%s", str.c_str());
        printf("\n");
        if (0 == type.compare("ack"))
        {
            /* ���յ�������ԭ�ⲻ�����ظ��ͻ��� */
            sendHandler(data, [&](const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code, std::size_t length) {
                std::string clientHost = point.address().to_string().c_str();
                int clientPort = (int)point.port();
                if (code)
                {
                    printf("---------- on send [%s:%d] fail, %d, %s\n", clientHost.c_str(), clientPort, code.value(),
                           code.message().c_str());
                }
                else
                {
                    printf("---------- on send [%s:%d] ok, length: %d\n", clientHost.c_str(), clientPort, (int)length);
                }
            });
        }
        else if (0 == type.compare("broker"))
        {
            std::lock_guard<std::recursive_mutex> locker(g_mutex);
            auto iter = g_clientMap.find(point);
            if (g_clientMap.end() != iter)
            {
                iter->second->handleRecv(data);
            }
        }
    });
    /* �������ӹرջص� */
    server->setConnectionCloseCallback([&](const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code) {
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        if (code)
        {
            printf("-------------------- on connection closed [%s:%d] fail, %d, %s\n", clientHost.c_str(), clientPort, code.value(),
                   code.message().c_str());
        }
        else
        {
            printf("-------------------- on connection closed [%s:%d]\n", clientHost.c_str(), clientPort);
        }
        std::lock_guard<std::recursive_mutex> locker(g_mutex);
        auto iter = g_clientMap.find(point);
        if (g_clientMap.end() != iter)
        {
            g_clientMap.erase(iter);
        }
    });
    /* �����߳�ר����������I/O�¼���ѯ */
    std::thread th([&, certFile, privateKeyFile, privateKeyFilePwd]() {
        /* ע��: ��������쳣����, ��Ϊ�����벻��ʱ�����쳣 */
        try
        {
#if (1 == ENABLE_NSOCKET_OPENSSL)
            auto sslContext = nsocket::TcpServer::getSslContext(certFile, privateKeyFile, privateKeyFilePwd);
            server->run(sslContext);
#else
            server->run();
#endif
        }
        catch (const std::exception& e)
        {
            printf("========== execption: %s\n", e.what());
        }
        catch (...)
        {
            printf("========== execption: unknown\n");
        }
    });
    th.detach();
    /* ���߳� */
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}
