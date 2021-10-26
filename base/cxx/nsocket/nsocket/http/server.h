#pragma once
#include <mutex>
#include <string>
#include <unordered_map>

#include "../tcp/tcp_server.h"
#include "request.h"
#include "response.h"
#include "router.h"

namespace nsocket
{
namespace http
{
/**
 * @brief HTTP������
 */
class Server final : public std::enable_shared_from_this<Server>
{
public:
    /**
     * @brief ���캯��
     * @param host ������ַ
     * @param port �˿�
     */
    Server(const std::string& host, unsigned int port);

    /**
     * @brief ����·��δ�ҵ��ص�
     * @param cb �ص�
     */
    void setRouterNotFoundCallback(const std::function<void(const REQUEST_PTR& req)>& cb);

    /**
     * @brief ���·��
     * @param uri URI
     * @param router ·��
     */
    void addRouter(const std::string& uri, const std::shared_ptr<Router>& router);

    /**
     * @brief ����
     */
#if (1 == ENABLE_NSOCKET_OPENSSL)
    void run(const std::shared_ptr<boost::asio::ssl::context>& sslContext = nullptr);
#else
    void run();
#endif

private:
    /**
     * @brief HTTP�Ự
     */
    struct Session
    {
        std::weak_ptr<TcpSession> wpTcpSession; /* TCP�Ự */
        std::shared_ptr<Request> req; /* ���� */
    };

private:
    /**
     * @brief ����������
     */
    void handleNewConnection(const std::weak_ptr<TcpSession>& wpSession);

    /**
     * @brief ������������
     */
    void handleConnectionData(const std::weak_ptr<TcpSession>& wpSession, const std::vector<unsigned char>& data);

    /**
     * @brief �������ӶϿ�
     */
    void handleConnectionClose(int64_t sid);

    /**
     * @brief ��������ͷ
     */
    void handleReqHead(const std::shared_ptr<Session>& session);

    /**
     * @brief ������������
     */
    void handleReqContent(const std::shared_ptr<Session>& session, size_t offset, const unsigned char* data, int dataLen);

    /**
     * @brief �����������
     */
    void handleReqFinish(const std::shared_ptr<Session>& session);

private:
    std::shared_ptr<TcpServer> m_tcpServer; /* TCP������ */
    std::mutex m_mutex;
    std::unordered_map<int64_t, std::shared_ptr<Session>> m_sessionMap; /* �Ự�� */
    std::function<void(const REQUEST_PTR& req)> m_routerNotFoundCb; /* ·��δ�ҵ��ص� */
    std::unordered_map<std::string, std::shared_ptr<Router>> m_routerMap; /* ·�ɱ� */
};
} // namespace http
} // namespace nsocket
