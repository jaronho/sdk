#pragma once

#include <stdio.h>
#include <string.h>
#include <string>

#include "../utility/charset/charset.h"
#include "../utility/cmdline/cmdline.h"
#include "../utility/filesystem/file_info.h"
#include "../utility/filesystem/path_info.h"
#include "../utility/strtool/strtool.h"
#include "../utility/system/system.h"

void testCharset(int argc, char** argv)
{
    printf("current locale: %s\n\n", utility::Charset::getLocale().c_str());
    cmdline::parser parser;
    parser.add<std::string>("dir", 'd', "directory", true);
    parser.add<int>("recursive", 'r', "whether recursive sub directory", false, 0);
    parser.parse_check(argc, argv);
    /* 解析参数 */
    auto path = parser.get<std::string>("dir");
    auto subdir = parser.get<int>("recursive");
    utility::PathInfo pi(path);
    if (pi.exist())
    {
        printf("path: %s, exist\n\n", pi.path().c_str());
        pi.traverse(
            [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
                printf("\n========== DIR === %s ===", name.c_str());
                if (utility::Charset::isAscii(name))
                {
                    printf(" ASCII\n");
                }
                else
                {
                    printf(" charset:");
                    switch (utility::Charset::getCoding(name))
                    {
                    case utility::Charset::Coding::gbk:
                        printf(" GBK\n");
                        printf("             UTF8: %s\n", utility::Charset::gbkToUtf8(name).c_str());
                        break;
                    case utility::Charset::Coding::utf8:
                        printf(" UTF8\n");
                        break;
                    case utility::Charset::Coding::unknown:
                        printf(" Unknown\n");
                        break;
                    }
                }
                return (subdir > 0);
            },
            [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
                printf("----- FILE --- %s --- ", name.c_str());
                if (utility::Charset::isAscii(name))
                {
                    printf(" ASCII\n");
                }
                else
                {
                    printf(" charset:");
                    switch (utility::Charset::getCoding(name))
                    {
                    case utility::Charset::Coding::gbk:
                        printf(" GBK\n");
                        printf("         UTF8: %s\n", utility::Charset::gbkToUtf8(name).c_str());
                        break;
                    case utility::Charset::Coding::utf8:
                        printf(" UTF8\n");
                        break;
                    case utility::Charset::Coding::unknown:
                        printf(" Unknown\n");
                        break;
                    }
                }
                utility::FileInfo fi(name);
                if (fi.isTextFile())
                {
                    auto content = fi.readAll();
                    if (content.empty())
                    {
                        printf("    file content empty\n");
                    }
                    else
                    {
                        printf("      text file content charset:");
                        switch (utility::Charset::getCoding(content))
                        {
                        case utility::Charset::Coding::gbk:
                            printf(" GBK\n");
                            break;
                        case utility::Charset::Coding::utf8:
                            printf(" UTF8\n");
                            break;
                        case utility::Charset::Coding::unknown:
                            printf(" Unknown\n");
                            break;
                        }
                    }
                }
            },
            nullptr, true);
    }
    else
    {
        printf("path: %s, not exist\n", pi.path().c_str());
    }
    printf("\n");
}
