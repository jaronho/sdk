#include "ftp.h"

namespace npacket
{
uint32_t FtpParser::getProtocol() const
{
    return ApplicationProtocol::FTP;
}

bool FtpParser::parse(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& transportHeader, const uint8_t* payload,
                      uint32_t payloadLen)
{
    return false;
}
} // namespace npacket
