#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief SM4上下文
     */
    typedef struct
    {
        int mode; /* 加/解密 */
        unsigned long sk[32]; /* SM4 subkeys */
    } sm4_context_t;

    /**
     * @brief 设置密钥(加密模式)
     * @param ctx 上下文
     * @param key 密钥(16字节)
     */
    void sm4SetKeyEnc(sm4_context_t* ctx, const unsigned char key[16]);

    /**
     * @brief 设置密钥(解密模式)
     * @param ctx 上下文
     * @param key 密钥(16字节)
     */
    void sm4SetKeyDec(sm4_context_t* ctx, const unsigned char key[16]);

    /**
     * @brief SM4-ECB加/解密(自动补位)
     * @param ctx SM4上下文
     * @param in 输入数据
     * @param inLength 输入数据长度
     * @param out [输出]加解密后的数据(需要外部调用free释放内存)
     * @return 输出数据长度(注意: 如果是解密且输入数据在加密时有补位, 则返回的数据长度需要减去补位长度)
     */
    int sm4CryptEcb(sm4_context_t* ctx, const unsigned char* in, int inLength, unsigned char** out);

    /**
     * @brief SM4-CBC加/解密(自动补位)
     * @param ctx SM4上下文
     * @param ivec 初始化向量
     * @param in 输入数据
     * @param inLength 输入数据长度
     * @param out [输出]加解密后的数据(需要外部调用free释放内存)
     * @return 输出数据长度(注意: 如果是解密且输入数据在加密时有补位, 则返回的数据长度需要减去补位长度)
     */
    int sm4CryptCbc(sm4_context_t* ctx, const unsigned char ivec[16], const unsigned char* in, int inLength, unsigned char** out);
#ifdef __cplusplus
}
#endif
