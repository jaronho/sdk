#include "signal_connection.h"

namespace threading
{
SignalConnection::SignalConnection(const SignalConnection& conn) : m_conn(conn.m_conn) {}

SignalConnection::SignalConnection(SignalConnection&& conn) noexcept : m_conn(conn.m_conn) {}

SignalConnection::SignalConnection(const boost::signals2::connection& conn) : m_conn(conn) {}

SignalConnection& SignalConnection::operator=(const SignalConnection& connection)
{
    m_conn = connection.m_conn;
    return *this;
}

bool SignalConnection::connected() const
{
    return m_conn.connected();
}

void SignalConnection::disconnect() const
{
    m_conn.disconnect();
}
} // namespace threading
