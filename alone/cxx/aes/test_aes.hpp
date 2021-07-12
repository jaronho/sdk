#include <string>

#include "aes.h"

void print(unsigned char* state)
{
    int i;
    for (i = 0; i < 16; i++)
    {
        printf("%s%X ", state[i] > 15 ? "" : "0", state[i]);
    }
    printf("\n\n");
}

void testAes()
{
    printf("\n============================== test aes =============================\n");
    unsigned char key[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    AES aes(key);

    unsigned char input[] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    printf("Input:\n");
    print(input);
    aes.encrypt(input);
    printf("After encrypt:\n");
    print(input);
    aes.decrypt(input);
    printf("After decrypt:\n");
    print(input);

    printf("==================================\n");

    char str[32] = "Hello, My AES Cipher!";
    int j;
    printf("Input:\n");
    for (j = 0; j < 32; j++)
    {
        printf("%X ", (unsigned char)str[j]);
    }
    printf("\n");
    printf("After encrypt:\n");
    aes.encrypt((void*)str);
    for (j = 0; j < 32; j++)
    {
        printf("%X ", (unsigned char)str[j]);
    }
    printf("\n");
    printf("After decrypt:\n");
    aes.decrypt((void*)str, 21);
    for (j = 0; j < 32; j++)
    {
        printf("%X ", (unsigned char)str[j]);
    }
    printf("\n");
}
