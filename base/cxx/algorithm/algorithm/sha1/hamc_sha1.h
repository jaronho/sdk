#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
    /** 
     * @brief hamc-sha1加密
     * @param data 数据
     * @param dataLen 数据长度
     * @param key 密钥
     * @param keyLen 密钥长度
     * @param digest [输出]20位哈希值
     */
    void hamcSha1(const unsigned char* data, int dataLen, const unsigned char* key, int keyLen, unsigned char digest[20]);

    /** 
     * @brief hamc-sha1加密
     * @param data 数据
     * @param dataLen 数据长度
     * @param key 密钥
     * @param keyLen 密钥长度
     * @return hamc-sha1字符串(40位小写)(需要外部调用free释放内存)
     */
    char* hamcSha1Str(const unsigned char* data, int dataLen, const unsigned char* key, int keyLen);
#ifdef __cplusplus
}
#endif
