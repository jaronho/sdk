#include "connect_service.h"

namespace nac
{
void ConnectService::setDataChannel(const std::shared_ptr<DataChannel>& dataChannel)
{
    m_connections.clear();
    if (dataChannel)
    {
        const std::weak_ptr<ConnectService> wpSelf = shared_from_this();
        m_connections.emplace_back(dataChannel->sigConnectStatus.connect([wpSelf](bool isConnected) -> void {
            const auto self = wpSelf.lock();
            if (self)
            {
                self->onConnectStatusChanged(isConnected);
            }
        }));
        m_connections.emplace_back(dataChannel->sigUpdateRecvTime.connect([wpSelf]() -> void {
            const auto self = wpSelf.lock();
            if (self)
            {
                self->onUpdateLastRecvTime();
            }
        }));
        m_connections.emplace_back(dataChannel->sigUpdateSendTime.connect([wpSelf]() -> void {
            const auto self = wpSelf.lock();
            if (self)
            {
                self->onUpdateLastSendTime();
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

bool ConnectService::connect(const std::string& address, unsigned int port, const std::string& certFile, const std::string& privateKeyFile,
                             const std::string& privateKeyFilePwd, unsigned int connectTimeout, unsigned int authBizCode,
                             unsigned int authTimeout, unsigned int heartbeatBizCode, unsigned int heartbeatInterval)
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
    if (heartbeatBizCode > 0 && 0 == heartbeatInterval)
    {
        ERROR_LOG(m_logger, "连接错误: 心跳间隔不能为0秒.");
        return false;
    }
    if (ConnectState::idle != m_connectState && ConnectState::disconnected != m_connectState)
    {
        WARN_LOG(m_logger, "连接失败: 当前状态 {} 不对.", m_connectState.load());
        return false;
    }
    INFO_LOG(m_logger, "连接配置信息: 服务器[{}:{}], 连接超时[{}秒], 鉴权(业务码[{}], 超时[{}秒]), 心跳(业务码[{}], 超时[{}秒]).", address,
             port, connectTimeout, authBizCode, authTimeout, heartbeatBizCode, heartbeatInterval);
    m_disconnectType = DisconnectType::unknown;
    updateConnectState(ConnectState::connecting);
    m_address = address;
    m_port = port;
    m_certFile = certFile;
    m_privateKeyFile = privateKeyFile;
    m_privateKeyFilePwd = privateKeyFilePwd;
    m_connectTimeout = connectTimeout;
    m_authBizCode = authBizCode;
    m_authTimeout = authTimeout;
    m_heartbeatBizCode = heartbeatBizCode;
    m_heartbeatInterval = heartbeatInterval;
    m_heartbeatMaxInterval = (m_heartbeatInterval * 4);
    m_offlineTime = (m_heartbeatMaxInterval + 1);
    m_offlineCheckInterval = ceil(m_offlineTime / 2.0);
    const auto dataChannel = m_wpDataChannel.lock();
    if (dataChannel)
    {
        startTimetoutTimer();
        return dataChannel->connect(address, port, certFile, privateKeyFile, privateKeyFilePwd);
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
    m_disconnectType = DisconnectType::unknown;
    updateConnectState(ConnectState::connecting);
    const auto dataChannel = m_wpDataChannel.lock();
    if (dataChannel)
    {
        startTimetoutTimer();
        return dataChannel->connect(m_address, m_port, m_certFile, m_privateKeyFile, m_privateKeyFilePwd);
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
    releaseConnection(DisconnectType::external_call);
    m_address.clear();
    m_port = 0;
    m_connectTimeout = 0;
    m_authBizCode = 0;
    m_authTimeout = 30;
    m_heartbeatBizCode = 0;
    m_heartbeatInterval = 15;
    m_heartbeatMaxInterval = (m_heartbeatInterval * 4);
    m_offlineTime = (m_heartbeatMaxInterval + 1);
    m_offlineCheckInterval = ceil(m_offlineTime / 2.0);
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

void ConnectService::onConnectStatusChanged(bool isConnected)
{
    stopTimeoutTimer();
    if (isConnected) /* 连接成功 */
    {
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
            }
        }
    }
    else /* 连接失败 */
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
}

void ConnectService::onUpdateLastRecvTime()
{
    auto t = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::steady_clock::now());
    m_lastRecvTime = t.time_since_epoch().count();
}

void ConnectService::onUpdateLastSendTime()
{
    auto t = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::steady_clock::now());
    m_lastSendTime = t.time_since_epoch().count();
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
            authData = m_authDataGenerator();
        }
        const std::weak_ptr<ConnectService> wpSelf = shared_from_this();
        auto seqId = sessionManager->sendMsg(m_authBizCode, 0, authData, m_authTimeout,
                                             [wpSelf](bool sendOk, unsigned int bizCode, int64_t seqId, const std::string& data) {
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
                return;
            }
        }
        ERROR_LOG(m_logger, "鉴权失败.");
        releaseConnection(DisconnectType::auth_fail);
        updateConnectState(ConnectState::disconnected);
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
            m_timeoutTimer = std::make_shared<threading::SteadyTimer>("nac.connect.timeout", std::chrono::seconds(m_connectTimeout),
                                                                      std::chrono::seconds::duration::zero(), [wpSelf]() {
                                                                          const auto self = wpSelf.lock();
                                                                          if (self)
                                                                          {
                                                                              self->onTimetoutTimer();
                                                                          }
                                                                      });
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
        if (m_heartbeatTimer)
        {
            m_heartbeatTimer->setDelay(std::chrono::seconds(m_heartbeatInterval));
            m_heartbeatTimer->setInterval(std::chrono::seconds(m_heartbeatInterval));
        }
        else
        {
            const std::weak_ptr<ConnectService> wpSelf = shared_from_this();
            m_heartbeatTimer = std::make_shared<threading::SteadyTimer>("nac.heartbeat", std::chrono::seconds(m_heartbeatInterval),
                                                                        std::chrono::seconds(m_heartbeatInterval), [wpSelf]() {
                                                                            const auto self = wpSelf.lock();
                                                                            if (self)
                                                                            {
                                                                                self->onHeartbeatTimer();
                                                                            }
                                                                        });
        }
        m_heartbeatTimer->start();
    }
}

void ConnectService::onHeartbeatTimer()
{
    auto t = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::steady_clock::now());
    auto now = t.time_since_epoch().count();
    /* 超过一定时间未向服务发送数据, 或者超过一定时间未发送心跳包, 需要发送心跳包来维持连接 */
    if ((now - m_lastSendTime > m_heartbeatInterval) || (now - m_lastSendHeartbeatTime > m_heartbeatMaxInterval))
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
        auto t = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::steady_clock::now());
        m_lastSendHeartbeatTime = t.time_since_epoch().count();
        const auto sessionManager = m_wpSessionManager.lock();
        if (sessionManager)
        {
            std::string heartbeatData;
            if (m_heartbeatDataGenerator)
            {
                heartbeatData = m_heartbeatDataGenerator();
            }
            sessionManager->sendMsg(m_heartbeatBizCode, 0, heartbeatData, 0, nullptr);
        }
    }
}

void ConnectService::startOfflineCheckTimer()
{
    if (m_heartbeatBizCode > 0)
    {
        if (m_offlineCheckTimer)
        {
            m_offlineCheckTimer->setDelay(std::chrono::seconds(m_offlineCheckInterval));
            m_offlineCheckTimer->setInterval(std::chrono::seconds(m_offlineCheckInterval));
        }
        else
        {
            const std::weak_ptr<ConnectService> wpSelf = shared_from_this();
            m_offlineCheckTimer =
                std::make_shared<threading::SteadyTimer>("nac.offline.check", std::chrono::seconds(m_offlineCheckInterval),
                                                         std::chrono::seconds(m_offlineCheckInterval), [wpSelf]() {
                                                             const auto self = wpSelf.lock();
                                                             if (self)
                                                             {
                                                                 self->onOfflineCheckTimer();
                                                             }
                                                         });
        }
        m_offlineCheckTimer->start();
    }
}

void ConnectService::onOfflineCheckTimer()
{
    auto t = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::steady_clock::now());
    auto now = t.time_since_epoch().count();
    bool isUnRecvServerData = false;
    /* 超过一定时间未收到服务端数据包, 表示掉线 */
    if (now - m_lastRecvTime > m_offlineTime)
    {
        isUnRecvServerData = true;
    }
    bool isDataChannelClose = false;
    const auto dataChannel = m_wpDataChannel.lock();
    if (dataChannel)
    {
        isDataChannelClose = !dataChannel->isOpened();
    }
    /* 数据通道关闭, 或者未收到服务端数据包 */
    if (isDataChannelClose || isUnRecvServerData)
    {
        if (ConnectState::connecting == m_connectState || ConnectState::connected == m_connectState)
        {
            WARN_LOG(m_logger, "网络掉线.");
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
} // namespace nac
