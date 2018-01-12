/**********************************************************************
 * Author:  jaron.ho
 * Date:    2009-09-23
 * Brief:   字节流类(网络协议序列化)
 **********************************************************************/
#ifndef _BYTE_ARRAY_H_
#define _BYTE_ARRAY_H_

#include <string>

class ByteArray {
public:
	ByteArray(int size = 0);
    virtual ~ByteArray(void);

public:
    static short swab16(short x);                           /* 大小端转换(短整型数字转为短整型数字) */
    static short swab16_array(char* pBuf);                  /* 大小端转换(字节流转为短整型数字) */

    static int swab32(int x);                               /* 大小端转换(整型数字转为整型数字) */
    static int swab32_array(char* pBuf);                    /* 大小端转换(字节流转为整型数字) */

    static int max_size(void);                              /* 网络消息最大长度 */

public:
    void print(void);                                       /* 打印当前内容 */
    void reuse(void);                                       /* 清空,重新使用当前字节流 */
    int getTotalLength(void);                               /* 获取字节流总长度 */
    int getCurrentLength(void);                             /* 获取字节流当前长度 */
    int getSpaceLength(void);                               /* 获取字节流剩余可写长度 */
    const char* getContent(void);                           /* 获取字节流内容 */
    bool setContent(const char* content, int len);          /* 设置字节流内容 */
	
public:
    bool read_bool(void);                                   /* 从字节流读取布尔型 */
    bool write_bool(bool value);                            /* 向字节流写入布尔型 */
	
    char read_char(void);                                   /* 从字节流读取字符型 */
    bool write_char(char value);                            /* 向字节流写入字符型 */
	
    unsigned char read_uchar(void);                         /* 从字节流读取无符号字符整型 */
    bool write_uchar(unsigned char value);                  /* 向字节流写入无符号字符整型 */
	
    short read_int16(void);                                 /* 从字节流读取16位整型 */
    bool write_int16(short value);                          /* 向字节流写入16位整型 */
	
    unsigned short read_uint16(void);                       /* 从字节流读取16位无符号整型 */
    bool write_uint16(unsigned short value);                /* 向字节流写入16位无符号整型 */
	
    int read_int(void);                                     /* 从字节流读取32位整型 */
    bool write_int(int value);                              /* 向字节流写入32位整型 */
	
    unsigned int read_uint(void);                           /* 从字节流读取32位无符号整型 */
    bool write_uint(unsigned int value);                    /* 向字节流写入32位无符号整型 */
	
    long read_long(void);                                   /* 从字节流读取长整型 */
    bool write_long(long value);                            /* 向字节流写入长整型 */
	
    unsigned long read_ulong(void);                         /* 从字节流读取无符号长整型 */
    bool write_ulong(unsigned long value);                  /* 向字节流写入无符号长整型 */
	
    long long read_int64(void);                             /* 从字节流读取64位整型 */
    bool write_int64(long long value);                      /* 向字节流写入64位整型 */
	
    unsigned long long read_uint64(void);                   /* 从字节流读取64位无符号整型 */
    bool write_uint64(unsigned long long value);            /* 向字节流写入64位无符号整型 */
	
    float read_float(void);                                 /* 从字节流读取浮点型 */
    bool write_float(float value);                          /* 向字节流写入浮点型 */
	
    double read_double(void);                               /* 从字节流读取双精度浮点型 */
    bool write_double(double value);                        /* 向字节流写入双精度浮点型 */
	
    std::string read_string(void);                          /* 从字节流读取字符串 */
    bool write_string(const std::string& str);              /* 向字节流写入字符串 */

private:
	bool copy(const char* buf, int n);
    char* read(int n);
    char* write(int n);

private:
    char* mContent;         /* 字节流内容指针 */
    int mTotalSize;         /* 字节流允许大小 */
    int mReadIndex;         /* 读取位置 */
    int mWriteIndex;        /* 写入位置 */
};

#endif	/* _BYTE_ARRAY_H_ */
