#pragma once
#include <string>

namespace nsocket
{
class Sha1 final
{
public:
    /**
     * @brief 初始化
     */
    void init();

    /** 
     * @brief 对字节流进行sha1加密(支持大数据分段)
     * @param input 原始字节流
     * @param inputLen 输入的字节流长度
     */
    void update(const unsigned char* input, unsigned int inputLen);

    /** 
     * @brief sha1加密结束
     * @param digest [输出]20位哈希值
     * @param convertToStr 是否转为字符串, true-转为字符串并返回, false-返回空字符串
     * @return 由digest哈希值转换后的十六进制字符串(40位小写)
     */
    std::string final(unsigned char digest[20], bool convertToStr);

    /** 
     * @brief sha1加密, 内部集成了init, update, final三个接口的调用(一般直接调用该接口即可)
     * @param input 原始字节流
     * @param inputLen 输入的字节流长度
     * @param digest [输出]20位哈希值
     */
    static void sign(const unsigned char* input, int inputLen, unsigned char digest[20]);

    /** 
     * @brief sha1加密, 内部集成了init, update, final三个接口的调用(一般直接调用该接口即可)
     * @param input 原始字节流
     * @param inputLen 输入的字节流长度
     * @return sha1字符串(40位小写)
     */
    static std::string sign(const unsigned char* input, int inputLen);

private:
    unsigned int m_state[5];
    unsigned int m_count[2];
    unsigned char m_buffer[64];
};
} // namespace nsocket
