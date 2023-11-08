#pragma once

#ifdef __cplusplus
namespace algorithm
{
extern "C"
{
#endif
    /** 
     * @brief rc4加解密
     * @param input 原始字节流(值会被修改)
     * @param length 输入的字节流长度
     * @param pszKey 密钥
     * @param keyLen 密钥长度
     * @return 加解密后的值(存储在input中)
     */
    unsigned char* rc4Crypto(unsigned char* input, unsigned long length, const unsigned char* pszKey, unsigned long keyLen);
#ifdef __cplusplus
}
} // namespace algorithm
#endif
