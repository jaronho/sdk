#pragma once
#include <string>
#include <vector>

namespace utilitiy
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
     * @brief 大小端转换(短整型值转为短整型值)
     * @param n 短整型值
     * @return 短整型值
     */
    static short swab16(short n);

    /**
     * @brief 大小端转换(字节流转为短整型值)
     * @param p 字节流(2个字节)
     * @return 短整型值
     */
    static short swab16(unsigned char p[2]);

    /**
     * @brief 大小端转换(整型值转为整型值)
     * @return 整型值
     */
    static int swab32(int n);

    /**
     * @brief 大小端转换(字节流转为整型值)
     * @param p 字节流(4个字节)
     * @return 整型值
     */
    static int swab32(unsigned char p[4]);

    /**
     * @brief 大小端转换(长整型值转为长整型值)
     * @return 长整型值
     */
    static long long swab64(long long n);

    /**
     * @brief 大小端转换(字节流转为长整型值)
     * @param p 字节流(8个字节)
     * @return 长整型值
     */
    static long long swab64(unsigned char p[8]);

    /**
     * @brief 获取数据类型所占字节数
     * @tparam T 数据类型
     * @return 占用字节数
     */
    template<typename T>
    static int bcount(T v)
    {
        return sizeof(T);
    }

    /**
     * @brief 获取字符串所占字节数
     * @param value 字符串值
     * @return 占用字节数
     */
    static int bcount(const std::string& value);

    /**
     * @brief 获取字节流所占字节数
     * @param value 字节流
     * @param len 长度
     * @return 占用字节数=头(4个字节)+长度
     */
    static int bcount(const unsigned char* value, int len);

    /**
     * @brief 获取字节流所占字节数
     * @param value 字节流
     * @return 占用字节数=头(4个字节)+长度
     */
    static int bcount(const std::vector<unsigned char>& value);

public:
    /**
     * @brief 构造函数
     * @param totalSize 字节流缓冲区总长度(字节, 选填), <=0时不预先分配内存
     *                  建议: 字节流长度上限(单个网络消息最大长度不超过1M, 超过极易导致物理服务器收发队列阻塞)
     */
    ByteArray(int totalSize = 0);

    virtual ~ByteArray();

    /**
     * @brief 分配字节流缓冲区内存
     * @param totalSize 字节流缓冲区总长度(字节)
     *                  建议: 字节流长度上限(单个网络消息最大长度不超过1M, 超过极易导致物理服务器收发队列阻塞)
     * @return true-成功, false-失败
     */
    bool allocate(int totalSize);

    /**
     * @brief 打印字节流内容(十六进制), 输出到控制台
     */
    void print();

    /**
     * @brief 重置缓冲区
     */
    void reset();

    /**
     * @brief 获取缓冲区总长度
     * @return 总长度
     */
    int getTotalSize();

    /**
     * @brief 获取缓冲区当前长度
     * @return 当前长度
     */
    int getCurrentSize();

    /**
     * @brief 获取缓冲区剩余可写长度
     * @return 剩余可写长度
     */
    int getSpaceSize();

    /**
     * @brief 获取字节流内容
     * @return 字节流内容
     */
    const unsigned char* getBuffer();

    /**
     * @brief 设置缓冲区内容
     * @param buffer 缓冲区内容
     * @param len 内容长度
     * @return true-成功, false-失败
     */
    bool setBuffer(const unsigned char* buffer, int len);

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
    short readInt16();

    /**
     * @brief 向缓冲区写入16位整型
     * @param value 16位整型值
     * @return true-成功, false-失败
     */
    bool writeInt16(short value);

    /**
     * @brief 从缓冲区读取16位无符号整型
     * @return 16位无符号整型值
     */
    unsigned short readUint16();

    /**
     * @brief 向缓冲区写入16位无符号整型
     * @param value 16位无符号整型值
     * @return true-成功, false-失败
     */
    bool writeUint16(unsigned short value);

    /**
     * @brief 从缓冲区读取32位整型
     * @return 32位整型值
     */
    int readInt();

    /**
     * @brief 向缓冲区写入32位整型
     * @param value 32位整型值
     * @return true-成功, false-失败
     */
    bool writeInt(int value);

    /**
     * @brief 从缓冲区读取32位无符号整型
     * @return 32位无符号整型值
     */
    unsigned int readUint();

    /**
     * @brief 向缓冲区写入32位无符号整型
     * @param value 32位无符号整型值
     * @return true-成功, false-失败
     */
    bool writeUint(unsigned int value);

    /**
     * @brief 从缓冲区读取长整型
     * @return 长整型值
     */
    long readLong();

    /**
     * @brief 向缓冲区写入长整型
     * @param value 长整型值
     * @return true-成功, false-失败
     */
    bool writeLong(long value);

    /**
     * @brief 从缓冲区读取无符号长整型
     * @return 无符号长整型值
     */
    unsigned long readUlong();

    /**
     * @brief 向缓冲区写入无符号长整型
     * @param value 无符号长整型值
     * @return true-成功, false-失败
     */
    bool writeUlong(unsigned long value);

    /**
     * @brief 从缓冲区读取64位整型
     * @return 64位整型值
     */
    long long readInt64();

    /**
     * @brief 向缓冲区写入64位整型
     * @param value 64位整型值
     * @return true-成功, false-失败
     */
    bool writeInt64(long long value);

    /**
     * @brief 从缓冲区读取64位无符号整型
     * @return 64位无符号整型值
     */
    unsigned long long readUint64();

    /**
     * @brief 向缓冲区写入64位无符号整型
     * @param value 64位无符号整型值
     * @return true-成功, false-失败
     */
    bool writeUint64(unsigned long long value);

    /**
     * @brief 从缓冲区读取单精度浮点型
     * @return 单精度浮点型值
     */
    float readFloat();

    /**
     * @brief 向缓冲区写入单精度浮点型
     * @param value 单精度浮点型值
     * @return true-成功, false-失败
     */
    bool writeFloat(float value);

    /**
     * @brief 从缓冲区读取双精度浮点型
     * @return 双精度浮点型值
     */
    double readDouble();

    /**
     * @brief 向缓冲区写入双精度浮点型
     * @param value 双精度浮点型值
     * @return true-成功, false-失败
     */
    bool writeDouble(double value);

    /**
     * @brief 从缓冲区读取字节流
     * @param len [输出]字节流长度
     * @return 字节流(需要外部调用free释放内存)
     */
    unsigned char* readBytes(unsigned int& len);

    /**
     * @brief 向缓冲区写入字节流
     * @param value 字节流
     * @param len 字节流长度
     * @return true-成功, false-失败
     */
    bool writeBytes(const unsigned char* value, unsigned int len);

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
    char* readString(unsigned int& len);

    /**
     * @brief 向缓冲区写入字符串
     * @param value 字符串
     * @param len 字符串长度
     * @return true-成功, false-失败
     */
    bool writeString(const char* value, unsigned int len);

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
    bool copy(const unsigned char* buf, int n);

    unsigned char* read(int n);

    unsigned char* write(int n);

private:
    unsigned char* m_buffer; /* 字节流缓冲区 */
    int m_totalSize; /* 缓冲区总长度 */
    int m_readIndex; /* 读取位置 */
    int m_writeIndex; /* 写入位置 */
};
} // namespace utilitiy
