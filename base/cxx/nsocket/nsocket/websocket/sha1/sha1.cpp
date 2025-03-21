#include "sha1.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace nsocket
{
#define SHA1STR_LEN 40

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. I got the idea of expanding during the round function from SSLeay */
#if BYTE_ORDER == LITTLE_ENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i], 24) & 0xFF00FF00) | (rol(block->l[i], 8) & 0x00FF00FF))
#elif BYTE_ORDER == BIG_ENDIAN
#define blk0(i) block->l[i]
#else
#error "Endianness not defined!"
#endif
#define blk(i) (block->l[i & 15] = rol(block->l[(i + 13) & 15] ^ block->l[(i + 8) & 15] ^ block->l[(i + 2) & 15] ^ block->l[i & 15], 1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v, w, x, y, z, i) \
    { \
        z += ((w & (x ^ y)) ^ y) + blk0(i) + 0x5A827999 + rol(v, 5); \
        w = rol(w, 30); \
    }
#define R1(v, w, x, y, z, i) \
    { \
        z += ((w & (x ^ y)) ^ y) + blk(i) + 0x5A827999 + rol(v, 5); \
        w = rol(w, 30); \
    }
#define R2(v, w, x, y, z, i) \
    { \
        z += (w ^ x ^ y) + blk(i) + 0x6ED9EBA1 + rol(v, 5); \
        w = rol(w, 30); \
    }
#define R3(v, w, x, y, z, i) \
    { \
        z += (((w | x) & y) | (w & x)) + blk(i) + 0x8F1BBCDC + rol(v, 5); \
        w = rol(w, 30); \
    }
#define R4(v, w, x, y, z, i) \
    { \
        z += (w ^ x ^ y) + blk(i) + 0xCA62C1D6 + rol(v, 5); \
        w = rol(w, 30); \
    }

/* Hash a single 512-bit block. This is the core of the algorithm. */
static void _sha1_transform(unsigned int state[5], const unsigned char buffer[64])
{
    typedef union
    {
        unsigned char c[64];
        unsigned int l[16];
    } CHAR64LONG16;
#ifdef SHA1HANDSOFF
    CHAR64LONG16 block[1]; /* use array to appear as a pointer */
    memcpy(block, buffer, 64);
#else
    /* The following had better never be used because it causes the
     * pointer-to-const buffer to be cast into a pointer to non-const.
     * And the result is written through.  I threw a "const" in, hoping
     * this will cause a diagnostic.
     */
    CHAR64LONG16* block = (CHAR64LONG16*)buffer;
#endif
    /* Copy context->state[] to working vars */
    unsigned int a = state[0];
    unsigned int b = state[1];
    unsigned int c = state[2];
    unsigned int d = state[3];
    unsigned int e = state[4];
    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a, b, c, d, e, 0);
    R0(e, a, b, c, d, 1);
    R0(d, e, a, b, c, 2);
    R0(c, d, e, a, b, 3);
    R0(b, c, d, e, a, 4);
    R0(a, b, c, d, e, 5);
    R0(e, a, b, c, d, 6);
    R0(d, e, a, b, c, 7);
    R0(c, d, e, a, b, 8);
    R0(b, c, d, e, a, 9);
    R0(a, b, c, d, e, 10);
    R0(e, a, b, c, d, 11);
    R0(d, e, a, b, c, 12);
    R0(c, d, e, a, b, 13);
    R0(b, c, d, e, a, 14);
    R0(a, b, c, d, e, 15);
    R1(e, a, b, c, d, 16);
    R1(d, e, a, b, c, 17);
    R1(c, d, e, a, b, 18);
    R1(b, c, d, e, a, 19);
    R2(a, b, c, d, e, 20);
    R2(e, a, b, c, d, 21);
    R2(d, e, a, b, c, 22);
    R2(c, d, e, a, b, 23);
    R2(b, c, d, e, a, 24);
    R2(a, b, c, d, e, 25);
    R2(e, a, b, c, d, 26);
    R2(d, e, a, b, c, 27);
    R2(c, d, e, a, b, 28);
    R2(b, c, d, e, a, 29);
    R2(a, b, c, d, e, 30);
    R2(e, a, b, c, d, 31);
    R2(d, e, a, b, c, 32);
    R2(c, d, e, a, b, 33);
    R2(b, c, d, e, a, 34);
    R2(a, b, c, d, e, 35);
    R2(e, a, b, c, d, 36);
    R2(d, e, a, b, c, 37);
    R2(c, d, e, a, b, 38);
    R2(b, c, d, e, a, 39);
    R3(a, b, c, d, e, 40);
    R3(e, a, b, c, d, 41);
    R3(d, e, a, b, c, 42);
    R3(c, d, e, a, b, 43);
    R3(b, c, d, e, a, 44);
    R3(a, b, c, d, e, 45);
    R3(e, a, b, c, d, 46);
    R3(d, e, a, b, c, 47);
    R3(c, d, e, a, b, 48);
    R3(b, c, d, e, a, 49);
    R3(a, b, c, d, e, 50);
    R3(e, a, b, c, d, 51);
    R3(d, e, a, b, c, 52);
    R3(c, d, e, a, b, 53);
    R3(b, c, d, e, a, 54);
    R3(a, b, c, d, e, 55);
    R3(e, a, b, c, d, 56);
    R3(d, e, a, b, c, 57);
    R3(c, d, e, a, b, 58);
    R3(b, c, d, e, a, 59);
    R4(a, b, c, d, e, 60);
    R4(e, a, b, c, d, 61);
    R4(d, e, a, b, c, 62);
    R4(c, d, e, a, b, 63);
    R4(b, c, d, e, a, 64);
    R4(a, b, c, d, e, 65);
    R4(e, a, b, c, d, 66);
    R4(d, e, a, b, c, 67);
    R4(c, d, e, a, b, 68);
    R4(b, c, d, e, a, 69);
    R4(a, b, c, d, e, 70);
    R4(e, a, b, c, d, 71);
    R4(d, e, a, b, c, 72);
    R4(c, d, e, a, b, 73);
    R4(b, c, d, e, a, 74);
    R4(a, b, c, d, e, 75);
    R4(e, a, b, c, d, 76);
    R4(d, e, a, b, c, 77);
    R4(c, d, e, a, b, 78);
    R4(b, c, d, e, a, 79);
    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    /* Wipe variables */
    a = b = c = d = e = 0;
#ifdef SHA1HANDSOFF
    memset(block, '\0', sizeof(block));
#endif
}

void Sha1::init()
{
    /* SHA1 initialization constants */
    m_state[0] = 0x67452301;
    m_state[1] = 0xEFCDAB89;
    m_state[2] = 0x98BADCFE;
    m_state[3] = 0x10325476;
    m_state[4] = 0xC3D2E1F0;
    m_count[0] = m_count[1] = 0;
}

void Sha1::update(const unsigned char* input, unsigned int inputLen)
{
    unsigned int j = m_count[0];
    if ((m_count[0] += inputLen << 3) < j)
    {
        m_count[1]++;
    }
    m_count[1] += (inputLen >> 29);
    j = (j >> 3) & 63;
    unsigned int i = 0;
    if ((j + inputLen) > 63)
    {
        memcpy(&m_buffer[j], input, (i = 64 - j));
        _sha1_transform(m_state, m_buffer);
        for (; i + 63 < inputLen; i += 64)
        {
            _sha1_transform(m_state, &input[i]);
        }
        j = 0;
    }
    memcpy(&m_buffer[j], &input[i], inputLen - i);
}

std::string Sha1::final(unsigned char digest[20], bool convertToStr)
{
    unsigned char finalcount[8];
#if 0 /* untested "improvement" by DHR */
    /* Convert m_count to a sequence of bytes
     * in finalcount.  Second element first, but
     * big-endian order within element.
     * But we do it all backwards.
     */
    unsigned char* fcp = &finalcount[8];
    for (int i = 0; i < 2; i++)
    {
        unsigned int t = m_ount[i];
        for (int j = 0; j < 4; t >>= 8, j++)
        {
            *--fcp = (unsigned char)t;
        }
    }
#else
    for (int i = 0; i < 8; i++)
    {
        finalcount[i] = (unsigned char)((m_count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255); /* Endian independent */
    }
#endif
    unsigned char c = 0200;
    update(&c, 1);
    while ((m_count[0] & 504) != 448)
    {
        c = 0000;
        update(&c, 1);
    }
    update(finalcount, 8); /* Should cause a _sha1_transform() */
    for (int i = 0; i < 20; i++)
    {
        digest[i] = (unsigned char)((m_state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
    }
    /* Wipe variables */
    memset(m_state, 0, sizeof(m_state));
    memset(m_count, 0, sizeof(m_count));
    memset(m_buffer, 0, sizeof(m_buffer));
    memset(&finalcount, '\0', sizeof(finalcount));
    if (!convertToStr) /* 不转字符串 */
    {
        return std::string();
    }
    /* 把20位哈希转为十六进制字符串(40位小写) */
    std::string digestStr;
    for (int i = 0; i < 20; i++)
    {
        char tmp[3] = {0};
#ifdef _WIN32
        sprintf_s(tmp, sizeof(tmp), "%02x", (unsigned char)digest[i]);
#else
        sprintf(tmp, "%02x", (unsigned char)digest[i]);
#endif
        digestStr += tmp;
    }
    return digestStr;
}

void Sha1::sign(const unsigned char* input, int inputLen, unsigned char digest[20])
{
    Sha1 s;
    s.init();
    s.update(input, inputLen);
    s.final(digest, false);
}

std::string Sha1::sign(const unsigned char* input, int inputLen)
{
    unsigned char digest[20];
    Sha1 s;
    s.init();
    s.update(input, inputLen);
    return s.final(digest, true);
}
} // namespace nsocket
