#include "base64.h"

#include <stdlib.h>

#ifdef __cplusplus
namespace algorithm
{
#endif
#define BASE64_PAD '='
#define BASE64DE_FIRST '+'
#define BASE64DE_LAST 'z'
#define BASE64_ENCODE_OUT_SIZE(s) ((unsigned int)((((s) + 2) / 3) * 4 + 1))
#define BASE64_DECODE_OUT_SIZE(s) ((unsigned int)(((s) / 4) * 3) + 1)

/* BASE 64 encode table */
static const unsigned char base64en[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                         'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                         'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                         'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

/* ASCII order for BASE 64 decode, 255 in unused character */
static const unsigned char base64de[] = {255, 255, 255, 255, 255, 255, 255, 255, /* nul, soh, stx, etx, eot, enq, ack, bel, */
                                         255, 255, 255, 255, 255, 255, 255, 255, /*  bs,  ht,  nl,  vt,  np,  cr,  so,  si, */
                                         255, 255, 255, 255, 255, 255, 255, 255, /* dle, dc1, dc2, dc3, dc4, nak, syn, etb, */
                                         255, 255, 255, 255, 255, 255, 255, 255, /* can,  em, sub, esc,  fs,  gs,  rs,  us, */
                                         255, 255, 255, 255, 255, 255, 255, 255, /*  sp, '!', '"', '#', '$', '%', '&', ''', */
                                         255, 255, 255, 62,  255, 255, 255, 63, /* '(', ')', '*', '+', ',', '-', '.', '/', */
                                         52,  53,  54,  55,  56,  57,  58,  59, /* '0', '1', '2', '3', '4', '5', '6', '7', */
                                         60,  61,  255, 255, 255, 255, 255, 255, /* '8', '9', ':', ';', '<', '=', '>', '?', */
                                         255, 0,   1,   2,   3,   4,   5,   6, /* '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', */
                                         7,   8,   9,   10,  11,  12,  13,  14, /* 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', */
                                         15,  16,  17,  18,  19,  20,  21,  22, /* 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', */
                                         23,  24,  25,  255, 255, 255, 255, 255, /* 'X', 'Y', 'Z', '[', '\', ']', '^', '_', */
                                         255, 26,  27,  28,  29,  30,  31,  32, /* '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', */
                                         33,  34,  35,  36,  37,  38,  39,  40, /* 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', */
                                         41,  42,  43,  44,  45,  46,  47,  48, /* 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', */
                                         49,  50,  51,  255, 255, 255, 255, 255}; /* 'x', 'y', 'z', '{', '|', '}', '~', del, */

unsigned int base64Encode(const unsigned char* in, unsigned int inLength, unsigned char** out)
{
    int flag;
    unsigned int i;
    unsigned int outLength;
    unsigned char ch;
    unsigned char prevCh;
    if (!in || 0 == inLength)
    {
        return 0;
    }
    flag = 0;
    outLength = 0;
    prevCh = 0;
    *out = (unsigned char*)malloc(BASE64_ENCODE_OUT_SIZE(inLength));
    if (*out)
    {
        for (i = 0; i < inLength; ++i)
        {
            ch = in[i];
            switch (flag)
            {
            case 0:
                flag = 1;
                (*out)[outLength++] = base64en[(ch >> 2) & 0x3F];
                break;
            case 1:
                flag = 2;
                (*out)[outLength++] = base64en[((prevCh & 0x3) << 4) | ((ch >> 4) & 0xF)];
                break;
            case 2:
                flag = 0;
                (*out)[outLength++] = base64en[((prevCh & 0xF) << 2) | ((ch >> 6) & 0x3)];
                (*out)[outLength++] = base64en[ch & 0x3F];
                break;
            }
            prevCh = ch;
        }
        switch (flag)
        {
        case 1:
            (*out)[outLength++] = base64en[(prevCh & 0x3) << 4];
            (*out)[outLength++] = BASE64_PAD;
            (*out)[outLength++] = BASE64_PAD;
            break;
        case 2:
            (*out)[outLength++] = base64en[(prevCh & 0xF) << 2];
            (*out)[outLength++] = BASE64_PAD;
            break;
        }
        (*out)[outLength] = 0;
    }
    return outLength;
}

unsigned int base64Decode(const unsigned char* in, unsigned int inLength, unsigned char** out)
{
    unsigned int i;
    unsigned int outLength;
    unsigned char ch;
    if (!in || 0 == inLength || (inLength & 0x3))
    {
        return 0;
    }
    outLength = 0;
    *out = (unsigned char*)malloc(BASE64_DECODE_OUT_SIZE(inLength));
    if (*out)
    {
        for (i = 0; i < inLength; ++i)
        {
            if (in[i] == BASE64_PAD)
            {
                break;
            }
            if (in[i] < BASE64DE_FIRST || in[i] > BASE64DE_LAST)
            {
                return 0;
            }
            ch = base64de[(unsigned char)in[i]];
            if (255 == ch)
            {
                return 0;
            }
            switch (i & 0x3)
            {
            case 0:
                (*out)[outLength] = (ch << 2) & 0xFF;
                break;
            case 1:
                (*out)[outLength++] |= (ch >> 4) & 0x3;
                (*out)[outLength] = (ch & 0xF) << 4;
                break;
            case 2:
                (*out)[outLength++] |= (ch >> 2) & 0xF;
                (*out)[outLength] = (ch & 0x3) << 6;
                break;
            case 3:
                (*out)[outLength++] |= ch;
                break;
            }
        }
    }
    return outLength;
}
#ifdef __cplusplus
} // namespace algorithm
#endif
