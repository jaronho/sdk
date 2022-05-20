#include "file_deleter.h"

#include "threading/async_proxy.h"
#include "utility/datetime/datetime.h"
#include "utility/filesystem/file_info.h"
#include "utility/filesystem/path_info.h"
#include "utility/strtool/strtool.h"

namespace toolkit
{
void FileDeleter::setFolderDeletedCallback(const std::function<void(const std::string& fullName, bool ok)>& callback)
{
    m_folderDeletedCb = callback;
}

void FileDeleter::setFileDeletedCallback(const std::function<void(const std::string& fullName, bool ok)>& callback)
{
    m_fileDeletedCb = callback;
}

void FileDeleter::start(int interval, const std::vector<FileDeleteConfig>& cfgList)
{
    if (interval <= 0)
    {
        interval = 60;
    }
    auto detectInterval = std::chrono::seconds(interval);
    {
        std::lock_guard<std::mutex> locker(m_mutexCfgList);
        m_cfgList = cfgList;
    }
    if (m_detectTimer)
    {
        m_detectTimer->setInterval(detectInterval);
    }
    else
    {
        m_detectTimer = std::make_shared<threading::SteadyTimer>(THREADING_CALLER, std::chrono::steady_clock::duration::zero(),
                                                                 detectInterval, [&]() { onDetectTimer(); });
    }
    m_detectTimer->start();
}

void FileDeleter::onDetectTimer()
{
    std::vector<FileDeleteConfig> cfgList;
    {
        std::lock_guard<std::mutex> locker(m_mutexCfgList);
        cfgList = m_cfgList;
    }
    /* 由于文件删除会进行I/O耗时操作, 这里异步执行 */
    threading::AsyncProxy::execute(THREADING_CALLER, [&, cfgList]() {
        for (auto cfg : cfgList)
        {
            handleConfig(cfg);
        }
    });
}

void FileDeleter::handleConfig(const FileDeleteConfig& cfg)
{
    if (cfg.expireSecond <= 0) /* 过期时间无效 */
    {
        return;
    }
    utility::PathInfo pi(cfg.folder);
    if (!pi.exist()) /* 目录不存在 */
    {
        return;
    }
    auto nowTimestamp = (int64_t)utility::DateTime::getNowTimestamp();
    auto folderCb = [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
        if (utility::StrTool::contains(name, "System Volume Information", false)) /* 文件系统目录(跳过) */
        {
            return false;
        }
        if (1 == depth && attr.isHidden) /* 第1层且为隐藏的目录(跳过) */
        {
            return false;
        }
        auto modifyTimestamp = (int64_t)utility::DateTime(attr.modifyTimeFmt()).toTimestamp();
        if (nowTimestamp - modifyTimestamp >= cfg.expireSecond) /* 过期, 需要删除 */
        {
            auto ok = utility::PathInfo(name).remove();
            if (m_folderDeletedCb)
            {
                m_folderDeletedCb(name, ok);
            }
        }
        return false;
    };
    auto fileCb = [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
        if (1 == depth && attr.isHidden) /* 第1层且为隐藏的文件(跳过) */
        {
            return;
        }
        auto modifyTimestamp = (int64_t)utility::DateTime(attr.modifyTimeFmt()).toTimestamp();
        if (nowTimestamp - modifyTimestamp >= cfg.expireSecond) /* 过期, 需要删除 */
        {
            auto ok = utility::FileInfo(name).remove();
            if (m_fileDeletedCb)
            {
                m_fileDeletedCb(name, ok);
            }
        }
    };
    if (cfg.both)
    {
        pi.traverse(folderCb, fileCb, nullptr, false);
    }
    else
    {
        pi.traverse(nullptr, fileCb, nullptr, false);
    }
}
} // namespace toolkit
