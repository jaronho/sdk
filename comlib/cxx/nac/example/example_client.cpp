#include <atomic>
#include <iostream>
#include <thread>

#include "../nac/access_ctrl.h"
#include "access_def.h"
#include "logger/logger_manager.h"
#include "utility/filesystem/file_info.h"
#include "utility/process/process.h"
#include "utility/strtool/strtool.h"

static logger::Logger s_logger;

class Login final : public nac::AccessObserver
{
public:
    Login()
    {
        subscribeAccessState([&](const nac::ConnectState& state) { onAccessState(state); });
        subscribeAccessMsg(nac::BizCode::notify_proc_upgrade,
                           [&](unsigned long long seqId, const nlohmann::json& data) { onNotifyProcUpgrage(seqId, data); });
    }

    ~Login() {}

    void reqLogin() {}

private:
    void onAccessState(const nac::ConnectState& state)
    {
        switch (state)
        {
        case nac::ConnectState::idle:
            break;
        case nac::ConnectState::connecting:
            break;
        case nac::ConnectState::connected:
            reqLogin();
            break;
        case nac::ConnectState::disconnected:
            break;
        }
    }

    void onNotifyProcUpgrage(unsigned long long seqId, const nlohmann::json& data) {}
};

int main(int argc, char* argv[])
{
    printf("***********************************************************************************************************\n");
    printf("** This is NAC client                                                                                    **\n");
    printf("** Options:                                                                                              **\n");
    printf("**                                                                                                       **\n");
    printf("** [-s]                   server address, default: 127.0.0.1                                             **\n");
    printf("** [-p]                   server port, default: 4335                                                     **\n");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("** [-cf]                  specify certificate file. e.g. client.crt, ca.crt                              **\n");
    printf("** [-pkf]                 specify private key file, e.g. client.key                                      **\n");
    printf("** [-pkp]                 specify private key file password, e.g. qq123456                               **\n");
#endif
    printf("**                                                                                                       **\n");
    printf("***********************************************************************************************************\n");
    printf("\n");
    std::string serverHost;
    int serverPort = 0;
    std::string certFile;
    std::string privateKeyFile;
    std::string privateKeyFilePwd;
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
                privateKeyFile = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pkp")) /* 私钥文件密码 */
        {
            ++i;
            if (i < argc)
            {
                privateKeyFilePwd = argv[i];
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
        serverPort = 4335;
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
    nac::AccessConfig acfg;
    acfg.address = serverHost;
    acfg.port = serverPort;
    acfg.certFile = certFile;
    acfg.privateKeyFile = privateKeyFile;
    acfg.privateKeyFilePwd = privateKeyFilePwd;
    acfg.connectTimeout = 3;
    acfg.retryInterval = {1};
    while (!nac::AccessCtrl::getInstance().start(acfg))
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::thread th([&]() {
        while (1)
        {
            threading::Timer::runOnce();
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
            nlohmann::json data;
            long long fileSize = 0;
            if (vec.size() > 1)
            {
                auto fileData = utility::FileInfo(vec[1]).readAll(fileSize, true);
                if (fileData)
                {
                    std::string errDesc;
                    if (!nlohmann::parse(fileData, data, &errDesc))
                    {
                        ERROR_LOG(s_logger, "解析文件 {} 失败, {}", vec[1], errDesc);
                    }
                    free(fileData);
                }
                else
                {
                    fileSize = 0;
                }
            }
            auto seqId =
                nac::AccessCtrl::getInstance().sendMsg((nac::BizCode)bizCode, 0, data,
                                                       [&, bizCode](bool ok, const nlohmann::json& data) {
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