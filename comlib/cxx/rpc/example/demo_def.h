#pragma once

enum class PROC_TYPE
{
    CLIENT1_FUNC = 0,
    CLIENT2_FUNC,
    CLIENT3_FUNC
};

std::string proc_name(const PROC_TYPE& type)
{
    switch (type)
    {
    case PROC_TYPE::CLIENT1_FUNC:
        return "client1Func";
    case PROC_TYPE::CLIENT2_FUNC:
        return "client2Func";
    case PROC_TYPE::CLIENT3_FUNC:
        return "client3Func";
    default:
        return "unknow proc [%d]" + std::to_string((int)type);
    }
}
