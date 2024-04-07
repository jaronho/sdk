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
 * @param conn ���Ӷ���, ���ڴ����������ݷ��ͺ����ӶϿ�
 * @param cid ����ID
 * @param req �������
 * @param uri ��ԴURI
 */
using NotHandler =
    std::function<void(const nsocket::http::Connector& conn, uint64_t cid, const nsocket::http::REQUEST_PTR& req, const std::string& uri)>;

/**
 * @brief Ŀ¼���ʴ�����
 * @param conn ���Ӷ���, ���ڴ����������ݷ��ͺ����ӶϿ�
 * @param keepAlive �����Ƿ񱣻�, ����Ǳ�������Ҫ���������Ͽ�����
 * @param rootDir ��Դ��Ŀ¼
 * @param uri ��ǰ���ʵ����Ŀ¼(��������Ŀ¼)
 */
using DirAccessHandler =
    std::function<void(const nsocket::http::Connector& conn, bool keepAlive, const std::string& rootDir, const std::string& uri)>;

/**
 * @brief �ļ���ȡ������
 * @param conn ���Ӷ���, ���ڴ����������ݷ��ͺ����ӶϿ�
 * @param keepAlive �����Ƿ񱣻�, ����Ǳ�������Ҫ���������Ͽ�����
 * @param fileName �ļ�����·��
 * @param fileSize �ļ���С(�ֽ�)
 */
using FileGetHandler =
    std::function<void(const nsocket::http::Connector& conn, bool keepAlive, const std::string& fileName, size_t fileSize)>;

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
     * @param reuseAddr �Ƿ������ö˿�, Ĭ�ϲ�����
     * @param bz ���ݻ�������С(�ֽ�)
     * @param handshakeTimeout ���ֳ�ʱʱ��
     */
    HttpFileServer(const std::string& name, size_t threadCount, const std::string& host, uint16_t port, bool reuseAddr = false,
                   size_t bz = 4096, const std::chrono::steady_clock::duration& handshakeTimeout = std::chrono::seconds(3));

    virtual ~HttpFileServer();

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
     * @param rootDir ��Դ��Ŀ¼, Ĭ��ʹ�ó�������Ŀ¼
     * @param sslOn �Ƿ���SSL, true-��, false-��
     * @param sslWay SSL��֤��ʽ, 1-����, 2-˫��
     * @param certFmt (֤��/˽Կ)�ļ���ʽ, 1-DER, 2-PEM
     * @param certFile ֤���ļ�, ����: client.crt
     * @param pkFile ˽Կ�ļ�, ����; client.key
     * @param pkPwd ˽Կ�ļ�����, ����: 123456
     * @param errDesc [���]��������
     * @return true-������, false-����ʧ��
     */
    bool run(std::string rootDir = "", bool sslOn = false, int sslWay = 1, int certFmt = 2, const std::string& certFile = "",
             const std::string& pkFile = "", const std::string& pkPwd = "", std::string* errDesc = nullptr);

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
     * @brief ��ȡ��Դ��Ŀ¼
     * @return ��Դ��Ŀ¼
     */
    std::string getRootDir();

private:
    /**
     * @brief ����Ĭ��·��
     */
    void handleDefaultRouter(uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn);

    /**
     * @brief ����Ŀ¼
     */
    void handleDir(const nsocket::http::Connector& conn, const std::string& rootDir, const std::string& uri);

    /**
     * @brief �����ļ�
     */
    void handleFile(const nsocket::http::Connector& conn, const std::string& fileName, size_t fileSize);

private:
    const std::string m_name; /* ������ */
    const size_t m_threadCount; /* �̸߳��� */
    const std::string m_host; /* ������ַ */
    const uint16_t m_port; /* �˿� */
    const bool m_reuseAddr; /* /* �Ƿ������ö˿� */
    const size_t m_bufferSize; /* ��������С */
    const std::chrono::steady_clock::duration m_handshakeTimeout; /* SSL���ֳ�ʱʱ�� */
    std::mutex m_mutexHttpServer;
    std::shared_ptr<nsocket::http::Server> m_httpServer = nullptr; /* HTTP������ */
    std::mutex m_mutexRootDir;
    std::string m_rootDir; /* �ļ���Դ��Ŀ¼ */
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
