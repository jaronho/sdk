#pragma once
#include <string>
#include <vector>

#include "file_info.h"
#include "path_info.h"

namespace utilitiy
{
/**
 * @brief 文件拷贝过滤函数
 * @param name 文件/目录名(全路径)
 * @param attr 文件/目录属性
 * @param depth 文件/目录深度(从1开始)
 * @return true-过滤, false-不过滤
 */
using FileCopyFilterFunc = std::function<bool(const std::string& name, const FileAttribute& attr, int depth)>;

/**
 * @brief 文件拷贝停止函数
 * @return true-停止, false-继续
 */
using FileCopyStopFunc = std::function<bool()>;

/**
 * @brief 文件拷贝开始回调
 * @param totalCount 总文件数
 * @param totalSize 总文件大小(字节)
 */
using FileCopyBeginCallback = std::function<void(int totalCount, size_t totalSize)>;

/**
 * @brief 文件拷贝总进度回调
 * @param totalCount 总文件数
 * @param index 当前拷贝的索引(从0开始)
 * @param srcFile 当前拷贝的源文件
 */
using FileCopyTotalProgressCallback = std::function<void(int totalCount, int index, const std::string& srcFile)>;

/**
 * @brief 文件拷贝单个进度回调
 * @param srcFile 当前拷贝的源文件
 * @param fileSize 当前拷贝的文件大小
 * @param copiedSize 当前已拷贝的文件大小
 */
using FileCopySingleProgressCallback = std::function<void(const std::string& srcFile, size_t fileSize, size_t copiedSize)>;

class FileCopy
{
public:
    /**
     * @brief 构造函数
     * @param srcPath 源目录
     * @param srcFilelist 源文件列表(可传空列表, 表示对源目录进行全部拷贝)
     * @param destPath 目标目录
     * @param clearDest 拷贝前是否对目标目录进行清空
     * @param coverDest 若目标目录已存在同名文件是否覆盖, true-覆盖, false-重命名要拷贝的文件
     * @param filterFunc 过滤函数
     * @param stopFunc 停止函数(选填)
     * @param tmpSuffix 临时后缀名(选填), 默认不使用临时文件
     */
    FileCopy(const std::string& srcPath, const std::vector<std::string>& srcFilelist, const std::string& destPath, bool clearDest,
             bool coverDest, const FileCopyFilterFunc& filterFunc, const FileCopyStopFunc& stopFunc = nullptr,
             const std::string& tmpSuffix = "");
    FileCopy() = default;
    virtual ~FileCopy() = default;

    /**
     * @brief 设置回调函数(主要用于日志打印或者界面显示)
     * @param beginCb 开始回调
     * @param totalProgressCb 总进度回调
     * @param singleProgressCb 单个进度回调
     */
    void setCallback(const FileCopyBeginCallback& beginCb, const FileCopyTotalProgressCallback& totalProgressCb,
                     const FileCopySingleProgressCallback& singleProgressCb);

    /**
     * @brief 开始
     * @param failSrcFile 失败时源文件(选填)
     * @param failDestFile 失败时目标文件(选填)
     * @param failCode 失败时错误码(选填), 可用于strerror函数获取描述信息
     * @return 拷贝结果
     */
    FileInfo::CopyResult start(std::string* failSrcFile = nullptr, std::string* failDestFile = nullptr, int* failCode = nullptr);

private:
    /**
     * @brief 拷贝所有文件
     * @return 拷贝结果
     */
    FileInfo::CopyResult copyAllFiles();

    /**
     * @brief 拷贝指定文件
     * @return 拷贝结果
     */
    FileInfo::CopyResult copyAssignFiles();

    /**
     * @brief 拷贝源文件列表
     * @param srcFilelist 源文件列表
     * @param srcFileSize 源文件总大小
     * @return 拷贝结果
     */
    FileInfo::CopyResult copySrcFileList(const std::vector<std::string>& srcFilelist, size_t srcFileSize);

    /**
     * @brief 检测目标文件是否存在同名
     * @param destFile 目标文件名
     * @return 若存在同名目标文件, 则加上后缀并返回, 若不存在则返回原有目标文件名
     */
    std::string checkDestFile(const std::string& destFile);

private:
    utilitiy::PathInfo m_srcPathInfo; /* 源目录 */
    std::vector<std::string> m_srcFilelist; /* 源文件列表 */
    utilitiy::PathInfo m_destPathInfo; /* 目标目录 */
    std::vector<std::string> m_destFilelist; /* 已拷贝的目标文件列表 */
    bool m_clearDestPath; /* 拷贝前是否清空目标目录 */
    bool m_coverDestFile; /* 当目标目录已有同名文件时是否覆盖 */
    std::string m_tmpSuffix; /* 临时后缀名 */
    FileCopyFilterFunc m_filterFunc; /* 过滤函数 */
    FileCopyStopFunc m_stopFunc; /* 停止函数 */
    FileCopyBeginCallback m_beginCallback; /* 开始回调 */
    FileCopyTotalProgressCallback m_totalProgressCallback; /* 总进度回调 */
    FileCopySingleProgressCallback m_singleProgressCallback; /* 单个进度回调 */
    std::string m_failSrcFile; /* 失败时的源文件 */
    std::string m_failDestFile; /* 失败时的目标文件 */
    int m_failCode; /* 失败时的错误码 */
};
} // namespace utilitiy
