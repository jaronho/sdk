/* md5.h - header file for md5.c */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
   rights reserved.

   License to copy and use this software is granted provided that it
   is identified as the "RSA Data Security, Inc. md5 Message-Digest
   Algorithm" in all material mentioning or referencing this software
   or this function.

   License is also granted to make and use derivative works provided
   that such works are identified as "derived from the RSA Data
   Security, Inc. md5 Message-Digest Algorithm" in all material
   mentioning or referencing the derived work.

   RSA Data Security, Inc. makes no representations concerning either
   the merchantability of this software or the suitability of this
   software for any particular purpose. It is provided "as is"
   without express or implied warranty of any kind.

   These notices must be retained in any copies of any part of this
   documentation and/or software.

   2007-09-15 Last modified by cheungmine.
 */

#ifndef _MD5_H_
#define _MD5_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* md5 context. */
typedef struct {
    unsigned int state[4];                  /* state (ABCD) */
    unsigned int count[2];                  /* number of bits, modulo 2^64 (lsb first) */
    unsigned char buffer[64];               /* input buffer */
} md5_ctx_t;

extern void md5_init(md5_ctx_t* context);
extern void md5_update(md5_ctx_t* context, const unsigned char* input, unsigned int inputLen);
extern void md5_fini(md5_ctx_t* context, unsigned char digest[16]);

extern const char* md5_sign(const unsigned char* input, unsigned int inputLen);

#ifdef __cplusplus
}
#endif

#endif  /* _MD5_H_ */

