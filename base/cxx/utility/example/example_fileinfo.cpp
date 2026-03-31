#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <chrono>
#include <iostream>

#include "../utility/filesystem/fs_define.h"

int main(int argc, char** argv)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    /* 关闭控制台程序的快速编辑模式, 否则会出现点击界面, 程序将会变成阻塞状态, 不按回车无法继续运行 */
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE; /* 移除快速编辑模式 */
    SetConsoleMode(hStdin, mode);
#endif
    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            if (i > 1)
            {
                printf("--------------------------------------------------\n");
            }
            utility::FileAttribute attr;
            utility::getFileAttribute(argv[i], attr);
            if (utility::getFileAttribute(argv[i], attr))
            {
                printf("        %s: %s\n", attr.isFile ? "文件" : "目录", argv[i]);
                printf("    创建时间: %s\n", attr.createTimeFmt().c_str());
                printf("最后修改时间: %s\n", attr.modifyTimeFmt().c_str());
                printf("        大小: %zu 字节\n", attr.size);
                printf("    是否链接: %s\n", attr.isSymLink ? "是" : "否");
#ifdef _WIN32
                printf("    是否系统: %s\n", attr.isSystem ? "是" : "否");
#endif
                printf("    是否隐藏: %s\n", attr.isHidden ? "是" : "否");
                printf("    是否可写: %s\n", attr.isWritable ? "是" : "否(只读)");
                printf("  是否可执行: %s\n", attr.isExecutable ? "是" : "否");
            }
            else
            {
                printf("目标: %s 不存在\n", argv[i]);
            }
        }
    }
    else
    {
        printf("请输入目标, 多个目标用空格分隔\n");
    }
    return 0;
}
