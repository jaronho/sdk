#pragma once
#include <chrono>
#include <functional>
#include <mutex>
#include <string>

#include "nsocket/http/server.h"

namespace hfs
{
/**
 * @brief ��Դ������(404)/����������(405)������
 * @param cid ����ID
 * @param req �������
 * @param conn ������, ���ڴ����������ݷ��ͺ����ӶϿ�
 * @param uri ��ԴURI
 */
using NotHandler =
    std::function<void(uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn, const std::string& uri)>;

/**
 * @brief Ŀ¼���ʴ�����
 * @param cid ����ID
 * @param req �������
 * @param conn ������, ���ڴ����������ݷ��ͺ����ӶϿ�
 * @param keepAlive �����Ƿ񱣻�, ����Ǳ�������Ҫ���������Ͽ�����
 * @param rootDir ��Դ��Ŀ¼
 * @param uri ��ǰ���ʵ����Ŀ¼(��������Ŀ¼)
 */
using DirAccessHandler = std::function<void(uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn,
                                            bool keepAlive, const std::string& rootDir, const std::string& uri)>;

/**
 * @brief �ļ���ȡ������
 * @param cid ����ID
 * @param req �������
 * @param conn ������, ���ڴ����������ݷ��ͺ����ӶϿ�
 * @param keepAlive �����Ƿ񱣻�, ����Ǳ�������Ҫ���������Ͽ�����
 * @param fileName �ļ�����·��
 * @param fileSize �ļ���С(�ֽ�)
 */
using FileGetHandler = std::function<void(uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn,
                                          bool keepAlive, const std::string& fileName, size_t fileSize)>;

/**
 * @brief HTTP�ļ�����(ע��: ��Ҫʵ����Ϊ����ָ�����ᱨ��)
 */
class HttpFileServer final : public std::enable_shared_from_this<HttpFileServer>
{
public:
    /**
     * @brief ���캯��
     * @param name ����������
     * @param threadCount �̸߳���
     * @param host ������ַ
     * @param port �˿�
     * @param rootDir ��Դ��Ŀ¼, Ĭ��ʹ�ó�������Ŀ¼, Ĭ�ϳ����ļ�����Ŀ¼
     * @param fileBlockSize ÿ�ζ�ȡ���ļ����С(�ֽ�), ȡֵ��Χ[4Kb - 16Mb], Ĭ��1Mb
     * @param reuseAddr �Ƿ������ö˿�, Ĭ�ϲ�����
     * @param bz ���ݻ�������С(�ֽ�)
     * @param handshakeTimeout ���ֳ�ʱʱ��
     */
    HttpFileServer(const std::string& name, size_t threadCount, const std::string& host, uint16_t port, std::string rootDir = "",
                   size_t fileBlockSize = 1024 * 1024, bool reuseAddr = false, size_t bz = 4096,
                   const std::chrono::steady_clock::duration& handshakeTimeout = std::chrono::seconds(3));

    virtual ~HttpFileServer();

    /**
     * @brief ��ȡ��Դ��Ŀ¼
     * @return ��Դ��Ŀ¼
     */
    std::string getRootDir() const;

    /**
     * @brief ���÷�������������(�������õĻ���ʹ���ڲ�Ĭ�ϴ�����)
     * @param handler ������
     */
    void setNotAllowHandler(const NotHandler& handler);

    /**
     * @brief ������Դ�����ڴ�����(�������õĻ���ʹ���ڲ�Ĭ�ϴ�����)
     * @param handler ������
     */
    void setNotFoundHandler(const NotHandler& handler);

    /**
     * @brief ����Ŀ¼���ʴ�����(�������õĻ���ʹ���ڲ�Ĭ�ϴ�����)
     * @param handler ������
     */
    void setDirAccessHandler(const DirAccessHandler& handler);

    /**
     * @brief �����ļ���ȡ������(�������õĻ���ʹ���ڲ�Ĭ�ϴ�����)
     * @param handler ������
     */
    void setFileGetHandler(const FileGetHandler& handler);

    /**
     * @brief ���·��
     * @param methods �����б�, Ϊ��ʱ��ʾ֧�����з���(ע��: ��������֧��1��), ����: {Method::GET}, {Method::POST}
     * @param uriList ����URI�б�, ����: {"/", "/index","/index.htm", "/index.html"}
     * @param router ·��
     * @return ��������ӹ���URI�б�
     */
    std::vector<std::string> addRouter(const std::vector<nsocket::http::Method>& methods, const std::vector<std::string>& uriList,
                                       const std::shared_ptr<nsocket::http::Router>& router);

    /**
     * @brief ����(������)
     * @param sslOn �Ƿ���SSL, true-��, false-��
     * @param sslWay SSL��֤��ʽ, 1-����, 2-˫��
     * @param certFmt (֤��/˽Կ)�ļ���ʽ, 1-DER, 2-PEM
     * @param certFile ֤���ļ�, ����: client.crt
     * @param pkFile ˽Կ�ļ�, ����; client.key
     * @param pkPwd ˽Կ�ļ�����, ����: 123456
     * @param errDesc [���]��������
     * @return true-������, false-����ʧ��
     */
    bool run(bool sslOn = false, int sslWay = 1, int certFmt = 2, const std::string& certFile = "", const std::string& pkFile = "",
             const std::string& pkPwd = "", std::string* errDesc = nullptr);

    /**
     * @brief ֹͣ
     */
    void stop();

    /**
     * @brief �Ƿ�������
     * @return true-������, false-��������
     */
    bool isRunning();

    /**
     * @brief Ĭ��Ŀ¼���ʴ�����
     * @param cid ����ID
     * @param req �������
     * @param conn ������, ���ڴ����������ݷ��ͺ����ӶϿ�
     * @param keepAlive �����Ƿ񱣻�, ����Ǳ�����������󽫶Ͽ�����
     * @param rootDir ��Դ��Ŀ¼
     * @param uri ��ǰ���ʵ����Ŀ¼(��������Ŀ¼)
     */
    void defaultDirAccessHandler(uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn, bool keepAlive,
                                 const std::string& rootDir, const std::string& uri);

    /**
     * @brief Ĭ���ļ���ȡ������
     * @param cid ����ID
     * @param req �������
     * @param conn ������, ���ڴ����������ݷ��ͺ����ӶϿ�
     * @param keepAlive �����Ƿ񱣻�, ����Ǳ�����������󽫶Ͽ�����
     * @param fileName �ļ�����·��
     * @param fileSize �ļ���С(�ֽ�)
     */
    void defaultFileGetHandler(uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn, bool keepAlive,
                               const std::string& fileName, size_t fileSize);

private:
    /**
     * @brief ����Ĭ��·��
     * @param cid ����ID
     * @param req �������
     * @param conn ������
     */
    void handleDefaultRouter(uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn);

private:
    const std::string m_name; /* ������ */
    const size_t m_threadCount; /* �̸߳��� */
    const std::string m_host; /* ������ַ */
    const uint16_t m_port; /* �˿� */
    const bool m_reuseAddr; /* /* �Ƿ������ö˿� */
    const size_t m_bufferSize; /* ��������С */
    const std::chrono::steady_clock::duration m_handshakeTimeout; /* SSL���ֳ�ʱʱ�� */
    std::string m_rootDir; /* �ļ���Դ��Ŀ¼ */
    size_t m_fileBlockSize; /* ÿ�ζ�ȡ���ļ����С */
    std::mutex m_mutexHttpServer;
    std::shared_ptr<nsocket::http::Server> m_httpServer = nullptr; /* HTTP������ */
    std::mutex m_mutexNotAllowHandler;
    NotHandler m_notAllowHandler = nullptr; /* �������������� */
    std::mutex m_mutexNotFoundHandler;
    NotHandler m_notFoundHandler = nullptr; /* ��Դ�����ڴ����� */
    std::mutex m_mutexDirAccessHandler;
    DirAccessHandler m_dirAccessHandler = nullptr; /* Ŀ¼���ʴ����� */
    std::mutex m_mutexFileGetHandler;
    FileGetHandler m_fileGetHandler = nullptr; /* �ļ���ȡ������ */
};
} // namespace hfs
