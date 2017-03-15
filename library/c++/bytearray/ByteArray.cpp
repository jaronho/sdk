/**********************************************************************
* Author：jaron.ho
* Date：2009-9-23
* Brief：字节流类(网络协议序列化)
**********************************************************************/
#include "ByteArray.h"

const static int MAX_MSG_SIZE = 1024 * 1024;		// 单个网络消息最大长度(超过极易导致物理服务器收发队列阻塞)
//----------------------------------------------------------------------
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
//----------------------------------------------------------------------
ByteArray::~ByteArray() {
    delete[] mContent;
	mContent = NULL;
}
//----------------------------------------------------------------------
short ByteArray::swab16(short x)  {
	return (((x&0x00ff) << 8) | ((x&0x00ff00) >> 8));
}
//----------------------------------------------------------------------
short ByteArray::swab16_array(char* pBuf) {
	unsigned char* p = (unsigned char*)pBuf;
	short ret = ((p[0] << 8) | p[1]);
	return ret;
}
//----------------------------------------------------------------------
int ByteArray::swab32(int x) {
	return (((x&0x000000ff) << 24) | ((x&0x0000ff00) << 8) | ((x&0x00ff0000) >> 8) | ((x&0xff000000) >> 24));
}
//----------------------------------------------------------------------
int ByteArray::swab32_array(char* pBuf) {
	unsigned char* p = (unsigned char*)pBuf;
	int ret = ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
	return ret;
}
//----------------------------------------------------------------------
int ByteArray::max_size(void) {
	return MAX_MSG_SIZE;
}
//----------------------------------------------------------------------
void ByteArray::print(void) {
	printf("==================== ByteArray: max=%d, length=%d, space=%d\n", mTotalSize, strlen(mContent), getSpaceLength());
	printf("===== s:%s\n", mContent);		// 字符串
	printf("===== o:%o\n", mContent);		// 无符号8进制
	printf("===== d:%d\n", mContent);		// 有符号10进制
	printf("===== u:%u\n", mContent);		// 无符号10进制
	printf("===== x:%x\n", mContent);		// 无符号16进制
	printf("===== p:%p\n", mContent);		// 以16进制形式输出指针
	printf("========================================\n");
}
//----------------------------------------------------------------------
void ByteArray::reuse(void) {
	memset(mContent, 0, mTotalSize);
	mReadIndex = 0;
	mWriteIndex = 0;
}
//----------------------------------------------------------------------
int ByteArray::getTotalLength(void) {
	return mTotalSize;
}
//----------------------------------------------------------------------
int ByteArray::getCurrentLength(void) {
	return abs(mWriteIndex - mReadIndex);
}
//----------------------------------------------------------------------
int ByteArray::getSpaceLength() {
	return mTotalSize - mWriteIndex;
}
//----------------------------------------------------------------------
const char* ByteArray::getContent(void) {
	return mContent;
}
//----------------------------------------------------------------------
bool ByteArray::setContent(const char* content, int len) {
	if (len > mTotalSize) {
		return false;
	}
	memcpy(mContent, content, len);
	mWriteIndex = len;
	return true;
}
//----------------------------------------------------------------------
short ByteArray::read_int16(void) {
	short w = *((short*)read());
	read(sizeof(short));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_int16(short value) {
	short* pV = (short*)write();
    *pV = value;
	return write(sizeof(short));
}
//----------------------------------------------------------------------
unsigned short ByteArray::read_uint16(void) {
	unsigned short w = *((unsigned short*)read());
	read(sizeof(unsigned short));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_uint16(unsigned short value) {
	unsigned short* pV = (unsigned short*)write();
    *pV = value;
	return write(sizeof(unsigned short));
}
//----------------------------------------------------------------------
int ByteArray::read_int(void) {
	int w = *((int*)read());
	read(sizeof(int));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_int(int value) {
	int* pV = (int*)write();
    *pV = value;
	return write(sizeof(int));
}
//----------------------------------------------------------------------
unsigned int ByteArray::read_uint(void) {
	unsigned int w = *((unsigned int*)read());
	read(sizeof(unsigned int));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_uint(unsigned int value) {
	unsigned int *pV = (unsigned int*)write();
    *pV = value;
	return write(sizeof(unsigned int));
}
//----------------------------------------------------------------------
long ByteArray::read_long(void) {
	long w = *((long*)read());
	read(sizeof(long));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_long(long value) {
	long* pV = (long*)write();
    *pV = value;
	return write(sizeof(long));
}
//----------------------------------------------------------------------
unsigned long ByteArray::read_ulong(void) {
	unsigned long w = *((unsigned long*)read());
	read(sizeof(unsigned long));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_ulong(unsigned long value) {
	unsigned long* pV = (unsigned long*)write();
    *pV = value;
	return write(sizeof(unsigned long));
}
//----------------------------------------------------------------------
long long ByteArray::read_int64(void) {
	long long w = *((long long*)read());
	read(sizeof(long long));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_int64(long long value) {
	long long* pV = (long long*)write();
    *pV = value;
	return write(sizeof(long long));
}
//----------------------------------------------------------------------
unsigned long long ByteArray::read_uint64(void) {
	unsigned long long w = *((unsigned long long*)read());
	read(sizeof(unsigned long long));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_uint64(unsigned long long value) {
	unsigned long long* pV = (unsigned long long*)write();
    *pV = value;
	return write(sizeof(unsigned long long));
}
//----------------------------------------------------------------------
float ByteArray::read_float(void) {
	float w = *((float*)read());
	read(sizeof(float));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_float(float value) {
	float* pV = (float*)write();
    *pV = value;
	return write(sizeof(float));
}
//----------------------------------------------------------------------
double ByteArray::read_double(void) {
	double w = *((double*)read());
	read(sizeof(double));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_double(double value) {
	double* pV = (double*)write();
    *pV = value;
	return write(sizeof(double));
}
//----------------------------------------------------------------------
bool ByteArray::read_bool(void) {
	bool w = *((bool*)read());
	read(sizeof(bool));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_bool(bool value) {
	bool* pV = (bool*)write();
    *pV = value;
	return write(sizeof(bool));
}
//----------------------------------------------------------------------
char ByteArray::read_char(void) {
	char w = *((char*)read());
	read(sizeof(char));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_char(char value) {
	char* pV = (char*)write();
    *pV = value;
	return write(sizeof(char));
}
//----------------------------------------------------------------------
unsigned char ByteArray::read_uchar(void) {
	unsigned char w = *((unsigned char*)read());
	read(sizeof(unsigned char));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_uchar(unsigned char value) {
	unsigned char* pV = (unsigned char*)write();
    *pV = value;
	return write(sizeof(unsigned char));
}
//----------------------------------------------------------------------
unsigned char* ByteArray::read_string(unsigned char* info, unsigned int len) {
	unsigned char* result = (unsigned char*)memcpy(info, read(), len);
	read(len);
	return result;
}
//----------------------------------------------------------------------
bool ByteArray::write_string(const char* str) {
	return write_string(std::string(str));
}
//----------------------------------------------------------------------
std::string ByteArray::read_string(void) {
	unsigned int len = read_uint();
	std::string strValue(read(), len);
	read(len);
	return strValue;
}
//----------------------------------------------------------------------
bool ByteArray::write_string(const std::string& str) {
	if (!write_uint((unsigned int)(str.length()))) {
		return false;
	}
	return copy(str.data(), str.length());
}
//----------------------------------------------------------------------
bool ByteArray::copy(const char* buf, int n) {
	if (getSpaceLength() < n) {
		return false;
	}
	memcpy(write(), buf, n);
	write(n);
	return true;
}
//----------------------------------------------------------------------
char* ByteArray::read(void) {
	return mContent + mReadIndex;
}
//----------------------------------------------------------------------
bool ByteArray::read(int n) {
	if (mReadIndex + n > mTotalSize) {
		return false;
	}
	mReadIndex += n;
	return true;
}
//----------------------------------------------------------------------
char* ByteArray::write(void) {
	return mContent + mWriteIndex;
}
//----------------------------------------------------------------------
bool ByteArray::write(int n) {
	if (mWriteIndex + n > mTotalSize) {
		return false;
	}
	mWriteIndex += n;
	return true;
}
//----------------------------------------------------------------------