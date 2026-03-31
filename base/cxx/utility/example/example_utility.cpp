#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <iostream>

#include "test_bytearray.hpp"
#include "test_charset.hpp"
#include "test_datetime.hpp"
#include "test_filesystem.hpp"
#include "test_mmfile.hpp"
#include "test_module.hpp"
#include "test_net.hpp"
#include "test_process.hpp"
#include "test_system.hpp"
#include "test_timewatch.hpp"
#include "test_util.hpp"

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
    testBytearry();
#if 0
    testCharset(argc, argv);
#else
    testDateTime();
    testFilesystem();
    testMmfile();
    testModule();
    testNet();
    testProcess();
    testSystem();
    testTimewatch();
    testUtil();
#endif
    return 0;
}
