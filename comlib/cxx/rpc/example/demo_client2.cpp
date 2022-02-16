#include <iostream>
#include <thread>

#include "../rpc/rpc_client.h"
#include "demo_def.h"

static std::shared_ptr<rpc::Client> s_client = nullptr;
static bool s_syncFlag = true;

std::vector<unsigned char> client2Func(const std::vector<unsigned char>& data)
{
    static int64_t i = 0;
    std::string str = "this is client 2, " + std::to_string(++i);
    return std::vector<unsigned char>(str.begin(), str.end());
}

void callClient(const std::string& replyer, const PROC_TYPE& proc)
{
    auto tm1 = std::chrono::steady_clock::now();
    if (s_syncFlag)
    {
        std::vector<unsigned char> replyData;
        auto code = s_client->call(replyer, (int)proc, {}, replyData);
        auto diff = std::chrono::steady_clock::now() - tm1;
        printf("--- call [%s].[%s], %s, return: %s, cost: %lld(ms)\n", replyer.c_str(), proc_name(proc).c_str(),
               rpc::error_desc(code).c_str(), std::string(replyData.begin(), replyData.end()).c_str(),
               std::chrono::duration_cast<std::chrono::milliseconds>(diff).count());
    }
    else
    {
        s_client->callAsync(
            replyer, (int)proc, {}, [&, replyer, proc, tm1](const std::vector<unsigned char> replyData, const rpc::ErrorCode& code) {
                auto diff = std::chrono::steady_clock::now() - tm1;
                printf("--- callAsync [%s].[%s], %s, return: %s, cost: %lld(ms)\n", replyer.c_str(), proc_name(proc).c_str(),
                       rpc::error_desc(code).c_str(), std::string(replyData.begin(), replyData.end()).c_str(),
                       std::chrono::duration_cast<std::chrono::milliseconds>(diff).count());
            });
    }
}

int main(int argc, char* argv[])
{
    if (argc > 1 && 0 == strcmp("0", argv[1]))
    {
        s_syncFlag = false;
    }
    s_client = std::make_shared<rpc::Client>("cli_2", "127.0.0.1", 4335);
    s_client->setRegHandler([&](const rpc::ErrorCode& code) { printf("register to broker %s\n", rpc::error_desc(code).c_str()); });
    s_client->setCallHandler([&](const std::string& callId, int proc, const std::vector<unsigned char>& data) {
        switch ((PROC_TYPE)proc)
        {
        case PROC_TYPE::CLIENT2_FUNC:
            return client2Func(data);
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
        callClient("cli_1", PROC_TYPE::CLIENT1_FUNC);
        /* 调用客户端3接口 */
        callClient("cli_3", PROC_TYPE::CLIENT3_FUNC);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return 0;
}
