#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
    /* sha1上下文 */
    typedef struct
    {
        unsigned int state[5];
        unsigned int count[2];
        unsigned char buffer[64];
    } sha1_ctx_t;

    /** 
     * @brief sha1上下文初始化
     * @param context 上下文
     */
    void sha1Init(sha1_ctx_t* context);

    /** 
     * @brief 对字节流进行sha1加密(支持大数据分段)
     * @param context 上下文
     * @param input 原始字节流
     * @param inputLen 输入的字节流长度
     */
    void sha1Update(sha1_ctx_t* context, const unsigned char* input, unsigned int inputLen);

    /** 
     * @brief sha1加密结束
     * @param context 上下文
     * @param digest [输出]20位哈希值
     * @param convertToStr 是否转为字符串, >0则转为字符串并返回, 否则返回空指针
     * @return 由digest哈希值转换后的十六进制字符串(40位小写)(需要外部调用free释放内存)
     */
    char* sha1Final(sha1_ctx_t* context, unsigned char digest[20], int convertToStr);

    /** 
     * @brief sha1加密, 内部集成了sha1Init, sha1Update, sha1Fini三个接口的调用(一般直接调用该接口即可)
     * @param input 原始字节流
     * @param inputLen 输入的字节流长度
     * @param digest [输出]16位哈希值
     */
    void sha1Sign(const unsigned char* input, int inputLen, unsigned char digest[20]);

    /** 
     * @brief sha1加密, 内部集成了sha1Init, sha1Update, sha1Fini三个接口的调用(一般直接调用该接口即可)
     * @param input 原始字节流
     * @param inputLen 输入的字节流长度
     * @return sha1字符串(40位小写)(需要外部调用free释放内存)
     */
    char* sha1SignStr(const unsigned char* input, int inputLen);
#ifdef __cplusplus
}
#endif
