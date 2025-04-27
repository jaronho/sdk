#include "file_copy.h"

namespace utility
{
FileCopy::FileCopy(const std::string& srcPath, const std::string& destPath, bool clearDest, bool coverDest,
                   const FileCopyDestNameAlterFunc& destNameAlterFunc, const FileCopyFilterFunc& filterFunc,
                   const FileCopyStopFunc& stopFunc, const std::string& tmpSuffix, const std::vector<FileInfo::CopyBlock>& blocks,
                   unsigned int retryTime)
    : m_srcPathInfo(srcPath, true)
    , m_destPathInfo(destPath, true)
    , m_clearDestPath(clearDest)
    , m_coverDestFile(coverDest)
    , m_blocks(blocks)
    , m_retryTime(retryTime)
    , m_destNameAlterFunc(destNameAlterFunc)
    , m_filterFunc(filterFunc)
    , m_stopFunc(stopFunc)
    , m_errCode(0)
{
    if (!tmpSuffix.empty())
    {
        if ('.' == tmpSuffix[0])
        {
            m_tmpSuffix = tmpSuffix;
        }
        else
        {
            m_tmpSuffix = "." + tmpSuffix;
        }
    }
}

void FileCopy::setCallback(const FileCopyBeginCallback& beginCb, const FileCopyTotalProgressCallback& totalProgressCb,
                           const FileCopySingleProgressCallback& singleProgressCb, const FileCopySingleOkCallback& singleOkCb)
{
    m_beginCallback = beginCb;
    m_totalProgressCallback = totalProgressCb;
    m_singleProgressCallback = singleProgressCb;
    m_singleOkCallback = singleOkCb;
}

FileInfo::CopyResult FileCopy::start(std::vector<std::string>& srcFilelist, std::vector<FileCopyDestInfo>* destFilelist,
                                     std::string* failSrcFile, FileCopyDestInfo* failDestFile, int* errCode)
{
    if (destFilelist)
    {
        destFilelist->clear();
    }
    if (failSrcFile)
    {
        failSrcFile->clear();
    }
    if (failDestFile)
    {
        *failDestFile = FileCopyDestInfo();
    }
    if (errCode)
    {
        *errCode = 0;
    }
    m_failSrcFile.clear();
    m_failDestFile = FileCopyDestInfo();
    m_errCode = 0;
    if (m_clearDestPath)
    {
        m_destPathInfo.clear();
    }
    FileInfo::CopyResult result;
    if (srcFilelist.empty())
    {
        result = copyAllFiles(srcFilelist, destFilelist);
    }
    else
    {
        result = copyAssignFiles(srcFilelist, destFilelist);
    }
    if (FileInfo::CopyResult::ok != result)
    {
        if (failSrcFile)
        {
            *failSrcFile = m_failSrcFile;
        }
        if (failDestFile)
        {
            *failDestFile = m_failDestFile;
        }
        if (errCode)
        {
            *errCode = m_errCode;
        }
    }
    return result;
}

FileInfo::CopyResult FileCopy::copyAllFiles(std::vector<std::string>& srcFilelist, std::vector<FileCopyDestInfo>* destFilelist)
{
    size_t totalFileSize = 0;
    m_srcPathInfo.traverse(
        [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            if (m_filterFunc && m_filterFunc(name, attr, depth)) /* 目录被过滤 */
            {
                return false;
            }
            return true;
        },
        [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            if (m_filterFunc && m_filterFunc(name, attr, depth)) /* 文件被过滤 */
            {
                return;
            }
            srcFilelist.emplace_back(name);
            totalFileSize += attr.size;
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
        return FileInfo::CopyResult::stop;
    }
    return copySrcFileList(srcFilelist, totalFileSize, m_blocks, destFilelist);
}

FileInfo::CopyResult FileCopy::copyAssignFiles(const std::vector<std::string>& srcFilelist, std::vector<FileCopyDestInfo>* destFilelist)
{
    size_t totalFileSize = 0;
    for (auto srcFile : srcFilelist)
    {
        if (m_stopFunc && m_stopFunc())
        {
            return FileInfo::CopyResult::stop;
        }
        auto fileSize = utility::FileInfo(srcFile).size();
        if (fileSize > 0)
        {
            totalFileSize += fileSize;
        }
    }
    return copySrcFileList(srcFilelist, totalFileSize, m_blocks, destFilelist);
}

FileInfo::CopyResult FileCopy::copySrcFileList(const std::vector<std::string>& srcFilelist, size_t totalFileSize,
                                               const std::vector<FileInfo::CopyBlock>& blocks, std::vector<FileCopyDestInfo>* destFilelist)
{
    if (destFilelist)
    {
        destFilelist->clear();
    }
    size_t totalFileCount = srcFilelist.size();
    if (m_beginCallback)
    {
        m_beginCallback(totalFileCount, totalFileSize);
    }
    for (size_t index = 0; index < totalFileCount; ++index)
    {
        if (m_stopFunc && m_stopFunc())
        {
            return FileInfo::CopyResult::stop;
        }
        utility::FileInfo srcFileInfo(srcFilelist[index]);
        if (0 != srcFileInfo.name().find(m_srcPathInfo.path())) /* 源文件路径不正确 */
        {
            return FileInfo::CopyResult::src_open_failed;
        }
        /* 目标文件信息 */
        auto srcRelativePath = srcFileInfo.name().substr(m_srcPathInfo.path().size());
        std::string destRelativePath;
        if (m_destNameAlterFunc)
        {
            destRelativePath = m_destNameAlterFunc(srcRelativePath);
        }
        if (destRelativePath.empty())
        {
            destRelativePath = srcRelativePath;
        }
        if (!destRelativePath.empty() && ('\\' == destRelativePath[0] || '/' == destRelativePath[0]))
        {
            destRelativePath.erase(0, 1);
        }
        FileCopyDestInfo di;
        di.showFile = m_destPathInfo.path() + srcRelativePath;
        di.realFile = m_destPathInfo.path() + destRelativePath;
        /* 判断或创建目标目录 */
        utility::PathInfo destPathInfo(utility::FileInfo(di.realFile).path());
        if (!destPathInfo.exist() && !destPathInfo.create()) /* 目标目录不存在且创建失败 */
        {
            m_failSrcFile = srcFileInfo.name();
            m_failDestFile = di;
            m_errCode = errno;
            return FileInfo::CopyResult::dest_open_failed;
        }
        auto srcAttr = srcFileInfo.attribute();
        if (m_totalProgressCallback)
        {
            m_totalProgressCallback(totalFileCount, index + 1, srcFileInfo.name(), srcAttr);
        }
        /* 执行拷贝操作 */
        int errCode = 0;
        if (!m_coverDestFile)
        {
            di.realFile = checkDestFile(di.realFile); /* 检测目标文件是否已存在并重命名 */
        }
        auto destFileTmp = di.realFile + m_tmpSuffix; /* 临时文件名 */
        auto result = srcFileInfo.copy(
            destFileTmp, &errCode,
            [&](size_t now, size_t total) {
                if (m_stopFunc && m_stopFunc())
                {
                    return false;
                }
                if (m_singleProgressCallback)
                {
                    m_singleProgressCallback(srcFileInfo.name(), total, now);
                }
                return true;
            },
            blocks, m_retryTime);
        /* 拷贝结果处理 */
        if (FileInfo::CopyResult::ok == result)
        {
            if (0 != destFileTmp.compare(di.realFile))
            {
                utility::FileInfo(di.realFile).remove();
                if (0 != rename(destFileTmp.c_str(), di.realFile.c_str())) /* 临时文件名改为正式文件名 */
                {
                    m_failSrcFile = srcFileInfo.name();
                    m_failDestFile = di;
                    m_errCode = errno;
                    return FileInfo::CopyResult::dest_open_failed;
                }
            }
            if (m_singleOkCallback)
            {
                m_singleOkCallback(srcFileInfo.name(), srcAttr, di);
            }
            if (destFilelist)
            {
                destFilelist->emplace_back(di);
            }
        }
        else
        {
            m_failSrcFile = srcFileInfo.name();
            di.realFile = destFileTmp;
            m_failDestFile = di;
            m_errCode = errCode;
            return result;
        }
    }
    return FileInfo::CopyResult::ok;
}

std::string FileCopy::checkDestFile(const std::string& destFile)
{
    auto newDestFile = destFile;
    utility::FileInfo fi(newDestFile);
    if (fi.exist())
    {
        const auto suffix = fi.extname().empty() ? "" : ("." + fi.extname());
        unsigned int num = 1;
        newDestFile = fi.path() + fi.basename() + "(" + std::to_string(num) + ")" + suffix;
        while (utility::FileInfo(newDestFile).exist())
        {
            ++num;
            newDestFile = fi.path() + fi.basename() + "(" + std::to_string(num) + ")" + suffix;
        }
    }
    return newDestFile;
}
} // namespace utility
