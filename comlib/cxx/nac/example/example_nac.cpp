#include <atomic>
#include <iostream>
#include <thread>

#include "../nac/access_ctrl.h"
#include "utility/filesystem/file_info.h"
#include "utility/process/process.h"

namespace nac
{
/**
 * @brief 业务码
 */
enum class BizCode
{
    /*********************************************************************
     * 客户端请求(`req_`), 取值: 1xxx
     *********************************************************************/
    req_hearbeat = 1001, /* 心跳 */
    req_auth = 1002, /* 鉴权 */
    req_login = 1003, /* 登录 */
    /*********************************************************************
     * 服务端通知(`notify_`), 取值: 2xxx
     *********************************************************************/
};
} // namespace nac

static logger::Logger s_logger;

class Login final : public nac::AccessObserver
{
public:
    Login()
    {
        subscribeAccessState([&](const nac::ConnectState& state) {
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
        });
    }

    ~Login()
    {
        int a = 0;
    }

    void reqLogin()
    {
        nlohmann::json data;
        nac::AccessCtrl::getInstance().sendMsg(nac::BizCode::req_login, data, [&](bool ok, const nlohmann::json& data) {});
    }
};

int main(int argc, char* argv[])
{
    /* 初始日志模块 */
    logger::LogConfig lcfg;
    lcfg.path = utility::FileInfo(utility::Process::getProcessExeFile()).path();
    lcfg.name = "access_"; /* 设置默认日志文件名前缀 */
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
    acfg.address = "192.168.3.109";
    acfg.port = 9090;
    acfg.certFile = "";
    acfg.privateKeyFile = "";
    acfg.privateKeyFilePwd = "";
    acfg.connectTimeout = 3;
    acfg.authBizCode = nac::BizCode::req_auth;
    acfg.authTimeout = 30;
    acfg.heartbeatBizCode = nac::BizCode::req_hearbeat;
    acfg.heatbeatInterval = 15;
    acfg.retryInterval = {1};
    while (!nac::AccessCtrl::getInstance().start(acfg))
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    /* 主线程 */
    while (1)
    {
        threading::Timer::runOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}
