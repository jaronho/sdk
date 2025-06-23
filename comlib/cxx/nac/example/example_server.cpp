#include <atomic>
#include <iostream>
#include <thread>
#ifdef _WIN32
#include <Windows.h>
#endif

#include "access_def.h"
#include "logger/logger_manager.h"
#include "nsocket/payload.h"
#include "nsocket/tcp/tcp_server.h"
#include "utility/bytearray/bytearray.h"
#include "utility/filesystem/file_info.h"
#include "utility/process/process.h"

static const size_t MAX_BODY_SIZE = 10 * 1024 * 1024; /* 最大包体大小 */

/**
 * @brief 包头
 */
struct PacketHead
{
    static uint32_t headSize()
    {
        return (sizeof(int32_t) + sizeof(int32_t) + sizeof(int32_t) + sizeof(int64_t));
    }

    int32_t version = 0; /* 版本号(4个字节) */
    int32_t bodyLen = 0; /* 包体长度(4个字节) */
    int32_t bizCode = 0; /* 业务码(4个字节) */
    int64_t seqId = 0; /* 序列ID(8个字节) */
};

/**
 * @brief 数据包
 */
struct Packet
{
    int32_t version = 0; /* 版本号(4个字节) */
    int32_t bizCode = 0; /* 业务码(4个字节) */
    int64_t seqId = 0; /* 序列ID(8个字节) */
    std::string data; /* 业务数据 */
};

/**
 * @brief 客户端信息
 */
struct ClientInfo
{
    std::weak_ptr<nsocket::TcpConnection> wpConn; /* 连接 */
    std::shared_ptr<nsocket::Payload> payload; /* 负载数据 */
    std::shared_ptr<PacketHead> pktHead; /* 包头 */
};

static logger::Logger s_logger;
std::recursive_mutex g_mutex;
std::unordered_map<boost::asio::ip::tcp::endpoint, ClientInfo> g_clientMap; /* 客户端映射表 */

void handleNewConnection(const std::shared_ptr<nsocket::TcpConnection>& conn)
{
    if (conn)
    {
        auto point = conn->getRemoteEndpoint();
        ClientInfo ci;
        ci.wpConn = conn;
        ci.payload = std::make_shared<nsocket::Payload>(PacketHead::headSize());
        ci.pktHead = std::make_shared<PacketHead>();
        ci.pktHead->version = NAC_PROTOCOL_VERSION;
        std::lock_guard<std::recursive_mutex> locker(g_mutex);
        auto iter = g_clientMap.find(point);
        if (g_clientMap.end() == iter)
        {
            g_clientMap.insert(std::make_pair(point, ci));
        }
    }
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    printf("***********************************************************************************************************\n");
    printf("** This is NAC server                                                                                    **\n");
    printf("** Options:                                                                                              **\n");
    printf("**                                                                                                       **\n");
    printf("** [-s]                   server address, default: 127.0.0.1                                             **\n");
    printf("** [-p]                   server port, default: 4444                                                     **\n");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("** [-tls]                 specify enable ssl [0-disable, 1-enable]. default: 0                           **\n");
    printf("** [-way]                 specify ssl way verify [1, 2], default: 1                                      **\n");
    printf("** [-pem]                 specify file format [1-DER, 2-PEM]. default: 2                                 **\n");
    printf("** [-cf]                  specify certificate file. e.g. client.crt, ca.crt                              **\n");
    printf("** [-pkf]                 specify private key file, e.g. client.key                                      **\n");
    printf("** [-pkp]                 specify private key file password, e.g. qq123456                               **\n");
#endif
    printf("**                                                                                                       **\n");
    printf("***********************************************************************************************************\n");
    printf("\n");
    std::string serverHost;
    int serverPort = 0;
    int sslOn = 0;
    int sslWay = 1;
    int certFmt = 2;
    std::string certFile;
    std::string pkFile;
    std::string pkPwd;
    for (int i = 1; i < argc;)
    {
        const char* key = argv[i];
        if (0 == strcmp(key, "-s")) /* 服务器地址 */
        {
            ++i;
            if (i < argc)
            {
                serverHost = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-p")) /* 服务器端口 */
        {
            ++i;
            if (i < argc)
            {
                serverPort = atoi(argv[i]);
                ++i;
            }
        }
#if (1 == ENABLE_NSOCKET_OPENSSL)
        else if (0 == strcmp(key, "-tls")) /* 是否启用TLS */
        {
            ++i;
            if (i < argc)
            {
                sslOn = atoi(argv[i]);
                ++i;
            }
        }
        else if (0 == strcmp(key, "-way")) /* SSL校验 */
        {
            ++i;
            if (i < argc)
            {
                sslWay = atoi(argv[i]);
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pem")) /* 文件格式 */
        {
            ++i;
            if (i < argc)
            {
                certFmt = atoi(argv[i]);
                ++i;
            }
        }
        else if (0 == strcmp(key, "-cf")) /* 证书文件 */
        {
            ++i;
            if (i < argc)
            {
                certFile = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pkf")) /* 私钥文件 */
        {
            ++i;
            if (i < argc)
            {
                pkFile = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pkp")) /* 私钥文件密码 */
        {
            ++i;
            if (i < argc)
            {
                pkPwd = argv[i];
                ++i;
            }
        }
#endif
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
        serverPort = 4444;
    }
    if (sslOn < 0)
    {
        sslOn = 0;
    }
    else if (sslOn > 1)
    {
        sslOn = 1;
    }
    if (sslWay < 1)
    {
        sslWay = 1;
    }
    else if (sslWay > 2)
    {
        sslWay = 2;
    }
    if (certFmt < 1)
    {
        certFmt = 1;
    }
    else if (certFmt > 2)
    {
        certFmt = 2;
    }
    /* 初始日志模块 */
    logger::LogConfig lcfg;
    lcfg.path = utility::FileInfo(utility::Process::getProcessExeFile()).path();
    lcfg.name = "server_"; /* 设置默认日志文件名前缀 */
    lcfg.fileExtName = ".log";
    lcfg.fileMaxSize = 10 * 1024 * 1024;
    lcfg.fileMaxCount = 5;
    lcfg.newFolderDaily = true;
    lcfg.consoleMode = 1;
    logger::LoggerManager::setConfig(lcfg);
    s_logger = logger::LoggerManager::getLogger();
    /* 创建服务器 */
    auto server = std::make_shared<nsocket::TcpServer>("tcp_server", 10, serverHost, serverPort, false, 10 * 1024 * 1024);
    /* 设置新连接回调 */
    server->setNewConnectionCallback([&](const std::weak_ptr<nsocket::TcpConnection>& wpConn) {
        const auto conn = wpConn.lock();
        if (conn)
        {
            auto point = conn->getRemoteEndpoint();
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            INFO_LOG(s_logger, "新建客户端连接[{}], [{}:{}]", conn->getId(), clientHost, clientPort);
            if (!conn->isEnableSSL())
            {
                handleNewConnection(conn);
            }
        }
    });
    /* 设置握手成功回调 */
    server->setHandshakeOkCallback([&](const std::weak_ptr<nsocket::TcpConnection>& wpConn) {
        const auto conn = wpConn.lock();
        if (conn)
        {
            auto point = conn->getRemoteEndpoint();
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            INFO_LOG(s_logger, "客户端验证成功[{}], [{}:{}]", conn->getId(), clientHost, clientPort);
            handleNewConnection(conn);
        }
    });
    /* 设置握手失败回调 */
    server->setHandshakeFailCallback([&](uint64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code) {
        std::string clientHost = point.address().to_string().c_str();
        int clientPort = (int)point.port();
        WARN_LOG(s_logger, "客户端验证失败[{}], [{}:{}], {}, {}", cid, clientHost, clientPort, code.value(), code.message());
    });
    /* 设置连接数据回调 */
    server->setConnectionDataCallback([&](const std::weak_ptr<nsocket::TcpConnection>& wpConn, const std::vector<unsigned char>& data) {
        const auto conn = wpConn.lock();
        if (conn)
        {
            auto point = conn->getRemoteEndpoint();
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            //DEBUG_LOG(s_logger, "收到客户端数据[{}] [{}:{}], 长度: {}\n{}", conn->getId(), clientHost, clientPort, data.size(),
            //          std::string(data.begin(), data.end()));
            /* 解包 */
            {
                std::lock_guard<std::recursive_mutex> locker(g_mutex);
                auto iter = g_clientMap.find(point);
                if (g_clientMap.end() != iter)
                {
                    auto payload = iter->second.payload;
                    auto pktHead = iter->second.pktHead;
                    payload->unpack(
                        data,
                        [&](const std::vector<unsigned char>& head) {
                            int offset = 0;
                            auto version = utility::ByteArray::read32(head.data() + offset, true); /* 版本号 */
                            if (version != pktHead->version)
                            {
                                ERROR_LOG(s_logger, "数据解析错误: 包版本号 {} 与 {} 不匹配.", version, pktHead->version);
                                return -1;
                            }
                            offset += sizeof(version);
                            pktHead->bodyLen = utility::ByteArray::read32(head.data() + offset, true); /* 包体长度 */
                            if (pktHead->bodyLen >= MAX_BODY_SIZE) /* 限制数据包大小 */
                            {
                                ERROR_LOG(s_logger, "数据解析错误: 包体大小 {} 太长.", pktHead->bodyLen);
                                return -1;
                            }
                            offset += sizeof(pktHead->bodyLen);
                            pktHead->bizCode = utility::ByteArray::read32(head.data() + offset, true); /* 业务码 */
                            offset += sizeof(pktHead->bizCode);
                            pktHead->seqId = utility::ByteArray::read64(head.data() + offset, true); /* 序列ID */
                            return pktHead->bodyLen;
                        },
                        [&](const std::vector<unsigned char>& body) {
                            auto pkt = std::make_shared<Packet>();
                            pkt->version = pktHead->version;
                            pkt->bizCode = pktHead->bizCode;
                            pkt->seqId = pktHead->seqId;
                            if (!body.empty())
                            {
                                pkt->data.insert(pkt->data.end(), body.begin(), body.end());
                            }
                            auto dataLength = payload->getHeadLen() + body.size();
                            if (pkt->data.empty())
                            {
                                INFO_LOG(s_logger, "收到客户端数据[{}] [{}:{}], 总长度: {}, bizCode: {}, seqId: {}, 包体长度: {}",
                                         conn->getId(), clientHost, clientPort, dataLength, pkt->bizCode, pkt->seqId, body.size());
                            }
                            else
                            {
                                INFO_LOG(s_logger, "收到客户端数据[{}] [{}:{}]\n{}", conn->getId(), clientHost, clientPort, pkt->data);
                                INFO_LOG(s_logger, "总长度: {}, bizCode: {}, seqId: {}, 包体长度: {}", dataLength, pkt->bizCode, pkt->seqId,
                                         body.size());
                            }
                            /* 应答 */
                            std::vector<unsigned char> buffer;
                            utility::ByteArray::write32(buffer, pkt->version, true); /* 版本号 */
                            utility::ByteArray::write32(buffer, 0, true); /* 包体长度 */
                            utility::ByteArray::write32(buffer, pkt->bizCode, true); /* 业务码 */
                            utility::ByteArray::write64(buffer, pkt->seqId, true); /* 序列ID */
                            conn->send(buffer, nullptr);
                        });
                }
            }
        }
    });
    /* 设置连接关闭回调 */
    server->setConnectionCloseCallback(
        [&](int64_t cid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code) {
            std::string clientHost = point.address().to_string().c_str();
            int clientPort = (int)point.port();
            if (code)
            {
                ERROR_LOG(s_logger, "客户端连接断开[{}], [{}:{}] 失败, {}, {}", cid, clientHost, clientPort, code.value(), code.message());
            }
            else
            {
                INFO_LOG(s_logger, "客户端连接断开[{}], [{}:{}]", cid, clientHost, clientPort);
            }
            std::lock_guard<std::recursive_mutex> locker(g_mutex);
            auto iter = g_clientMap.find(point);
            if (g_clientMap.end() != iter)
            {
                g_clientMap.erase(iter);
            }
        });
    /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
    try
    {
        server->run(sslOn, sslWay, certFmt, certFile, pkFile, pkPwd);
        /* 主线程 */
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    catch (const std::exception& e)
    {
        ERROR_LOG(s_logger, "异常: {}", e.what());
    }
    catch (...)
    {
        ERROR_LOG(s_logger, "异常: 未知.");
    }
    return 0;
}
