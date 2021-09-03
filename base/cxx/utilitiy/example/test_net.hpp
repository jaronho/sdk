#pragma once
#include <iostream>

#include "../utilitiy/net/net.h"

void testNet()
{
    printf("\n============================== test net =============================\n");
    std::string ipv4_1 = "202.112.14.137";
    std::string subnetMask_1 = "255.255.255.224";
    auto ret = utilitiy::Net::isIPv4(ipv4_1);
    printf("--- ip: %s is ipv4: %s, inner: %s\n", ipv4_1.c_str(), ret ? "true" : "false",
           utilitiy::Net::isIPv4Inner(ipv4_1) ? "true" : "false");
    auto ipv4Info = utilitiy::Net::calcIPv4Info(ipv4_1, subnetMask_1);
    printf("--- netmask: %s\n", subnetMask_1.c_str());
    printf("--- network: %s\n", ipv4Info.network.c_str());
    printf("--- host: %s\n", ipv4Info.host.c_str());
    printf("--- broadcast: %s\n", ipv4Info.broadcast.c_str());
    printf("--- default gateway: %s\n", ipv4Info.defaultGateway.c_str());
    printf("--- host count: %d\n", ipv4Info.hostCount);
    printf("\n-------------------- net card:\n");
    auto interfaceList = utilitiy::Net::getAllInterfaces();
    for (int i = 0; i < interfaceList.size(); ++i)
    {
        const auto& iface = interfaceList[i];
        printf("----- [%d]\n", i + 1);
        printf("name: %s type: %s(%d)\n", iface.name.c_str(), iface.typeStr().c_str(), iface.realType);
        printf("mac: ");
        for (int j = 0; j < iface.mac.size(); ++j)
        {
            if (j > 0)
            {
                printf(":");
            }
            printf("%s", iface.mac[j].c_str());
        }
        printf("\n");
#ifdef _WIN32
        printf("desc: %s\n", iface.desc.c_str());
        printf("ipv4 list:\n");
        for (int j = 0; j < iface.ipv4List.size(); ++j)
        {
            printf("    ip: %s netmask: %s\n", iface.ipv4List[j].ipv4.c_str(), iface.ipv4List[j].netmask.c_str());
        }
#else
        printf("ipv4: %s\n", iface.ipv4.c_str());
        printf("netmask: %s\n", iface.netmask.c_str());
        printf("broadcast: %s\n", iface.broadcast.c_str());
#endif
    }
}
