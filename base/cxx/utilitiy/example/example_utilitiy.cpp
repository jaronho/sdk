#include <chrono>
#include <iostream>
#ifndef _WIN32
#include <sys/mount.h>
#endif

#include "../utilitiy/cmdline/cmdline.h"
#include "../utilitiy/filesystem/file_copy.h"
#include "../utilitiy/strtool/strtool.h"
#include "test_filesystem.hpp"
#include "test_module.hpp"
#include "test_process.hpp"
#include "test_system.hpp"
#include "test_timewatch.hpp"

void testFileCopy(int argc, char** argv)
{
    cmdline::parser parser;
    parser.add<std::string>("src", 's', "source directory", true);
    parser.add<std::string>("files", 'f', "assign source files to copy (divide with ','), will copy all src directory if not set", false);
    parser.add<std::string>("dest", 'd', "destination directory", true);
    parser.add<int>("clear", 'c', "whether clear destination directory before copy", false, 0);
    parser.add<int>("cover", 'r', "whether cover same name file in destination directory", false, 0);
#ifndef _WIN32
    parser.add<std::string>("udisk", 'u', "USB device name to mount, e.g. /dev/sdb", false, "");
    parser.add<int>("syncio", 'w', "whether synchronize I/O after copy", false, 0);
#endif
    parser.parse_check(argc, argv);
    /* 解析参数 */
    auto srcPath = parser.get<std::string>("src");
    auto filelist = parser.get<std::string>("files");
    auto destPath = parser.get<std::string>("dest");
    auto clearDest = parser.get<int>("clear");
    auto coverDest = parser.get<int>("cover");
#ifdef _WIN32
    auto udisk = parser.get<std::string>("udisk");
    auto syncio = parser.get<int>("syncio");
#endif
    /* 参数设置 */
    auto srcFilelist = utilitiy::StrTool::split(filelist, ",");
    auto filterFunc = [](const std::string& name, const utilitiy::FileAttribute& attr, int depth) {
#ifdef _WIN32
        if (attr.isSystem) /* 系统 */
        {
            if (attr.isDir)
            {
                printf("[%d] source directory %s is system, skip\n", depth, name.c_str());
            }
            else
            {
                printf("[%d] source file %s is sysem, skip\n", depth, name.c_str());
            }
            return true;
        }
#endif
        if (attr.isDir && std::string::npos != name.find("System Volume Information")) /* 文件系统目录(跳过) */
        {
            printf("[%d] source directory %s is system, skip\n", depth, name.c_str());
            return true;
        }
        if (1 == depth && attr.isHidden) /* 第1层且为隐藏的目录(跳过) */
        {
            if (attr.isDir)
            {
                printf("[%d] source directory %s is hidden, skip\n", depth, name.c_str());
            }
            else
            {
                printf("[%d] source file %s is hidden, skip\n", depth, name.c_str());
            }
            return true;
        }
        return false;
    };
    auto beginCb = [&](int totalCount, size_t totalSize) {
        printf("prepare copy, total count: %d, total size %zu byte\n", totalCount, totalSize);
    };
    auto totalProgressCb = [&](int totalCount, int index, const std::string& srcFile) {
        printf("\n===== progress: %d/%d, file: %s\n", (index + 1), totalCount, srcFile.c_str());
    };
    auto singleProgressCb = [&](const std::string& srcFile, size_t fileSize, size_t copiedSize) {
        printf("---------- file progress: %zu/%zu\n", copiedSize, fileSize);
    };
    std::string failSrcFile;
    std::string failDestFile;
    int failCode;
    /* 开始拷贝 */
#ifndef _WIN32
    if (!udisk.empty() && !destPath.empty() && "/" != destPath)
    {
        auto command = "dosfsck -v -a " + udisk;
        printf("execute command: %s\n", command.c_str());
        utilitiy::System::runCmd(command);
        command = "ntfsfix " + udisk;
        printf("execute command: %s\n", command.c_str());
        utilitiy::System::runCmd(command);
        int ret = mount(udisk.c_str(), destPath.c_str(), "vfat", MS_SYNCHRONOUS, "utf8"); /* 方法1 */
        if (0 == ret)
        {
            printf("mount USB(vfat): %s ok\n", udisk.c_str());
        }
        else
        {
            printf("mount USB(vfat): %s fail: %d %d %s\n", udisk.c_str(), ret, errno, strerror(errno));
            command = "ntfs-3g -o sync,noatime,big_writes " + udisk + " " + destPath; /* 方法2 */
            ret = utilitiy::System::runCmd(command);
            if (0 == ret)
            {
                printf("mount USB(ntfs-3g): %s ok\n", command.c_str());
            }
            else
            {
                printf("mount USB(ntfs-3g): %s fail: %d %d %s\n", command.c_str(), ret, errno, strerror(errno));
                command = "mount -o iocharset=utf8,sync,noatime,big_writes " + udisk + " " + destPath; /* 方法3 */
                ret = utilitiy::System::runCmd(command);
                if (0 == ret)
                {
                    printf("mount USB(command): %s ok\n", command.c_str());
                }
                else
                {
                    printf("mount USB(command): %s fail: %d %d %s\n", command.c_str(), ret, errno, strerror(errno));
                }
            }
        }
    }
#endif
    printf("start copy ...\n");
    std::chrono::steady_clock::time_point tm1 = std::chrono::steady_clock::now();
    utilitiy::FileCopy fc(srcPath, srcFilelist, destPath, clearDest, coverDest, filterFunc, nullptr, ".tmp");
    fc.setCallback(beginCb, totalProgressCb, singleProgressCb);
    auto result = fc.start(&failSrcFile, &failDestFile, &failCode);
    switch (result)
    {
    case utilitiy::FileInfo::CopyResult::OK:
        printf("\nfile copy ok\n");
        break;
    case utilitiy::FileInfo::CopyResult::SRC_OPEN_FAILED:
        printf("\nfile copy fail: source file %s open fail: %d %s\n", failSrcFile.c_str(), failCode, strerror(failCode));
    case utilitiy::FileInfo::CopyResult::DEST_OPEN_FAILED:
        printf("\nfile copy fail: destination file %s open fail: %d %s\n", failDestFile.c_str(), failCode, strerror(failCode));
    case utilitiy::FileInfo::CopyResult::MEMORY_FAILED:
        printf("\nfile copy fail: source file %s, destination file: %s, memory allocate fail: %d %s\n", failSrcFile.c_str(),
               failDestFile.c_str(), failCode, strerror(failCode));
    case utilitiy::FileInfo::CopyResult::STOP:
        printf("\nfile copy fail: source file %s, destination file: %s, stop\n", failSrcFile.c_str(), failDestFile.c_str());
    case utilitiy::FileInfo::CopyResult::NOT_EQUAL:
        printf("\nfile copy fail: source file %s != destination file %s\n", failSrcFile.c_str(), failDestFile.c_str());
    }
    std::chrono::steady_clock::time_point tm2 = std::chrono::steady_clock::now();
    printf("cost: %lld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(tm2 - tm1).count());
#ifndef _WIN32
    if (1 == syncio)
    {
        printf("synchronize I/O\n");
        utilitiy::System::runCmd("sync");
        std::chrono::steady_clock::time_point tm3 = std::chrono::steady_clock::now();
        printf("cost: %d ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(tm3 - tm2).count());
    }
    if (!udisk.empty() && !destPath.empty() && "/" != destPath)
    {
        std::chrono::steady_clock::time_point tm4 = std::chrono::steady_clock::now();
        auto command = "umount -f " + destPath;
        printf("execute command: %s\n", command.c_str());
        utilitiy::System::runCmd(command);
        std::chrono::steady_clock::time_point tm5 = std::chrono::steady_clock::now();
        printf("cost: %d ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(tm5 - tm4).count());
    }
#endif
    printf("finish\n");
}

int main(int argc, char** argv)
{
#if 1
    testFilesystem();
    testModule();
    testProcess();
    testSystem();
    testTimewatch();
#else
    testFileCopy(argc, argv);
#endif
    return 0;
}
