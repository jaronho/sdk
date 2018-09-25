/**********************************************************************
* Author:	jaron.ho
* Date:		2018-09-25
* Brief:	rgb24数据处理
**********************************************************************/
#include "rgb24.h"

unsigned char* rgb24Grayscale(unsigned char* rgb24, int width, int height, int clone) {
	assert(rgb24);
	assert(width > 0);
	assert(height > 0);
	int i = 0, j = 0, index = 0;
	int r = 0, g = 0, b = 0, grayvalue = 0;
    unsigned char* tmp = rgb24;
    if (clone) {
        tmp = (unsigned char*)malloc(sizeof(unsigned char) * width * height * 3);
        memset(tmp, 0, sizeof(unsigned char) * width * height * 3);
    }
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            index = (i * width + j) * 3;	// 每个像素占3位,所以乘以3
            r = rgb24[index];
            g = rgb24[index + 1];
            b = rgb24[index + 2];
            grayvalue = (int)(0.299 * r + 0.587 * g + 0.114 * b);	// 灰度值
            tmp[index] = tmp[index + 1] = tmp[index + 2] = (unsigned char)grayvalue;
        }
    }
    return tmp;
}

int* rgb24GrayscaleHistogram(const unsigned char* rgb24, int width, int height, int grayscaled) {
    assert(rgb24);
	assert(width > 0);
	assert(height > 0);
	int i = 0, j = 0, index = 0;
	int r = 0, g = 0, b = 0, grayvalue = 0;
    int* hist = (int*)malloc(sizeof(int) * 256);
    memset(hist, 0, sizeof(int) * 256);
	if (grayscaled) {	// 已灰度化
		for (i = 0; i < height; ++i) {
			for (j = 0; j < width; ++j) {
				index = (i * width + j) * 3;	// 每个像素占3位,所以乘以3
                grayvalue = rgb24[index];
				hist[grayvalue]++;
			}
		}
	} else {	// 未灰度化
		for (i = 0; i < height; ++i) {
			for (j = 0; j < width; ++j) {
				index = (i * width + j) * 3;	// 每个像素占3位,所以乘以3
                r = rgb24[index];
                g = rgb24[index + 1];
                b = rgb24[index + 2];
				grayvalue = (int)(0.299 * r + 0.587 * g + 0.114 * b);	// 灰度值
				hist[grayvalue]++;
			}
		}
	}
    return hist;
}

int rgb24BinarizationThresholdOTSU(const int* hist) {
    assert(hist);
	int i = 0, t = 0, maxT = 0;
	int sum = 0, count = 0;
    float u0 = 0, u1 = 0, w0 = 0, w1 = 0;
	float devi, maxDevi = 0; // 方差及最大方差
	for (i = 0; i < 256; ++i) {
		sum += hist[i];
	}
	for (t = 0; t < 255; t++) {
		count = 0;
		u0 = 0;
		// 阈值为t时,c0组的均值及产生的概率
		for (i = 0; i <= t; ++i) {
			count += hist[i];
			u0 += i * hist[i];
		}
		u0 = u0 / count;
		w0 = (float)count/sum;
		// 阈值为t时,c1组的均值及产生的概率
		u1 = 0;
		for (i = t + 1; i < 256; ++i) {
			u1 += i * hist[i];
		}
		u1 = u1 / (sum - count);
		w1 = 1 - w0;
		// 两类间方差
		devi = w0 * w1 * (u1 - u0) * (u1 - u0);
		// 记录最大的方差及最佳位置
		if (devi > maxDevi) {
			maxDevi = devi;
			maxT = t;
		}
	}
	return maxT;
}

unsigned char* rgb24Binarization(unsigned char* rgb24, int width, int height, int threshold, int clone, int* count) {
	assert(rgb24);
	assert(width);
	assert(height);
    assert(threshold > 0 && threshold < 255);
    assert(count);
	int i = 0, j = 0, index = 0;
	int r = 0, g = 0, b = 0, value = 0;
    unsigned char* tmp = rgb24;
    if (clone) {
        tmp = (unsigned char*)malloc(sizeof(unsigned char) * width * height * 3);
        memset(tmp, 0, sizeof(unsigned char) * width * height * 3);
    }
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            index = (i * width + j) * 3;	// 每个像素占3位,所以乘以3
            r = rgb24[index];
            g = rgb24[index + 1];
            b = rgb24[index + 2];
            value = 0;
            if ((r + g + b) / 3 >= threshold) {
                value = 255;
                ++(*count);
            }
            tmp[index] = tmp[index + 1] = tmp[index + 2] = (unsigned char)value;
        }
    }
    return tmp;
}
