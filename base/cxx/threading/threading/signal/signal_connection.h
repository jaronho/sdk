#pragma once
#include <boost/signals2.hpp>

namespace threading
{
/**
 * @brief 信号连接
 */
class SignalConnection
{
    friend class ScopedSignalConnection;

public:
    SignalConnection() = default;

    SignalConnection(const SignalConnection& conn);

    SignalConnection(SignalConnection&& conn) noexcept;

    SignalConnection(const boost::signals2::connection& conn);

    SignalConnection& operator=(const SignalConnection& conn);

    /**
     * @brief 判断是否连接中
     * @return true-是, false-否
     */
    bool connected() const;

    /**
     * @brief 断开连接
     */
    void disconnect() const;

protected:
    boost::signals2::connection m_conn; /* 信号连接对象 */
};
} // namespace threading
