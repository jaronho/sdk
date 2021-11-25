#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
    /** 
     * @brief 对原始字节流进行base64编码
     * @param in 输入的原始字节流
     * @param inLength 输入的字节流长度
     * @param out [输出]编码后的字节流(需要外部调用free释放内存)
     * @return 输出的长度, 0-表示编码失败
     */
    unsigned int base64Encode(const unsigned char* in, unsigned int inLength, unsigned char** out);

    /** 
     * @brief 解码base64字节流
     * @param in 输入的base64字节流
     * @param inLength 输入的字节流长度
     * @param out [输出]解码后的字节流(需要外部调用free释放内存)
     * @return 输出的长度, 0-表示解码失败
     */
    unsigned int base64Decode(const unsigned char* in, unsigned int inLength, unsigned char** out);
#ifdef __cplusplus
}
#endif
