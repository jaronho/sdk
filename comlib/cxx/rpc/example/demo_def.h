#pragma once
#include <string>

enum class ProcType
{
    client1_func = 0,
    client2_func,
    client3_func
};

std::string proc_name(const ProcType& type)
{
    switch (type)
    {
    case ProcType::client1_func:
        return "client1Func";
    case ProcType::client2_func:
        return "client2Func";
    case ProcType::client3_func:
        return "client3Func";
    default:
        return "unknow proc [%d]" + std::to_string((int)type);
    }
}
