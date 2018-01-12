/******************************************************************************
* Author:	jaron.ho
* Date:		2018-01-11
* Brief:	algorithm
******************************************************************************/
#include "Algorithm.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//--------------------------------------------------------------------------
unsigned int Algorithm::colorGrayscale(unsigned int red, unsigned int green, unsigned int blue) {
    red = red > 255 ? 255 : red;
    green = green > 255 ? 255 : green;
    blue = blue > 255 ? 255 : blue;
    return (int)(red * 0.299f + green * 0.587f + blue * 0.114f + 0.5f);
}
//--------------------------------------------------------------------------
unsigned char* Algorithm::cryptoXOR(unsigned char* data, const unsigned char* key) {
    if (!data || 0 == strcmp("", (char*)data) || !key || 0 == strcmp("", (char*)key)) {
        return data;
    }
    unsigned int dataLength = strlen((char*)data);
    unsigned int keyLength = strlen((char*)key);
    for (unsigned int i = 0; i < dataLength; ++i) {
        data[i] = data[i] ^ key[i % keyLength];
    }
    return data;
}
//--------------------------------------------------------------------------
unsigned int Algorithm::hashCode(const unsigned char* data) {
    if (!data) {
        return 0;
    }
    const unsigned char* end = data + strlen((char*)data);
    unsigned int hash = 0;
    for (hash = 0; data < end; ++data) {
        hash *= 16777619;
        hash ^= (unsigned int)toupper(*data);
    }
    return hash;
}
//--------------------------------------------------------------------------
int Algorithm::mathRound(float num) {
    return (int)(num + 0.5f);
}
//--------------------------------------------------------------------------
double Algorithm::mathRand(double start, double end) {
    static bool seedFlag = true;
    if (seedFlag) {
        seedFlag = false;
        srand(unsigned(time(0)));
    }
    return start + (end - start) * rand() / RAND_MAX;
}
//--------------------------------------------------------------------------
bool Algorithm::mathProbability(unsigned int probability) {
    if (0 == probability) {
        return false;
    }
    unsigned int randRes = (unsigned int)mathRand(0, 100);	// init set rand range [0, 100)
    if (0 == randRes) {	// make sure range is [1, 100]
        randRes += 1;
    }
    return randRes <= probability;
}
//--------------------------------------------------------------------------
