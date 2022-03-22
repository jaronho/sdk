#include "hamc_sha1.h"

#include <memory.h>

#include "sha1.h"

void hamcSha1(const unsigned char* data, int dataLen, const unsigned char* key, int keyLen, unsigned char digest[20])
{
    int blockSize = 64;
    unsigned char k0[64];
    int k0Len;
    unsigned char innerPadding[64];
    unsigned char outerPadding[64];
    unsigned char* part;
    unsigned char total[84];
    int partSize;
    sha1_ctx_t ctx;
    int i;
    for (i = 0; i < blockSize; ++i)
    {
        k0[i] = 0x00;
        innerPadding[i] = 0x36;
        outerPadding[i] = 0x5c;
    }
    if (keyLen > blockSize)
    {
        sha1Init(&ctx);
        sha1Update(&ctx, key, keyLen);
        sha1Final(&ctx, digest, 0);
        for (i = 0; i < 20; ++i)
        {
            k0[i] = digest[i];
        }
        k0Len = blockSize;
    }
    else
    {
        for (i = 0; i < keyLen; i++)
        {
            k0[i] = key[i];
        }
        k0Len = keyLen;
    }
    for (i = 0; i < k0Len; ++i)
    {
        innerPadding[i] ^= k0[i];
        outerPadding[i] ^= k0[i];
    }
    partSize = blockSize + dataLen;
    part = (unsigned char*)malloc((size_t)partSize);
    if (!part)
    {
        return;
    }
    for (i = 0; i < blockSize; ++i)
    {
        *(part + i) = innerPadding[i];
    }
    for (i = 0; i < dataLen; ++i)
    {
        *(part + blockSize + i) = *(data + i);
    }
    sha1Init(&ctx);
    sha1Update(&ctx, part, partSize);
    sha1Final(&ctx, digest, 0);
    free(part);
    for (i = 0; i < blockSize; ++i)
    {
        total[i] = outerPadding[i];
    }
    for (i = 0; i < 20; ++i)
    {
        total[blockSize + i] = digest[i];
    }
    sha1Init(&ctx);
    sha1Update(&ctx, total, 84);
    sha1Final(&ctx, digest, 0);
}

char* hamcSha1Str(const unsigned char* data, int dataLen, const unsigned char* key, int keyLen)
{
    unsigned char digest[20];
    char* digestStr;
    int i;
    char tmp[3];
    hamcSha1(data, dataLen, key, keyLen, digest);
    /* 把20位哈希转为十六进制字符串(40位小写) */
    digestStr = (char*)malloc(sizeof(char) * (2 * sizeof(digest) + 1));
    if (!digestStr)
    {
        return NULL;
    }
    digestStr[0] = 0;
    for (i = 0; i < 20; i++)
    {
        memset(tmp, 0, sizeof(tmp));
#ifdef _WIN32
        sprintf_s(tmp, sizeof(tmp), "%02x", (unsigned char)digest[i]);
#else
        sprintf(tmp, "%02x", (unsigned char)digest[i]);
#endif
        strcat(digestStr, tmp);
    }
    return digestStr;
}
