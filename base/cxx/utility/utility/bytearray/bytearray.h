#pragma once
#include <string>
#include <vector>

namespace utility
{
/**
 * @brief 基础类型定义(保证平台的通用性, 尽量不要使用long)
 */
typedef bool BOOL; /* 布尔型(1个字节) */
typedef char CHAR; /* 字符型(1个字节) */
typedef unsigned char UCHAR; /* 无符号字符型(1个字节) */
typedef short INT16; /* 16位整型(2个字节) */
typedef unsigned short UINT16; /* 16位无符号整型(2个字节) */
typedef int INT32; /* 32位整型(4个字节) */
typedef unsigned int UINT32; /* 32位无符号整型(4个字节) */
typedef long long INT64; /* 64位整型(8个字节) */
typedef unsigned long long UINT64; /* 64位无符号整型(8个字节) */
typedef float FLOAT; /* 32位浮点型(4个字节) */
typedef double DOUBLE; /* 64位浮点型(8个字节) */

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
    static BOOL isBigEndium();

    /**
     * @brief 大小端转换(短整型值转为短整型值)
     * @param n 短整型值
     * @return 短整型值
     */
    static INT16 swab16(INT16 n);

    /**
     * @brief 大小端转换(字节流转为短整型值)
     * @param p 字节流(2个字节)
     * @return 短整型值
     */
    static INT16 swab16(const UCHAR p[2]);

    /**
     * @brief 大小端转换(整型值转为整型值)
     * @return 整型值
     */
    static INT32 swab32(INT32 n);

    /**
     * @brief 大小端转换(字节流转为整型值)
     * @param p 字节流(4个字节)
     * @return 整型值
     */
    static INT32 swab32(const UCHAR p[4]);

    /**
     * @brief 大小端转换(长整型值转为长整型值)
     * @return 长整型值
     */
    static INT64 swab64(INT64 n);

    /**
     * @brief 大小端转换(字节流转为长整型值)
     * @param p 字节流(8个字节)
     * @return 长整型值
     */
    static INT64 swab64(const UCHAR p[8]);

    /**
     * @brief 获取数据类型所占字节数
     * @tparam T 数据类型
     * @return 占用字节数
     */
    template<typename T>
    static UINT32 bcount(T v)
    {
        return sizeof(T);
    }

    /**
     * @brief 获取字符串所占字节数
     * @param value 字符串值
     * @return 占用字节数
     */
    static UINT32 bcount(const std::string& value);

    /**
     * @brief 获取字节流所占字节数
     * @param value 字节流
     * @param len 长度
     * @return 占用字节数=头(4个字节)+长度
     */
    static UINT32 bcount(const UCHAR* value, UINT32 len);

    /**
     * @brief 获取字节流所占字节数
     * @param value 字节流
     * @return 占用字节数=头(4个字节)+长度
     */
    static UINT32 bcount(const std::vector<UCHAR>& value);

public:
    /**
     * @brief 构造函数
     * @param totalSize 字节流缓冲区总长度(字节, 选填), <=0时不预先分配内存
     *                  建议: 字节流长度上限(单个网络消息最大长度不超过1M, 超过极易导致物理服务器收发队列阻塞)
     */
    ByteArray(UINT32 totalSize = 0);

    virtual ~ByteArray();

    /**
     * @brief 分配字节流缓冲区内存
     * @param totalSize 字节流缓冲区总长度(字节)
     *                  建议: 字节流长度上限(单个网络消息最大长度不超过1M, 超过极易导致物理服务器收发队列阻塞)
     * @return true-成功, false-失败
     */
    BOOL allocate(UINT32 totalSize);

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
    UINT32 getTotalSize();

    /**
     * @brief 获取缓冲区当前长度
     * @return 当前长度
     */
    UINT32 getCurrentSize();

    /**
     * @brief 获取缓冲区剩余可写长度
     * @return 剩余可写长度
     */
    UINT32 getSpaceSize();

    /**
     * @brief 获取字节流内容
     * @return 字节流内容
     */
    const UCHAR* getBuffer();

    /**
     * @brief 设置缓冲区内容
     * @param buffer 缓冲区内容
     * @param len 内容长度
     * @return true-成功, false-失败
     */
    BOOL setBuffer(const UCHAR* buffer, UINT32 len);

    /**
     * @brief 从缓冲区读取布尔型
     * @return 布尔值
     */
    BOOL readBool();

    /**
     * @brief 向缓冲区写入布尔型
     * @param value 布尔值
     * @return true-成功, false-失败
     */
    BOOL writeBool(BOOL value);

    /**
     * @brief 从缓冲区读取字符型
     * @return 字符值
     */
    CHAR readChar();

    /**
     * @brief 向缓冲区写入字符型
     * @param value 字符值
     * @return true-成功, false-失败
     */
    BOOL writeChar(CHAR value);

    /**
     * @brief 从缓冲区读取无符号字符型
     * @return 无符号字符值
     */
    UCHAR readUchar();

    /**
     * @brief 向缓冲区写入无符号字符型
     * @param value 无符号字符值
     * @return true-成功, false-失败
     */
    BOOL writeUchar(UCHAR value);

    /**
     * @brief 从缓冲区读取16位整型
     * @return 16位整型值
     */
    INT16 readInt16();

    /**
     * @brief 向缓冲区写入16位整型
     * @param value 16位整型值
     * @return true-成功, false-失败
     */
    BOOL writeInt16(INT16 value);

    /**
     * @brief 从缓冲区读取16位无符号整型
     * @return 16位无符号整型值
     */
    UINT16 readUint16();

    /**
     * @brief 向缓冲区写入16位无符号整型
     * @param value 16位无符号整型值
     * @return true-成功, false-失败
     */
    BOOL writeUint16(UINT16 value);

    /**
     * @brief 从缓冲区读取32位整型
     * @return 32位整型值
     */
    INT32 readInt32();

    /**
     * @brief 向缓冲区写入32位整型
     * @param value 32位整型值
     * @return true-成功, false-失败
     */
    BOOL writeInt32(INT32 value);

    /**
     * @brief 从缓冲区读取32位无符号整型
     * @return 32位无符号整型值
     */
    UINT32 readUint32();

    /**
     * @brief 向缓冲区写入32位无符号整型
     * @param value 32位无符号整型值
     * @return true-成功, false-失败
     */
    BOOL writeUint32(UINT32 value);

    /**
     * @brief 从缓冲区读取64位整型
     * @return 64位整型值
     */
    INT64 readInt64();

    /**
     * @brief 向缓冲区写入64位整型
     * @param value 64位整型值
     * @return true-成功, false-失败
     */
    BOOL writeInt64(INT64 value);

    /**
     * @brief 从缓冲区读取64位无符号整型
     * @return 64位无符号整型值
     */
    UINT64 readUint64();

    /**
     * @brief 向缓冲区写入64位无符号整型
     * @param value 64位无符号整型值
     * @return true-成功, false-失败
     */
    BOOL writeUint64(UINT64 value);

    /**
     * @brief 从缓冲区读取单精度浮点型
     * @return 单精度浮点型值
     */
    FLOAT readFloat();

    /**
     * @brief 向缓冲区写入单精度浮点型
     * @param value 单精度浮点型值
     * @return true-成功, false-失败
     */
    BOOL writeFloat(FLOAT value);

    /**
     * @brief 从缓冲区读取双精度浮点型
     * @return 双精度浮点型值
     */
    DOUBLE readDouble();

    /**
     * @brief 向缓冲区写入双精度浮点型
     * @param value 双精度浮点型值
     * @return true-成功, false-失败
     */
    BOOL writeDouble(DOUBLE value);

    /**
     * @brief 从缓冲区读取字节流
     * @param len [输出]字节流长度
     * @return 字节流(需要外部调用free释放内存)
     */
    UCHAR* readBytes(UINT32& len);

    /**
     * @brief 向缓冲区写入字节流
     * @param value 字节流
     * @param len 字节流长度
     * @return true-成功, false-失败
     */
    BOOL writeBytes(const UCHAR* value, UINT32 len);

    /**
     * @brief 从缓冲区读取字节流
     * @param bytes [输出]字节流
     */
    void readBytes(std::vector<UCHAR>& bytes);

    /**
     * @brief 向缓冲区写入字节流
     * @param value 字节流
     * @return true-成功, false-失败
     */
    BOOL writeBytes(const std::vector<UCHAR>& value);

    /**
     * @brief 从缓冲区读取字符串
     * @param len [输出]字符串长度
     * @return 字符串(需要外部调用free释放内存)
     */
    CHAR* readString(UINT32& len);

    /**
     * @brief 向缓冲区写入字符串
     * @param value 字符串
     * @param len 字符串长度
     * @return true-成功, false-失败
     */
    BOOL writeString(const CHAR* value, UINT32 len);

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
    BOOL writeString(const std::string& value);

private:
    BOOL copy(const UCHAR* buf, UINT32 n);

    UCHAR* read(UINT32 n);

    UCHAR* write(UINT32 n);

private:
    UCHAR* m_buffer; /* 字节流缓冲区 */
    UINT32 m_totalSize; /* 缓冲区总长度 */
    UINT32 m_readIndex; /* 读取位置 */
    UINT32 m_writeIndex; /* 写入位置 */
};
} // namespace utility
