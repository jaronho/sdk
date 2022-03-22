#include "rc4.h"

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
namespace algorithm
{
#endif
unsigned char* rc4Crypto(unsigned char* input, unsigned long length, const unsigned char* pszKey)
{
    unsigned int keySize = 0;
    unsigned char sbox[256] = {0}, key[256] = {0}, temp = 0;
    unsigned long index = 0;
    int i = 0, j = 0, x = 0, y = 0, k = 0;
    if (!input || 0 == length || !pszKey)
    {
        return input;
    }
    keySize = strlen((char*)pszKey);
    if (0 == keySize)
    {
        return input;
    }
    for (i = 0; i < 256; ++i)
    {
        sbox[i] = i;
        key[i] = pszKey[i % keySize];
    }
    for (i = 0, j = 0; i < 256; ++i)
    {
        j = (sbox[i] + key[i] + j) % 256;
        temp = sbox[i];
        sbox[i] = sbox[j];
        sbox[j] = temp;
    }
    for (index = 0; index < length; ++index)
    {
        x = (x + 1) % 256;
        y = (y + sbox[x]) % 256;
        temp = sbox[x];
        sbox[x] = sbox[y];
        sbox[y] = temp;
        k = (sbox[x] + sbox[y]) % 256;
        temp = input[index] ^ sbox[k];
        if ('\0' == temp)
        {
            temp = input[index];
        }
        input[index] = temp;
    }
    return input;
}
#ifdef __cplusplus
} // namespace algorithm
#endif
