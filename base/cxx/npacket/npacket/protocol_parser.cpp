#include "protocol_parser.h"

namespace npacket
{
uint32_t ProtocolParser::getParentProtocol() const
{
    return ApplicationProtocol::NONE;
}
} // namespace npacket
