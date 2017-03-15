/**********************************************************************
* Author:	jaron.ho
* Date:		2014-03-21
* Brief:	packet crypto
**********************************************************************/
#ifndef _PACKET_CRYPTO_H_
#define _PACKET_CRYPTO_H_

char* PacketCryptoEncode(const char* data, unsigned int dataLen, unsigned int* destLen, unsigned long xorKey = 0, unsigned long shiftKey = 0, unsigned char encodeCount = 0);
char* PacketCryptoDecode(char* data, unsigned int dataLen, unsigned int* destLen, unsigned long xorKey = 0, unsigned long shiftKey = 0, unsigned char* encodeCount = NULL);

#endif	// _PACKET_CRYPTO_H_