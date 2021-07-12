#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
    /* md5上下文 */
    typedef struct
    {
        unsigned int state[4]; /* state (ABCD) */
        unsigned int count[2]; /* number of bits, modulo 2^64 (lsb first) */
        unsigned char buffer[64]; /* input buffer */
    } md5_ctx_t;

    /** 
     * @brief md5上下文初始化
     * @param context 上下文
     */
    void md5Init(md5_ctx_t* context);

    /** 
     * @brief 对字节流进行md5加密(支持大数据分段)
     * @param context 上下文
     * @param input 原始字节流
     * @param inLength 输入的字节流长度
     */
    void md5Update(md5_ctx_t* context, const unsigned char* input, unsigned int inputLen);

    /** 
     * @brief md5加密结束
     * @param context 上下文
     * @param digest [输出]16位哈希值
     * @return 由digest哈希值转换后的十六进制字符串(32位小写)
     */
    char* md5Fini(md5_ctx_t* context, unsigned char digest[16]);

    /** 
     * @brief md5加密, 内部集成了md5Init, md5Update, md5Fini三个接口的调用(一般直接调用该接口即可)
     * @param input 原始字节流
     * @param inLength 输入的字节流长度
     * @return md5字符串(32位小写)
     */
    char* md5Sign(const unsigned char* input, unsigned int inputLen);
#ifdef __cplusplus
}
#endif
