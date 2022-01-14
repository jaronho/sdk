#pragma once
#include <boost/signals2.hpp>

#include "signal_connection.h"

namespace threading
{
/**
 * @brief 作用域信号连接, 当离开作用域时连接就会自动断开
 */
class ScopedSignalConnection
{
public:
    ScopedSignalConnection() = default;

    ScopedSignalConnection(const SignalConnection& conn);

    ScopedSignalConnection(SignalConnection&& conn);

    ScopedSignalConnection(const boost::signals2::connection& conn);

    ScopedSignalConnection& operator=(SignalConnection&& conn);

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
    boost::signals2::scoped_connection m_conn;
};
} // namespace threading
