/**********************************************************************
* Author:	jaron.ho
* Date:		2018-08-10
* Brief:	bitmap
**********************************************************************/
#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <string>
#include <vector>

/*
 * Represents a single Pixel in the image. A Pixel has red, green, and blue
 * components that are mixed to form a color. Each of these values can range
 * from 0 to 255
 */
class Pixel {
public:
	Pixel(void) : r(0), g(0), b(0) {}
	Pixel(int r, int g, int b) {
        this->r = r; this->g = g; this->b = b;
    }
    int r, g, b;
};

/* To abbreviate a pixel matrix built as a vector of vectors */
typedef std::vector<std::vector<Pixel>> PixelMatrix;

/*
 * Represents a bitmap where a grid of pixels (in row-major order)
 * describes the color of each pixel within the image. Limited to Windows BMP
 * formatted images with no compression and 24 bit color depth.
 */
class Bitmap {
public:
    /*
     * Brief:   Opens a file as its name is provided and reads pixel-by-pixel the colors
     *          into a matrix of RGB pixels. Any errors will cout but will result in an
     *          empty matrix (with no rows and no columns).
     *
     * Param:   filename - to be opened and read as a matrix of pixels
     * Return:  int
     *          0.ok
     *          1.could not open file
     *          2.file is not in proper BMP format
     *          3.bitmap file is not 24bit
     *          4.bitmap file is compressed
     */
    int open(const std::string& filename);

    /*
     * Brief:   Saves the current image, represented by the matrix of pixels, as a
     *          Windows BMP file with the name provided by the parameter. File extension
     *          is not forced but should be .bmp. Any errors will cout and will NOT 
     *          attempt to save the file.
     *
     * Param:   filename - to be written as a bmp image
     * Return:  int
     *          0.ok
     *          1.could not open file
     *          2.bitmap is not a valid image
     */
    int save(const std::string& filename);

    /*
     * Brief:   Validates whether or not the current matrix of pixels represents a
     *          proper image with non-zero-size rows and consistent non-zero-size
     *          columns for each row. In addition, each pixel in the matrix is validated
     *          to have red, green, and blue components with values between 0 and 255
     * Param:   void
     * Return:  bool - whether or not the matrix is a valid image
     */
    bool isImage(void);

    /*
    * Brief:   Get width of the bitmap image
    * Param:   void
    * Return:  int
    */
    int width(void);

    /*
    * Brief:   Get height of the bitmap image
    * Param:   void
    * Return:  int
    */
    int height(void);

    /*
     * Brief:   Provides a vector of vector of pixels representing the bitmap
     * Param:   void
     * Return:  PixelMatrix, the bitmap image, represented by a matrix of RGB pixels
     */
    PixelMatrix toPixelMatrix(void);

    /*
     * Brief:   Overwrites the current bitmap with that represented by a matrix of pixels. 
     *          Does not validate that the new matrix of pixels is a proper image.
     * Param:   pixels - a matrix of pixels to represent a bitmap
     * Return:  void
     */
    void fromPixelMatrix(const PixelMatrix& pixels);

private:
    PixelMatrix mPixels;
    int mWidth;
    int mHeight;
};

#endif  // _BITMAP_H_

/*
************************************************** sample_01

int main() {
    Bitmap image;
    std::vector<std::vector<Pixel>> bmp;
    Pixel rgb;
    // read a file 111.bmp and convert it to a pixel matrix
    image.open("111.bmp");
    // verify that the file opened was a valid image
    if (image.isImage()) {
        bmp = image.toPixelMatrix();
        // take all the redness out of the top-left pixel
        rgb = bmp[0][0];
        rgb.r = 0;
        // put changed image back into matrix, update the bitmap and save it
        bmp[0][0] = rgb;
        image.fromPixelMatrix(bmp);
        image.save("222.bmp");
    }
    return 0;
    }
*/
