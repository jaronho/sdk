/**********************************************************************
* Author:	jaron.ho
* Date:		2014-03-21
* Brief:	packet crypto
**********************************************************************/
#include <memory.h>
#include <stdlib.h>
#include <time.h>
//----------------------------------------------------------------------
// �������
unsigned int _PacketCrypto_RandInt() {
	static int initSeed = 0;
	if (0 == initSeed) {
		initSeed = 1;
		srand((unsigned int)time(NULL));
	}
	return (rand() << 16) | rand();
}
//----------------------------------------------------------------------
// ȡ�������ֵ
unsigned int _PacketCrypto_ProduceXor(unsigned int dataLen, unsigned int randVal, unsigned long xorKey) {
	return (unsigned int)(dataLen ^ xorKey ^ randVal);
}
//----------------------------------------------------------------------
// ȡ������λֵ
unsigned int _PacketCrypto_ProduceShift(unsigned int dataLen, unsigned int randVal, unsigned long shiftKey) {
	return (dataLen ^ shiftKey ^ randVal) % 32;
}
//----------------------------------------------------------------------
// ���
unsigned int _PacketCrypto_Xor(unsigned int data, unsigned int v) {
	data ^= v;
	return data;
}
//----------------------------------------------------------------------
// ����
unsigned int _PacketCrypto_LeftShift(unsigned int data, int n) {
	unsigned int da = data;
	da &= 0xFFFFFFFF << (32 - n);
	da >>= 32 - n;
	data <<= n;
	data |= da;
	return data;
}
//----------------------------------------------------------------------
// ����
unsigned int _PacketCrypto_RightShift(unsigned int data, int n) {
	unsigned int da = data;
	da &= 0xFFFFFFFF >> (32 - n);
	da <<= 32 - n;
	data >>= n;
	data |= da;
	return data;
}
//----------------------------------------------------------------------
// 4�ֽڶ���
char* _PacketCrypto_EncodeAlignWith4Byte(char* data, int dataLen) {
	// ���Ҫ�������ֽڲ��ܶ���(������һ���ֽ������油���ֽ���)
	int k = 4 - (dataLen + 1) % 4;
	if (4 == k) {
		k = 0;
	}
	data -= (k + 1);
	*data = (char)k;
	return data;
}
//----------------------------------------------------------------------
// ������
char* _PacketCrypto_DecodeAlignWith4Byte(char* data) {
	data += (int)(*data + 1);
	return data;
}
//----------------------------------------------------------------------
// ����
char* _PacketCrypto_EncodeData(char* data, char* dataEnd, unsigned long xorKey, unsigned long shiftKey, unsigned char encodeCount) {
	unsigned int dataLen = 0, nRand = 0, keyXor = 0, keyShift = 0;
	unsigned int* pInt = NULL;
	// 1.����
	dataLen = (unsigned int)(dataEnd - data);
	--data;
	++dataLen;
	*data = encodeCount;
	// 2.����
	data = _PacketCrypto_EncodeAlignWith4Byte(data, dataLen);
	dataLen = (unsigned int)(dataEnd - data);	// ���ȷ����ı���
	// 3.����
	nRand = _PacketCrypto_RandInt();
	keyXor = _PacketCrypto_ProduceXor(dataLen, nRand, xorKey);
	keyShift = _PacketCrypto_ProduceShift(dataLen, nRand, shiftKey);
	for (pInt = (unsigned int*)data; pInt < (unsigned int*)dataEnd; ++pInt) {
		*pInt = _PacketCrypto_Xor(*pInt, keyXor);
		*pInt = _PacketCrypto_LeftShift(*pInt, (int)keyShift);
	}
	// 4.д�����ֵ
	data -= sizeof(unsigned int);
	*((unsigned int*)data) = nRand;
	return data;
}
//----------------------------------------------------------------------
// ����
char* _PacketCrypto_DecodeData(char* data, char* dataEnd, unsigned long xorKey, unsigned long shiftKey, unsigned char* encodeCount) {
	unsigned int dataLen = 0, nRand = 0, keyXor = 0, keyShift = 0;
	unsigned int* pInt = NULL;
	// 1.��ȡ���ֵ
	nRand = *((unsigned int*)data);
	data += sizeof(unsigned int);
	dataLen = (unsigned int)(dataEnd - data);
	// 2.����
	keyXor = _PacketCrypto_ProduceXor(dataLen, nRand, xorKey);
	keyShift = _PacketCrypto_ProduceShift(dataLen, nRand, shiftKey);
	for (pInt = (unsigned int*)data; pInt < (unsigned int*)dataEnd; ++pInt) {
		*pInt = _PacketCrypto_RightShift(*pInt, (int)keyShift);
		*pInt = _PacketCrypto_Xor(*pInt, keyXor);
	}
	// 3.������
	data = _PacketCrypto_DecodeAlignWith4Byte(data);
	// 4.����
	if (encodeCount) {
		*encodeCount = *data;
	}
	++data;
	return data;
}
//----------------------------------------------------------------------
// ������ܺ���������ݳ���(byte)
unsigned int _PacketCrypto_CalcEncodeBinarySize(unsigned int dataLen) {
	unsigned int k = 0;
	// 1.����(1�ֽ�)
	++dataLen;
	// 2.����(1-4�ֽ�)
	k = 4 - (dataLen + 1) % 4;
	if (4 == k) {
		k = 0;
	}
	dataLen += (k + 1);
	// 3.���ֵ(4�ֽ�)
	dataLen += 4;
	return dataLen;
}
//----------------------------------------------------------------------
/*
* ����
* data-Ҫ���ܵ�����;dataLen-���ݳ���;destLen-���ؼ��ܺ�����ݳ���;xorKey-����ʱ����key;shiftKey-����ʱλ�Ƶ�key;encodeCount-���ܴ���,����ڼ������ݵĵ�һλ
* ���ؼ��ܺ������,�ǵ�Ҫfree�ͷ��ڴ�
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
* ����
* data-Ҫ���ܵ�����;dataLen-���ݳ���;destLen-���ؽ��ܺ�����ݳ���;xorKey-����ʱ����key;shiftKey-����ʱλ�Ƶ�key;encodeCount-���ܴ���,����ڽ������ݵĵ�һλ
* ���ؽ��ܺ������,�ǵ�Ҫfree�ͷ��ڴ�
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