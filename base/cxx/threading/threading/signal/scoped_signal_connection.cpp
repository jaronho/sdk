#include "scoped_signal_connection.h"

namespace threading
{
ScopedSignalConnection::ScopedSignalConnection(const SignalConnection& conn) : m_conn(conn.m_conn) {}

ScopedSignalConnection::ScopedSignalConnection(SignalConnection&& conn) : m_conn(conn.m_conn) {}

ScopedSignalConnection::ScopedSignalConnection(const boost::signals2::connection& conn) : m_conn(conn) {}

ScopedSignalConnection& ScopedSignalConnection::operator=(SignalConnection&& conn)
{
    m_conn = conn.m_conn;
    return *this;
}

bool ScopedSignalConnection::connected() const
{
    return m_conn.connected();
}

void ScopedSignalConnection::disconnect() const
{
    m_conn.disconnect();
}
} // namespace threading
