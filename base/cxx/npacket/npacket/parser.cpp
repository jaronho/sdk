#include "parser.h"

namespace npacket
{
bool ProtocolParser::addChild(const std::shared_ptr<ProtocolParser>& parser)
{
    if (parser && parser->getProtocol() != getProtocol())
    {
        std::lock_guard<std::mutex> locker(m_mutexChildren);
        for (auto item : m_children)
        {
            if (item && item->getProtocol() == parser->getProtocol())
            {
                return false;
            }
        }
        m_children.emplace_back(parser);
        return true;
    }
    return false;
}

void ProtocolParser::removeChild(uint32_t protocol)
{
    std::lock_guard<std::mutex> locker(m_mutexChildren);
    for (auto iter = m_children.begin(); m_children.end() != iter; ++iter)
    {
        if (*iter && (*iter)->getProtocol() == protocol)
        {
            m_children.erase(iter);
            break;
        }
    }
}

std::vector<std::shared_ptr<ProtocolParser>> ProtocolParser::getChildren()
{
    std::lock_guard<std::mutex> locker(m_mutexChildren);
    return m_children;
}
} // namespace npacket
