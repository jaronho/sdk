#include "../toolkit/net_config.h"
#include "utility/cmdline/cmdline.h"
#include "utility/strtool/strtool.h"

int main(int argc, char** argv)
{
    cmdline::parser parser;
    parser.add<std::string>("method", 'm', "[add, del] bridge", true, "add");
    parser.add<std::string>("bridge", 'b', "bridge name, e.g. \"br0\"", true);
    parser.add<std::string>("ports", 'p', "bridge ports, e.g. \"enp2s0 enp4s0\", no need when 'method' is 'del'", false, "");
    printf("%s\n", parser.usage().c_str());
    parser.parse_check(argc, argv);
    /* 解析参数 */
    auto method = parser.get<std::string>("method");
    auto bridgeName = parser.get<std::string>("bridge");
    auto bridgePorts = parser.get<std::string>("ports");
    if ("add" != method && "del" != method)
    {
        printf("method '%s' is invalid, [add, del]\n", method.c_str());
        return 0;
    }
    if (bridgeName.empty())
    {
        printf("bridge name is empty\n");
        return 0;
    }
    printf("bridge name: %s\n", bridgeName.c_str());
    if ("add" == method)
    {
        auto portList = utility::StrTool::split(bridgePorts, " ");
        printf("bridge ports:");
        for (auto port : portList)
        {
            printf(" %s", port.c_str());
        }
        printf("\n");
        if (toolkit::NetConfig::modifyBridge(bridgeName, portList))
        {
            printf("modify bridge\n");
        }
        else
        {
            printf("modify bridge (no need)\n");
        }
    }
    else if ("del" == method)
    {
        if (toolkit::NetConfig::deleteBridge(bridgeName))
        {
            printf("delete bridge\n");
        }
        else
        {
            printf("delete bridge (no need)\n");
        }
    }
    return 0;
}
