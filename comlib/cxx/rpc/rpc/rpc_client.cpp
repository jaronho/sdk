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
    m_msgHandler = nullptr;
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

void Client::setRegHandler(const std::function<void(bool ret)>& handler)
{
    m_regHandler = handler;
}

void Client::setMsgHandler(const std::function<void(const std::string& srcId, const std::vector<unsigned char>& data)>& handler)
{
    m_msgHandler = handler;
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

void Client::send(const std::string& targetId, const std::vector<unsigned char>& data,
                  const std::function<void(const std::string& targetId, bool ret)>& onSendCb)
{
    if (!m_registered)
    {
        printf(">>>>>>>>>> on send fail, isn't initialized yet\n");
        if (onSendCb)
        {
            onSendCb(targetId, false);
        }
        return;
    }
    msg_req_send_data req;
    req.seq_id = algorithm::Snowflake::easyGenerate();
    req.target_id = targetId;
    req.data = data;
    utilitiy::ByteArray ba;
    req.encode(ba);
    std::vector<unsigned char> buffer;
    nsocket::Payload::pack(ba.getBuffer(), ba.getCurrentSize(), buffer);
    m_tcpClient->send(buffer, [&, seqId = req.seq_id, targetId, onSendCb](const boost::system::error_code& code, std::size_t length) {
        if (code)
        {
            printf(">>>>>>>>>> on send fail, %d, %s\n", code.value(), code.message().c_str());
            m_registered = false;
            m_tcpClient->stop();
            if (onSendCb)
            {
                onSendCb(targetId, false);
            }
        }
        else
        {
            printf(">>>>>>>>>> on send ok, length: %d\n", (int)length);
            m_sendCbMap.insert(std::make_pair(seqId, onSendCb));
        }
    });
}

void Client::handleConnection(const boost::system::error_code& code)
{
    if (code)
    {
        printf("------------------------------ on connect fail, %d, %s\n", code.value(), code.message().c_str());
        m_registered = false;
        m_tcpClient->stop();
    }
    else
    {
        printf("++++++++++++++++++++++++++++++ on connect ok\n");
        reqRegister();
    }
}

void Client::handleRecvData(const std::vector<unsigned char>& data)
{
    /* 信息打印 */
    {
        printf("<<<<<<<<<<<<<<<<<<< on recv data, length: %d\n", (int)data.size());
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
    case MsgType::NOTIFY_REGISTER_RESULT: {
        msg_notify_register_result resp;
        resp.decode(ba);
        printf("<<<<< msg [NOTIFY_REGISTER_RESULT], desc: %s\n", resp.desc.c_str());
        if (resp.ok)
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
            m_regHandler(resp.ok);
        }
    }
    break;
    case MsgType::NOTIFY_SEND_DATA_RESULT: {
        msg_req_send_data_result resp;
        resp.decode(ba);
        printf("<<<<< msg [NOTIFY_SEND_DATA_RESULT], seq id: %lld, target id: %s, desc: %s\n", resp.seq_id, resp.target_id.c_str(),
               resp.desc.c_str());
        auto iter = m_sendCbMap.find(resp.seq_id);
        if (m_sendCbMap.end() != iter)
        {
            auto onSendCb = iter->second;
            m_sendCbMap.erase(iter);
            if (onSendCb)
            {
                onSendCb(resp.target_id, resp.ok);
            }
        }
    }
    break;
    case MsgType::NOTIFY_RECV_DATA: {
        msg_notify_recv_data msg;
        msg.decode(ba);
        printf("<<<<< msg [NOTIFY_RECV_DATA], seq id: %lld, src id: %s, data length: %d\n", msg.seq_id, msg.src_id.c_str(),
               (int)msg.data.size());
        if (m_msgHandler)
        {
            m_msgHandler(msg.src_id, msg.data);
        }
    }
    break;
    default: {
        printf("********** msg [%d], dont't deal **********\n", (int)type);
    }
    break;
    }
}

void Client::reqRegister()
{
    /* 向服务器注册 */
    msg_req_register req;
    req.self_id = m_id;
    utilitiy::ByteArray ba;
    req.encode(ba);
    std::vector<unsigned char> data;
    nsocket::Payload::pack(ba.getBuffer(), ba.getCurrentSize(), data);
    m_tcpClient->send(data, [&](const boost::system::error_code& code, std::size_t length) {
        if (code)
        {
            printf(">>>>>>>>>> on req register fail, %d, %s\n", code.value(), code.message().c_str());
            m_tcpClient->stop();
        }
        else
        {
            printf(">>>>>>>>>> on req register ok, length: %d\n", (int)length);
        }
    });
}
} // namespace rpc
