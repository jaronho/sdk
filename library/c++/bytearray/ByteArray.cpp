/**********************************************************************
* Author：jaron.ho
* Date：2009-9-23 19:00
* Brief：字节流类(网络协议序列化)
**********************************************************************/
#include "ByteArray.h"

const static int MAX_MSG_SIZE = 1024 * 1024;		// 单个网络消息最大长度(超过极易导致物理服务器收发队列阻塞)
//----------------------------------------------------------------------
ByteArray::ByteArray() {
	m_pContent = new char[MAX_MSG_SIZE];
	memset(m_pContent, 0, MAX_MSG_SIZE);
	m_nTotalSize = MAX_MSG_SIZE;
	m_nRdptr = 0;
	m_nWrPtr = 0;
}
//----------------------------------------------------------------------
ByteArray::~ByteArray() {
    delete[] m_pContent;
	m_pContent = NULL;
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
	printf("==================== ByteArray: max=%d, length=%d, space=%d\n", m_nTotalSize, strlen(m_pContent), space());
	printf("===== s:%s\n", m_pContent);		// 字符串
	printf("===== o:%o\n", m_pContent);		// 无符号8进制
	printf("===== d:%d\n", m_pContent);		// 有符号10进制
	printf("===== u:%u\n", m_pContent);		// 无符号10进制
	printf("===== x:%x\n", m_pContent);		// 无符号16进制
	printf("===== p:%p\n", m_pContent);		// 以16进制形式输出指针
	printf("========================================\n");
}
//----------------------------------------------------------------------
void ByteArray::reuse(void) {
	memset(m_pContent, 0, MAX_MSG_SIZE);
	m_nTotalSize = MAX_MSG_SIZE;
	m_nRdptr = 0;
	m_nWrPtr = 0;
}
//----------------------------------------------------------------------
int ByteArray::getCurLength(void) {
	return abs(m_nWrPtr - m_nRdptr);
}
//----------------------------------------------------------------------
const char* ByteArray::getContent(void) {
	return m_pContent;
}
//----------------------------------------------------------------------
bool ByteArray::setContent(const char* content, int len) {
	if (len > m_nTotalSize) {
		return false;
	}
	memcpy(m_pContent, content, len);
	m_nWrPtr = len;
	return true;
}
//----------------------------------------------------------------------
int ByteArray::space() {
	return m_nTotalSize - m_nWrPtr;
}
//----------------------------------------------------------------------
bool ByteArray::copy(const char* buf, int n) {
	if (space() < n) {
		return false;
	}
	memcpy(wr_ptr(), buf, n);
	wr_ptr(n);
	return true;
}
//----------------------------------------------------------------------
short ByteArray::read_int16(void) {
	short w = *((short*)rd_ptr());
	rd_ptr(sizeof(short));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_int16(short value) {
	short* pV = (short*)wr_ptr();
    *pV = value;
	return wr_ptr(sizeof(short));
}
//----------------------------------------------------------------------
unsigned short ByteArray::read_uint16(void) {
	unsigned short w = *((unsigned short*)rd_ptr());
	rd_ptr(sizeof(unsigned short));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_uint16(unsigned short value) {
	unsigned short* pV = (unsigned short*)wr_ptr();
    *pV = value;
	return wr_ptr(sizeof(unsigned short));
}
//----------------------------------------------------------------------
int ByteArray::read_int(void) {
	int w = *((int*)rd_ptr());
	rd_ptr(sizeof(int));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_int(int value) {
	int* pV = (int*)wr_ptr();
    *pV = value;
	return wr_ptr(sizeof(int));
}
//----------------------------------------------------------------------
unsigned int ByteArray::read_uint(void) {
	unsigned int w = *((unsigned int*)rd_ptr());
	rd_ptr(sizeof(unsigned int));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_uint(unsigned int value) {
	unsigned int *pV = (unsigned int*)wr_ptr();
    *pV = value;
	return wr_ptr(sizeof(unsigned int));
}
//----------------------------------------------------------------------
long ByteArray::read_long(void) {
	long w = *((long*)rd_ptr());
	rd_ptr(sizeof(long));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_long(long value) {
	long* pV = (long*)wr_ptr();
    *pV = value;
	return wr_ptr(sizeof(long));
}
//----------------------------------------------------------------------
unsigned long ByteArray::read_ulong(void) {
	unsigned long w = *((unsigned long*)rd_ptr());
	rd_ptr(sizeof(unsigned long));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_ulong(unsigned long value) {
	unsigned long* pV = (unsigned long*)wr_ptr();
    *pV = value;
	return wr_ptr(sizeof(unsigned long));
}
//----------------------------------------------------------------------
long long ByteArray::read_int64(void) {
	long long w = *((long long*)rd_ptr());
	rd_ptr(sizeof(long long));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_int64(long long value) {
	long long* pV = (long long*)wr_ptr();
    *pV = value;
	return wr_ptr(sizeof(long long));
}
//----------------------------------------------------------------------
unsigned long long ByteArray::read_uint64(void) {
	unsigned long long w = *((unsigned long long*)rd_ptr());
	rd_ptr(sizeof(unsigned long long));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_uint64(unsigned long long value) {
	unsigned long long* pV = (unsigned long long*)wr_ptr();
    *pV = value;
	return wr_ptr(sizeof(unsigned long long));
}
//----------------------------------------------------------------------
float ByteArray::read_float(void) {
	float w = *((float*)rd_ptr());
	rd_ptr(sizeof(float));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_float(float value) {
	float* pV = (float*)wr_ptr();
    *pV = value;
	return wr_ptr(sizeof(float));
}
//----------------------------------------------------------------------
double ByteArray::read_double(void) {
	double w = *((double*)rd_ptr());
	rd_ptr(sizeof(double));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_double(double value) {
	double* pV = (double*)wr_ptr();
    *pV = value;
	return wr_ptr(sizeof(double));
}
//----------------------------------------------------------------------
bool ByteArray::read_bool(void) {
	bool w = *((bool*)rd_ptr());
	rd_ptr(sizeof(bool));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_bool(bool value) {
	bool* pV = (bool*)wr_ptr();
    *pV = value;
	return wr_ptr(sizeof(bool));
}
//----------------------------------------------------------------------
char ByteArray::read_char(void) {
	char w = *((char*)rd_ptr());
	rd_ptr(sizeof(char));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_char(char value) {
	char* pV = (char*)wr_ptr();
    *pV = value;
	return wr_ptr(sizeof(char));
}
//----------------------------------------------------------------------
unsigned char ByteArray::read_uchar(void) {
	unsigned char w = *((unsigned char*)rd_ptr());
	rd_ptr(sizeof(unsigned char));
	return w;
}
//----------------------------------------------------------------------
bool ByteArray::write_uchar(unsigned char value) {
	unsigned char* pV = (unsigned char*)wr_ptr();
    *pV = value;
	return wr_ptr(sizeof(unsigned char));
}
//----------------------------------------------------------------------
unsigned char* ByteArray::read_string(unsigned char* info, unsigned int len) {
	unsigned char* result = (unsigned char*)memcpy(info, rd_ptr(), len);
	rd_ptr(len);
	return result;
}
//----------------------------------------------------------------------
bool ByteArray::write_string(const char* str) {
	return write_string(std::string(str));
}
//----------------------------------------------------------------------
std::string ByteArray::read_string(void) {
	unsigned int len = read_uint();
	std::string strValue(rd_ptr(), len);
	rd_ptr(len);
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
char* ByteArray::rd_ptr(void) {
	return m_pContent + m_nRdptr;
}
//----------------------------------------------------------------------
char* ByteArray::wr_ptr(void) {
	return m_pContent + m_nWrPtr;
}
//----------------------------------------------------------------------
bool ByteArray::rd_ptr(int n) {
	if (m_nRdptr + n > m_nTotalSize) {
		return false;
	}
	m_nRdptr += n;
	return true;
}
//----------------------------------------------------------------------
bool ByteArray::wr_ptr(int n) {
	if (m_nWrPtr + n > m_nTotalSize) {
		return false;
	}
	m_nWrPtr += n;
	return true;
}
//----------------------------------------------------------------------