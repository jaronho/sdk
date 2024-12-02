#include "connect_service.h"

namespace nac
{
namespace tcli
{
void ConnectService::setDataChannel(const std::shared_ptr<DataChannel>& dataChannel)
{
    m_connections.clear();
    if (dataChannel)
    {
        const std::weak_ptr<ConnectService> wpSelf = shared_from_this();
        m_connections.emplace_back(dataChannel->sigConnectStatus.connect([wpSelf](const boost::system::error_code& code) -> void {
            const auto self = wpSelf.lock();
            if (self)
            {
                self->onConnectStatusChanged(code);
            }
        }));
        m_connections.emplace_back(dataChannel->sigUpdateRecvTime.connect([wpSelf](std::chrono::steady_clock::time_point ntp) -> void {
            const auto self = wpSelf.lock();
            if (self)
            {
                self->onUpdateLastRecvTime(ntp);
            }
        }));
        m_connections.emplace_back(dataChannel->sigUpdateSendTime.connect([wpSelf](std::chrono::steady_clock::time_point ntp) -> void {
            const auto self = wpSelf.lock();
            if (self)
            {
                self->onUpdateLastSendTime(ntp);
            }
        }));
    }
    m_wpDataChannel = dataChannel;
}

void ConnectService::setSessionManager(const std::shared_ptr<SessionManager>& sessionManager)
{
    m_wpSessionManager = sessionManager;
}

void ConnectService::setConnectCallback(const std::function<void(const ConnectState& state)>& callback)
{
    m_connectCb = callback;
}

void ConnectService::setAuthDataGenerator(const std::function<std::string()>& generator)
{
    m_authDataGenerator = generator;
}

void ConnectService::setAuthResultCallback(const std::function<bool(const std::string& data)>& callback)
{
    m_authResultCb = callback;
}

void ConnectService::setHeartbeatDataGenerator(const std::function<std::string()>& generator)
{
    m_heartbeatDataGenerator = generator;
}

bool ConnectService::connect(unsigned short localPort, const std::string& address, unsigned int port, bool sslOn, int sslWay, int certFmt,
                             const std::string& certFile, const std::string& pkFile, const std::string& pkPwd, unsigned int connectTimeout,
                             int32_t authBizCode, unsigned int authTimeout, int32_t heartbeatBizCode, unsigned int heartbeatInterval,
                             bool heartbeatFixedSend, unsigned int offlineTime)
{
    if (address.empty() || 0 == port)
    {
        ERROR_LOG(m_logger, "连接错误: 服务器[{}:{}]错误.", address, port);
        return false;
    }
    if (authBizCode > 0 && 0 == authTimeout)
    {
        ERROR_LOG(m_logger, "连接错误: 鉴权超时不能为0秒.");
        return false;
    }
    if (heartbeatBizCode > 0)
    {
        if (0 == heartbeatInterval)
        {
            ERROR_LOG(m_logger, "连接错误: 心跳间隔不能为0秒.");
            return false;
        }
        if (offlineTime <= heartbeatInterval)
        {
            ERROR_LOG(m_logger, "连接错误: 掉线判定时间[{}秒]必须大于心跳间隔[{}秒].", offlineTime, heartbeatInterval);
            return false;
        }
    }
    if (ConnectState::idle != m_connectState && ConnectState::disconnected != m_connectState)
    {
        WARN_LOG(m_logger, "连接失败: 当前状态 {} 不对.", m_connectState.load());
        return false;
    }
    INFO_LOG(m_logger,
             "连接配置: 服务器[{}:{}], 连接超时[{}秒], 鉴权(业务码[{}], 超时[{}秒]), 心跳(业务码[{}], 间隔[{}秒], {}间隔发送, "
             "掉线判定时间[{}秒]).",
             address, port, connectTimeout, authBizCode, authTimeout, heartbeatBizCode, heartbeatInterval,
             heartbeatFixedSend ? "[固定]" : "[非固定]", offlineTime);
    m_disconnectType = DisconnectType::unknown;
    updateConnectState(ConnectState::connecting);
    m_localPort = localPort;
    m_address = address;
    m_port = port;
    m_enableTls = sslOn;
    m_sslWay = sslWay;
    m_certFmt = certFmt;
    m_certFile = certFile;
    m_privateKeyFile = pkFile;
    m_privateKeyFilePwd = pkPwd;
    m_connectTimeout = connectTimeout;
    m_authBizCode = authBizCode;
    m_authTimeout = authTimeout;
    m_heartbeatBizCode = heartbeatBizCode;
    m_heartbeatInterval = heartbeatInterval;
    m_heartbeatFixedSend = heartbeatFixedSend;
    m_offlineTime = offlineTime;
    const auto dataChannel = m_wpDataChannel.lock();
    if (dataChannel)
    {
        startTimetoutTimer();
        return dataChannel->connect(localPort, address, port, sslOn, sslWay, certFmt, certFile, pkFile, pkPwd);
    }
    return false;
}

bool ConnectService::reconnect()
{
    if (ConnectState::disconnected != m_connectState)
    {
        WARN_LOG(m_logger, "重连失败: 当前状态 {} 不对.", m_connectState.load());
        return false;
    }
    INFO_LOG(m_logger, "重新连接.");
    m_disconnectType = DisconnectType::unknown;
    updateConnectState(ConnectState::connecting);
    const auto dataChannel = m_wpDataChannel.lock();
    if (dataChannel)
    {
        startTimetoutTimer();
        return dataChannel->connect(m_localPort, m_address, m_port, m_enableTls, m_sslWay, m_certFmt, m_certFile, m_privateKeyFile,
                                    m_privateKeyFilePwd);
    }
    return false;
}

void ConnectService::disconnect()
{
    if (ConnectState::idle == m_connectState)
    {
        WARN_LOG(m_logger, "断开连接失败: 当前状态 {} 不对.", m_connectState.load());
        return;
    }
    INFO_LOG(m_logger, "断开连接.");
    releaseConnection(DisconnectType::external_call);
    updateConnectState(ConnectState::idle);
}

ConnectState ConnectService::getConnectState() const
{
    return m_connectState;
}

void ConnectService::releaseConnection(const DisconnectType& type)
{
    m_disconnectType = type;
    stopAllTimer();
    /* 断开数据通道 */
    const auto dataChannel = m_wpDataChannel.lock();
    if (dataChannel)
    {
        dataChannel->disconnect();
    }
    /* 清除会话路由 */
    const auto sessionManager = m_wpSessionManager.lock();
    if (sessionManager)
    {
        sessionManager->clearSessionMap();
    }
}

void ConnectService::onConnectStatusChanged(const boost::system::error_code& code)
{
    INFO_LOG(m_logger, "连接状态变更: [{}] {}.", code.value(), code.message());
    stopTimeoutTimer();
    if (code) /* 连接失败 */
    {
        if (DisconnectType::unknown == m_disconnectType)
        {
            if (ConnectState::connecting == m_connectState || ConnectState::connected == m_connectState)
            {
                releaseConnection(DisconnectType::unknown);
                updateConnectState(ConnectState::disconnected);
            }
        }
    }
    else /* 连接成功 */
    {
        {
            std::lock_guard<std::mutex> locker(m_mutexTimePoint);
            m_lastRecvTime = m_lastSendTime = std::chrono::steady_clock::now();
        }
        if (m_authBizCode > 0) /* 需要鉴权 */
        {
            if (ConnectState::connecting == m_connectState)
            {
                sendAuthMsg();
            }
        }
        else /* 无需鉴权 */
        {
            if (ConnectState::connecting == m_connectState)
            {
                updateConnectState(ConnectState::connected);
                startHeartbeatTimer();
                startOfflineCheckTimer();
            }
        }
    }
}

void ConnectService::onUpdateLastRecvTime(std::chrono::steady_clock::time_point ntp)
{
    std::lock_guard<std::mutex> locker(m_mutexTimePoint);
    m_lastRecvTime = ntp;
}

void ConnectService::onUpdateLastSendTime(std::chrono::steady_clock::time_point ntp)
{
    std::lock_guard<std::mutex> locker(m_mutexTimePoint);
    m_lastSendTime = ntp;
}

void ConnectService::sendAuthMsg()
{
    INFO_LOG(m_logger, "发送鉴权消息.");
    const auto sessionManager = m_wpSessionManager.lock();
    if (sessionManager)
    {
        std::string authData;
        if (m_authDataGenerator)
        {
            auto beg = std::chrono::steady_clock::now();
            authData = m_authDataGenerator();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg).count();
            if (elapsed > 1)
            {
                DEBUG_LOG(m_logger, "生成鉴权数据耗时: {} 毫秒.", elapsed);
            }
        }
        const std::weak_ptr<ConnectService> wpSelf = shared_from_this();
        auto seqId = sessionManager->sendMsg(m_authBizCode, 0, authData, m_authTimeout,
                                             [wpSelf](bool sendOk, int32_t bizCode, int64_t seqId, const std::string& data) {
                                                 const auto self = wpSelf.lock();
                                                 if (self)
                                                 {
                                                     self->onAuthResult(sendOk, data);
                                                 }
                                             });
        if (seqId > 0)
        {
            return;
        }
    }
    ERROR_LOG(m_logger, "鉴权消息发送失败.");
    if (ConnectState::connecting == m_connectState)
    {
        releaseConnection(DisconnectType::auth_fail);
        updateConnectState(ConnectState::disconnected);
    }
}

void ConnectService::onAuthResult(bool ok, const std::string& data)
{
    if (ConnectState::connecting == m_connectState)
    {
        if (ok) /* 鉴权发送成功 */
        {
            if (!m_authResultCb || m_authResultCb(data)) /* 鉴权成功 */
            {
                INFO_LOG(m_logger, "鉴权成功.");
                updateConnectState(ConnectState::connected);
                startHeartbeatTimer();
                startOfflineCheckTimer();
                return;
            }
        }
        ERROR_LOG(m_logger, "鉴权失败.");
        releaseConnection(DisconnectType::auth_fail);
        updateConnectState(ConnectState::disconnected);
    }
    else
    {
        INFO_LOG(m_logger, "收到鉴权应答.");
    }
}

void ConnectService::startTimetoutTimer()
{
    if (m_connectTimeout > 0)
    {
        if (m_timeoutTimer)
        {
            m_timeoutTimer->setDelay(std::chrono::seconds(m_connectTimeout));
        }
        else
        {
            const std::weak_ptr<ConnectService> wpSelf = shared_from_this();
            const auto dataChannel = m_wpDataChannel.lock();
            m_timeoutTimer = threading::SteadyTimer::onceTimer(
                "nac.tcli.connect.timeout", std::chrono::seconds(m_connectTimeout),
                [wpSelf](const std::chrono::steady_clock::time_point& tp) {
                    const auto self = wpSelf.lock();
                    if (self)
                    {
                        self->onTimetoutTimer();
                    }
                },
                dataChannel ? dataChannel->getPktExecutor().lock() : nullptr);
        }
        m_timeoutTimer->start();
    }
}

void ConnectService::onTimetoutTimer()
{
    WARN_LOG(m_logger, "连接失败: 超时({}秒).", m_connectTimeout);
    releaseConnection(DisconnectType::connect_timeout);
    updateConnectState(ConnectState::disconnected);
}

void ConnectService::stopTimeoutTimer()
{
    if (m_timeoutTimer)
    {
        m_timeoutTimer->stop();
    }
}

void ConnectService::startHeartbeatTimer()
{
    if (m_heartbeatBizCode > 0)
    {
        if (!m_heartbeatTimer)
        {
            const std::weak_ptr<ConnectService> wpSelf = shared_from_this();
            const auto dataChannel = m_wpDataChannel.lock();
            m_heartbeatTimer = threading::SteadyTimer::loopTimer(
                "nac.tcli.heartbeat", std::chrono::seconds(1),
                [wpSelf](const std::chrono::steady_clock::time_point& tp) {
                    const auto self = wpSelf.lock();
                    if (self)
                    {
                        self->onHeartbeatTimer();
                    }
                },
                dataChannel ? dataChannel->getPktExecutor().lock() : nullptr);
        }
        m_heartbeatTimer->start();
    }
}

void ConnectService::onHeartbeatTimer()
{
    auto ntp = std::chrono::steady_clock::now();
    const int ELAPSED_DELTA = 500; /* 时间增量(毫秒), 这是由于定时器触发没办法精确到毫秒会有误差 */
    std::chrono::steady_clock::time_point lastTime;
    {
        std::lock_guard<std::mutex> locker(m_mutexTimePoint);
        lastTime = m_heartbeatFixedSend ? m_lastHeartbeatTime : m_lastSendTime;
    }
    auto elapsedLastSend = std::chrono::duration_cast<std::chrono::milliseconds>(ntp - lastTime).count();
    if ((elapsedLastSend + ELAPSED_DELTA) >= (m_heartbeatInterval * 1000))
    {
        if (ConnectState::connected == m_connectState)
        {
            sendHeartbeatMsg();
        }
    }
}

void ConnectService::sendHeartbeatMsg()
{
    if (m_heartbeatBizCode > 0) /* 需要定时发送心跳 */
    {
        const auto sessionManager = m_wpSessionManager.lock();
        if (sessionManager)
        {
            std::string heartbeatData;
            if (m_heartbeatDataGenerator)
            {
                auto beg = std::chrono::steady_clock::now();
                heartbeatData = m_heartbeatDataGenerator();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg).count();
                if (elapsed > 1)
                {
                    DEBUG_LOG(m_logger, "生成心跳数据耗时: {} 毫秒.", elapsed);
                }
            }
            const std::weak_ptr<ConnectService> wpSelf = shared_from_this();
            sessionManager->sendMsg(m_heartbeatBizCode, 0, heartbeatData, 0,
                                    [wpSelf](bool sendOk, int32_t bizCode, int64_t seqId, const std::string& data) {
                                        const auto self = wpSelf.lock();
                                        if (self && sendOk)
                                        {
                                            std::lock_guard<std::mutex> locker(self->m_mutexTimePoint);
                                            self->m_lastHeartbeatTime = std::chrono::steady_clock::now();
                                        }
                                    });
        }
    }
}

void ConnectService::startOfflineCheckTimer()
{
    if (m_heartbeatBizCode > 0)
    {
        if (!m_offlineCheckTimer)
        {
            const std::weak_ptr<ConnectService> wpSelf = shared_from_this();
            const auto dataChannel = m_wpDataChannel.lock();
            m_offlineCheckTimer = threading::SteadyTimer::loopTimer(
                "nac.tcli.offline.check", std::chrono::seconds(1),
                [wpSelf](const std::chrono::steady_clock::time_point& tp) {
                    const auto self = wpSelf.lock();
                    if (self)
                    {
                        self->onOfflineCheckTimer();
                    }
                },
                dataChannel ? dataChannel->getPktExecutor().lock() : nullptr);
        }
        m_offlineCheckTimer->start();
    }
}

void ConnectService::onOfflineCheckTimer()
{
    auto ntp = std::chrono::steady_clock::now();
    bool isUnRecvServerData = false;
    bool isUnSendServerData = false;
    std::string unRecvMsg, unSendMsg;
    /* 超过一定时间未收到服务端数据包(注意: 这里进行>判断, 不进行=判断), 表示掉线 */
    long long elapsedLastRecv = 0;
    {
        std::lock_guard<std::mutex> locker(m_mutexTimePoint);
        elapsedLastRecv = std::chrono::duration_cast<std::chrono::milliseconds>(ntp - m_lastRecvTime).count();
    }
    if (elapsedLastRecv > (m_offlineTime * 1000))
    {
        isUnRecvServerData = true;
        unRecvMsg = "超过[" + std::to_string(elapsedLastRecv) + "]毫秒未收到包)";
        /* 超过一定时间向服务端发送数据包 */
        long long elapsedLastSend = 0;
        {
            std::lock_guard<std::mutex> locker(m_mutexTimePoint);
            elapsedLastSend = std::chrono::duration_cast<std::chrono::milliseconds>(ntp - m_lastSendTime).count();
        }
        if (elapsedLastSend > (m_heartbeatInterval * 1000))
        {
            isUnSendServerData = true;
            unSendMsg = "超过[" + std::to_string(elapsedLastSend) + "]毫秒未发送包)";
        }
    }
    bool isDataChannelClosed = false;
    const auto dataChannel = m_wpDataChannel.lock();
    if (dataChannel)
    {
        isDataChannelClosed = !dataChannel->isOpened();
    }
    /* 数据通道关闭, 或者未收到服务端数据包 */
    if (isDataChannelClosed || isUnRecvServerData)
    {
        if (ConnectState::connecting == m_connectState || ConnectState::connected == m_connectState)
        {
            WARN_LOG(m_logger, "网络掉线(原因: {}.", (isDataChannelClosed ? "通道被关闭)" : (isUnSendServerData ? unSendMsg : unRecvMsg)));
            releaseConnection(DisconnectType::offline);
            updateConnectState(ConnectState::disconnected);
        }
    }
}

void ConnectService::stopAllTimer()
{
    stopTimeoutTimer();
    if (m_heartbeatTimer)
    {
        m_heartbeatTimer->stop();
    }
    if (m_offlineCheckTimer)
    {
        m_offlineCheckTimer->stop();
    }
}

void ConnectService::updateConnectState(const ConnectState& state)
{
    INFO_LOG(m_logger, "更新连接状态: {} => {}.", m_connectState.load(), state);
    if (state == m_connectState)
    {
        return;
    }
    m_connectState = state;
    if (m_connectCb)
    {
        m_connectCb(m_connectState);
    }
}
} // namespace tcli
} // namespace nac
