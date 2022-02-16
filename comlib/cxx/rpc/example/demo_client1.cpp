#include <iostream>
#include <thread>

#include "../rpc/rpc_client.h"
#include "demo_def.h"

static std::shared_ptr<rpc::Client> s_client = nullptr;

std::vector<unsigned char> client1Func(const std::vector<unsigned char>& data)
{
    static int64_t i = 0;
    std::string str = "this is client 1, " + std::to_string(++i);
    return std::vector<unsigned char>(str.begin(), str.end());
}

void callClient2(const PROC_TYPE& proc)
{
    auto tp = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    printf("---------- %lld, call [cli_2] start\n", tt);
    std::vector<unsigned char> replyData;
    auto code = s_client->call("cli_2", (int)proc, {}, replyData);
    printf("--- call [cli_2].[%s], %s, return: %s\n", proc_name(proc).c_str(), rpc::error_desc(code).c_str(),
           std::string(replyData.begin(), replyData.end()).c_str());

    //s_client->callAsync("cli_2", (int)proc, {}, [&, proc](const std::vector<unsigned char> replyData, const rpc::ErrorCode& code) {
    //    printf("--- call [cli_2].[%s], %s, return: %s\n", proc_name(proc).c_str(), rpc::error_desc(code).c_str(),
    //           std::string(replyData.begin(), replyData.end()).c_str());
    //});
}

void callClient3(const PROC_TYPE& proc)
{
    auto tp = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    printf("---------- %lld, call [cli_3] start\n", tt);
    std::vector<unsigned char> replyData;
    auto code = s_client->call("cli_3", (int)proc, {}, replyData);
    printf("--- call [cli_3].[%s], %s, return: %s\n", proc_name(proc).c_str(), rpc::error_desc(code).c_str(),
           std::string(replyData.begin(), replyData.end()).c_str());

    //s_client->callAsync("cli_3", (int)proc, {}, [&, proc](const std::vector<unsigned char> replyData, const rpc::ErrorCode& code) {
    //    printf("--- call [cli_3].[%s], %s, return: %s\n", proc_name(proc).c_str(), rpc::error_desc(code).c_str(),
    //           std::string(replyData.begin(), replyData.end()).c_str());
    //});
}

int main(int argc, char* argv[])
{
    s_client = std::make_shared<rpc::Client>("cli_1", "127.0.0.1", 4335);
    s_client->setRegHandler([&](const rpc::ErrorCode& code) { printf("register to broker %s\n", rpc::error_desc(code).c_str()); });
    s_client->setCallHandler([&](const std::string& callId, int proc, const std::vector<unsigned char>& data) {
        switch ((PROC_TYPE)proc)
        {
        case PROC_TYPE::CLIENT1_FUNC:
            return client1Func(data);
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
        /* 调用客户端2接口 */
        callClient2(PROC_TYPE::CLIENT2_FUNC);
        /* 调用客户端3接口 */
        callClient3(PROC_TYPE::CLIENT3_FUNC);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}
