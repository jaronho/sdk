#include "file_deleter.h"

#include "threading/async_proxy.h"
#include "utility/datetime/datetime.h"
#include "utility/filesystem/file_info.h"
#include "utility/filesystem/path_info.h"
#include "utility/strtool/strtool.h"

namespace toolkit
{
struct InfoInner
{
    InfoInner(const std::string& name, const utility::FileAttribute& attr, int depth) : name(name), attr(attr), depth(depth) {}
    std::string name;
    utility::FileAttribute attr;
    int depth;
};

void FileDeleter::setFolderDeletedCallback(const FolderDeletedCallback& callback)
{
    m_folderDeletedCb = callback;
}

void FileDeleter::setFileDeletedCallback(const FileDeletedCallback& callback)
{
    m_fileDeletedCb = callback;
}

void FileDeleter::start(int interval, const std::vector<ExpireConfig>& expireCfgList)
{
    if (interval <= 0)
    {
        interval = 60;
    }
    auto detectInterval = std::chrono::seconds(interval);
    if (m_detectTimer)
    {
        return;
    }
    {
        std::lock_guard<std::mutex> locker(m_mutexCfg);
        m_expireCfgList = expireCfgList;
    }
    m_detectTimer = threading::SteadyTimer::onceTimer(THREADING_CALLER, detectInterval,
                                                      [&](const std::chrono::steady_clock::time_point& tp) { onDetectTimer(); });
    m_detectTimer->start();
    onDetectTimer();
}

void FileDeleter::start(int hour, int minute, const std::vector<ExpireConfig>& expireCfgList)
{
    if (hour < 0 || hour > 23)
    {
        hour = 0;
    }
    if (minute < 0 || minute > 59)
    {
        minute = 0;
    }
    if (m_detectTimer)
    {
        return;
    }
    {
        std::lock_guard<std::mutex> locker(m_mutexCfg);
        m_expireCfgList = expireCfgList;
    }
    m_detectTimer = threading::SteadyTimer::onceTimer(THREADING_CALLER, std::chrono::minutes(1),
                                                      [&, hour, minute](const std::chrono::steady_clock::time_point& tp) {
                                                          static int s_hour = -1;
                                                          static int s_minute = -1;
                                                          auto dt = utility::DateTime::getNow();
                                                          if (hour == dt.hour && minute == dt.minute)
                                                          {
                                                              if (hour != s_hour && minute != s_minute)
                                                              {
                                                                  s_hour = hour;
                                                                  s_minute = minute;
                                                                  onDetectTimer();
                                                              }
                                                          }
                                                          else
                                                          {
                                                              s_hour = -1;
                                                              s_minute = -1;
                                                          }
                                                      });
    m_detectTimer->start();
    onDetectTimer();
}

void FileDeleter::deleteOccupy(const OccupyConfig& cfg, const FolderDeletedCallback& folderDeletedCb,
                               const FileDeletedCallback& fileDeletedCb)
{
    if (cfg.clearSize <= 0) /* 不需要清除 */
    {
        return;
    }
    utility::PathInfo pi(cfg.folder);
    if (!pi.exist()) /* 目录不存在 */
    {
        return;
    }
    std::vector<InfoInner> folderList;
    std::vector<InfoInner> fileList;
    auto folderCb = [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
        if (utility::StrTool::contains(name, "$RECYCLE.BIN", false)
            || utility::StrTool::contains(name, "System Volume Information", false)) /* 文件系统目录(跳过) */
        {
            return false;
        }
        folderList.emplace_back(InfoInner(name, attr, depth));
        return true;
    };
    auto fileCb = [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
        for (auto ignoreFile : cfg.ignoreFileList)
        {
            if (name == ignoreFile) /* 忽略 */
            {
                return;
            }
        }
        fileList.emplace_back(InfoInner(name, attr, depth));
    };
    pi.traverse(folderCb, fileCb, nullptr, true);
    /* 删除最早的文件 */
    std::sort(fileList.begin(), fileList.end(), [](InfoInner a, InfoInner b) { return a.attr.modifyTime < b.attr.modifyTime; });
    size_t deletedSize = 0;
    for (auto file : fileList)
    {
        if (deletedSize >= cfg.clearSize)
        {
            break;
        }
        auto ok = utility::FileInfo(file.name).remove();
        if (fileDeletedCb)
        {
            fileDeletedCb(file.name, file.attr, file.depth, ok);
        }
        if (ok)
        {
            deletedSize += file.attr.size;
        }
    }
    /* 删除空目录 */
    for (auto folder : folderList)
    {
        auto pi = utility::PathInfo(folder.name);
        if (pi.empty(true))
        {
            auto ok = pi.remove();
            if (folderDeletedCb)
            {
                folderDeletedCb(folder.name, folder.attr, folder.depth, ok);
            }
        }
    }
}

void FileDeleter::deleteExpired(const std::vector<ExpireConfig>& cfgList, const FolderDeletedCallback& folderDeletedCb,
                                const FileDeletedCallback& fileDeletedCb)
{
    for (auto cfg : cfgList)
    {
        if (cfg.expireTime <= 0) /* 过期时间无效 */
        {
            continue;
        }
        utility::PathInfo pi(cfg.folder);
        if (!pi.exist()) /* 目录不存在 */
        {
            continue;
        }
        auto nowTimestamp = (int64_t)utility::DateTime::getNowTimestamp();
        std::vector<InfoInner> folderList;
        auto folderCb = [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            if (utility::StrTool::contains(name, "$RECYCLE.BIN", false)
                || utility::StrTool::contains(name, "System Volume Information", false)) /* 文件系统目录(跳过) */
            {
                return false;
            }
            folderList.emplace_back(InfoInner(name, attr, depth));
            return true;
        };
        auto fileCb = [&](const std::string& name, const utility::FileAttribute& attr, int depth) {
            for (auto ignoreFile : cfg.ignoreFileList)
            {
                if (name == ignoreFile) /* 忽略 */
                {
                    return;
                }
            }
            auto modifyTimestamp = (int64_t)utility::DateTime(attr.modifyTimeFmt()).toTimestamp();
            if (nowTimestamp - modifyTimestamp >= cfg.expireTime) /* 过期, 需要删除 */
            {
                auto ok = utility::FileInfo(name).remove();
                if (fileDeletedCb)
                {
                    fileDeletedCb(name, attr, depth, ok);
                }
            }
        };
        pi.traverse(folderCb, fileCb, nullptr, true);
        /* 删除空目录 */
        for (auto folder : folderList)
        {
            auto pi = utility::PathInfo(folder.name);
            if (pi.empty(true))
            {
                auto ok = pi.remove();
                if (folderDeletedCb)
                {
                    folderDeletedCb(folder.name, folder.attr, folder.depth, ok);
                }
            }
        }
    }
}

void FileDeleter::onDetectTimer()
{
    std::vector<ExpireConfig> expireCfgList;
    {
        std::lock_guard<std::mutex> locker(m_mutexCfg);
        expireCfgList = m_expireCfgList;
    }
    if (!expireCfgList.empty())
    {
        /* 由于文件删除会进行I/O耗时操作, 这里异步执行 */
        const std::weak_ptr<threading::SteadyTimer> wpDetectTimer = m_detectTimer;
        threading::AsyncProxy::execute(
            THREADING_CALLER, [&, wpDetectTimer, expireCfgList, folderDeletedCb = m_folderDeletedCb, fileDeletedCb = m_fileDeletedCb]() {
                const auto detectTimer = wpDetectTimer.lock();
                if (detectTimer)
                {
                    deleteExpired(expireCfgList, folderDeletedCb, fileDeletedCb);
                    detectTimer->start();
                }
            });
    }
}
} // namespace toolkit
