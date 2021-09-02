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
    auto netCardList = utilitiy::Net::getNetCards();
    for (int i = 0; i < netCardList.size(); ++i)
    {
        printf("----- [%d]\n", i + 1);
        printf("name: %s\n", netCardList[i].name.c_str());
        printf("mac: ");
        for (int j = 0; j < netCardList[i].mac.size(); ++j)
        {
            if (j > 0)
            {
                printf(":");
            }
            printf("%s", netCardList[i].mac[j].c_str());
        }
        printf("\n");
        printf("type: %s\n", netCardList[i].typeStr().c_str());
        printf("desc: %s\n", netCardList[i].desc.c_str());
        printf("ip:\n");
        for (int j = 0; j < netCardList[i].ipv4List.size(); ++j)
        {
            printf("    %s\n", netCardList[i].ipv4List[j].c_str());
        }
    }
}
