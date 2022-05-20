#include "rpc_client.h"

#include "algorithm/snowflake/snowflake.h"
#include "threading/thread_proxy.hpp"
#include "threading/timer/steady_timer.h"

namespace rpc
{
static threading::ExecutorPtr s_executor = nullptr;

static void pack(const msg_base* msg, std::vector<unsigned char>& buffer)
{
    utility::ByteArray ba;
    msg->encode(ba);
    utility::ByteArray::write32(buffer, ba.getCurrentSize(), true);
    buffer.insert(buffer.end(), ba.getBuffer(), ba.getBuffer() + ba.getCurrentSize());
}

class Client::Session
{
public:
    Session(Client& client, const msg_call& mc, const std::shared_ptr<std::promise<msg_reply>>& promise)
        : m_client(client), m_call(mc), m_promise(promise)
    {
    }

    Session(Client& client, const msg_call& mc, const REPLY_FUNC& replyFunc) : m_client(client), m_call(mc), m_replyFunc(replyFunc) {}

    ~Session()
    {
        stopTimer();
    }

    bool startTimer(const std::chrono::steady_clock::duration& timeout)
    {
        if (std::chrono::steady_clock::duration::zero() == timeout)
        {
            return false;
        }
        if (m_replyFunc)
        {
            if (!m_timer)
            {
                auto name = "session_" + std::to_string(m_call.seq_id);
                m_timer = std::make_shared<threading::SteadyTimer>(
                    name, timeout, std::chrono::steady_clock::duration::zero(), [&]() { onTimeout(); }, s_executor);
            }
            m_timer->start();
        }
        return true;
    }

    void onTimeout()
    {
        stopTimer();
        auto replyFunc = m_replyFunc; /* 需要先缓存 */
        m_client.onSessionTimeout(m_call.seq_id);
        if (replyFunc)
        {
            replyFunc({}, rpc::ErrorCode::timeout);
        }
    }

    void onFailed()
    {
        stopTimer();
        if (m_replyFunc)
        {
            m_replyFunc({}, rpc::ErrorCode::call_broker_failed);
        }
    }

    void onReply(const msg_reply& mr)
    {
        stopTimer();
        if (m_promise)
        {
            m_promise->set_value(mr);
        }
        else if (m_replyFunc)
        {
            m_replyFunc(mr.data, mr.code);
        }
    }

private:
    void stopTimer()
    {
        if (m_timer)
        {
            m_timer->stop();
            m_timer.reset();
        }
    }

private:
    std::shared_ptr<threading::SteadyTimer> m_timer = nullptr;
    msg_call m_call;
    std::shared_ptr<std::promise<msg_reply>> m_promise = nullptr;
    REPLY_FUNC m_replyFunc = nullptr;
    Client& m_client;
};

#if (1 == ENABLE_NSOCKET_OPENSSL)
Client::Client(const std::string& id, const std::string& brokerHost, int brokerPort, const std::string& certFile,
               const std::string& privateKeyFile, const std::string& privateKeyFilePwd)
#else
Client::Client(const std::string& id, const std::string& brokerHost, int brokerPort)
#endif
{
    if (id.empty())
    {
        throw std::exception(std::logic_error("arg 'id' is empty"));
    }
    m_payload = std::make_shared<nsocket::Payload>(4);
    m_bindHandler = nullptr;
    m_callHandler = nullptr;
    m_id = id;
    m_brokerHost = brokerHost;
    m_serverPort = brokerPort;
#if (1 == ENABLE_NSOCKET_OPENSSL)
    m_certFile = certFile;
    m_privateKeyFile = privateKeyFile;
    m_privateKeyFilePwd = privateKeyFilePwd;
#endif
    m_running = false;
    m_binded = false;
}

void Client::setBindHandler(const BIND_HANDLER& handler)
{
    m_bindHandler = handler;
}

void Client::setCallHandler(const CALL_HANDLER& handler)
{
    m_callHandler = handler;
}

void Client::run(bool async, std::chrono::steady_clock::duration retryTime)
{
    if (retryTime <= std::chrono::steady_clock::duration::zero())
    {
        retryTime = std::chrono::milliseconds(3000);
    }
    if (m_running)
    {
        return;
    }
    m_running = true;
    if (!s_executor)
    {
        s_executor = threading::ThreadProxy::createAsioExecutor("rpc_timer", 1);
    }
    while (m_running)
    {
        /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
        try
        {
            m_payload->reset();
            m_tcpClient = std::make_shared<nsocket::TcpClient>();
            m_tcpClient->setConnectCallback([&, async](const boost::system::error_code& code) { handleConnection(code, async); });
            m_tcpClient->setDataCallback([&](const std::vector<unsigned char>& data) { handleRecvData(data); });
#if (1 == ENABLE_NSOCKET_OPENSSL)
            m_tcpClient->run(m_brokerHost, m_serverPort,
                             nsocket::TcpClient::getSsl2WayContext(m_certFile, m_privateKeyFile, m_privateKeyFilePwd), async);
#else
            m_tcpClient->run(m_brokerHost, m_serverPort, async);
#endif
        }
        catch (const std::exception& e)
        {
            printf("******************** execption: %s\n", e.what());
        }
        catch (...)
        {
            printf("******************** execption: unknown\n");
        }
        std::this_thread::sleep_for(retryTime);
    }
}

rpc::ErrorCode Client::call(const std::string& replyer, int proc, const std::vector<unsigned char>& data,
                            std::vector<unsigned char>& replyData, const std::chrono::steady_clock::duration& timeout)
{
    static algorithm::Snowflake sf(0, getpid());
    replyData.clear();
    if (!m_binded)
    {
        return ErrorCode::unbind;
    }
    msg_call mc;
    mc.seq_id = algorithm::Snowflake::easyGenerate();
    mc.caller = m_id;
    mc.replyer = replyer;
    mc.proc = proc;
    mc.data = data;
    mc.timeout = (int)std::chrono::duration<double, std::milli>(timeout).count();
    std::vector<unsigned char> buffer;
    pack(&mc, buffer);
    auto result = std::make_shared<std::promise<msg_reply>>();
    auto future = result->get_future().share();
    {
        if (timeout > std::chrono::steady_clock::duration::zero())
        {
            std::lock_guard<std::mutex> locker(m_mutexSessionMap);
            m_sessionMap.insert(std::make_pair(mc.seq_id, std::make_shared<Session>(*this, mc, result)));
        }
        else
        {
            return ErrorCode::timeout;
        }
    }
    std::size_t length;
    auto code = m_tcpClient->send(buffer, length);
    if (code)
    {
        m_binded = false;
        m_tcpClient->stop();
        std::lock_guard<std::mutex> locker(m_mutexSessionMap);
        auto iter = m_sessionMap.find(mc.seq_id);
        if (m_sessionMap.end() != iter)
        {
            m_sessionMap.erase(iter);
        }
        return ErrorCode::call_broker_failed;
    }
    if (timeout > std::chrono::steady_clock::duration::zero())
    {
        auto waitResult = future.wait_for(timeout);
        if (std::future_status::timeout == waitResult) /* 超时判断 */
        {
            std::lock_guard<std::mutex> locker(m_mutexSessionMap);
            auto iter = m_sessionMap.find(mc.seq_id);
            if (m_sessionMap.end() != iter)
            {
                m_sessionMap.erase(iter);
            }
            return ErrorCode::timeout;
        }
    }
    msg_reply mr = future.get();
    replyData = std::move(mr.data);
    return mr.code;
}

void Client::callAsync(const std::string& replyer, int proc, const std::vector<unsigned char>& data, const REPLY_FUNC& replyFunc,
                       const std::chrono::steady_clock::duration& timeout)
{
    if (!m_binded)
    {
        if (replyFunc)
        {
            replyFunc({}, ErrorCode::unbind);
        }
        return;
    }
    msg_call mc;
    mc.seq_id = algorithm::Snowflake::easyGenerate();
    mc.caller = m_id;
    mc.replyer = replyer;
    mc.proc = proc;
    mc.data = data;
    mc.timeout = (int)std::chrono::duration<double, std::milli>(timeout).count();
    std::vector<unsigned char> buffer;
    pack(&mc, buffer);
    {
        auto session = std::make_shared<Session>(*this, mc, replyFunc);
        if (session->startTimer(timeout))
        {
            std::lock_guard<std::mutex> locker(m_mutexSessionMap);
            m_sessionMap.insert(std::make_pair(mc.seq_id, session));
        }
        else
        {
            if (replyFunc)
            {
                replyFunc({}, ErrorCode::timeout);
            }
            return;
        }
    }
    std::size_t length;
    auto code = m_tcpClient->send(buffer, length);
    if (code)
    {
        m_binded = false;
        m_tcpClient->stop();
        std::shared_ptr<Session> session = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutexSessionMap);
            auto iter = m_sessionMap.find(mc.seq_id);
            if (m_sessionMap.end() != iter)
            {
                session = iter->second;
                m_sessionMap.erase(iter);
            }
        }
        if (session)
        {
            session->onFailed();
        }
    }
}

void Client::handleConnection(const boost::system::error_code& code, bool async)
{
    if (code)
    {
        printf("------------------------------ connect fail, %d, %s\n", code.value(), code.message().c_str());
        m_binded = false;
        m_tcpClient->stop();
    }
    else
    {
        printf("++++++++++++++++++++++++++++++ connect ok\n");
        reqBind(async);
    }
}

void Client::handleRecvData(const std::vector<unsigned char>& data)
{
#if 0
    /* 信息打印 */
    {
        printf("<<<<<<<<<<<<<<<<<<< recv data, length: %d\n", (int)data.size());
        /* 以十六进制格式打印数据 */
        printf("<<<<<<<<<< [hex format]\n");
        for (size_t i = 0; i < data.size(); ++i)
        {
            printf("%02X ", data[i]);
        }
        printf("\n");
        /* 以字符串格式打印数据 */
        printf("<<<<<<<<<< [string format]\n");
        std::string str(data.begin(), data.end());
        printf("%s", str.c_str());
        printf("\n");
    }
#endif
    /* 逻辑处理 */
    m_payload->unpack(
        data,
        [&](const std::vector<unsigned char>& head) {
            int bodyLen = utility::ByteArray::read32(head.data(), true);
            if (bodyLen >= msg_base::maxsize())
            {
                return -1;
            }
            return bodyLen;
        },
        [&](const std::vector<unsigned char>& body) {
            utility::ByteArray ba;
            ba.setBuffer(body.data(), body.size());
            /* 解析消息类型 */
            MsgType type = (MsgType)ba.readInt32();
            /* 处理消息 */
            handleMsg(type, ba);
        });
}

void Client::handleMsg(const MsgType& type, utility::ByteArray& ba)
{
    switch (type)
    {
    case MsgType::bind_result: {
        msg_bind_result resp;
        resp.decode(ba);
        printf("<<<<< [bind_result], desc: %s\n", error_desc(resp.code).c_str());
        if (ErrorCode::ok == resp.code)
        {
            m_binded = true;
        }
        else
        {
            m_running = false;
            m_tcpClient->stop();
        }
        if (m_bindHandler)
        {
            m_bindHandler(resp.code);
        }
    }
    break;
    case MsgType::call: {
        msg_call mc;
        mc.decode(ba);
        /* 应答调用方 */
        if (m_callHandler)
        {
            msg_reply mr;
            mr.seq_id = mc.seq_id;
            mr.caller = mc.caller;
            mr.replyer = mc.replyer;
            mr.proc = mc.proc;
            try
            {
                mr.data = m_callHandler(mc.caller, mc.proc, mc.data);
                mr.code = ErrorCode::ok;
            }
            catch (...)
            {
                mr.data = {};
                mr.code = ErrorCode::replyer_inner_error;
            }
            std::vector<unsigned char> buffer;
            pack(&mr, buffer);
            m_tcpClient->sendAsync(buffer, nullptr);
        }
    }
    break;
    case MsgType::reply: {
        msg_reply mr;
        mr.decode(ba);
        std::shared_ptr<Session> session = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutexSessionMap);
            auto iter = m_sessionMap.find(mr.seq_id);
            if (m_sessionMap.end() != iter)
            {
                session = iter->second;
                m_sessionMap.erase(iter);
            }
        }
        if (session)
        {
            session->onReply(mr);
        }
        else
        {
            printf("********** [reply], seq id: %lld, replyer: %s, proc: %d, can't find session **********\n", mr.seq_id,
                   mr.replyer.c_str(), mr.proc);
        }
    }
    break;
    default: {
        printf("********** msg [%d], unknown type **********\n", (int)type);
    }
    break;
    }
}

void Client::reqBind(bool async)
{
    /* 向服务器绑定 */
    msg_bind req;
    req.self_id = m_id;
    std::vector<unsigned char> buffer;
    pack(&req, buffer);
    if (async)
    {
        m_tcpClient->sendAsync(buffer, [&](const boost::system::error_code& code, std::size_t length) {
            if (code)
            {
                printf(">>>>>>>>>> bind fail, %d, %s\n", code.value(), code.message().c_str());
                m_tcpClient->stop();
            }
            else
            {
                printf(">>>>>>>>>> bind ok, length: %d\n", (int)length);
            }
        });
    }
    else
    {
        std::size_t length;
        auto code = m_tcpClient->send(buffer, length);
        if (code)
        {
            printf(">>>>>>>>>> bind fail, %d, %s\n", code.value(), code.message().c_str());
            m_tcpClient->stop();
        }
        else
        {
            printf(">>>>>>>>>> bind ok, length: %d\n", (int)length);
        }
    }
}

void Client::onSessionTimeout(int64_t seqId)
{
    std::lock_guard<std::mutex> locker(m_mutexSessionMap);
    auto iter = m_sessionMap.find(seqId);
    if (m_sessionMap.end() != iter)
    {
        m_sessionMap.erase(iter);
    }
}
} // namespace rpc
