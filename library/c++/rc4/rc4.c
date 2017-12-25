/**********************************************************************
* Author:	jaron.ho
* Date:		2014-03-21
* Brief:	rc4 algorithm
**********************************************************************/
#include "RC4.h"
#include <stdio.h>
#include <string.h>
//----------------------------------------------------------------------
unsigned char* rc4_crypto(unsigned char* input, unsigned long length, const unsigned char* psz_key) {
	unsigned int keySize = 0;
	unsigned char sbox[256] = {0}, key[256] = {0}, temp = 0;
	unsigned long index = 0;
	int i = 0, j = 0, x = 0, y = 0, k = 0;

	if (NULL == input || 0 == length || NULL == psz_key) {
		return input;
	}
	keySize = strlen((char*)psz_key);
	if (0 == keySize) {
		return input;
	}
	for (i=0; i<256; ++i) {
		sbox[i] = i;
		key[i] = psz_key[i % keySize];
	}
	for (i=0, j=0; i<256; ++i) {
		j = (sbox[i] + key[i] + j) % 256;
		temp = sbox[i];
		sbox[i] = sbox[j];
		sbox[j] = temp;
	}
	for (index=0; index<length; ++index) {
		x = (x + 1) % 256;
		y = (y + sbox[x]) % 256;
		temp = sbox[x];
		sbox[x] = sbox[y];
		sbox[y] = temp;
		k = (sbox[x] + sbox[y]) % 256;
		temp = input[index] ^ sbox[k];
		if ('\0' == temp) {
			temp = input[index];
		}
		input[index] = temp;
	}
	return input;
}
//----------------------------------------------------------------------