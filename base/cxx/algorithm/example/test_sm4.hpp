#pragma once

#include <stdio.h>
#include <string.h>
#include <string>

#include "../algorithm/sm4/sm4.h"

void testSm4()
{
    printf("\n============================== test sm4 =============================\n");
    std::string key = "beyondinfo@2020k";
    std::string input = "qq12345678901234567890";
    printf("[ECB]   input: %s\n", input.c_str());
    sm4_context_t ctx;
    /* ECB加密 */
    sm4SetKeyEnc(&ctx, (const unsigned char*)key.c_str());
    unsigned char* out1 = NULL;
    int len1 = sm4CryptEcb(&ctx, (const unsigned char*)input.c_str(), input.size(), &out1);
    int pad1 = len1 - input.size();
    printf("[ECB] encrypt: ");
    for (int i = 0; i < len1; i++)
    {
        printf("%c", out1[i]);
    }
    printf("\n");
    /* ECB解密 */
    sm4SetKeyDec(&ctx, (const unsigned char*)key.c_str());
    unsigned char* out2 = NULL;
    int len2 = sm4CryptEcb(&ctx, out1, len1, &out2);
    len2 -= pad1; /* 解密后的长度需要减去补位长度 */
    printf("[ECB] decrypt: ");
    for (int i = 0; i < len2; i++)
    {
        printf("%c", out2[i]);
    }
    if (out1)
    {
        free(out1);
    }
    if (out2)
    {
        free(out2);
    }
    printf("\n");
    printf("\n");
#if 1
    /* CBC解密 */
    unsigned char ivec[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10};
    printf("[CBC]   input: %s\n", input.c_str());
    sm4SetKeyEnc(&ctx, (const unsigned char*)key.c_str());
    unsigned char* out3 = NULL;
    int len3 = sm4CryptCbc(&ctx, ivec, (const unsigned char*)input.c_str(), input.size(), &out3);
    int pad2 = len3 - input.size();
    printf("[CBC] encrypt: ");
    for (int i = 0; i < len3; ++i)
    {
        printf("%c", out3[i]);
    }
    printf("\n");
    sm4SetKeyDec(&ctx, (const unsigned char*)key.c_str());
    unsigned char* out4 = NULL;
    int len4 = sm4CryptCbc(&ctx, ivec, out3, len3, &out4);
    len4 -= pad2; /* 解密后的长度需要减去补位长度 */
    printf("[CBC] decrypt: ");
    for (int i = 0; i < len4; ++i)
    {
        printf("%c", out4[i]);
    }
    printf("\n");
    if (out3)
    {
        free(out3);
    }
    if (out4)
    {
        free(out4);
    }
#endif
}
