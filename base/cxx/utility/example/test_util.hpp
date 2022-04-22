#pragma once
#include <iostream>

#include "../utility/util/util.h"

void testUtil()
{
    printf("\n============================== test util =============================\n");
    {
        std::string url = "http://192.168.3.16:9001/resource/file/微信截图_20210812153113.png";
        printf("url encode before: %s\n", url.c_str());
        url = utility::Util::urlEncode(url);
        printf(" url encode after: %s\n", url.c_str());
    }
    printf("\n");
    {
        std::string url = "http://192.168.3.16:9001/resource/file/%E5%BE%AE%E4%BF%A1%E6%88%AA%E5%9B%BE_20210812153113.png";
        printf("url decode before: %s\n", url.c_str());
        url = utility::Util::urlDecode(url);
        printf(" url decode after: %s\n", url.c_str());
    }
}
