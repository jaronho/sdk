/**********************************************************************
 * Author:  jaron.ho
 * Date:    2019-01-17
 * Brief:	number base convert
 **********************************************************************/
#ifndef _BASE_CONVERT_H_
#define _BASE_CONVERT_H_

#include <string>

class BaseConvert {
public:
    /*
     * Brief:	check whether binary string
     * Param:	binStr - string, e.g. 1010
     * Return:	bool
     */
    static bool isBin(const std::string& binStr);

    /*
     * Brief:	check whether hexadecimal string
     * Param:	hexStr - string, e.g. FF88FF,0xFF88FF
     * Return:	bool
     */
    static bool isHex(const std::string& hexStr);

    /*
     * Brief:	convert a decimalism number to binary string
     * Param:	dec - a decimalism number
     *          bit - binary complement
     * Return:	string
     */
    static std::string dec2bin(unsigned int dec, unsigned int bit = 0);

    /*
     * Brief:	convert a binary string to decimalism number
     * Param:	binStr - a binary string, e.g. 1010
     * Return:	unsigned int
     */
    static unsigned int bin2dec(const std::string& binStr);

    /*
     * Brief:	convert a decimalism number to hexadecimal string
     * Param:	dec - a decimalism number
     *          bit - binary complement
     *          prefix - whether show "0x"
     *			isUpper - is letter upper
     * Return:	string, e.g. FF88FF
     */
    static std::string dec2hex(unsigned int dec, unsigned int bit = 0, bool prefix = true, bool isUpper = false);

    /*
     * Brief:	convert a hexadecimal string to decimalism number
     * Param:	dec - a hexadecimal string, e.g. FF88FF,0xFF88FF
     * Return:	unsigned int
     */
    static unsigned int hex2dec(const std::string& hexStr);
};

#endif  //_BASE_CONVERT_H_
