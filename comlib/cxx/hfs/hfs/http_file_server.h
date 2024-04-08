#pragma once
#include <chrono>
#include <functional>
#include <mutex>
#include <string>

#include "nsocket/http/server.h"

namespace hfs
{
/**
 * @brief 资源不存在(404)/方法不允许(405)处理器
 * @param cid 连接ID
 * @param req 请求对象
 * @param conn 连接器, 用于处理器内数据发送和连接断开
 * @param uri 资源URI
 */
using NotHandler =
    std::function<void(uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn, const std::string& uri)>;

/**
 * @brief 目录访问处理器
 * @param cid 连接ID
 * @param req 请求对象
 * @param conn 连接器, 用于处理器内数据发送和连接断开
 * @param keepAlive 连接是否保活, 如果非保活则需要处理器最后断开连接
 * @param rootDir 资源根目录
 * @param uri 当前访问的相对目录(不包含根目录)
 */
using DirAccessHandler = std::function<void(uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn,
                                            bool keepAlive, const std::string& rootDir, const std::string& uri)>;

/**
 * @brief 文件获取处理器
 * @param cid 连接ID
 * @param req 请求对象
 * @param conn 连接器, 用于处理器内数据发送和连接断开
 * @param keepAlive 连接是否保活, 如果非保活则需要处理器最后断开连接
 * @param fileName 文件完整路径
 * @param fileSize 文件大小(字节)
 */
using FileGetHandler = std::function<void(uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn,
                                          bool keepAlive, const std::string& fileName, size_t fileSize)>;

/**
 * @brief HTTP文件服务(注意: 需要实例化为共享指针否则会报错)
 */
class HttpFileServer final : public std::enable_shared_from_this<HttpFileServer>
{
public:
    /**
     * @brief 构造函数
     * @param name 服务器名称
     * @param threadCount 线程个数
     * @param host 主机地址
     * @param port 端口
     * @param rootDir 资源根目录, 默认使用程序所在目录, 默认程序文件所在目录
     * @param fileBlockSize 每次读取的文件块大小(字节), 取值范围[4Kb - 16Mb], 默认1Mb
     * @param reuseAddr 是否允许复用端口, 默认不复用
     * @param bz 数据缓冲区大小(字节)
     * @param handshakeTimeout 握手超时时间
     */
    HttpFileServer(const std::string& name, size_t threadCount, const std::string& host, uint16_t port, std::string rootDir = "",
                   size_t fileBlockSize = 1024 * 1024, bool reuseAddr = false, size_t bz = 4096,
                   const std::chrono::steady_clock::duration& handshakeTimeout = std::chrono::seconds(3));

    virtual ~HttpFileServer();

    /**
     * @brief 获取资源根目录
     * @return 资源根目录
     */
    std::string getRootDir() const;

    /**
     * @brief 设置方法不允许处理器(若不设置的话则使用内部默认处理器)
     * @param handler 处理器
     */
    void setNotAllowHandler(const NotHandler& handler);

    /**
     * @brief 设置资源不存在处理器(若不设置的话则使用内部默认处理器)
     * @param handler 处理器
     */
    void setNotFoundHandler(const NotHandler& handler);

    /**
     * @brief 设置目录访问处理器(若不设置的话则使用内部默认处理器)
     * @param handler 处理器
     */
    void setDirAccessHandler(const DirAccessHandler& handler);

    /**
     * @brief 设置文件获取处理器(若不设置的话则使用内部默认处理器)
     * @param handler 处理器
     */
    void setFileGetHandler(const FileGetHandler& handler);

    /**
     * @brief 添加路由
     * @param methods 方法列表, 为空时表示支持所有方法(注意: 正常有且支持1种), 例如: {Method::GET}, {Method::POST}
     * @param uriList 服务URI列表, 例如: {"/", "/index","/index.htm", "/index.html"}
     * @param router 路由
     * @return 返回已添加过的URI列表
     */
    std::vector<std::string> addRouter(const std::vector<nsocket::http::Method>& methods, const std::vector<std::string>& uriList,
                                       const std::shared_ptr<nsocket::http::Router>& router);

    /**
     * @brief 运行(非阻塞)
     * @param sslOn 是否开启SSL, true-是, false-否
     * @param sslWay SSL验证方式, 1-单向, 2-双向
     * @param certFmt (证书/私钥)文件格式, 1-DER, 2-PEM
     * @param certFile 证书文件, 例如: client.crt
     * @param pkFile 私钥文件, 例如; client.key
     * @param pkPwd 私钥文件密码, 例如: 123456
     * @param errDesc [输出]错误描述
     * @return true-运行中, false-运行失败
     */
    bool run(bool sslOn = false, int sslWay = 1, int certFmt = 2, const std::string& certFile = "", const std::string& pkFile = "",
             const std::string& pkPwd = "", std::string* errDesc = nullptr);

    /**
     * @brief 停止
     */
    void stop();

    /**
     * @brief 是否运行中
     * @return true-运行中, false-非运行中
     */
    bool isRunning();

    /**
     * @brief 默认目录访问处理器
     * @param cid 连接ID
     * @param req 请求对象
     * @param conn 连接器, 用于处理器内数据发送和连接断开
     * @param keepAlive 连接是否保活, 如果非保活则处理器最后将断开连接
     * @param rootDir 资源根目录
     * @param uri 当前访问的相对目录(不包含根目录)
     */
    void defaultDirAccessHandler(uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn, bool keepAlive,
                                 const std::string& rootDir, const std::string& uri);

    /**
     * @brief 默认文件获取处理器
     * @param cid 连接ID
     * @param req 请求对象
     * @param conn 连接器, 用于处理器内数据发送和连接断开
     * @param keepAlive 连接是否保活, 如果非保活则处理器最后将断开连接
     * @param fileName 文件完整路径
     * @param fileSize 文件大小(字节)
     */
    void defaultFileGetHandler(uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn, bool keepAlive,
                               const std::string& fileName, size_t fileSize);

private:
    /**
     * @brief 处理默认路由
     * @param cid 连接ID
     * @param req 请求对象
     * @param conn 连接器
     */
    void handleDefaultRouter(uint64_t cid, const nsocket::http::REQUEST_PTR& req, const nsocket::http::Connector& conn);

private:
    const std::string m_name; /* 服务名 */
    const size_t m_threadCount; /* 线程个数 */
    const std::string m_host; /* 主机地址 */
    const uint16_t m_port; /* 端口 */
    const bool m_reuseAddr; /* /* 是否允许复用端口 */
    const size_t m_bufferSize; /* 缓冲区大小 */
    const std::chrono::steady_clock::duration m_handshakeTimeout; /* SSL握手超时时间 */
    std::string m_rootDir; /* 文件资源根目录 */
    size_t m_fileBlockSize; /* 每次读取的文件块大小 */
    std::mutex m_mutexHttpServer;
    std::shared_ptr<nsocket::http::Server> m_httpServer = nullptr; /* HTTP服务器 */
    std::mutex m_mutexNotAllowHandler;
    NotHandler m_notAllowHandler = nullptr; /* 方法不允许处理器 */
    std::mutex m_mutexNotFoundHandler;
    NotHandler m_notFoundHandler = nullptr; /* 资源不存在处理器 */
    std::mutex m_mutexDirAccessHandler;
    DirAccessHandler m_dirAccessHandler = nullptr; /* 目录访问处理器 */
    std::mutex m_mutexFileGetHandler;
    FileGetHandler m_fileGetHandler = nullptr; /* 文件获取处理器 */
};
} // namespace hfs
