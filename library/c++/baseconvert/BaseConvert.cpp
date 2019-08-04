/**********************************************************************
 * Author:  jaron.ho
 * Date:    2019-01-17
 * Brief:	number base convert
 **********************************************************************/
#include "BaseConvert.h"
#include <math.h>
#include <algorithm>
/*********************************************************************/
bool BaseConvert::isBin(const std::string& binStr) {
    size_t len = binStr.size();
    if (0 == len) {
        return false;
    }
    for (size_t i = 0; i < len; ++i) {
        const char& ch = binStr.at(i);
        if ('0' != ch && '1' != ch) {
            return false;
        }
    }
    return true;
}
/*********************************************************************/
bool BaseConvert::isHex(const std::string& hexStr) {
    size_t len = hexStr.size();
    size_t start = 0;
    if (len >= 3) {
        start = 2;
        const char& ch2 = hexStr.at(1);
        if ('x' == ch2 || 'X' == ch2) {
            if ('0' != hexStr.at(0)) {
                return false;
            }
        } else if ((ch2 < '0' || ch2 > '9') && (ch2 < 'A' || ch2 > 'F') && (ch2 < 'a' || ch2 > 'f')) {
            return false;
        }
    }
    for (size_t i = start; i < len; ++i) {
        const char& ch = hexStr.at(i);
        if ((ch < '0' || ch > '9') && (ch < 'A' || ch > 'F') && (ch < 'a' || ch > 'f')) {
            return false;
        }
    }
    return true;
}
/*********************************************************************/
std::string BaseConvert::dec2bin(unsigned int dec, unsigned int bit) {
    std::string binStr;
    for (unsigned int i = dec; i; i = i / 2) {
        binStr += i % 2 ? '1' : '0';
    }
    if (0 == dec) {
        binStr = '0';
    }
    while (binStr.size() < bit) {
        binStr += '0';
    }
    std::reverse(binStr.begin(), binStr.end());
    return binStr;
}
/*********************************************************************/
unsigned int BaseConvert::bin2dec(const std::string& binStr) {
    unsigned int dec = 0;
    for (size_t i = 0, len = binStr.size(); i < len; ++i) {
        const char& ch = binStr.at(i);
        if ('0' == ch) {
            continue;
        } else if ('1' == ch) {
            dec += static_cast<unsigned int>(pow(2, len - i - 1));
        } else {
            return 0;
        }
    }
    return dec;
}
/*********************************************************************/
static std::string dec2hexImpl(unsigned int dec, bool isUpper) {
    std::string hexStr = "";
    unsigned int temp = dec / 16;
    unsigned int left = dec % 16;
    if (temp > 0) {
        hexStr += dec2hexImpl(temp, isUpper);
    }
    if (left < 10) {
        hexStr += (static_cast<char>(left) + '0');
    } else {
        hexStr += ((isUpper ? 'A' : 'a') + static_cast<char>(left) - 10);
    }
    return hexStr;
}
/*********************************************************************/
std::string BaseConvert::dec2hex(unsigned int dec, unsigned int bit, bool prefix, bool isUpper) {
    std::string hexStr = dec2hexImpl(dec, isUpper);
    while (hexStr.size() < bit) {
        hexStr = '0' + hexStr;
    }
    if (prefix) {
        hexStr = "0x" + hexStr;
    }
    return hexStr;
}
/*********************************************************************/
unsigned int BaseConvert::hex2dec(const std::string& hexStr) {
    unsigned int dec = 0;
    size_t len = hexStr.size();
    size_t start = 0;
    if (len >= 3) {
        const char& ch2 = hexStr.at(1);
        if ('x' == ch2 || 'X' == ch2) {
            if ('0' != hexStr.at(0)) {
                return 0;
            }
            start = 2;
        } else if ((ch2 < '0' || ch2 > '9') && (ch2 < 'A' || ch2 > 'F') && (ch2 < 'a' || ch2 > 'f')) {
            return 0;
        }
    }
    for (size_t i = start; i < len; ++i) {
        const char& ch = hexStr.at(i);
        if (ch >= '0' && ch <= '9') {
            dec += (static_cast<unsigned int>(ch) - 48) * static_cast<unsigned int>(pow(16, len - i - 1));
        } else if (ch >= 'A' && ch <= 'F') {
            dec += (static_cast<unsigned int>(ch) - 55) * static_cast<unsigned int>(pow(16, len - i - 1));
        } else if (ch >= 'a' && ch <= 'f') {
            dec += (static_cast<unsigned int>(ch) - 87) * static_cast<unsigned int>(pow(16, len - i - 1));
        } else {
            return 0;
        }
    }
    return dec;
}
/*********************************************************************/
