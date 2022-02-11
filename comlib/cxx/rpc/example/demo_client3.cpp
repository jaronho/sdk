#include <iostream>
#include <thread>

#include "../rpc/rpc_client.h"
#include "demo_def.h"

static std::shared_ptr<rpc::Client> s_client = nullptr;

std::vector<unsigned char> client3Func(const std::vector<unsigned char>& data)
{
    std::string str = "this is client 3";
    return std::vector<unsigned char>(str.begin(), str.end());
}

void callClient1(const PROC_TYPE& proc)
{
    printf("---------- call [cli_1] start\n");
    std::vector<unsigned char> replyData;
    auto code = s_client->call("cli_1", (int)proc, {}, replyData, std::chrono::milliseconds(0));
    printf("--- call [cli_1].[%s], %s, return: %s\n", proc_name(proc).c_str(), rpc::error_desc(code).c_str(),
           std::string(replyData.begin(), replyData.end()).c_str());
}

void callClient2(const PROC_TYPE& proc)
{
    printf("---------- call [cli_2] start\n");
    std::vector<unsigned char> replyData;
    auto code = s_client->call("cli_2", (int)proc, {}, replyData, std::chrono::milliseconds(0));
    printf("--- call [cli_2].[%s], %s, return: %s\n", proc_name(proc).c_str(), rpc::error_desc(code).c_str(),
           std::string(replyData.begin(), replyData.end()).c_str());
}

int main(int argc, char* argv[])
{
    s_client = std::make_shared<rpc::Client>("cli_3", "127.0.0.1", 4335);
    s_client->setRegHandler([&](const rpc::ErrorCode& code) { printf("register to broker %s\n", rpc::error_desc(code).c_str()); });
    s_client->setCallHandler([&](const std::string& callId, int proc, const std::vector<unsigned char>& data) {
        switch ((PROC_TYPE)proc)
        {
        case PROC_TYPE::CLIENT3_FUNC:
            return client3Func(data);
        default:
            printf("unhandle proc [%s] called by [%s]\n", proc_name((PROC_TYPE)proc).c_str(), callId.c_str());
        }
        return std::vector<unsigned char>();
    });
    /* 创建线程专门用于网络I/O事件轮询 */
    std::thread th([&]() { s_client->run(); });
    th.detach();
    /* 主线程 */
    while (1)
    {
        /* 调用客户端1接口 */
        callClient1(PROC_TYPE::CLIENT1_FUNC);
        /* 调用客户端2接口 */
        callClient2(PROC_TYPE::CLIENT2_FUNC);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}
