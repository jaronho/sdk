#pragma once
#include <functional>
#include <mutex>
#include <vector>

#include "threading/timer/steady_timer.h"

namespace toolkit
{
/**
 * @brief 文件删除配置
 */
struct FileDeleteConfig
{
    std::string folder; /* 指定要在哪个目录进行删除 */
    bool both = true; /* 是否删除文件和目录, true-是, false-只删除文件 */
    size_t expireSecond = (3600 * 24 * 30); /* 过期时间(秒), 文件/目录最后修改时间过期则删除, 默认30天 */
};

/**
 * @brief 文件删除器(依赖异步任务,定时器)
 */
class FileDeleter final
{
public:
    /**
     * @brief 设置目录被删除回调
     * @param callback 回调, 参数: fullName-目录名(全路径), ok-是否删除成功
     */
    void setFolderDeletedCallback(const std::function<void(const std::string& fullName, bool ok)>& callback);

    /**
     * @brief 设置文件被删除回调
     * @param callback 回调, 参数: fullName-文件名(全路径), ok-是否删除成功
     */
    void setFileDeletedCallback(const std::function<void(const std::string& fullName, bool ok)>& callback);

    /**
     * @brief 开始
     * @param interval 检测周期(秒)
     * @param cfgList 配置列表
     */
    void start(int interval, const std::vector<FileDeleteConfig>& cfgList);

private:
    /**
     * @brief 响应检测定时器
     */
    void onDetectTimer();

    /**
     * @brief 处理配置
     */
    void handleConfig(const FileDeleteConfig& cfg);

private:
    threading::SteadyTimerPtr m_detectTimer = nullptr; /* 检测定时器 */
    std::mutex m_mutexCfgList;
    std::vector<FileDeleteConfig> m_cfgList; /* 配置列表 */
    std::function<void(const std::string& name, bool ok)> m_folderDeletedCb = nullptr; /* 目录被删除回调 */
    std::function<void(const std::string& name, bool ok)> m_fileDeletedCb = nullptr; /* 文件被删除回调 */
};
} // namespace toolkit
