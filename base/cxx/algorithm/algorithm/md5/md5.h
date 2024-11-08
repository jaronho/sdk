#pragma once
#include <stdio.h>

#ifdef __cplusplus
namespace algorithm
{
extern "C"
{
#endif
    /**
     * @brief md5上下文
     */
    typedef struct
    {
        unsigned int state[4]; /* state (ABCD) */
        unsigned int count[2]; /* number of bits, modulo 2^64 (lsb first) */
        unsigned char buffer[64]; /* input buffer */
    } md5_context_t;

    /** 
     * @brief md5上下文初始化
     * @param context 上下文
     */
    void md5Init(md5_context_t* context);

    /** 
     * @brief 对字节流进行md5加密(支持大数据分段)
     * @param context 上下文
     * @param input 原始字节流
     * @param inputLen 输入的字节流长度
     */
    void md5Update(md5_context_t* context, const unsigned char* input, unsigned int inputLen);

    /** 
     * @brief md5加密结束
     * @param context 上下文
     * @param digest [输出]16位哈希值
     * @param convertToStr 是否转为字符串, >0则转为字符串并返回, 否则返回空指针
     * @return 由digest哈希值转换后的十六进制字符串(32位小写)(需要外部调用free释放内存)
     */
    char* md5Fini(md5_context_t* context, unsigned char digest[16], int convertToStr);

    /** 
     * @brief md5加密, 内部集成了md5Init, md5Update, md5Fini三个接口的调用(一般直接调用该接口即可)
     * @param input 原始字节流
     * @param inputLen 输入的字节流长度
     * @param digest [输出]16位哈希值
     */
    void md5Sign(const unsigned char* input, unsigned int inputLen, unsigned char digest[16]);

    /** 
     * @brief md5加密, 内部集成了md5Init, md5Update, md5Fini三个接口的调用(一般直接调用该接口即可)
     * @param input 原始字节流
     * @param inputLen 输入的字节流长度
     * @return md5字符串(32位小写)(需要外部调用free释放内存)
     */
    char* md5SignStr(const unsigned char* input, unsigned int inputLen);

    /** 
     * @brief md5加密文件, 内部集成了md5Init, md5Update, md5Fini三个接口的调用(一般直接调用该接口即可)
     * @param handle 文件句柄
     * @param blockSize 每次读取的文件块大小(字节), 最小1024字节
     * @return md5字符串(32位小写)(需要外部调用free释放内存)
     */
    char* md5SignFileHandle(FILE* handle, unsigned long long blockSize);

    /** 
     * @brief md5加密文件, 内部集成了md5Init, md5Update, md5Fini三个接口的调用(一般直接调用该接口即可)
     * @param filename 文件路径
     * @param blockSize 每次读取的文件块大小(字节), 最小1024字节
     * @return md5字符串(32位小写)(需要外部调用free释放内存)
     */
    char* md5SignFile(const char* filename, unsigned long long blockSize);
#ifdef __cplusplus
}
} // namespace algorithm
#endif
