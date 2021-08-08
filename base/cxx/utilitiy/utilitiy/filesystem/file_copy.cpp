#include "file_copy.h"

namespace utilitiy
{
FileCopy::FileCopy(const std::string& srcPath, const std::vector<std::string>& srcFilelist, const std::string& destPath, bool clearDest,
                   bool coverDest, const FileCopyFilterFunc& filterFunc, const FileCopyStopFunc& stopFunc)
    : m_srcPathInfo(srcPath, true)
    , m_srcFilelist(srcFilelist)
    , m_destPathInfo(destPath, true)
    , m_clearDestPath(clearDest)
    , m_coverDestFile(coverDest)
    , m_filterFunc(filterFunc)
    , m_stopFunc(stopFunc)
    , m_failCode(0)
{
}

void FileCopy::setCallback(const FileCopyBeginCallback& beginCb, const FileCopyTotalProgressCallback& totalProgressCb,
                           const FileCopySingleProgressCallback& singleProgressCb)
{
    m_beginCallback = beginCb;
    m_totalProgressCallback = totalProgressCb;
    m_singleProgressCallback = singleProgressCb;
}

FileInfo::CopyResult FileCopy::start(std::string* failSrcFile, std::string* failDestFile, int* failCode)
{
    m_destFilelist.clear();
    m_failSrcFile.clear();
    m_failDestFile.clear();
    m_failCode = 0;
    if (m_clearDestPath)
    {
        m_destPathInfo.clear();
    }
    FileInfo::CopyResult result;
    if (m_srcFilelist.empty())
    {
        result = copyAllFiles();
    }
    else
    {
        result = copyAssignFiles();
    }
    if (FileInfo::CopyResult::OK != result)
    {
        if (failSrcFile)
        {
            *failSrcFile = m_failSrcFile;
        }
        if (failDestFile)
        {
            *failDestFile = m_failDestFile;
        }
        if (failCode)
        {
            *failCode = m_failCode;
        }
    }
    return result;
}

FileInfo::CopyResult FileCopy::copyAllFiles()
{
    std::vector<std::string> srcFilelist;
    size_t srcFileSize = 0;
    m_srcPathInfo.traverse(
        [&](const std::string& name, const utilitiy::FileAttribute& attr, int depth) {
            if (m_filterFunc && m_filterFunc(name, attr, depth)) /* 目录被过滤 */
            {
                return false;
            }
            return true;
        },
        [&](const std::string& name, const utilitiy::FileAttribute& attr, int depth) {
            if (m_filterFunc && m_filterFunc(name, attr, depth)) /* 文件被过滤 */
            {
                return;
            }
            srcFilelist.emplace_back(name);
            srcFileSize += attr.size;
        },
        [&]() {
            if (m_stopFunc && m_stopFunc())
            {
                return true;
            }
            return false;
        },
        true);
    if (m_stopFunc && m_stopFunc())
    {
        return FileInfo::CopyResult::STOP;
    }
    return copySrcFileList(srcFilelist, srcFileSize);
}

FileInfo::CopyResult FileCopy::copyAssignFiles()
{
    size_t srcFileSize = 0;
    for (auto srcFile : m_srcFilelist)
    {
        if (m_stopFunc && m_stopFunc())
        {
            return FileInfo::CopyResult::STOP;
        }
        srcFileSize += utilitiy::FileInfo(srcFile).size();
    }
    return copySrcFileList(m_srcFilelist, srcFileSize);
}

FileInfo::CopyResult FileCopy::copySrcFileList(const std::vector<std::string>& srcFilelist, size_t srcFileSize)
{
    size_t totalFileCount = srcFilelist.size();
    if (m_beginCallback)
    {
        m_beginCallback(totalFileCount, srcFileSize);
    }
    for (size_t index = 0; index < totalFileCount; ++index)
    {
        utilitiy::FileInfo srcFileInfo(srcFilelist[index]);
        auto destFile = m_destPathInfo.path() + srcFileInfo.name().substr(m_destPathInfo.path().size());
        if (!m_coverDestFile)
        {
            destFile = checkDestFile(destFile); /* 检测目标文件是否已存在并重命名 */
        }
        /* 判断或创建目标目录 */
        utilitiy::PathInfo destPathInfo(utilitiy::FileInfo(destFile).path());
        if (!destPathInfo.exist() && !destPathInfo.create()) /* 目标目录不存在且创建失败 */
        {
            m_failSrcFile = srcFileInfo.name();
            m_failDestFile = destFile;
            m_failCode = errno;
            return FileInfo::CopyResult::DEST_OPEN_FAILED;
        }
        if (m_totalProgressCallback)
        {
            m_totalProgressCallback(totalFileCount, index, srcFileInfo.name());
        }
        /* 执行拷贝操作 */
        int errCode = 0;
        auto result = srcFileInfo.copy(destFile, &errCode, [&](size_t now, size_t total) {
            if (m_stopFunc && m_stopFunc())
            {
                return false;
            }
            if (m_singleProgressCallback)
            {
                m_singleProgressCallback(srcFileInfo.name(), total, now);
            }
            return true;
        });
        /* 拷贝结果处理 */
        if (FileInfo::CopyResult::OK == result)
        {
            m_destFilelist.emplace_back(destFile);
        }
        else
        {
            m_failSrcFile = srcFileInfo.name();
            m_failDestFile = destFile;
            m_failCode = errCode;
            return result;
        }
    }
    return FileInfo::CopyResult::OK;
}

std::string FileCopy::checkDestFile(const std::string& destFile)
{
    int num = 0;
    auto newDestFile = destFile;
    utilitiy::FileInfo fi(newDestFile);
    while (fi.exist())
    {
        ++num;
        newDestFile = fi.path() + fi.basename() + "(" + std::to_string(num) + ")." + fi.extname();
        fi = utilitiy::FileInfo(newDestFile);
    }
    return newDestFile;
}
} // namespace utilitiy
