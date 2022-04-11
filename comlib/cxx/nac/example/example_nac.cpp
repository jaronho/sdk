#include <atomic>
#include <iostream>
#include <thread>

#include "../nac/access_ctrl.h"
#include "utility/filesystem/file_info.h"
#include "utility/process/process.h"

namespace nac
{
/**
 * @brief ҵ����
 */
enum class BizCode
{
    /*********************************************************************
     * �ͻ�������(`req_`), ȡֵ: 1xxx
     *********************************************************************/
    req_hearbeat = 1001, /* ���� */
    req_auth = 1002, /* ��Ȩ */
    req_login = 1003, /* ��¼ */
    /*********************************************************************
     * �����֪ͨ(`notify_`), ȡֵ: 2xxx
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
    /* ��ʼ��־ģ�� */
    logger::LogConfig lcfg;
    lcfg.path = utility::FileInfo(utility::Process::getProcessExeFile()).path();
    lcfg.name = "access_"; /* ����Ĭ����־�ļ���ǰ׺ */
    lcfg.fileExtName = ".log";
    lcfg.fileMaxSize = 10 * 1024 * 1024;
    lcfg.fileMaxCount = 5;
    lcfg.newFolderDaily = true;
    lcfg.consoleEnable = true;
    logger::LoggerManager::start(lcfg);
    s_logger = logger::LoggerManager::getLogger();
    /* ҵ��ģ�� */
    Login login;
    /* �����������ģ�� */
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
    /* ���߳� */
    while (1)
    {
        threading::Timer::runOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}
