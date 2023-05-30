#include <atomic>
#include <iostream>
#include <thread>

#include "../nac/tclient/access_ctrl.h"
#include "access_def.h"
#include "logger/logger_manager.h"
#include "utility/filesystem/file_info.h"
#include "utility/process/process.h"
#include "utility/strtool/strtool.h"

static logger::Logger s_logger;
static threading::ExecutorPtr s_logicExecutor = nullptr; /* 逻辑线程 */

class Login final : public nac::tcli::AccessObserver
{
public:
    Login()
    {
        subscribeAccessState([&](const nac::tcli::ConnectState& state) { onAccessState(state); });
        subscribeAccessMsg((int32_t)BizCode::notify_proc_upgrade,
                           [&](unsigned long long seqId, const std::string& data) { onNotifyProcUpgrage(seqId, data); });
    }

    ~Login() {}

    void reqLogin() {}

private:
    void onAccessState(const nac::tcli::ConnectState& state)
    {
        switch (state)
        {
        case nac::tcli::ConnectState::idle:
            break;
        case nac::tcli::ConnectState::connecting:
            break;
        case nac::tcli::ConnectState::connected:
            reqLogin();
            break;
        case nac::tcli::ConnectState::disconnected:
            break;
        }
    }

    void onNotifyProcUpgrage(unsigned long long seqId, const std::string& data) {}
};

int main(int argc, char* argv[])
{
    printf("***********************************************************************************************************\n");
    printf("** This is NAC client                                                                                    **\n");
    printf("** Options:                                                                                              **\n");
    printf("**                                                                                                       **\n");
    printf("** [-s]                   server address, default: 127.0.0.1                                             **\n");
    printf("** [-p]                   server port, default: 4444                                                     **\n");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("** [-tls]                 specify enable ssl [0-disable, 1-enable]. default: 0                           **\n");
    printf("** [-w]                   specify ssl way verify [0, 1, 2], default: 0                                   **\n");
    printf("** [-pem]                 specify file format [0-DER, 1-PEM]. default: 1                                 **\n");
    printf("** [-cf]                  specify certificate file. e.g. client.crt, ca.crt                              **\n");
    printf("** [-pkf]                 specify private key file, e.g. client.key                                      **\n");
    printf("** [-pkp]                 specify private key file password, e.g. qq123456                               **\n");
#endif
    printf("**                                                                                                       **\n");
    printf("***********************************************************************************************************\n");
    printf("\n");
    std::string serverHost;
    int serverPort = 0;
    int tls = 0;
    int sslWay = 1;
    int pem = 1;
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
                tls = atoi(argv[i]);
                ++i;
            }
        }
        else if (0 == strcmp(key, "-w")) /* SSL校验 */
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
                pem = atoi(argv[i]);
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
    if (tls < 0)
    {
        tls = 0;
    }
    else if (tls > 1)
    {
        tls = 1;
    }
    if (sslWay < 0)
    {
        sslWay = 1;
    }
    else if (sslWay > 2)
    {
        sslWay = 2;
    }
    if (pem < 0)
    {
        pem = 0;
    }
    else if (pem > 1)
    {
        pem = 1;
    }
    /* 初始日志模块 */
    logger::LogConfig lcfg;
    lcfg.path = utility::FileInfo(utility::Process::getProcessExeFile()).path();
    lcfg.name = "client_"; /* 设置默认日志文件名前缀 */
    lcfg.fileExtName = ".log";
    lcfg.fileMaxSize = 10 * 1024 * 1024;
    lcfg.fileMaxCount = 5;
    lcfg.newFolderDaily = true;
    lcfg.consoleEnable = true;
    logger::LoggerManager::start(lcfg);
    s_logger = logger::LoggerManager::getLogger();
    /* 业务模块 */
    Login login;
    /* 启动网络接入模块 */
    s_logicExecutor = threading::ThreadProxy::createAsioExecutor("logic", 1);
    nac::tcli::AccessCtrl::start(std::make_shared<nac::tcli::ProtocolAdapterCustom>(NAC_PROTOCOL_VERSION), s_logicExecutor);
    nac::tcli::AccessCtrl::setPacketVersionMismatchCallback([&](int32_t localVersion, int32_t pktVersion) {});
    nac::tcli::AccessCtrl::setPacketLengthAbnormalCallback([&](int32_t maxLength, int32_t pktLength) {});
    nac::tcli::AccessConfig acfg;
    acfg.address = serverHost;
    acfg.port = serverPort;
    acfg.sslOn = tls;
    acfg.sslWay = sslWay;
    acfg.certFmt = pem;
    acfg.certFile = certFile;
    acfg.pkFile = pkFile;
    acfg.pkPwd = pkPwd;
    acfg.connectTimeout = 3;
    acfg.retryInterval = {1};
    while (!nac::tcli::AccessCtrl::connect(acfg))
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::thread th([&]() {
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    th.detach();
    /* 主线程 */
    while (1)
    {
        /* 接收输入数据并发送 */
        char buffer[1024] = {0};
        std::cin.getline(buffer, sizeof(buffer));
        if (0 == strlen(buffer)) /* 输入为空继续等待 */
        {
            continue;
        }
        std::string str(buffer);
        utility::StrTool::trimDuplicate(str, ' ');
        auto vec = utility::StrTool::split(str, " ");
        if (vec.empty())
        {
            continue;
        }
        try
        {
            auto bizCode = std::atoi(vec[0].c_str());
            std::string data;
            long long fileSize = 0;
            if (vec.size() > 1)
            {
                auto fileData = utility::FileInfo(vec[1]).readAll(fileSize, true);
                if (fileData)
                {
                    data = fileData;
                    free(fileData);
                }
                else
                {
                    fileSize = 0;
                }
            }
            auto seqId = nac::tcli::AccessCtrl::sendMsg(
                bizCode, 0, data,
                [&, bizCode](bool ok, const std::string& data) {
                    INFO_LOG(s_logger, "响应消息, bizCode: {} {}", bizCode, ok ? "成功." : "失败.");
                },
                10);
            INFO_LOG(s_logger, "发送消息, 包大小: {}, bizCode: {} {}", (size_t)16 + fileSize, bizCode,
                     seqId > 0 ? "成功, seqId: " + std::to_string(seqId) + "." : "失败.");
        }
        catch (...)
        {
        }
    }
    return 0;
}
