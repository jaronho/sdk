#include <chrono>
#include <iostream>

#include "../utility/cmdline/cmdline.h"
#include "../utility/filesystem/file_copy.h"
#include "../utility/strtool/strtool.h"

void testFileCopy(int argc, char** argv)
{
    cmdline::parser parser;
    parser.add<std::string>("src", 's', "source directory", true);
    parser.add<std::string>("files", 'f', "assign source files to copy (divide with ','), will copy all src directory if not set", false);
    parser.add<std::string>("dest", 'd', "destination directory", true);
    parser.add<int>("clear", 'c', "whether clear destination directory before copy", false, 0);
    parser.add<int>("cover", 'r', "whether cover same name file in destination directory", false, 0);
    parser.parse_check(argc, argv);
    /* 解析参数 */
    auto srcPath = parser.get<std::string>("src");
    auto filelist = parser.get<std::string>("files");
    auto destPath = parser.get<std::string>("dest");
    auto clearDest = parser.get<int>("clear");
    auto coverDest = parser.get<int>("cover");
    /* 参数设置 */
    auto srcFilelist = utility::StrTool::split(filelist, ",");
    auto destNameAlterFunc = [](const std::string& relativePath) { return relativePath; };
    auto filterFunc = [](const std::string& name, const utility::FileAttribute& attr, int depth) {
        if (1 == depth)
        {
            if (attr.isDir)
            {
                auto dirName = utility::FileInfo(name).filename();
                if ("$RECYCLE.BIN" == dirName || "System Volume Information" == dirName) /* 跳过Windows文件系统目录 */
                {
                    printf("[%d] source directory %s is system, skip\n", depth, name.c_str());
                    return true;
                }
            }
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
            if (attr.isHidden) /* 隐藏 */
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
    auto singleOkCb = [&](const std::string& srcFile, const utility::FileAttribute& srcAttr, const utility::FileCopyDestInfo& destFile) {
        printf("copy '%s' to '%s' ok", srcFile.c_str(), destFile.realFile.c_str());
    };
    std::string failSrcFile;
    utility::FileCopyDestInfo failDestFile;
    int failCode;
    /* 开始拷贝 */
    printf("start copy ...\n");
    std::chrono::steady_clock::time_point tm1 = std::chrono::steady_clock::now();
    utility::FileCopy fc(srcPath, destPath, clearDest, coverDest, destNameAlterFunc, filterFunc, nullptr, ".tmp",
                         std::vector<utility::FileInfo::CopyBlock>{}, 3000);
    fc.setCallback(beginCb, totalProgressCb, singleProgressCb, singleOkCb);
    auto result = fc.start(srcFilelist, nullptr, &failSrcFile, &failDestFile, &failCode);
    switch (result)
    {
    case utility::FileInfo::CopyResult::ok:
        printf("\nfile copy ok\n");
        break;
    case utility::FileInfo::CopyResult::src_open_failed:
        printf("\nfile copy fail: source file %s open fail: %d %s\n", failSrcFile.c_str(), failCode, strerror(failCode));
        break;
    case utility::FileInfo::CopyResult::dest_open_failed:
        printf("\nfile copy fail: destination file %s open fail: %d %s\n", failDestFile.realFile.c_str(), failCode, strerror(failCode));
        break;
    case utility::FileInfo::CopyResult::memory_alloc_failed:
        printf("\nfile copy fail: source file %s, destination file: %s, memory allocate fail: %d %s\n", failSrcFile.c_str(),
               failDestFile.realFile.c_str(), failCode, strerror(failCode));
        break;
    case utility::FileInfo::CopyResult::src_read_failed:
        printf("\nfile copy fail: source file %s read fail\n", failSrcFile.c_str());
        break;
    case utility::FileInfo::CopyResult::dest_write_failed:
        printf("\nfile copy fail: destination file: %s write fail\n", failDestFile.realFile.c_str());
        break;
    case utility::FileInfo::CopyResult::stop:
        printf("\nfile copy fail: source file %s, destination file: %s, stop\n", failSrcFile.c_str(), failDestFile.realFile.c_str());
        break;
    case utility::FileInfo::CopyResult::size_unequal:
        printf("\nfile copy fail: source file %s != destination file %s\n", failSrcFile.c_str(), failDestFile.realFile.c_str());
        break;
    }
    std::chrono::steady_clock::time_point tm2 = std::chrono::steady_clock::now();
    printf("cost: %lld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(tm2 - tm1).count());
    printf("finish\n");
}
