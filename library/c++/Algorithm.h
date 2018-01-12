/**********************************************************************
 * Author:	jaron.ho
 * Date:    2018-01-11
 * Brief:	algorithm
 **********************************************************************/
#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_

class Algorithm {
public:
    /*
     * Brief:   calculate grayscale value
     * Param:   red - red channel value
     *          green - green channel value
     *          blue - blue channel value
     * Return: unsigned int
     */
    static unsigned int colorGrayscale(unsigned int red, unsigned int green, unsigned int blue);

    /*
     * Brief:	XOR crypto
     * Param:	data - data
     *          key - crypto key
     * Return:	unsigned char*
     */
    static unsigned char* cryptoXOR(unsigned char* data, const unsigned char* key);

    /*
     * Brief:	get hash code
     * Param:	data - any string
     * Return:	unsigned int
     */
    static unsigned int hashCode(const unsigned char* data);

    /*
     * Brief:	math round
     * Param:	num - number
     * Return:	int
     */
    static int mathRound(float num);

    /*
     * Brief:	get random number, [start, end)
     * Param:	start - start number
     *          end - end number
     * Return:	double
     */
    static double mathRand(double start, double end);

    /*
     * Brief:	get probability in a region
     * Param:	probability - (0, 100], 5 represent 5% or 0.05
     * Return:	bool
     */
    static bool mathProbability(unsigned int probability);
};

#endif	/* _ALGORITHM_H_ */
