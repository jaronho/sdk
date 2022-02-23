#pragma once

#include <stdio.h>
#include <string.h>
#include <string>

#include "../utilitiy/charset/charset.h"
#include "../utilitiy/cmdline/cmdline.h"
#include "../utilitiy/filesystem/file_info.h"
#include "../utilitiy/filesystem/path_info.h"
#include "../utilitiy/strtool/strtool.h"
#include "../utilitiy/system/system.h"

void testCharset(int argc, char** argv)
{
    printf("current locale: %s\n\n", utilitiy::Charset::getLocale().c_str());
    cmdline::parser parser;
    parser.add<std::string>("dir", 'd', "directory", true);
    parser.add<int>("recursive", 'r', "whether recursive sub directory", false, 0);
    parser.parse_check(argc, argv);
    /* 解析参数 */
    auto path = parser.get<std::string>("dir");
    auto subdir = parser.get<int>("recursive");
    utilitiy::PathInfo pi(path);
    if (pi.exist())
    {
        printf("path: %s, exist\n\n", pi.path().c_str());
        pi.traverse(
            [&](const std::string& name, const utilitiy::FileAttribute& attr, int depth) {
                printf("\n========== DIR === %s ===", name.c_str());
                if (utilitiy::Charset::isAscii(name))
                {
                    printf(" ASCII\n");
                }
                else
                {
                    printf(" charset:");
                    switch (utilitiy::Charset::getCoding(name))
                    {
                    case utilitiy::Charset::Coding::gbk:
                        printf(" GBK\n");
                        printf("             UTF8: %s\n", utilitiy::Charset::gbkToUtf8(name).c_str());
                        break;
                    case utilitiy::Charset::Coding::utf8:
                        printf(" UTF8\n");
                        break;
                    case utilitiy::Charset::Coding::unknown:
                        printf(" Unknown\n");
                        break;
                    }
                }
                return (subdir > 0);
            },
            [&](const std::string& name, const utilitiy::FileAttribute& attr, int depth) {
                printf("----- FILE --- %s --- ", name.c_str());
                if (utilitiy::Charset::isAscii(name))
                {
                    printf(" ASCII\n");
                }
                else
                {
                    printf(" charset:");
                    switch (utilitiy::Charset::getCoding(name))
                    {
                    case utilitiy::Charset::Coding::gbk:
                        printf(" GBK\n");
                        printf("         UTF8: %s\n", utilitiy::Charset::gbkToUtf8(name).c_str());
                        break;
                    case utilitiy::Charset::Coding::utf8:
                        printf(" UTF8\n");
                        break;
                    case utilitiy::Charset::Coding::unknown:
                        printf(" Unknown\n");
                        break;
                    }
                }
                utilitiy::FileInfo fi(name);
                if (fi.isTextFile())
                {
                    long long contentLen;
                    char* content = fi.data(contentLen, true);
                    if (content)
                    {
                        printf("      text file content charset:");
                        switch (utilitiy::Charset::getCoding(content))
                        {
                        case utilitiy::Charset::Coding::gbk:
                            printf(" GBK\n");
                            break;
                        case utilitiy::Charset::Coding::utf8:
                            printf(" UTF8\n");
                            break;
                        case utilitiy::Charset::Coding::unknown:
                            printf(" Unknown\n");
                            break;
                        }
                        free(content);
                    }
                    else
                    {
                        printf("    file content empty\n");
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
