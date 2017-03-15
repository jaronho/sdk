/**********************************************************************
* Author:	jaron.ho
* Date:		2014-03-21
* Brief:	packet crypto
**********************************************************************/
#include <memory.h>
#include <stdlib.h>
#include <time.h>
//----------------------------------------------------------------------
// 随机整数
unsigned int _PacketCrypto_RandInt() {
	static int initSeed = 0;
	if (0 == initSeed) {
		initSeed = 1;
		srand((unsigned int)time(NULL));
	}
	return (rand() << 16) | rand();
}
//----------------------------------------------------------------------
// 取加密异或值
unsigned int _PacketCrypto_ProduceXor(unsigned int dataLen, unsigned int randVal, unsigned long xorKey) {
	return (unsigned int)(dataLen ^ xorKey ^ randVal);
}
//----------------------------------------------------------------------
// 取加密移位值
unsigned int _PacketCrypto_ProduceShift(unsigned int dataLen, unsigned int randVal, unsigned long shiftKey) {
	return (dataLen ^ shiftKey ^ randVal) % 32;
}
//----------------------------------------------------------------------
// 异或
unsigned int _PacketCrypto_Xor(unsigned int data, unsigned int v) {
	data ^= v;
	return data;
}
//----------------------------------------------------------------------
// 左移
unsigned int _PacketCrypto_LeftShift(unsigned int data, int n) {
	unsigned int da = data;
	da &= 0xFFFFFFFF << (32 - n);
	da >>= 32 - n;
	data <<= n;
	data |= da;
	return data;
}
//----------------------------------------------------------------------
// 右移
unsigned int _PacketCrypto_RightShift(unsigned int data, int n) {
	unsigned int da = data;
	da &= 0xFFFFFFFF >> (32 - n);
	da <<= 32 - n;
	data >>= n;
	data |= da;
	return data;
}
//----------------------------------------------------------------------
// 4字节对齐
char* _PacketCrypto_EncodeAlignWith4Byte(char* data, int dataLen) {
	// 算出要补几个字节才能对齐(多用了一个字节用来存补的字节数)
	int k = 4 - (dataLen + 1) % 4;
	if (4 == k) {
		k = 0;
	}
	data -= (k + 1);
	*data = (char)k;
	return data;
}
//----------------------------------------------------------------------
// 反对齐
char* _PacketCrypto_DecodeAlignWith4Byte(char* data) {
	data += (int)(*data + 1);
	return data;
}
//----------------------------------------------------------------------
// 加密
char* _PacketCrypto_EncodeData(char* data, char* dataEnd, unsigned long xorKey, unsigned long shiftKey, unsigned char encodeCount) {
	unsigned int dataLen = 0, nRand = 0, keyXor = 0, keyShift = 0;
	unsigned int* pInt = NULL;
	// 1.步进
	dataLen = (unsigned int)(dataEnd - data);
	--data;
	++dataLen;
	*data = encodeCount;
	// 2.对齐
	data = _PacketCrypto_EncodeAlignWith4Byte(data, dataLen);
	dataLen = (unsigned int)(dataEnd - data);	// 长度发生改变了
	// 3.加密
	nRand = _PacketCrypto_RandInt();
	keyXor = _PacketCrypto_ProduceXor(dataLen, nRand, xorKey);
	keyShift = _PacketCrypto_ProduceShift(dataLen, nRand, shiftKey);
	for (pInt = (unsigned int*)data; pInt < (unsigned int*)dataEnd; ++pInt) {
		*pInt = _PacketCrypto_Xor(*pInt, keyXor);
		*pInt = _PacketCrypto_LeftShift(*pInt, (int)keyShift);
	}
	// 4.写入随机值
	data -= sizeof(unsigned int);
	*((unsigned int*)data) = nRand;
	return data;
}
//----------------------------------------------------------------------
// 解密
char* _PacketCrypto_DecodeData(char* data, char* dataEnd, unsigned long xorKey, unsigned long shiftKey, unsigned char* encodeCount) {
	unsigned int dataLen = 0, nRand = 0, keyXor = 0, keyShift = 0;
	unsigned int* pInt = NULL;
	// 1.读取随机值
	nRand = *((unsigned int*)data);
	data += sizeof(unsigned int);
	dataLen = (unsigned int)(dataEnd - data);
	// 2.解密
	keyXor = _PacketCrypto_ProduceXor(dataLen, nRand, xorKey);
	keyShift = _PacketCrypto_ProduceShift(dataLen, nRand, shiftKey);
	for (pInt = (unsigned int*)data; pInt < (unsigned int*)dataEnd; ++pInt) {
		*pInt = _PacketCrypto_RightShift(*pInt, (int)keyShift);
		*pInt = _PacketCrypto_Xor(*pInt, keyXor);
	}
	// 3.反对齐
	data = _PacketCrypto_DecodeAlignWith4Byte(data);
	// 4.步进
	if (encodeCount) {
		*encodeCount = *data;
	}
	++data;
	return data;
}
//----------------------------------------------------------------------
// 计算加密后二进制数据长度(byte)
unsigned int _PacketCrypto_CalcEncodeBinarySize(unsigned int dataLen) {
	unsigned int k = 0;
	// 1.步进(1字节)
	++dataLen;
	// 2.对齐(1-4字节)
	k = 4 - (dataLen + 1) % 4;
	if (4 == k) {
		k = 0;
	}
	dataLen += (k + 1);
	// 3.随机值(4字节)
	dataLen += 4;
	return dataLen;
}
//----------------------------------------------------------------------
/*
* 加密
* data-要加密的数据;dataLen-数据长度;destLen-返回加密后的数据长度;xorKey-加密时异或的key;shiftKey-加密时位移的key;encodeCount-加密次数,存放在加密数据的第一位
* 返回加密后的数据,记得要free释放内存
*/
char* PacketCryptoEncode(const char* data, unsigned int dataLen, unsigned int* destLen, unsigned long xorKey /*= 0*/, unsigned long shiftKey /*= 0*/, unsigned char encodeCount /*= 0*/) {
	char *dest = NULL, *p = NULL, *pRet = NULL;
	if (NULL == data || 0 == dataLen) {
		return NULL;
	}
	*destLen = _PacketCrypto_CalcEncodeBinarySize(dataLen);
	dest = (char*)malloc((*destLen)*sizeof(char));
	memset(dest, 0, *destLen);
	p = dest + *destLen - dataLen;
	memcpy(p, data, dataLen);
	pRet = _PacketCrypto_EncodeData(p, p + dataLen, xorKey, shiftKey, encodeCount);
	if (pRet != dest) {
		free(dest);
		dest = NULL;
	}
	return dest;
}
//----------------------------------------------------------------------
/*
* 解密
* data-要解密的数据;dataLen-数据长度;destLen-返回解密后的数据长度;xorKey-解密时异或的key;shiftKey-解密时位移的key;encodeCount-加密次数,存放在解密数据的第一位
* 返回解密后的数据,记得要free释放内存
*/
char* PacketCryptoDecode(char* data, unsigned int dataLen, unsigned int* destLen, unsigned long xorKey /*= 0*/, unsigned long shiftKey /*= 0*/, unsigned char* encodeCount /*= NULL*/) {
	char *dest = NULL, *p = NULL;
	if (NULL == data || 0 == dataLen) {
		return NULL;
	}
	p = _PacketCrypto_DecodeData(data, data + dataLen, xorKey, shiftKey, encodeCount);
	*destLen = dataLen - (unsigned int)(p - data);
	dest = (char*)malloc((*destLen + 1)*sizeof(char));
	memset(dest, 0, *destLen + 1);
	memcpy(dest, p, *destLen);
	return dest;
}
//----------------------------------------------------------------------