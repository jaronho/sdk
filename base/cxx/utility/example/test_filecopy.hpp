#include <chrono>
#include <iostream>

#include "../utility/cmdline/cmdline.h"
#include "../utility/filesystem/file_copy.h"
#include "../utility/strtool/strtool.h"

void testFileCopy(int argc, char** argv)
{
    cmdline::parser parser;
    parser.add<std::string>("src", 's', "源目录, 例如: /home/test", true);
    parser.add<std::string>("files", 'f', "指定源目录下要拷贝的文件(全路径), 多个用逗号','分隔, 如果没设置, 则将拷贝源目录所有文件", false);
    parser.add<std::string>("dest", 'd', "目标目录, 例如: /home/temp", true);
    parser.add<int>("clear", 'c', "拷贝前是否清空目标目录, 0-不清空, 1-清空", false, 0);
    parser.add<int>("cover", 'r', "是否覆盖目标目录同名文件, 否将另外命名, 0-否, 1-是", false, 0);
    parser.add<size_t>("syncsize", 'w', "定期同步磁盘大小(字节), 最小64Mb, 0-不同步, >0-新拷贝超过该值则同步", false, 0);
    parser.parse_check(argc, argv);
    /* 解析参数 */
    auto srcPath = parser.get<std::string>("src");
    auto filelist = parser.get<std::string>("files");
    auto destPath = parser.get<std::string>("dest");
    auto clearDest = parser.get<int>("clear");
    auto coverDest = parser.get<int>("cover");
    auto syncSize = parser.get<size_t>("syncsize");
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
                    return true;
                }
            }
#ifdef _WIN32
            if (attr.isSystem) /* 系统 */
            {
                return true;
            }
#endif
        }
        return false;
    };
    auto beginCb = [&](int totalCount, size_t totalSize) {
        printf("[START] {\"total_count\":%d,\"total_size\":%zu}\n", totalCount, totalSize);
    };
    auto totalProgressCb = [&](int totalCount, int index, const std::string& srcFile, const utility::FileAttribute& srcAttr) {
        printf("[BEGIN] {\"total_count\":%d,\"index\":%d,\"src\":\"%s\",\"file_size\":%zu}\n", totalCount, index, srcFile.c_str(),
               srcAttr.size);
    };
    auto singleProgressCb = [&](const std::string& srcFile, size_t fileSize, size_t copiedSize) {
        printf("[PROGRESS] {\"src\":\"%s\",\"file_size\":%zu,\"copied_size\":%zu}\n", srcFile.c_str(), fileSize, copiedSize);
    };
    auto singleOkCb = [&](const std::string& srcFile, const utility::FileAttribute& srcAttr, const utility::FileCopyDestInfo& destFile) {
        printf("[END] {\"src\":\"%s\",\"dst\":\"%s\"}\n", srcFile.c_str(), destFile.realFile.c_str());
    };
    std::string failSrcFile;
    utility::FileCopyDestInfo failDestFile;
    int failCode;
    /* 开始拷贝 */
    std::chrono::steady_clock::time_point tm1 = std::chrono::steady_clock::now();
    utility::FileCopy fc(srcPath, destPath, clearDest, coverDest, destNameAlterFunc, filterFunc, nullptr, ".tmp_",
                         std::vector<utility::FileInfo::CopyBlock>{}, syncSize, 3000);
    fc.setCallback(beginCb, totalProgressCb, singleProgressCb, singleOkCb);
    auto result = fc.start(srcFilelist, nullptr, &failSrcFile, &failDestFile, &failCode);
    std::chrono::steady_clock::time_point tm2 = std::chrono::steady_clock::now();
    switch (result)
    {
    case utility::FileInfo::CopyResult::ok:
        printf("[OK] {\"time\":%lld}\n", std::chrono::duration_cast<std::chrono::milliseconds>(tm2 - tm1).count());
        break;
    case utility::FileInfo::CopyResult::src_open_failed:
        printf("[FAIL] {\"src\":\"%s\",\"dst\":\"%s\",\"code\":%d,\"msg\":\"source file open fail [%d] %s\"}\n", failSrcFile.c_str(),
               failDestFile.realFile.c_str(), result, failCode, strerror(failCode));
        break;
    case utility::FileInfo::CopyResult::dest_open_failed:
        printf("[FAIL] {\"src\":\"%s\",\"dst\":\"%s\",\"code\":%d,\"msg\":\"destination file open fail [%d] %s\"}\n", failSrcFile.c_str(),
               failDestFile.realFile.c_str(), result, failCode, strerror(failCode));
        break;
    case utility::FileInfo::CopyResult::memory_alloc_failed:
        printf("[FAIL] {\"src\":\"%s\",\"dst\":\"%s\",\"code\":%d,\"msg\":\"memory allocate fail [%d] %s\"}\n", failSrcFile.c_str(),
               failDestFile.realFile.c_str(), result, failCode, strerror(failCode));
        break;
    case utility::FileInfo::CopyResult::src_read_failed:
        printf("[FAIL] {\"src\":\"%s\",\"dst\":\"%s\",\"code\":%d,\"msg\":\"source file read fail [%d] %s\"}\n", failSrcFile.c_str(),
               failDestFile.realFile.c_str(), result, failCode, strerror(failCode));
        break;
    case utility::FileInfo::CopyResult::dest_write_failed:
        printf("[FAIL] {\"src\":\"%s\",\"dst\":\"%s\",\"code\":%d,\"msg\":\"destination file write fail [%d] %s\"}\n", failSrcFile.c_str(),
               failDestFile.realFile.c_str(), result, failCode, strerror(failCode));
        break;
    case utility::FileInfo::CopyResult::stop:
        printf("[FAIL] {\"src\":\"%s\",\"dst\":\"%s\",\"code\":%d,\"msg\":\"stop\"}\n", failSrcFile.c_str(), failDestFile.realFile.c_str(),
               result);
        break;
    case utility::FileInfo::CopyResult::size_unequal:
        printf("[FAIL] {\"src\":\"%s\",\"dst\":\"%s\",\"code\":%d,\"msg\":\"source file != destination file\"}\n", failSrcFile.c_str(),
               failDestFile.realFile.c_str(), result);
        break;
    default:
        printf("[FAIL] {\"src\":\"%s\",\"dst\":\"%s\",\"code\":%d,\"msg\":\"unknown\"}\n", failSrcFile.c_str(),
               failDestFile.realFile.c_str(), result);
        break;
    }
}
