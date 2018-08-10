/**********************************************************************
* Author:	jaron.ho
* Date:		2018-08-10
* Brief:	bitmap
**********************************************************************/
#include "bitmap.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

typedef unsigned char uchar_t;
typedef unsigned int uint32_t;
typedef unsigned short int uint16_t;
typedef signed int int32_t;
typedef signed short int int16_t;

const int MIN_RGB = 0;
const int MAX_RGB = 255;
const int BMP_MAGIC_ID = 2;

/* Windows BMP-specific format data */
struct bmpfile_magic {
    uchar_t magic[BMP_MAGIC_ID];
};

struct bmpfile_header {
    uint32_t file_size;
    uint16_t creator1;
    uint16_t creator2;
    uint32_t bmp_offset;
};

struct bmpfile_dib_info {
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t num_planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t bmp_byte_size;
    int32_t hres;
    int32_t vres;
    uint32_t num_colors;
    uint32_t num_important_colors;
};

int Bitmap::open(const std::string& filename) {
	std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
    /* clear data if already holds information */
    for (size_t i = 0; i < mPixels.size(); ++i) {
        mPixels[i].clear();
    }
    mPixels.clear();
    mWidth = 0;
    mHeight = 0;
	if (file.fail()) {
		std::cout << filename << " could not be opened. Does it exist? Is it already open by another program?\n";
        return 1;
	}
	bmpfile_magic magic;
	file.read((char*)(&magic), sizeof(magic));
	/* Check to make sure that the first two bytes of the file are the "BM" identifier that identifies a bitmap image. */
	if ('B' != magic.magic[0] || 'M' != magic.magic[1]) {
        file.close();
		std::cout << filename << " is not in proper BMP format.\n";
        return 2;
	}
	bmpfile_header header;
	file.read((char*)(&header), sizeof(header));
	bmpfile_dib_info dib_info;
	file.read((char*)(&dib_info), sizeof(dib_info));
	/* Check for this here and so that we know later whether we need to insert each row at the bottom or top of the image. */
	bool flip = true;
	if (dib_info.height < 0) {
		flip = false;
		dib_info.height = -dib_info.height;
	}
	/* Only support for 24-bit images */
	if (24 != dib_info.bits_per_pixel) {
        file.close();
		std::cout << filename << " uses " << dib_info.bits_per_pixel << "bits per pixel (bit depth). Bitmap only supports 24bit.\n";
        return 3;
	}
	/* No support for compressed images */
	if (0 != dib_info.compression) {
        file.close();
		std::cout << filename << " is compressed. Bitmap only supports uncompressed images.\n";
        return 4;
	}
	file.seekg(header.bmp_offset);
	/* Read the pixels for each row and column of Pixels in the image. */
	for (int row = 0; row < dib_info.height; ++row) {
		std::vector<Pixel> row_data;
		for (int col = 0; col < dib_info.width; ++col) {
			int blue = file.get();
			int green = file.get();
			int red = file.get();
			row_data.push_back(Pixel(red, green, blue));
		}
		/* Rows are padded so that they're always a multiple of 4 bytes. This line skips the padding at the end of each row. */
		file.seekg(dib_info.width % 4, std::ios::cur);
		if (flip) {
            mPixels.insert(mPixels.begin(), row_data);
		} else {
            mPixels.push_back(row_data);
		}
	}
	file.close();
    return 0;
}

int Bitmap::save(const std::string& filename) {
	std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
	if (file.fail()) {
		std::cout << filename << " could not be opened for editing. Is it already open by another program or is it read-only?\n";
        return 1;
	}
    if (!isImage()) {
        file.close();
		std::cout << "Bitmap cannot be saved. It is not a valid image.\n";
        return 2;
	}
	/* Write all the header information that the BMP file format requires. */
    bmpfile_magic magic;
    magic.magic[0] = 'B';
    magic.magic[1] = 'M';
    file.write((char*)(&magic), sizeof(magic));
    bmpfile_header header = { 0 };
    header.bmp_offset = sizeof(bmpfile_magic) + sizeof(bmpfile_header) + sizeof(bmpfile_dib_info);
    header.file_size = header.bmp_offset + (mPixels.size() * 3 + mPixels[0].size() % 4) * mPixels.size();
    file.write((char*)(&header), sizeof(header));
    bmpfile_dib_info dib_info = { 0 };
    dib_info.header_size = sizeof(bmpfile_dib_info);
    dib_info.width = mPixels[0].size();
    dib_info.height = mPixels.size();
    dib_info.num_planes = 1;
    dib_info.bits_per_pixel = 24;
    dib_info.compression = 0;
    dib_info.bmp_byte_size = 0;
    dib_info.hres = 2835;
    dib_info.vres = 2835;
    dib_info.num_colors = 0;
    dib_info.num_important_colors = 0;
    file.write((char*)(&dib_info), sizeof(dib_info));
    /* Write each row and column of Pixels into the image file -- we write the rows upside-down to satisfy the easiest BMP format. */
    for (int row = mPixels.size() - 1; row >= 0; --row) {
        const std::vector<Pixel>& row_data = mPixels[row];
        for (size_t col = 0; col < row_data.size(); ++col) {
            const Pixel& pix = row_data[col];
            file.put((uchar_t)(pix.b));
            file.put((uchar_t)(pix.g));
            file.put((uchar_t)(pix.r));
        }
        /* Rows are padded so that they're always a multiple of 4 bytes. This line skips the padding at the end of each row. */
        for (size_t i = 0; i < row_data.size() % 4; ++i) {
            file.put(0);
        }
    }
    file.close();
    return 0;
}

bool Bitmap::isImage(void) {
	const size_t HEIGHT = mPixels.size();
	if (0 == HEIGHT || 0 == mPixels[0].size()) {
		return false;
	}
	const size_t WIDTH = mPixels[0].size();
	for (size_t row = 0; row < HEIGHT; ++row) {
		if (WIDTH != mPixels[row].size()) {
			return false;
		}
		for (size_t col = 0; col < WIDTH; ++col) {
			Pixel current = mPixels[row][col];
			if (current.r > MAX_RGB || current.r < MIN_RGB ||
				current.g > MAX_RGB || current.g < MIN_RGB ||
				current.b > MAX_RGB || current.b < MIN_RGB) {
				return false;
            }
		}
	}
	return true;
}

int Bitmap::width(void) {
    if (0 == mWidth && isImage()) {
        mWidth = mPixels[0].size();
    }
    return mWidth;
}

int Bitmap::height(void) {
    if (0 == mHeight && isImage()) {
        mHeight = mPixels.size();
    }
    return mHeight;
}

PixelMatrix Bitmap::toPixelMatrix(void) {
	if (isImage()) {
		return mPixels;
	}
	return PixelMatrix();
}

void Bitmap::fromPixelMatrix(const PixelMatrix& values) {
    mPixels = values;
    mWidth = 0;
    mHeight = 0;
}
