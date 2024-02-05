#include "filter.h"

#include <stdio.h>

std::vector<std::string> Filter::protoList()
{
    std::vector<std::string> pl;
    pl.emplace_back("ehternet");
    pl.emplace_back("ipv4");
    pl.emplace_back("arp");
    pl.emplace_back("ipv6");
    pl.emplace_back("tcp");
    pl.emplace_back("udp");
    pl.emplace_back("icmp");
    pl.emplace_back("icmpv6");
    pl.emplace_back("ftp");
    pl.emplace_back("ftp-data");
    pl.emplace_back("iec103");
    return pl;
}

Filter& Filter::getInstance()
{
    static Filter s_instance;
    return s_instance;
}

void Filter::setCondition(const std::string& condition)
{
    if (condition.empty())
    {
        return;
    }
    m_showEthernet = ("ehternet" == condition);
    m_showIpv4 = ("ipv4" == condition);
    m_showArp = ("arp" == condition);
    m_showIpv6 = ("ipv6" == condition);
    m_showTcp = ("tcp" == condition);
    m_showUdp = ("udp" == condition);
    m_showIcmp = ("icmp" == condition);
    m_showIcmpv6 = ("icmpv6" == condition);
    m_showFtp = ("ftp" == condition);
    m_showFtpData = ("ftp-data" == condition);
    m_showIec103 = ("iec103" == condition);
}

bool Filter::showEthernet() const
{
    return m_showEthernet;
}

bool Filter::showIpv4() const
{
    return m_showIpv4;
}

bool Filter::showArp() const
{
    return m_showArp;
}

bool Filter::showIpv6() const
{
    return m_showIpv6;
}

bool Filter::showTcp() const
{
    return m_showTcp;
}

bool Filter::showUdp() const
{
    return m_showUdp;
}

bool Filter::showIcmp() const
{
    return m_showIcmp;
}

bool Filter::showIcmpv6() const
{
    return m_showIcmpv6;
}

bool Filter::showFtp() const
{
    return m_showFtp;
}

bool Filter::showFtpData() const
{
    return m_showFtpData;
}

bool Filter::showIec103() const
{
    return m_showIec103;
}
