#include <chrono>
#include <iostream>

#include "../utility/filesystem/file_info.h"

int main(int argc, char** argv)
{
    bool firstFlag = true;
    for (int i = 1; i < argc; ++i)
    {
        utility::FileInfo fi(argv[i]);
        if (fi.exist())
        {
            if (!firstFlag)
            {
                printf("--------------------------------------------------\n");
            }
            auto attr = fi.attribute();
            printf("        文件: %s\n", fi.name().c_str());
            printf("    创建时间: %s\n", attr.createTimeFmt().c_str());
            printf("最后修改时间: %s\n", attr.modifyTimeFmt().c_str());
            printf("        大小: %zu 字节\n", attr.size);
            printf("是否链接文件: %s\n", attr.isSymLink ? "是" : "否");
            printf("是否系统文件: %s\n", attr.isSystem ? "是" : "否");
            printf("是否隐藏文件: %s\n", attr.isHidden ? "是" : "否");
            printf("是否可写文件: %s\n", attr.isWritable ? "是" : "否(只读)");
            printf("  是否可执行: %s\n", attr.isExecutable ? "是" : "否");
        }
    }
    return 0;
}
