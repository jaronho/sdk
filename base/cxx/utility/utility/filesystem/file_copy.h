#pragma once
#include <string>
#include <vector>

#include "file_info.h"
#include "path_info.h"

namespace utility
{
/**
 * @brief 目标文件信息
 */
struct FileCopyDestInfo
{
    std::string showFile; /* 展示的文件(全路径) */
    std::string realFile; /* 实际的文件(全路径) */
};

/**
 * @brief 文件拷贝目标文件名变更函数
 * @param relativePath 目标文件名(包含相对路径)
 * @return 变更后的目标文件名
 */
using FileCopyDestNameAlterFunc = std::function<std::string(const std::string& relativePath)>;

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

/**
 * @brief 文件拷贝单个成功回调
 * @param srcFile 当前拷贝的源文件
 * @param srcAttr 当前拷贝的源文件属性
 * @param destFile 目标文件
 */
using FileCopySingleOkCallback =
    std::function<void(const std::string& srcFile, const FileAttribute& srcAttr, const FileCopyDestInfo& destFile)>;

/**
 * @brief 文件拷贝
 */
class FileCopy
{
public:
    /**
     * @brief 构造函数
     * @param srcPath 源目录
     * @param destPath 目标目录
     * @param clearDest 拷贝前是否对目标目录进行清空
     * @param coverDest 若目标目录已存在同名文件是否覆盖, true-覆盖, false-重命名要拷贝的文件
     * @param destNameAlterFunc 目标文件名变更函数
     * @param filterFunc 过滤函数
     * @param stopFunc 停止函数(选填)
     * @param tmpSuffix 临时后缀名(选填), 默认不使用临时文件
     * @param blocks 拷贝块大小, 为空时表示使用默认(最大64Kb)
     * @param retryTime 读写失败时重试时间(毫秒), 为0表示一直重试
     */
    FileCopy(const std::string& srcPath, const std::string& destPath, bool clearDest, bool coverDest,
             const FileCopyDestNameAlterFunc& destNameAlterFunc, const FileCopyFilterFunc& filterFunc,
             const FileCopyStopFunc& stopFunc = nullptr, const std::string& tmpSuffix = "",
             const std::vector<FileInfo::CopyBlock>& blocks = {}, unsigned int retryTime = 3000);
    FileCopy() = default;
    virtual ~FileCopy() = default;

    /**
     * @brief 设置回调函数(主要用于日志打印或者界面显示)
     * @param beginCb 开始回调
     * @param totalProgressCb 总进度回调
     * @param singleProgressCb 单个进度回调
     * @param singleOkCb 单个成功回调
     */
    void setCallback(const FileCopyBeginCallback& beginCb, const FileCopyTotalProgressCallback& totalProgressCb,
                     const FileCopySingleProgressCallback& singleProgressCb, const FileCopySingleOkCallback& singleOkCb);

    /**
     * @brief 开始
     * @param srcFilelist [输入/输出]源文件列表(可传空列表, 表示对源目录进行全部拷贝), 注意: 所有源文件都必须具有相同的源目录srcPath
     * @param destFilelist [输出]目标文件列表(选填)
     * @param failSrcFile [输出]失败时源文件(选填)
     * @param failDestFile [输出]失败时目标文件(选填)
     * @param errCode [输出]失败时错误码(选填), 可用于strerror函数获取描述信息
     * @return 拷贝结果
     */
    FileInfo::CopyResult start(std::vector<std::string>& srcFilelist, std::vector<FileCopyDestInfo>* destFilelist,
                               std::string* failSrcFile = nullptr, FileCopyDestInfo* failDestFile = nullptr, int* errCode = nullptr);

private:
    /**
     * @brief 拷贝所有文件
     * @param srcFilelist [输出]源文件列表
     * @param destFilelist [输出]目标文件列表
     * @return 拷贝结果
     */
    FileInfo::CopyResult copyAllFiles(std::vector<std::string>& srcFilelist, std::vector<FileCopyDestInfo>* destFilelist);

    /**
     * @brief 拷贝指定文件
     * @param srcFilelist 源文件列表
     * @param destFilelist [输出]目标文件列表
     * @return 拷贝结果
     */
    FileInfo::CopyResult copyAssignFiles(const std::vector<std::string>& srcFilelist, std::vector<FileCopyDestInfo>* destFilelist);

    /**
     * @brief 拷贝源文件列表
     * @param srcFilelist 源文件列表
     * @param totalFileSize 源文件总大小
     * @param blocks 拷贝块大小
     * @param destFilelist [输出]目标文件列表
     * @return 拷贝结果
     */
    FileInfo::CopyResult copySrcFileList(const std::vector<std::string>& srcFilelist, size_t totalFileSize,
                                         const std::vector<FileInfo::CopyBlock>& blocks, std::vector<FileCopyDestInfo>* destFilelist);

    /**
     * @brief 检测目标文件是否存在同名
     * @param destFile 目标文件名
     * @return 若存在同名目标文件, 则加上后缀并返回, 若不存在则返回原有目标文件名
     */
    std::string checkDestFile(const std::string& destFile);

private:
    utility::PathInfo m_srcPathInfo; /* 源目录 */
    utility::PathInfo m_destPathInfo; /* 目标目录 */
    bool m_clearDestPath; /* 拷贝前是否清空目标目录 */
    bool m_coverDestFile; /* 当目标目录已有同名文件时是否覆盖 */
    std::string m_tmpSuffix; /* 临时后缀名 */
    std::vector<FileInfo::CopyBlock> m_blocks; /* 文件块列表 */
    unsigned int m_retryTime; /* 重试时间 */
    FileCopyDestNameAlterFunc m_destNameAlterFunc; /* 目标文件名变更函数 */
    FileCopyFilterFunc m_filterFunc; /* 过滤函数 */
    FileCopyStopFunc m_stopFunc; /* 停止函数 */
    FileCopyBeginCallback m_beginCallback; /* 开始回调 */
    FileCopyTotalProgressCallback m_totalProgressCallback; /* 总进度回调 */
    FileCopySingleProgressCallback m_singleProgressCallback; /* 单个进度回调 */
    FileCopySingleOkCallback m_singleOkCallback; /* 单个成功回调 */
    std::string m_failSrcFile; /* 失败时的源文件 */
    FileCopyDestInfo m_failDestFile; /* 失败时的目标文件 */
    int m_errCode; /* 失败时的错误码 */
};
} // namespace utility
