/****************************************************************************
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#include "base64.h"

#include <stdio.h>
#include <stdlib.h>

const unsigned char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64DecodeImpl(const unsigned char* input, unsigned int input_len, unsigned char* output, unsigned int* output_len);

int base64DecodeImpl(const unsigned char* input, unsigned int input_len, unsigned char* output, unsigned int* output_len)
{
    static char inalphabet[256], decoder[256];
    int i, bits, c = 0, charCount, errors = 0;
    unsigned int inputIdx = 0;
    unsigned int outputIdx = 0;
    for (i = sizeof(alphabet) - 1; i >= 0; i--)
    {
        inalphabet[alphabet[i]] = 1;
        decoder[alphabet[i]] = i;
    }
    charCount = 0;
    bits = 0;
    for (inputIdx = 0; inputIdx < input_len; inputIdx++)
    {
        c = input[inputIdx];
        if ('=' == c)
        {
            break;
        }
        if (c > 255 || !inalphabet[c])
        {
            continue;
        }
        bits += decoder[c];
        charCount++;
        if (4 == charCount)
        {
            output[outputIdx++] = (bits >> 16);
            output[outputIdx++] = ((bits >> 8) & 0xff);
            output[outputIdx++] = (bits & 0xff);
            bits = 0;
            charCount = 0;
        }
        else
        {
            bits <<= 6;
        }
    }
    if ('=' == c)
    {
        switch (charCount)
        {
        case 1:
            fprintf(stderr, "base64Decode: encoding incomplete: at least 2 bits missing");
            errors++;
            break;
        case 2:
            output[outputIdx++] = (bits >> 10);
            break;
        case 3:
            output[outputIdx++] = (bits >> 16);
            output[outputIdx++] = ((bits >> 8) & 0xff);
            break;
        }
    }
    else if (inputIdx < input_len)
    {
        if (charCount)
        {
            fprintf(stderr, "base64 encoding incomplete: at least %d bits truncated", ((4 - charCount) * 6));
            errors++;
        }
    }
    *output_len = outputIdx;
    return errors;
}

unsigned int base64Decode(const unsigned char* in, unsigned int inLength, unsigned char** out)
{
    unsigned int outLength = 0;
    /* should be enough to store 6-bit buffers in 8-bit buffers */
    *out = (unsigned char*)malloc(inLength * 3.0f / 4.0f + 1);
    if (*out)
    {
        int ret = base64DecodeImpl(in, inLength, *out, &outLength);
        if (0 == ret)
        {
            *(*out + outLength) = '\0';
        }
        else
        {
            printf("Base64Utils: error decoding");
            free(*out);
            *out = NULL;
            outLength = 0;
        }
    }
    return outLength;
}
