#pragma once
#include <string>

namespace utility
{
/**
 * @brief 内存映射文件
 */
class MMFile final
{
public:
    /**
     * @brief 访问模式
     */
    enum class AccessMode
    {
        read_only, /* 只读 */
        read_write, /* 读写 */
        create /* 创建 */
    };

public:
    /**
     * @brief 构造函数
     */
    MMFile() = default;
    ~MMFile();

    /**
     * @brief 打开
     * @param fullName 全路径文件名, 例如: /home/test/111.txt
     * @param mode 访问模式
     * @param blockSize 读写块大小(单位: 字节)
     * @param initialSize 初始文件大小(单位: 字节), 当mode为create时才用到
     * @return true-成功, false-失败
     */
    bool open(const std::string& fullName, const AccessMode& mode, size_t blockSize = 1024, size_t initialSize = 0);

    /**
     * @brief 重新调整文件大小
     * @param newSize 新的大小
     * @return true-成功, false-失败
     */
    bool resize(size_t newSize);

    /**
     * @brief 定位到指定位置
     * @param offset 偏移量
     * @param whence 定位方式, 值: SEEK_CUR, SEEK_END, SEEK_SET
     * @return true-成功, false-失败
     */
    bool seek(size_t offset, int whence);

    /**
     * @brief 写文件
     * @param data 数据内容
     * @param size 数据长度
     * @return 已写长度, =0-写失败, >0-写成功
     */
    size_t write(const void* data, size_t size);

    /**
     * @brief 读文件
     * @param data 缓存区
     * @param size 读取长度
     * @return 已读长度, =0-读失败, >0-读成功
     */
    size_t read(void* data, size_t size);

    /**
     * @brief 同步到磁盘
     * @return true-成功, false-失败
     */
    bool sync();

    /**
     * @brief 关闭
     */
    void close();

    /**
     * @brief 是否已打开
     * @return true-已打开, false-未打开
     */
    bool isOpen() const;

    /**
     * @brief 获取文件大小
     * @return 文件大小
     */
    size_t getFileSize() const;

    /**
     * @brief 获取块数据
     * @return 块数据
     */
    void* getBlockData() const;

    /**
     * @brief 获取当前写位置
     * @return 当前写位置
     */
    size_t getCurrentWritePositon() const;

    /**
     * @brief 获取最后出错信息
     * @return 出错信息
     */
    int getLastError() const;

private:
    /**
     * @brief 是否可写
     * @return true-可写, false-不可写
     */
    bool isWritable() const;

    /**
     * @brief 映射块
     * @param offset 偏移值
     * @param blockSize 块大小
     * @return 块数据
     */
    void* mapBlock(size_t offset, size_t blockSize);

    /**
     * @brief 取消映射
     */
    void unmapBlock();

#ifdef _WIN32
    typedef void* HANDLE;
    HANDLE m_file = 0;
    HANDLE m_mapping = 0;
#else
    int m_fd = -1;
#endif
    size_t m_fileSize = 0; /* 文件大小 */
    size_t m_blockSize = 0; /* 默认读写块大小(单位: 字节) */
    void* m_blockData = 0; /* 块数据 */
    size_t m_currentWritePositon = 0; /* 当前写位置 */
    size_t m_currentBlockOffset = 0; /* 当前映射块的起始偏移量 */
    int m_lastError = 0; /* 最后出错信息 */
};
} // namespace utility
