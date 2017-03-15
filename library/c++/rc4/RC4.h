/**********************************************************************
* Author:	jaron.ho
* Date:		2014-03-21
* Brief:	RC4 algorithm
**********************************************************************/
#ifndef _RC4_H_
#define _RC4_H_

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char* rc4_crypto(unsigned char* input, unsigned long length, const unsigned char* psz_key);

#ifdef __cplusplus
}
#endif

#endif	// _RC4_H_
