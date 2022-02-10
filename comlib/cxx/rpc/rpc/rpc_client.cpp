#include "rpc_client.h"

#include "algorithm/snowflake/snowflake.h"

namespace rpc
{
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
    m_payload = std::make_shared<nsocket::Payload>(msg_base::maxsize());
    m_regHandler = nullptr;
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
    m_registered = false;
}

void Client::setRegHandler(const REG_HANDLER& handler)
{
    m_regHandler = handler;
}

void Client::setCallHandler(const CALL_HANDLER& handler)
{
    m_callHandler = handler;
}

void Client::run()
{
    if (m_running)
    {
        return;
    }
    m_running = true;
    while (m_running)
    {
        /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
        try
        {
            m_payload->reset();
            m_tcpClient = std::make_shared<nsocket::TcpClient>();
            m_tcpClient->setConnectCallback([&](const boost::system::error_code& code) { handleConnection(code); });
            m_tcpClient->setDataCallback([&](const std::vector<unsigned char>& data) { handleRecvData(data); });
#if (1 == ENABLE_NSOCKET_OPENSSL)
            m_tcpClient->run(m_brokerHost, m_serverPort,
                             nsocket::TcpClient::getSslContext(m_certFile, m_privateKeyFile, m_privateKeyFilePwd));
#else
            m_tcpClient->run(m_brokerHost, m_serverPort);
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
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void Client::call(const std::string& replyId, const std::vector<unsigned char>& data, const REPLY_FUNC& replyFunc,
                  const std::chrono::steady_clock::duration& timeout)
{
    if (!m_registered)
    {
        printf(">>>>>>>>>> call fail, unregister\n");
        if (replyFunc)
        {
            replyFunc({}, ErrorCode::UNREGISTER);
        }
        return;
    }
    msg_call mc;
    mc.seq_id = algorithm::Snowflake::easyGenerate();
    mc.call_id = m_id;
    mc.reply_id = replyId;
    mc.data = data;
    utilitiy::ByteArray ba;
    mc.encode(ba);
    std::vector<unsigned char> buffer;
    nsocket::Payload::pack(ba.getBuffer(), ba.getCurrentSize(), buffer);
    auto result = std::make_shared<std::promise<void>>();
    auto future = result->get_future().share();
    m_tcpClient->send(buffer, [&, seqId = mc.seq_id, replyFunc, result](const boost::system::error_code& code, std::size_t length) {
        if (code)
        {
            printf(">>>>>>>>>> call fail, %d, %s\n", code.value(), code.message().c_str());
            m_registered = false;
            m_tcpClient->stop();
            if (replyFunc)
            {
                replyFunc({}, ErrorCode::CALL_BROKER_FAILED);
            }
            result->set_value();
        }
        else
        {
            printf(">>>>>>>>>> call ok, length: %d\n", (int)length);
            ReplyObj obj;
            obj.func = replyFunc;
            obj.result = result;
            std::lock_guard<std::mutex> locker(m_mutex);
            m_replyObjMap.insert(std::make_pair(seqId, obj));
        }
    });
    if (timeout > std::chrono::steady_clock::duration::zero())
    {
        auto waitResult = future.wait_for(timeout);
        if (std::future_status::timeout == waitResult) /* 超时判断 */
        {
            {
                std::lock_guard<std::mutex> locker(m_mutex);
                auto iter = m_replyObjMap.find(mc.seq_id);
                if (m_replyObjMap.end() != iter)
                {
                    m_replyObjMap.erase(iter);
                }
            }
            printf(">>>>>>>>>> call fail, timeout\n");
            if (replyFunc)
            {
                replyFunc({}, ErrorCode::TIMEOUT);
            }
            return;
        }
    }
    future.get();
}

void Client::handleConnection(const boost::system::error_code& code)
{
    if (code)
    {
        printf("------------------------------ connect fail, %d, %s\n", code.value(), code.message().c_str());
        m_registered = false;
        m_tcpClient->stop();
    }
    else
    {
        printf("++++++++++++++++++++++++++++++ connect ok\n");
        reqRegister();
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
        [&](const std::vector<unsigned char>& body) {
            utilitiy::ByteArray ba;
            ba.setBuffer(body.data(), body.size());
            /* 解析消息类型 */
            MsgType type = (MsgType)ba.readInt();
            /* 处理消息 */
            handleMsg(type, ba);
        },
        [&](const std::vector<unsigned char>& data) {});
}

void Client::handleMsg(const MsgType& type, utilitiy::ByteArray& ba)
{
    switch (type)
    {
    case MsgType::REGISTER_RESULT: {
        msg_register_result resp;
        resp.decode(ba);
        printf("<<<<< [REGISTER_RESULT], desc: %s\n", error_desc(resp.code).c_str());
        if (ErrorCode::OK == resp.code)
        {
            m_registered = true;
        }
        else
        {
            m_running = false;
            m_tcpClient->stop();
        }
        if (m_regHandler)
        {
            m_regHandler(resp.code);
        }
    }
    break;
    case MsgType::CALL: {
        msg_call mc;
        mc.decode(ba);
        printf("<<<<< [CALL], seq id: %lld, call id: %s, reply id: %s, data length: %d\n", mc.seq_id, mc.call_id.c_str(),
               mc.reply_id.c_str(), (int)mc.data.size());
        /* 应答调用方 */
        if (m_callHandler)
        {
            msg_reply mr;
            mr.seq_id = mc.seq_id;
            mr.call_id = mc.call_id;
            mr.reply_id = mc.reply_id;
            mr.data = m_callHandler(mc.call_id, mc.data);
            utilitiy::ByteArray ba;
            mr.encode(ba);
            std::vector<unsigned char> buffer;
            nsocket::Payload::pack(ba.getBuffer(), ba.getCurrentSize(), buffer);
            m_tcpClient->send(buffer, nullptr);
        }
    }
    break;
    case MsgType::REPLY: {
        msg_reply mr;
        mr.decode(ba);
        printf("<<<<< [REPLY], seq id: %lld, call id: %s, reply id: %s, desc: %s\n", mr.seq_id, mr.call_id.c_str(), mr.reply_id.c_str(),
               error_desc(mr.code).c_str());
        ReplyObj obj;
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            auto iter = m_replyObjMap.find(mr.seq_id);
            if (m_replyObjMap.end() != iter)
            {
                obj = iter->second;
                m_replyObjMap.erase(iter);
            }
        }
        if (obj.func)
        {
            obj.func(mr.data, mr.code);
        }
        if (obj.result)
        {
            obj.result->set_value();
        }
    }
    break;
    default: {
        printf("********** msg [%d], unknown type **********\n", (int)type);
    }
    break;
    }
}

void Client::reqRegister()
{
    /* 向服务器注册 */
    msg_register reg;
    reg.self_id = m_id;
    utilitiy::ByteArray ba;
    reg.encode(ba);
    std::vector<unsigned char> data;
    nsocket::Payload::pack(ba.getBuffer(), ba.getCurrentSize(), data);
    m_tcpClient->send(data, [&](const boost::system::error_code& code, std::size_t length) {
        if (code)
        {
            printf(">>>>>>>>>> register fail, %d, %s\n", code.value(), code.message().c_str());
            m_tcpClient->stop();
        }
        else
        {
            printf(">>>>>>>>>> register ok, length: %d\n", (int)length);
        }
    });
}
} // namespace rpc
