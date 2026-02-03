#include "protocol_parser.h"

namespace npacket
{
uint32_t ProtocolParser::getParentProtocol() const noexcept
{
    return ApplicationProtocol::NONE;
}

void ProtocolParser::reset() {}
} // namespace npacket
