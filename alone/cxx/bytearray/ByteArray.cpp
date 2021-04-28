/**********************************************************************
 * Author:  jaron.ho
 * Date:    2009-09-23
 * Brief:   字节流类(序列化)
 **********************************************************************/
#include "ByteArray.h"
#include <assert.h>
#include <string.h>

static const int DEFAULT_SIZE = 1024 * 1024;		                           /* 默认字节流大小(单个网络消息最大长度,超过极易导致物理服务器收发队列阻塞) */
/*********************************************************************/
ByteArray::ByteArray(int size) {
	if (size <= 0) {
		size = DEFAULT_SIZE;
	}
	mContent = (unsigned char*)malloc(sizeof(unsigned char) * size);
    assert(mContent);
	memset(mContent, 0, size);
	mTotalSize = size;
	mReadIndex = 0;
	mWriteIndex = 0;
}
/*********************************************************************/
ByteArray::~ByteArray(void) {
    free(mContent);
	mContent = NULL;
}
/*********************************************************************/
bool ByteArray::isBigEndium(void) {
    /*
        int为1(大端存储方式): 00 00 00 01
        int为1(小端存储方式): 01 00 00 00
        强制转换为char(大端): 00
        强制转换为char(小端): 01
    */
    int a = 1;
    if (0 == *(char*)&a) {  /* 大端 */
        return true;
    }
    return false;   /* 小端 */
}
/*********************************************************************/
short ByteArray::swab16(short n)  {
	return (((n & 0x00FF) << 8) | ((n & 0xFF00) >> 8));
}
/*********************************************************************/
short ByteArray::swab16_array(unsigned char* p) {
	short ret = ((p[0] << 8) | p[1]);
	return ret;
}
/*********************************************************************/
int ByteArray::swab32(int n) {
	return (((n & 0x000000FF) << 24) | ((n & 0x0000FF00) << 8) | ((n & 0x00FF0000) >> 8) | ((n & 0xFF000000) >> 24));
}
/*********************************************************************/
int ByteArray::swab32_array(unsigned char* p) {
	int ret = ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
	return ret;
}
/*********************************************************************/
int ByteArray::def_size(void) {
	return DEFAULT_SIZE;
}
/*********************************************************************/
void ByteArray::print(void) {
    printf("==================== byte array: max=%d, length=%d, space=%d\n", mTotalSize, getCurrentLength(), getSpaceLength());
    for (int i = 0, len = getCurrentLength(); i < len; ++i) {
        printf("%02x ", *(mContent + i));
    }
    printf("\n");
	printf("========================================\n");
}
/*********************************************************************/
void ByteArray::reuse(void) {
	memset(mContent, 0, mTotalSize);
	mReadIndex = 0;
	mWriteIndex = 0;
}
/*********************************************************************/
int ByteArray::getTotalLength(void) {
	return mTotalSize;
}
/*********************************************************************/
int ByteArray::getCurrentLength(void) {
	return abs(mWriteIndex - mReadIndex);
}
/*********************************************************************/
int ByteArray::getSpaceLength() {
	return mTotalSize - mWriteIndex;
}
/*********************************************************************/
const unsigned char* ByteArray::getContent(void) {
	return mContent;
}
/*********************************************************************/
bool ByteArray::setContent(const unsigned char* content, int len) {
	assert(content);
	if (len > mTotalSize) {
		return false;
	}
	memcpy(mContent, content, len);
	mWriteIndex = len;
	return true;
}
/*********************************************************************/
bool ByteArray::read_bool(void) {
	unsigned char* p = read(sizeof(bool));
	if (!p) {
		return false;
	}
	bool r = *((bool*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_bool(bool value) {
	unsigned char* p = write(sizeof(bool));
	if (!p) {
		return false;
	}
	bool* w = (bool*)p;
    *w = value;
	return true;
}
/*********************************************************************/
char ByteArray::read_char(void) {
	unsigned char* p = read(sizeof(char));
	if (!p) {
		return false;
	}
	char r = *((char*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_char(char value) {
	unsigned char* p = write(sizeof(char));
	if (!p) {
		return false;
	}
	char* w = (char*)p;
    *w = value;
	return true;
}
/*********************************************************************/
unsigned char ByteArray::read_uchar(void) {
	unsigned char* p = read(sizeof(unsigned char));
	if (!p) {
		return false;
	}
	unsigned char r = *((unsigned char*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_uchar(unsigned char value) {
	unsigned char* p = write(sizeof(unsigned char));
	if (!p) {
		return false;
	}
    unsigned char* w = (unsigned char*)p;
    *w = value;
	return true;
}
/*********************************************************************/
short ByteArray::read_int16(void) {
	unsigned char* p = read(sizeof(short));
	if (!p) {
		return 0;
	}
	short r = *((short*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_int16(short value) {
	unsigned char* p = write(sizeof(short));
	if (!p) {
		return false;
	}
	short* w = (short*)p;
    *w = value;
	return true;
}
/*********************************************************************/
unsigned short ByteArray::read_uint16(void) {
	unsigned char* p = read(sizeof(unsigned short));
	if (!p) {
		return 0;
	}
	unsigned short r = *((unsigned short*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_uint16(unsigned short value) {
	unsigned char* p = write(sizeof(unsigned short));
	if (!p) {
		return false;
	}
	unsigned short* w = (unsigned short*)p;
    *w = value;
	return true;
}
/*********************************************************************/
int ByteArray::read_int(void) {
	unsigned char* p = read(sizeof(int));
	if (!p) {
		return 0;
	}
	int r = *((int*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_int(int value) {
	unsigned char* p = write(sizeof(int));
	if (!p) {
		return false;
	}
	int* w = (int*)p;
    *w = value;
	return true;
}
/*********************************************************************/
unsigned int ByteArray::read_uint(void) {
	unsigned char* p = read(sizeof(unsigned int));
	if (!p) {
		return 0;
	}
	unsigned int r = *((unsigned int*)p);
	return r;
}
//----------------------------------------------------------------------
bool ByteArray::write_uint(unsigned int value) {
	unsigned char* p = write(sizeof(unsigned int));
	if (!p) {
		return false;
	}
	unsigned int* w = (unsigned int*)p;
    *w = value;
	return true;
}
/*********************************************************************/
long ByteArray::read_long(void) {
	unsigned char* p = read(sizeof(long));
	if (!p) {
		return 0;
	}
	long r = *((long*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_long(long value) {
	unsigned char* p = write(sizeof(long));
	if (!p) {
		return false;
	}
	long* w = (long*)p;
    *w = value;
	return true;
}
/*********************************************************************/
unsigned long ByteArray::read_ulong(void) {
	unsigned char* p = read(sizeof(unsigned long));
	if (!p) {
		return 0;
	}
	unsigned long r = *((unsigned long*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_ulong(unsigned long value) {
	unsigned char* p = write(sizeof(unsigned long));
	if (!p) {
		return false;
	}
	unsigned long* w = (unsigned long*)p;
    *w = value;
	return true;
}
/*********************************************************************/
long long ByteArray::read_int64(void) {
	unsigned char* p = read(sizeof(long long));
	if (!p) {
		return 0;
	}
	long long r = *((long long*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_int64(long long value) {
	unsigned char* p = write(sizeof(long long));
	if (!p) {
		return false;
	}
	long long* w = (long long*)p;
    *w = value;
	return true;
}
/*********************************************************************/
unsigned long long ByteArray::read_uint64(void) {
	unsigned char* p = read(sizeof(unsigned long long));
	if (!p) {
		return 0;
	}
	unsigned long long r = *((unsigned long long*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_uint64(unsigned long long value) {
	unsigned char* p = write(sizeof(unsigned long long));
	if (!p) {
		return false;
	}
	unsigned long long* w = (unsigned long long*)p;
    *w = value;
	return true;
}
/*********************************************************************/
float ByteArray::read_float(void) {
	unsigned char* p = read(sizeof(float));
	if (!p) {
		return 0;
	}
	float r = *((float*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_float(float value) {
	unsigned char* p = write(sizeof(float));
	if (!p) {
		return false;
	}
	float* w = (float*)p;
    *w = value;
	return true;
}
/*********************************************************************/
double ByteArray::read_double(void) {
	unsigned char* p = read(sizeof(double));
	if (!p) {
		return 0;
	}
	double r = *((double*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_double(double value) {
	unsigned char* p = write(sizeof(double));
	if (!p) {
		return false;
	}
	double* w = (double*)p;
    *w = value;
	return true;
}
/*********************************************************************/
char* ByteArray::read_string(unsigned int* len) {
	assert(len);
    *len = read_uint();
	if (0 == *len) {
		return NULL;
	}
	unsigned char* p = read(*len);
	if (!p) {
		return NULL;
	}
    char* str = (char*)malloc(sizeof(char) * (*len) + 1);
    strcpy(str, (char*)p);
    *(str + (*len)) = '\0';
    return str;
}
/*********************************************************************/
bool ByteArray::write_string(const char* str, unsigned int len) {
	assert(str);
    if (!write_uint(len)) {
		return false;
	}
	return copy((const unsigned char*)str, len);
}
/*********************************************************************/
std::string ByteArray::read_string(void) {
	unsigned int len = read_uint();
	if (0 == len) {
		return "";
	}
	unsigned char* p = read(len);
	if (!p) {
		return "";
	}
    return std::string((char*)p, len);
}
/*********************************************************************/
bool ByteArray::write_string(const std::string& str) {
	if (!write_uint((unsigned int)(str.length()))) {
		return false;
	}
	return copy((const unsigned char*)str.data(), str.length());
}
/*********************************************************************/
bool ByteArray::copy(const unsigned char* buf, int n) {
	if (getSpaceLength() < n) {
		return false;
	}
	unsigned char* p = write(n);
	if (!p) {
		return false;
	}
	memcpy(p, buf, n);
	return true;
}
/*********************************************************************/
unsigned char* ByteArray::read(int n) {
	if (mReadIndex + n > mTotalSize) {
		return NULL;
	}
	unsigned char* p = mContent + mReadIndex;
	mReadIndex += n;
	return p;
}
/*********************************************************************/
unsigned char* ByteArray::write(int n) {
	if (mWriteIndex + n > mTotalSize) {
		return NULL;
	}
	unsigned char* p = mContent + mWriteIndex;
	mWriteIndex += n;
	return p;
}
/*********************************************************************/
