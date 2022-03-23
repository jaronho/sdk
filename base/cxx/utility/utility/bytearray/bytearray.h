#pragma once
#include <math.h>
#include <string>
#include <vector>

namespace utility
{
/**
 * @brief  字节流(序列化/反序列化)
 */
class ByteArray
{
public:
    /**
     * @brief 判断主机是否为大端模式, 说明: 网络字节序为大端, 而主机字节序一般为小端
     * @return true-大端, false-小端
     */
    static bool isBigEndium();

    /**
     * @brief 从字节流读取短整型值
     * @param p 字节流(2个字节)
     * @param bigEndium 字节流是否大端模式
     * @return 短整型值
     */
    static int16_t read16(const unsigned char p[2], bool bigEndium = true);

    /**
     * @brief 写短整型值到字节流
     * @param p [输出]字节流(2个字节)
     * @param n 短整型值
     * @param bigEndium 是否大端模式写入
     */
    static void write16(unsigned char p[2], int16_t n, bool bigEndium = true);

    /**
     * @brief 写短整型值到字节数组
     * @param v [输出]字节数组
     * @param n 短整型值
     * @param bigEndium 是否大端模式写入
     */
    static void write16(std::vector<unsigned char>& v, int16_t n, bool bigEndium = true);

    /**
     * @brief 大小端转换(短整型值转为短整型值)
     * @param n 短整型值
     * @return 短整型值
     */
    static int16_t swab16(int16_t n);

    /**
     * @brief 大小端转换(字节流转为短整型值)
     * @param p 字节流(2个字节)
     * @return 短整型值
     */
    static int16_t swab16(const unsigned char p[2]);

    /**
     * @brief 从字节流读取整型值
     * @param p 字节流(4个字节)
     * @param bigEndium 字节流是否大端模式
     * @return 整型值
     */
    static int32_t read32(const unsigned char p[4], bool bigEndium = true);

    /**
     * @brief 写整型值到字节流
     * @param p [输出]字节流(4个字节)
     * @param n 整型值
     * @param bigEndium 是否大端模式写入
     */
    static void write32(unsigned char p[4], int32_t n, bool bigEndium = true);

    /**
     * @brief 写整型值到字节数组
     * @param v [输出]字节数组
     * @param n 整型值
     * @param bigEndium 是否大端模式写入
     */
    static void write32(std::vector<unsigned char>& v, int32_t n, bool bigEndium = true);

    /**
     * @brief 大小端转换(整型值转为整型值)
     * @return 整型值
     */
    static int32_t swab32(int32_t n);

    /**
     * @brief 大小端转换(字节流转为整型值)
     * @param p 字节流(4个字节)
     * @return 整型值
     */
    static int32_t swab32(const unsigned char p[4]);

    /**
     * @brief 从字节流读取长整型值
     * @param p 字节流(8个字节)
     * @param bigEndium 字节流是否大端模式
     * @return 长整型值
     */
    static int64_t read64(const unsigned char p[8], bool bigEndium = true);

    /**
     * @brief 写长整型值到字节流
     * @param p [输出]字节流(4个字节)
     * @param n 长整型值
     * @param bigEndium 是否大端模式写入
     */
    static void write64(unsigned char p[8], int64_t n, bool bigEndium = true);

    /**
     * @brief 写长整型值到字节数组
     * @param v [输出]字节数组
     * @param n 长整型值
     * @param bigEndium 是否大端模式写入
     */
    static void write64(std::vector<unsigned char>& v, int64_t n, bool bigEndium = true);

    /**
     * @brief 大小端转换(长整型值转为长整型值)
     * @return 长整型值
     */
    static int64_t swab64(int64_t n);

    /**
     * @brief 大小端转换(字节流转为长整型值)
     * @param p 字节流(8个字节)
     * @return 长整型值
     */
    static int64_t swab64(const unsigned char p[8]);

    /**
     * @brief 获取数据类型所占字节数
     * @tparam T 数据类型
     * @return 占用字节数
     */
    template<typename T>
    static uint32_t bcount(T v)
    {
        return sizeof(T);
    }

    /**
     * @brief 获取字符串所占字节数
     * @param value 字符串值
     * @return 占用字节数
     */
    static uint32_t bcount(const std::string& value);

    /**
     * @brief 获取字节流所占字节数
     * @param value 字节流
     * @param len 长度
     * @return 占用字节数=头(4个字节)+长度
     */
    static uint32_t bcount(const unsigned char* value, uint32_t len);

    /**
     * @brief 获取字节流所占字节数
     * @param value 字节流
     * @return 占用字节数=头(4个字节)+长度
     */
    static uint32_t bcount(const std::vector<unsigned char>& value);

public:
    /**
     * @brief 构造函数
     * @param totalSize 字节流缓冲区总长度(字节, 选填), <=0时不预先分配内存
     *                  建议: 字节流长度上限(单个网络消息最大长度不超过1M, 超过极易导致物理服务器收发队列阻塞)
     */
    ByteArray(uint32_t totalSize = 0);

    virtual ~ByteArray();

    /**
     * @brief 分配字节流缓冲区内存
     * @param totalSize 字节流缓冲区总长度(字节)
     *                  建议: 字节流长度上限(单个网络消息最大长度不超过1M, 超过极易导致物理服务器收发队列阻塞)
     * @return true-成功, false-失败
     */
    bool allocate(uint32_t totalSize);

    /**
     * @brief 打印字节流内容(十六进制), 输出到控制台
     */
    void print() const;

    /**
     * @brief 重置缓冲区
     */
    void reset();

    /**
     * @brief 获取缓冲区总长度
     * @return 总长度
     */
    uint32_t getTotalSize() const;

    /**
     * @brief 获取缓冲区当前长度
     * @return 当前长度
     */
    uint32_t getCurrentSize() const;

    /**
     * @brief 获取缓冲区剩余可写长度
     * @return 剩余可写长度
     */
    uint32_t getSpaceSize() const;

    /**
     * @brief 获取字节流内容
     * @return 字节流内容
     */
    const unsigned char* getBuffer() const;

    /**
     * @brief 设置缓冲区内容
     * @param buffer 缓冲区内容
     * @param len 内容长度
     * @return true-成功, false-失败
     */
    bool setBuffer(const unsigned char* buffer, uint32_t len);

    /**
     * @brief 从缓冲区读取布尔型
     * @return 布尔值
     */
    bool readBool();

    /**
     * @brief 向缓冲区写入布尔型
     * @param value 布尔值
     * @return true-成功, false-失败
     */
    bool writeBool(bool value);

    /**
     * @brief 从缓冲区读取字符型
     * @return 字符值
     */
    char readChar();

    /**
     * @brief 向缓冲区写入字符型
     * @param value 字符值
     * @return true-成功, false-失败
     */
    bool writeChar(char value);

    /**
     * @brief 从缓冲区读取无符号字符型
     * @return 无符号字符值
     */
    unsigned char readUchar();

    /**
     * @brief 向缓冲区写入无符号字符型
     * @param value 无符号字符值
     * @return true-成功, false-失败
     */
    bool writeUchar(unsigned char value);

    /**
     * @brief 从缓冲区读取16位整型
     * @return 16位整型值
     */
    int16_t readInt16();

    /**
     * @brief 向缓冲区写入16位整型
     * @param value 16位整型值
     * @return true-成功, false-失败
     */
    bool writeInt16(int16_t value);

    /**
     * @brief 从缓冲区读取16位无符号整型
     * @return 16位无符号整型值
     */
    uint16_t readUint16();

    /**
     * @brief 向缓冲区写入16位无符号整型
     * @param value 16位无符号整型值
     * @return true-成功, false-失败
     */
    bool writeUint16(uint16_t value);

    /**
     * @brief 从缓冲区读取32位整型
     * @return 32位整型值
     */
    int32_t readInt32();

    /**
     * @brief 向缓冲区写入32位整型
     * @param value 32位整型值
     * @return true-成功, false-失败
     */
    bool writeInt32(int32_t value);

    /**
     * @brief 从缓冲区读取32位无符号整型
     * @return 32位无符号整型值
     */
    uint32_t readUint32();

    /**
     * @brief 向缓冲区写入32位无符号整型
     * @param value 32位无符号整型值
     * @return true-成功, false-失败
     */
    bool writeUint32(uint32_t value);

    /**
     * @brief 从缓冲区读取64位整型
     * @return 64位整型值
     */
    int64_t readInt64();

    /**
     * @brief 向缓冲区写入64位整型
     * @param value 64位整型值
     * @return true-成功, false-失败
     */
    bool writeInt64(int64_t value);

    /**
     * @brief 从缓冲区读取64位无符号整型
     * @return 64位无符号整型值
     */
    uint64_t readUint64();

    /**
     * @brief 向缓冲区写入64位无符号整型
     * @param value 64位无符号整型值
     * @return true-成功, false-失败
     */
    bool writeUint64(uint64_t value);

    /**
     * @brief 从缓冲区读取单精度浮点型
     * @return 单精度浮点型值
     */
    float_t readFloat();

    /**
     * @brief 向缓冲区写入单精度浮点型
     * @param value 单精度浮点型值
     * @return true-成功, false-失败
     */
    bool writeFloat(float_t value);

    /**
     * @brief 从缓冲区读取双精度浮点型
     * @return 双精度浮点型值
     */
    double_t readDouble();

    /**
     * @brief 向缓冲区写入双精度浮点型
     * @param value 双精度浮点型值
     * @return true-成功, false-失败
     */
    bool writeDouble(double_t value);

    /**
     * @brief 从缓冲区读取字节流
     * @param len [输出]字节流长度
     * @return 字节流(需要外部调用free释放内存)
     */
    unsigned char* readBytes(uint32_t& len);

    /**
     * @brief 向缓冲区写入字节流
     * @param value 字节流
     * @param len 字节流长度
     * @return true-成功, false-失败
     */
    bool writeBytes(const unsigned char* value, uint32_t len);

    /**
     * @brief 从缓冲区读取字节流
     * @param bytes [输出]字节流
     */
    void readBytes(std::vector<unsigned char>& bytes);

    /**
     * @brief 向缓冲区写入字节流
     * @param value 字节流
     * @return true-成功, false-失败
     */
    bool writeBytes(const std::vector<unsigned char>& value);

    /**
     * @brief 从缓冲区读取字符串
     * @param len [输出]字符串长度
     * @return 字符串(需要外部调用free释放内存)
     */
    char* readString(uint32_t& len);

    /**
     * @brief 向缓冲区写入字符串
     * @param value 字符串
     * @param len 字符串长度
     * @return true-成功, false-失败
     */
    bool writeString(const char* value, uint32_t len);

    /**
     * @brief 从缓冲区读取字符串
     * @param str [输出]字符串
     */
    void readString(std::string& str);

    /**
     * @brief 向缓冲区写入字符串
     * @param value 字符串
     * @return true-成功, false-失败
     */
    bool writeString(const std::string& value);

private:
    bool copy(const unsigned char* buf, uint32_t n);

    unsigned char* read(uint32_t n);

    unsigned char* write(uint32_t n);

private:
    unsigned char* m_buffer; /* 字节流缓冲区 */
    uint32_t m_totalSize; /* 缓冲区总长度 */
    uint32_t m_readIndex; /* 读取位置 */
    uint32_t m_writeIndex; /* 写入位置 */
};
} // namespace utility
