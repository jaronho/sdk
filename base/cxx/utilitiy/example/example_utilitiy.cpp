#include <chrono>
#include <iostream>

#include "../utilitiy/cmdline/cmdline.h"
#include "../utilitiy/filesystem/file_copy.h"
#include "../utilitiy/strtool/strtool.h"
#include "test_filesystem.hpp"
#include "test_process.hpp"
#include "test_system.hpp"

void testFileCopy(int argc, char** argv)
{
    cmdline::parser parser;
    parser.add<std::string>("src", 's', "源目录", true);
    parser.add<std::string>("files", 'f', "指定要拷贝的源文件列表(逗号','分隔), 如果不设则表示拷贝整个源目录", false);
    parser.add<std::string>("dest", 'd', "目标目录", true);
    parser.add<int>("clear", 'c', "拷贝前是否清空目标目录", false, 0);
    parser.add<int>("cover", 'r', "目标目录存在同名文件时是否覆盖", false, 0);
    parser.add<std::string>("udisk", 'u', "USB设备名称, 如: /dev/sdb", false, "");
    parser.add<int>("syncio", 'w', "拷贝后是否同步磁盘I/O操作", false, 0);
    parser.parse_check(argc, argv);
    /* 解析参数 */
    auto srcPath = parser.get<std::string>("src");
    auto filelist = parser.get<std::string>("files");
    auto destPath = parser.get<std::string>("dest");
    auto clearDest = parser.get<int>("clear");
    auto coverDest = parser.get<int>("cover");
    auto udisk = parser.get<std::string>("udisk");
    auto syncio = parser.get<int>("syncio");
    /* 参数设置 */
    auto srcFilelist = utilitiy::StrTool::split(filelist, ",");
    auto filterFunc = [](const std::string& name, const utilitiy::FileAttribute& attr, int depth) {
#ifdef _WIN32
        if (attr.isSystem) /* 系统 */
        {
            if (attr.isDir)
            {
                printf("第 %d 层源目录 %s 为系统目录, 跳过\n", depth, name.c_str());
            }
            else
            {
                printf("第 %d 层源文件 %s 为系统文件, 跳过\n", depth, name.c_str());
            }
            return true;
        }
#endif
        if (attr.isDir && std::string::npos != name.find("System Volume Information")) /* 文件系统目录(跳过) */
        {
            printf("第 %d 层源目录 %s 为系统目录, 跳过\n", depth, name.c_str());
            return true;
        }
        if (1 == depth && attr.isHidden) /* 第1层且为隐藏的目录(跳过) */
        {
            if (attr.isDir)
            {
                printf("第 %d 层源目录 %s 为隐藏目录, 跳过\n", depth, name.c_str());
            }
            else
            {
                printf("第 %d 层源文件 %s 为隐藏文件, 跳过\n", depth, name.c_str());
            }
            return true;
        }
        return false;
    };
    auto beginCb = [&](int totalCount, size_t totalSize) {
        printf("准备拷贝文件, 总共 %d 个文件, 总大小 %zu 字节\n", totalCount, totalSize);
    };
    auto totalProgressCb = [&](int totalCount, int index, const std::string& srcFile) {
        printf("\n===== 进度: %d/%d, 当前文件: %s\n", (index + 1), totalCount, srcFile.c_str());
    };
    auto singleProgressCb = [&](const std::string& srcFile, size_t fileSize, size_t copiedSize) {
        printf("---------- 文件进度: %zu/%zu\n", copiedSize, fileSize);
    };
    std::string failSrcFile;
    std::string failDestFile;
    int failCode;
    /* 开始拷贝 */
    if (!udisk.empty() && !destPath.empty() && "/" != destPath)
    {
        auto command = "dosfsck -v -a " + udisk;
        printf("执行命令: %s\n", command.c_str());
        utilitiy::System::runCmd(command);
        command = "ntfsfix " + udisk;
        printf("执行命令: %s\n", command.c_str());
        utilitiy::System::runCmd(command);
        command = "mount -o async,iocharset=utf8 " + udisk + " " + destPath;
        printf("执行命令: %s\n", command.c_str());
        utilitiy::System::runCmd(command);
    }
    printf("拷贝开始...\n");
    std::chrono::steady_clock::time_point tm1 = std::chrono::steady_clock::now();
    utilitiy::FileCopy fc(srcPath, srcFilelist, destPath, clearDest, coverDest, filterFunc);
    fc.setCallback(beginCb, totalProgressCb, singleProgressCb);
    auto result = fc.start(&failSrcFile, &failDestFile, &failCode);
    switch (result)
    {
    case utilitiy::FileInfo::CopyResult::OK:
        printf("\n文件拷贝成功\n");
        break;
    case utilitiy::FileInfo::CopyResult::SRC_OPEN_FAILED:
        printf("\n文件拷贝失败: 源文件 %s 打开失败: %d %s\n", failSrcFile.c_str(), failCode, strerror(failCode));
    case utilitiy::FileInfo::CopyResult::DEST_OPEN_FAILED:
        printf("\n文件拷贝失败: 目标文件 %s 打开失败: %d %s\n", failDestFile.c_str(), failCode, strerror(failCode));
    case utilitiy::FileInfo::CopyResult::MEMORY_FAILED:
        printf("\n文件拷贝失败: 源文件 %s, 目标文件: %s, 内存分配失败: %d %s\n", failSrcFile.c_str(), failDestFile.c_str(), failCode,
               strerror(failCode));
    case utilitiy::FileInfo::CopyResult::STOP:
        printf("\n文件拷贝失败: 源文件 %s, 目标文件: %s, 停止拷贝\n", failSrcFile.c_str(), failDestFile.c_str());
    case utilitiy::FileInfo::CopyResult::NOT_EQUAL:
        printf("\n文件拷贝失败: 源文件%s 和目标文件 %s 不相等\n", failSrcFile.c_str(), failDestFile.c_str());
    }
    std::chrono::steady_clock::time_point tm2 = std::chrono::steady_clock::now();
    printf("耗时: %d 毫秒\n", std::chrono::duration_cast<std::chrono::milliseconds>(tm2 - tm1).count());
    if (1 == syncio)
    {
        printf("同步磁盘I/O\n");
        utilitiy::System::runCmd("sync");
        std::chrono::steady_clock::time_point tm3 = std::chrono::steady_clock::now();
        printf("耗时: %d 毫秒\n", std::chrono::duration_cast<std::chrono::milliseconds>(tm3 - tm2).count());
    }
    if (!udisk.empty() && !destPath.empty() && "/" != destPath)
    {
        std::chrono::steady_clock::time_point tm4 = std::chrono::steady_clock::now();
        auto command = "umount -f " + destPath;
        printf("执行命令: %s\n", command.c_str());
        utilitiy::System::runCmd(command);
        std::chrono::steady_clock::time_point tm5 = std::chrono::steady_clock::now();
        printf("耗时: %d 毫秒\n", std::chrono::duration_cast<std::chrono::milliseconds>(tm5 - tm4).count());
    }
    printf("完毕\n");
}

int main(int argc, char** argv)
{
#if 1
    testFilesystem();
    testProcess();
    testSystem();
#else
    testFileCopy(argc, argv);
#endif
    return 0;
}
