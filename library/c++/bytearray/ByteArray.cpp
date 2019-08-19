/**********************************************************************
 * Author:  jaron.ho
 * Date:    2009-09-23
 * Brief:   字节流类(网络协议序列化)
 **********************************************************************/
#include "ByteArray.h"
#include <string.h>

const static int MAX_MSG_SIZE = 1024 * 1024;		/* 单个网络消息最大长度(超过极易导致物理服务器收发队列阻塞) */
/*********************************************************************/
ByteArray::ByteArray(int size) {
	if (size <= 0 || size > MAX_MSG_SIZE) {
		size = MAX_MSG_SIZE;
	}
	mContent = new char[size];
	memset(mContent, 0, size);
	mTotalSize = size;
	mReadIndex = 0;
	mWriteIndex = 0;
}
/*********************************************************************/
ByteArray::~ByteArray(void) {
    delete[] mContent;
	mContent = NULL;
}
/*********************************************************************/
bool ByteArray::isBigEndium(void) {
    /*
        int大端存储方式: 0x00 00 00 01
        int小端存储方式: 0x01 00 00 00
        char大端: 0x00
        char小端: 0x01
    */
    int a = 1;
    if (0 == *(char*)&a) {  /* 大端 */
        return true;
    }
    return false;   /* 小端 */
}
/*********************************************************************/
short ByteArray::swab16(short x)  {
	return (((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8));
}
/*********************************************************************/
short ByteArray::swab16_array(char* pBuf) {
	unsigned char* p = (unsigned char*)pBuf;
	short ret = ((p[0] << 8) | p[1]);
	return ret;
}
/*********************************************************************/
int ByteArray::swab32(int x) {
	return (((x & 0x000000FF) << 24) | ((x & 0x0000FF00) << 8) | ((x & 0x00FF0000) >> 8) | ((x & 0xFF000000) >> 24));
}
/*********************************************************************/
int ByteArray::swab32_array(char* pBuf) {
	unsigned char* p = (unsigned char*)pBuf;
	int ret = ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
	return ret;
}
/*********************************************************************/
int ByteArray::max_size(void) {
	return MAX_MSG_SIZE;
}
/*********************************************************************/
void ByteArray::print(void) {
    printf("==================== byte array: max=%d, length=%d, space=%d\n", mTotalSize, strlen(mContent), getSpaceLength());
	printf("%s\n", mContent);		// 字符串
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
const char* ByteArray::getContent(void) {
	return mContent;
}
/*********************************************************************/
bool ByteArray::setContent(const char* content, int len) {
	if (len > mTotalSize) {
		return false;
	}
	memcpy(mContent, content, len);
	mWriteIndex = len;
	return true;
}
/*********************************************************************/
bool ByteArray::read_bool(void) {
	char* p = read(sizeof(bool));
	if (!p) {
		return false;
	}
	bool r = *((bool*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_bool(bool value) {
	char* p = write(sizeof(bool));
	if (!p) {
		return false;
	}
	bool* w = (bool*)p;
    *w = value;
	return true;
}
/*********************************************************************/
char ByteArray::read_char(void) {
	char* p = read(sizeof(char));
	if (!p) {
		return false;
	}
	char r = *((char*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_char(char value) {
	char* p = write(sizeof(char));
	if (!p) {
		return false;
	}
	char* w = (char*)p;
    *w = value;
	return true;
}
/*********************************************************************/
unsigned char ByteArray::read_uchar(void) {
	char* p = read(sizeof(unsigned char));
	if (!p) {
		return false;
	}
	unsigned char r = *((unsigned char*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_uchar(unsigned char value) {
	char* p = write(sizeof(unsigned char));
	if (!p) {
		return false;
	}
	unsigned char* w = (unsigned char*)p;
    *w = value;
	return true;
}
/*********************************************************************/
short ByteArray::read_int16(void) {
	char* p = read(sizeof(short));
	if (!p) {
		return 0;
	}
	short r = *((short*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_int16(short value) {
	char* p = write(sizeof(short));
	if (!p) {
		return false;
	}
	short* w = (short*)p;
    *w = value;
	return true;
}
/*********************************************************************/
unsigned short ByteArray::read_uint16(void) {
	char* p = read(sizeof(unsigned short));
	if (!p) {
		return 0;
	}
	unsigned short r = *((unsigned short*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_uint16(unsigned short value) {
	char* p = write(sizeof(unsigned short));
	if (!p) {
		return false;
	}
	unsigned short* w = (unsigned short*)p;
    *w = value;
	return true;
}
/*********************************************************************/
int ByteArray::read_int(void) {
	char* p = read(sizeof(int));
	if (!p) {
		return 0;
	}
	int r = *((int*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_int(int value) {
	char* p = write(sizeof(int));
	if (!p) {
		return false;
	}
	int* w = (int*)p;
    *w = value;
	return true;
}
/*********************************************************************/
unsigned int ByteArray::read_uint(void) {
	char* p = read(sizeof(unsigned int));
	if (!p) {
		return 0;
	}
	unsigned int r = *((unsigned int*)p);
	return r;
}
//----------------------------------------------------------------------
bool ByteArray::write_uint(unsigned int value) {
	char* p = write(sizeof(unsigned int));
	if (!p) {
		return false;
	}
	unsigned int* w = (unsigned int*)p;
    *w = value;
	return true;
}
/*********************************************************************/
long ByteArray::read_long(void) {
	char* p = read(sizeof(long));
	if (!p) {
		return 0;
	}
	long r = *((long*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_long(long value) {
	char* p = write(sizeof(long));
	if (!p) {
		return false;
	}
	long* w = (long*)p;
    *w = value;
	return true;
}
/*********************************************************************/
unsigned long ByteArray::read_ulong(void) {
	char* p = read(sizeof(unsigned long));
	if (!p) {
		return 0;
	}
	unsigned long r = *((unsigned long*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_ulong(unsigned long value) {
	char* p = write(sizeof(unsigned long));
	if (!p) {
		return false;
	}
	unsigned long* w = (unsigned long*)p;
    *w = value;
	return true;
}
/*********************************************************************/
long long ByteArray::read_int64(void) {
	char* p = read(sizeof(long long));
	if (!p) {
		return 0;
	}
	long long r = *((long long*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_int64(long long value) {
	char* p = write(sizeof(long long));
	if (!p) {
		return false;
	}
	long long* w = (long long*)p;
    *w = value;
	return true;
}
/*********************************************************************/
unsigned long long ByteArray::read_uint64(void) {
	char* p = read(sizeof(unsigned long long));
	if (!p) {
		return 0;
	}
	unsigned long long r = *((unsigned long long*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_uint64(unsigned long long value) {
	char* p = write(sizeof(unsigned long long));
	if (!p) {
		return false;
	}
	unsigned long long* w = (unsigned long long*)p;
    *w = value;
	return true;
}
/*********************************************************************/
float ByteArray::read_float(void) {
	char* p = read(sizeof(float));
	if (!p) {
		return 0;
	}
	float r = *((float*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_float(float value) {
	char* p = write(sizeof(float));
	if (!p) {
		return false;
	}
	float* w = (float*)p;
    *w = value;
	return true;
}
/*********************************************************************/
double ByteArray::read_double(void) {
	char* p = read(sizeof(double));
	if (!p) {
		return 0;
	}
	double r = *((double*)p);
	return r;
}
/*********************************************************************/
bool ByteArray::write_double(double value) {
	char* p = write(sizeof(double));
	if (!p) {
		return false;
	}
	double* w = (double*)p;
    *w = value;
	return true;
}
/*********************************************************************/
std::string ByteArray::read_string(void) {
	unsigned int len = read_uint();
	if (0 == len) {
		return "";
	}
	char* p = read(len);
	if (!p) {
		return "";
	}
    return std::string(p, len);
}
/*********************************************************************/
bool ByteArray::write_string(const std::string& str) {
	if (!write_uint((unsigned int)(str.length()))) {
		return false;
	}
	return copy(str.data(), str.length());
}
/*********************************************************************/
bool ByteArray::copy(const char* buf, int n) {
	if (getSpaceLength() < n) {
		return false;
	}
	char* p = write(n);
	if (!p) {
		return false;
	}
	memcpy(p, buf, n);
	return true;
}
/*********************************************************************/
char* ByteArray::read(int n) {
	if (mReadIndex + n > mTotalSize) {
		return NULL;
	}
	char* p = mContent + mReadIndex;
	mReadIndex += n;
	return p;
}
/*********************************************************************/
char* ByteArray::write(int n) {
	if (mWriteIndex + n > mTotalSize) {
		return NULL;
	}
	char* p = mContent + mWriteIndex;
	mWriteIndex += n;
	return p;
}
/*********************************************************************/
