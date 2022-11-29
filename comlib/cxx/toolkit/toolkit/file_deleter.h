#pragma once
#include <functional>
#include <mutex>
#include <vector>

#include "threading/timer/steady_timer.h"
#include "utility/filesystem/fs_define.h"

namespace toolkit
{
/**
 * @brief 文件删除器(依赖异步任务,定时器)
 */
class FileDeleter final
{
public:
    /**
     * @brief 占满配置
     */
    struct OccupyConfig
    {
        OccupyConfig() = default;
        OccupyConfig(const std::string& folder, size_t clearSize) : folder(folder), clearSize(clearSize) {}

        std::string folder; /* 目录 */
        size_t clearSize = 0; /* 要删除的文件总大小(字节) */
    };

    /**
     * @brief 过期配置
     */
    struct ExpireConfig
    {
        ExpireConfig() = default;
        ExpireConfig(const std::string& folder, size_t expireTime) : folder(folder), expireTime(expireTime) {}

        std::string folder; /* 目录 */
        size_t expireTime = (3600 * 24 * 30); /* 过期时间(秒), 文件最后修改时间过期则删除, 默认30天 */
    };

    /**
     * @brief 目录被删除回调
     * @param name 目录名(全路径)
     * @param attr 属性
     * @param depth 深度
     * @param ok 是否删除成功
     */
    using FolderDeletedCallback = std::function<void(const std::string& name, const utility::FileAttribute& attr, int depth, bool ok)>;

    /**
     * @brief 文件被删除回调
     * @param name 文件名(全路径)
     * @param attr 属性
     * @param depth 深度
     * @param ok 是否删除成功
     */
    using FileDeletedCallback = std::function<void(const std::string& name, const utility::FileAttribute& attr, int depth, bool ok)>;

public:
    /**
     * @brief 设置目录被删除回调
     * @param callback 回调
     */
    void setFolderDeletedCallback(const FolderDeletedCallback& callback);

    /**
     * @brief 设置文件被删除回调
     * @param callback 回调
     */
    void setFileDeletedCallback(const FileDeletedCallback& callback);

    /**
     * @brief 开始(周期循环检测)
     * @param interval 检测周期(秒)
     * @param occupyCfg 占满配置
     * @param expireCfgList 过期配置列表
     */
    void start(int interval, const OccupyConfig& occupyCfg, const std::vector<ExpireConfig>& expireCfgList);

    /**
     * @brief 开始(指定每天小时分钟检测)
     * @param hour 检测周期(秒)
     * @param occupyCfg 占满配置
     * @param expireCfgList 过期配置列表
     */
    void start(int hour, int minute, const OccupyConfig& occupyCfg, const std::vector<ExpireConfig>& expireCfgList);

    /**
     * @brief 删除占满的文件
     * @param cfg 配置
     * @param folderDeletedCb 目录被删除回调
     * @param fileDeletedCb 文件被删除回调
     */
    static void deleteOccupy(const OccupyConfig& cfg, const FolderDeletedCallback& folderDeletedCb,
                             const FileDeletedCallback& fileDeletedCb);

    /**
     * @brief 删除过期的文件
     * @param cfgList 配置列表
     * @param folderDeletedCb 目录被删除回调
     * @param fileDeletedCb 文件被删除回调
     */
    static void deleteExpired(const std::vector<ExpireConfig>& cfgList, const FolderDeletedCallback& folderDeletedCb,
                              const FileDeletedCallback& fileDeletedCb);

private:
    /**
     * @brief 响应检测定时器
     */
    void onDetectTimer();

private:
    threading::SteadyTimerPtr m_detectTimer = nullptr; /* 检测定时器 */
    std::mutex m_mutexCfg;
    OccupyConfig m_occupyCfg; /* 占满配置 */
    std::vector<ExpireConfig> m_expireCfgList; /* 过期配置列表 */
    FolderDeletedCallback m_folderDeletedCb = nullptr; /* 目录被删除回调 */
    FileDeletedCallback m_fileDeletedCb = nullptr; /* 文件被删除回调 */
};
} // namespace toolkit
